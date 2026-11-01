import BackgroundTasks
import Foundation
import Network
import UIKit
import UserNotifications

/// Phone notifications for online play (docs/notifications.md): trade offers,
/// friends coming online and tournaments starting.
///
/// While the game is connected the server sends each note on the game's own
/// connection (NetBridge hands the lines here), and it is shown when the app
/// is in the background. After that iOS stops the app; a background refresh
/// task asks the server for new notes whenever iOS lets it run.
final class Notifier {
    static let shared = Notifier()
    static let refreshTask = "io.github.rdiol12.pokimonmulti.notify"

    private static let kinds = ["trade", "friend", "tour", "fishing"]
    private let defaults = UserDefaults.standard
    private let lock = NSLock()
    /// The game is on screen: notes then show in the game, not as notifications.
    private var active = true

    private init() {
        let center = NotificationCenter.default
        center.addObserver(forName: UIApplication.didBecomeActiveNotification, object: nil, queue: .main) { [weak self] _ in
            self?.setActive(true)
        }
        center.addObserver(forName: UIApplication.willResignActiveNotification, object: nil, queue: .main) { [weak self] _ in
            self?.setActive(false)
        }
    }

    private func setActive(_ value: Bool) {
        lock.lock()
        active = value
        lock.unlock()
    }

    private var isActive: Bool {
        lock.lock()
        defer { lock.unlock() }
        return active
    }

    // MARK: settings

    var enabled: Bool { defaults.object(forKey: "notify.on") as? Bool ?? true }

    private func kindOn(_ kind: String) -> Bool {
        // Fishing tournaments happen every two hours, so they are opt-in
        defaults.object(forKey: "notify.kind.\(kind)") as? Bool ?? (kind != "fishing")
    }

    /// The page's settings: {on, trade, friend, tour, fishing}.
    func setSettings(_ settings: [String: Any]) {
        if let on = settings["on"] as? Bool { defaults.set(on, forKey: "notify.on") }
        for kind in Self.kinds {
            if let value = settings[kind] as? Bool { defaults.set(value, forKey: "notify.kind.\(kind)") }
        }
        if enabled { scheduleRefresh() } else { BGTaskScheduler.shared.cancel(taskRequestWithIdentifier: Self.refreshTask) }
    }

    // MARK: the game connection

    /// The line to send after the server's WELCOME, or nil when notifications are off.
    func helloLine() -> String? {
        guard enabled else { return nil }
        let token = defaults.string(forKey: "notify.token") ?? ""
        return "NOTIFY ON \(token)".trimmingCharacters(in: .whitespaces) + "\n"
    }

    /// Called for each line the server sends on the game connection.
    func serverLine(_ line: String, host: String, port: UInt16) {
        if line.hasPrefix("NOTIFYKEY ") {
            let f = line.split(separator: " ").map(String.init)
            guard f.count >= 5 else { return }
            let oldToken = defaults.string(forKey: "notify.token") ?? ""
            let stored = oldToken == f[3] ? defaults.integer(forKey: "notify.last") : 0
            defaults.set(host, forKey: "notify.host")
            defaults.set(Int(port), forKey: "notify.port")
            defaults.set(f[1], forKey: "notify.name")
            defaults.set(Int(f[2]) ?? 0, forKey: "notify.tid")
            defaults.set(f[3], forKey: "notify.token")
            defaults.set(max(stored, Int(f[4]) ?? 0), forKey: "notify.last")
            askPermissionOnce()
            scheduleRefresh()
        } else if line.hasPrefix("NOTE ") {
            let f = line.split(separator: " ", maxSplits: 3).map(String.init)
            guard f.count == 4, let id = Int(f[1]) else { return }
            note(id: id, kind: f[2], text: f[3])
        }
    }

    private func note(id: Int, kind: String, text: String) {
        guard id > defaults.integer(forKey: "notify.last") else { return }
        defaults.set(id, forKey: "notify.last")
        guard enabled, kindOn(kind), !isActive else { return }
        let content = UNMutableNotificationContent()
        content.title = Self.title(kind)
        content.body = text
        content.sound = .default
        let request = UNNotificationRequest(identifier: "note-\(id)", content: content, trigger: nil)
        UNUserNotificationCenter.current().add(request, withCompletionHandler: nil)
    }

