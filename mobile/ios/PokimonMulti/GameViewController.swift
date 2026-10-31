import UIKit
import WebKit

/// The whole app is one WKWebView showing the web app bundled in dist/
/// (mobile/dist): the mGBA core, touch controls and the netplay bridge.
/// Files are served through the app:// scheme so the page gets a stable
/// origin for IndexedDB; the page's TCP connections go through NetBridge.
final class GameViewController: UIViewController, WKNavigationDelegate, WKUIDelegate, WKScriptMessageHandler {
    static let scheme = "app"
    private var web: WKWebView!
    private var net: NetBridge!

    override func loadView() {
        let config = WKWebViewConfiguration()
        config.setURLSchemeHandler(BundleSchemeHandler(), forURLScheme: Self.scheme)
        config.allowsInlineMediaPlayback = true
        config.mediaTypesRequiringUserActionForPlayback = []
        config.websiteDataStore = .default()

        // Script messages are weakly proxied so the controller can be freed.
        let proxy = WeakMessageProxy(self)
        config.userContentController.add(proxy, name: "pokimonNet")
        config.userContentController.add(proxy, name: "pokimonFiles")

        web = WKWebView(frame: .zero, configuration: config)
        web.navigationDelegate = self
        web.uiDelegate = self
        web.isOpaque = false
        web.backgroundColor = UIColor(red: 0x26 / 255.0, green: 0x2B / 255.0, blue: 0x31 / 255.0, alpha: 1)
        web.scrollView.isScrollEnabled = false
        web.scrollView.bounces = false
        web.scrollView.contentInsetAdjustmentBehavior = .never
        if #available(iOS 16.4, *) {
            web.isInspectable = true
        }
        view = web
        net = NetBridge(web: web)
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        web.load(URLRequest(url: URL(string: "\(Self.scheme)://game/index.html")!))
        UIApplication.shared.isIdleTimerDisabled = true
    }

    override var prefersStatusBarHidden: Bool { true }
    override var prefersHomeIndicatorAutoHidden: Bool { true }
    override var preferredScreenEdgesDeferringSystemGestures: UIRectEdge { .all }

    func userContentController(_ controller: WKUserContentController, didReceive message: WKScriptMessage) {
        guard let body = message.body as? [String: Any] else { return }
        switch message.name {
        case "pokimonNet":
            net.handle(body)
        case "pokimonFiles":
            if let name = body["name"] as? String, let data = (body["data"] as? String).flatMap({ Data(base64Encoded: $0) }) {
                share(name: name, data: data)
            }
        default:
            break
        }
    }

    /// Export save: hands the file to the share sheet (Save to Files, AirDrop...).
    private func share(name: String, data: Data) {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent(name)
        do {
            try data.write(to: url)
        } catch {
            return
        }
        let sheet = UIActivityViewController(activityItems: [url], applicationActivities: nil)
        sheet.popoverPresentationController?.sourceView = view
        sheet.popoverPresentationController?.sourceRect = CGRect(x: view.bounds.midX, y: view.bounds.midY, width: 1, height: 1)
        present(sheet, animated: true)
    }

    // Links (the releases page, target=_blank) open in Safari.
    func webView(_ webView: WKWebView, createWebViewWith configuration: WKWebViewConfiguration,
                 for action: WKNavigationAction, windowFeatures: WKWindowFeatures) -> WKWebView? {
        if let url = action.request.url, url.scheme == "https" {
            UIApplication.shared.open(url)
        }
        return nil
    }

    // If iOS kills the web content to free memory, start it again.
    func webViewWebContentProcessDidTerminate(_ webView: WKWebView) {
        net.closeAll()
        webView.reload()
    }
}

/// Serves app://game/<path> from the bundle's dist folder (a folder
/// reference keeps its name in the bundle).
final class BundleSchemeHandler: NSObject, WKURLSchemeHandler {
    private static let types: [String: String] = [
        "html": "text/html", "js": "text/javascript", "css": "text/css", "json": "application/json",
        "wasm": "application/wasm", "lua": "text/plain", "png": "image/png", "txt": "text/plain",
    ]
    private let root = Bundle.main.resourceURL!.appendingPathComponent("dist")

    func webView(_ webView: WKWebView, start task: WKURLSchemeTask) {
        guard let url = task.request.url else { return }
        var path = url.path
        if path.isEmpty || path == "/" { path = "/index.html" }
        let file = root.appendingPathComponent(String(path.dropFirst())).standardizedFileURL
        guard file.path.hasPrefix(root.standardizedFileURL.path),
              let data = try? Data(contentsOf: file, options: .mappedIfSafe) else {
            let res = HTTPURLResponse(url: url, statusCode: 404, httpVersion: "HTTP/1.1", headerFields: [:])!
            task.didReceive(res)
            task.didFinish()
            return
        }
        let type = Self.types[file.pathExtension.lowercased()] ?? "application/octet-stream"
        let res = HTTPURLResponse(url: url, statusCode: 200, httpVersion: "HTTP/1.1", headerFields: [
            "Content-Type": type,
            "Content-Length": String(data.count),
            "Cache-Control": "no-cache",
        ])!
        task.didReceive(res)
        task.didReceive(data)
        task.didFinish()
    }

    func webView(_ webView: WKWebView, stop task: WKURLSchemeTask) {}
}

/// WKUserContentController keeps its handlers strongly; this breaks the cycle.
final class WeakMessageProxy: NSObject, WKScriptMessageHandler {
    weak var target: WKScriptMessageHandler?

    init(_ target: WKScriptMessageHandler) {
        self.target = target
    }

    func userContentController(_ controller: WKUserContentController, didReceive message: WKScriptMessage) {
        target?.userContentController(controller, didReceive: message)
    }
}
