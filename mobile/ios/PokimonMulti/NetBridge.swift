import Foundation
import Network
import WebKit

/// TCP connections to the netplay server for the page (www/net.js). The page
/// posts {op: connect|send|close, id, ...} to webkit.messageHandlers.pokimonNet;
/// what arrives goes back through window.PokimonNet.opened/data/closed.
final class NetBridge {
    private weak var web: WKWebView?
    private var connections: [Int: NWConnection] = [:]
    private let queue = DispatchQueue(label: "netplay")

    init(web: WKWebView) {
        self.web = web
    }

    func handle(_ body: [String: Any]) {
        guard let op = body["op"] as? String, let id = (body["id"] as? NSNumber)?.intValue else { return }
        switch op {
        case "connect":
            guard let host = body["host"] as? String, let port = (body["port"] as? NSNumber)?.uint16Value else { return }
            connect(id: id, host: host, port: port)
        case "send":
            guard let text = body["text"] as? String, let conn = connections[id] else { return }
            let data = text.data(using: .isoLatin1) ?? Data(text.utf8)
            conn.send(content: data, completion: .contentProcessed { _ in })
        case "close":
            connections.removeValue(forKey: id)?.cancel()
        default:
            break
        }
    }

    func closeAll() {
        for conn in connections.values { conn.cancel() }
        connections.removeAll()
    }

    private func connect(id: Int, host: String, port: UInt16) {
        guard let nwPort = NWEndpoint.Port(rawValue: port) else {
            call("PokimonNet.closed(\(id),'bad port')")
            return
        }
        let tcp = NWProtocolTCP.Options()
        tcp.noDelay = true
        tcp.connectionTimeout = 8
        let conn = NWConnection(host: NWEndpoint.Host(host), port: nwPort, using: NWParameters(tls: nil, tcp: tcp))
        connections[id] = conn
        queue.async { self.endpoints[id] = (host, port); self.lineBuffers[id] = "" }
        conn.stateUpdateHandler = { [weak self] state in
            guard let self = self else { return }
            switch state {
            case .ready:
                self.call("PokimonNet.opened(\(id))")
                self.receive(id: id, conn: conn)
            case .waiting(let error), .failed(let error):
                conn.cancel()
                self.finish(id: id, reason: "\(error)")
            case .cancelled:
                self.finish(id: id, reason: "")
            default:
                break
            }
        }
        conn.start(queue: queue)
    }

    private func receive(id: Int, conn: NWConnection) {
        conn.receive(minimumIncompleteLength: 1, maximumLength: 65536) { [weak self] data, _, done, error in
            guard let self = self else { return }
            if let data = data, !data.isEmpty {
                self.call("PokimonNet.data(\(id),'\(data.base64EncodedString())')")
                self.scanLines(id: id, conn: conn, data: data)
            }
            if done || error != nil {
                conn.cancel()
                return
            }
            self.receive(id: id, conn: conn)
        }
    }

    /// Notifications ride on the game connection (Notifier). Only this queue touches lineBuffers.
    private var lineBuffers: [Int: String] = [:]
    private var endpoints: [Int: (String, UInt16)] = [:]

    private func scanLines(id: Int, conn: NWConnection, data: Data) {
        guard let text = String(data: data, encoding: .isoLatin1) else { return }
        var buffer = (lineBuffers[id] ?? "") + text
        while let newline = buffer.firstIndex(of: "\n") {
            let line = String(buffer[..<newline])
            buffer.removeSubrange(...newline)
            if line.hasPrefix("WELCOME ") {
                if let hello = Notifier.shared.helloLine() {
                    conn.send(content: hello.data(using: .isoLatin1), completion: .contentProcessed { _ in })
                }
            } else if line.hasPrefix("NOTE"), let endpoint = endpoints[id] {
                DispatchQueue.main.async { Notifier.shared.serverLine(line, host: endpoint.0, port: endpoint.1) }
            }
        }
        lineBuffers[id] = buffer.count > 4096 ? "" : buffer
    }

    private func finish(id: Int, reason: String) {
        DispatchQueue.main.async {
            self.connections.removeValue(forKey: id)
        }
        let safe = reason.replacingOccurrences(of: "'", with: "").replacingOccurrences(of: "\\", with: "")
        call("PokimonNet.closed(\(id),'\(safe)')")
    }

    private func call(_ js: String) {
        DispatchQueue.main.async {
            self.web?.evaluateJavaScript(js, completionHandler: nil)
        }
    }
}