    private static func title(_ kind: String) -> String {
        switch kind {
        case "trade": return "Trade offer"
        case "friend": return "Friend online"
        case "tour": return "Tournament"
        case "fishing": return "Fishing tournament"
        default: return "Pokimon Multi"
        }
    }

    private func askPermissionOnce() {
        guard !defaults.bool(forKey: "notify.asked") else { return }
        defaults.set(true, forKey: "notify.asked")
        UNUserNotificationCenter.current().requestAuthorization(options: [.alert, .sound, .badge]) { _, _ in }
    }

    // MARK: background checks

    func registerRefresh() {
        BGTaskScheduler.shared.register(forTaskWithIdentifier: Self.refreshTask, using: nil) { task in
            guard let task = task as? BGAppRefreshTask else { return }
            self.scheduleRefresh()
            let finished = NSLock()
            var done = false
            let finish: (Bool) -> Void = { ok in
                finished.lock()
                defer { finished.unlock() }
                if !done {
                    done = true
                    task.setTaskCompleted(success: ok)
                }
            }
            task.expirationHandler = { finish(false) }
            self.poll { finish(true) }
        }
    }

    func scheduleRefresh() {
        guard enabled, !(defaults.string(forKey: "notify.token") ?? "").isEmpty else { return }
        let request = BGAppRefreshTaskRequest(identifier: Self.refreshTask)
        request.earliestBeginDate = Date(timeIntervalSinceNow: 15 * 60)
        try? BGTaskScheduler.shared.submit(request)
    }

    /// Asks the server for notes since the last one seen.
    func poll(completion: @escaping () -> Void) {
        let token = defaults.string(forKey: "notify.token") ?? ""
        let host = defaults.string(forKey: "notify.host") ?? ""
        let port = UInt16(clamping: defaults.integer(forKey: "notify.port"))
        guard enabled, !token.isEmpty, !host.isEmpty, let nwPort = NWEndpoint.Port(rawValue: port) else {
            completion()
            return
        }
        let name = defaults.string(forKey: "notify.name") ?? ""
        let request = "NOTIFYPOLL \(name) \(defaults.integer(forKey: "notify.tid")) \(token) \(defaults.integer(forKey: "notify.last"))\n"
        let tcp = NWProtocolTCP.Options()
        tcp.connectionTimeout = 8
        let conn = NWConnection(host: NWEndpoint.Host(host), port: nwPort, using: NWParameters(tls: nil, tcp: tcp))
        let queue = DispatchQueue(label: "notify-poll")
        var buffer = ""
        var finished = false
        let finish = {
            guard !finished else { return }
            finished = true
            conn.cancel()
            completion()
        }
        func receive() {
            conn.receive(minimumIncompleteLength: 1, maximumLength: 65536) { data, _, done, error in
                if let data = data, let text = String(data: data, encoding: .isoLatin1) {
                    buffer += text
                    while let newline = buffer.firstIndex(of: "\n") {
                        let line = String(buffer[..<newline])
                        buffer.removeSubrange(...newline)
                        if line.hasPrefix("NOTEEND") {
                            if line.trimmingCharacters(in: .whitespaces) == "NOTEEND -1" {
                                // The server no longer knows this phone: register again next time online
                                self.defaults.removeObject(forKey: "notify.token")
                            }
                            finish()
                            return
                        }
                        self.serverLine(line, host: host, port: port)
                    }
                }
                if done || error != nil {
                    finish()
                    return
                }
                receive()
            }
        }
        conn.stateUpdateHandler = { state in
            switch state {
            case .ready:
                conn.send(content: request.data(using: .isoLatin1), completion: .contentProcessed { _ in })
                receive()
            case .failed, .waiting, .cancelled:
                queue.async { finish() }
            default:
                break
            }
        }
        conn.start(queue: queue)
        queue.asyncAfter(deadline: .now() + 20) { finish() }
    }
}
