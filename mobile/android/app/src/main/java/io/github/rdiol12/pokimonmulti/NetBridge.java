package io.github.rdiol12.pokimonmulti;

import android.util.Base64;
import android.webkit.JavascriptInterface;
import android.webkit.WebView;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * window.PokimonNative: TCP connections to the netplay server for the page
 * (www/net.js). Each connection has a reader thread that hands what arrives
 * to window.PokimonNet.data(id, base64); writes go through one thread.
 */
public class NetBridge {
    private static final int CONNECT_TIMEOUT_MS = 8000;

    private final MainActivity activity;
    private final WebView web;
    private final Map<Integer, Socket> sockets = new ConcurrentHashMap<>();
    private final ExecutorService writer = Executors.newSingleThreadExecutor();

    NetBridge(MainActivity activity, WebView web) {
        this.activity = activity;
        this.web = web;
    }

    @JavascriptInterface
    public void connect(int id, String host, int port) {
        Thread t = new Thread(() -> run(id, host, port), "netplay-" + id);
        t.setDaemon(true);
        t.start();
    }

    private void run(int id, String host, int port) {
        Socket socket = new Socket();
        try {
            socket.setTcpNoDelay(true);
            socket.connect(new InetSocketAddress(host, port), CONNECT_TIMEOUT_MS);
            sockets.put(id, socket);
            call("PokimonNet.opened(" + id + ")");
            InputStream in = socket.getInputStream();
            byte[] buf = new byte[8192];
            StringBuilder line = new StringBuilder();
            int n;
            while ((n = in.read(buf)) > 0) {
                String b64 = Base64.encodeToString(buf, 0, n, Base64.NO_WRAP);
                call("PokimonNet.data(" + id + ",'" + b64 + "')");
                // Notifications ride on the game connection (Notifier); this
                // keeps working while the page is paused in the background.
                for (int i = 0; i < n; i++) {
                    char ch = (char) (buf[i] & 0xFF);
                    if (ch == '\n') {
                        serverLine(id, line.toString(), host, port);
                        line.setLength(0);
                    } else if (line.length() < 512) {
                        line.append(ch);
                    }
                }
            }
            call("PokimonNet.closed(" + id + ",'server closed the connection')");
        } catch (IOException e) {
            call("PokimonNet.closed(" + id + ",'" + quote(e.getClass().getSimpleName()) + "')");
        } finally {
            sockets.remove(id);
            try { socket.close(); } catch (IOException ignored) { }
        }
    }

    private void serverLine(int id, String line, String host, int port) {
        if (line.startsWith("WELCOME ")) {
            String hello = Notifier.helloLine(activity);
            if (hello != null) send(id, hello);
        } else if (line.startsWith("NOTE") && Notifier.onServerLine(activity, line, host, port)) {
            activity.runOnUiThread(activity::askNotificationPermission);
        }
    }

    /** The notification settings for the page's menu, as JSON (see Notifier.settingsJson). */
    @JavascriptInterface
    public String notifySettings() {
        return Notifier.settingsJson(activity);
    }

    @JavascriptInterface
    public void setNotifySettings(String json) {
        Notifier.setSettings(activity, json);
    }

    @JavascriptInterface
    public void send(int id, String text) {
        Socket socket = sockets.get(id);
        if (socket == null) return;
        byte[] bytes = text.getBytes(StandardCharsets.ISO_8859_1);
        writer.execute(() -> {
            try {
                OutputStream out = socket.getOutputStream();
                out.write(bytes);
                out.flush();
            } catch (IOException e) {
                try { socket.close(); } catch (IOException ignored) { }
            }
        });
    }

    @JavascriptInterface
    public void close(int id) {
        Socket socket = sockets.remove(id);
        if (socket == null) return;
        writer.execute(() -> {
            try { socket.close(); } catch (IOException ignored) { }
        });
    }

    /** Export save: the page hands over the file as base64. */
    @JavascriptInterface
    public void saveFile(String name, String base64) {
        byte[] data = Base64.decode(base64, Base64.DEFAULT);
        activity.runOnUiThread(() -> activity.saveFile(name, data));
    }

    void closeAll() {
        for (Socket s : sockets.values()) {
            try { s.close(); } catch (IOException ignored) { }
        }
        sockets.clear();
        writer.shutdownNow();
    }

    private void call(String js) {
        web.post(() -> web.evaluateJavascript(js, null));
    }

    private static String quote(String s) {
        return s.replace("\\", "").replace("'", "");
    }
}
