package io.github.rdiol12.pokimonmulti;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * The whole app is one WebView showing the web app from the APK's assets
 * (mobile/dist): the mGBA core, touch controls and the netplay bridge. This
 * class serves those files, opens the file pickers, and gives the page real
 * TCP sockets through {@link NetBridge}.
 */
public class MainActivity extends Activity {
    // A https origin of our own, so the page gets IndexedDB, wasm and a
    // secure context. Requests to it are answered from the assets.
    static final String ORIGIN = "https://appassets.androidplatform.net";
    private static final int PICK_FILE = 1;
    private static final int CREATE_FILE = 2;
    private static final Map<String, String> MIME = new HashMap<>();

    static {
        MIME.put("html", "text/html");
        MIME.put("js", "text/javascript");
        MIME.put("mjs", "text/javascript");
        MIME.put("css", "text/css");
        MIME.put("json", "application/json");
        MIME.put("wasm", "application/wasm");
        MIME.put("lua", "text/plain");
        MIME.put("png", "image/png");
        MIME.put("txt", "text/plain");
    }

    private WebView web;
    private NetBridge net;
    private ValueCallback<Uri[]> pendingPick;
    private byte[] pendingSave;

    @SuppressLint("SetJavaScriptEnabled")
    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        web = new WebView(this);
        setContentView(web);
        hideSystemBars();

        WebSettings s = web.getSettings();
        s.setJavaScriptEnabled(true);
        s.setDomStorageEnabled(true);
        s.setDatabaseEnabled(true);
        s.setMediaPlaybackRequiresUserGesture(false);
        s.setAllowFileAccess(false);
        s.setAllowContentAccess(false);

        net = new NetBridge(this, web);
        web.addJavascriptInterface(net, "PokimonNative");
        web.setWebViewClient(new WebViewClient() {
            @Override
            public WebResourceResponse shouldInterceptRequest(WebView view, WebResourceRequest request) {
                return serveAsset(request.getUrl());
            }

            @Override
            public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
                Uri url = request.getUrl();
                if (url.toString().startsWith(ORIGIN)) return false;
                // Links (the releases page) open in the browser.
                startActivity(new Intent(Intent.ACTION_VIEW, url));
                return true;
            }
        });
        web.setWebChromeClient(new WebChromeClient() {
            @Override
            public boolean onShowFileChooser(WebView view, ValueCallback<Uri[]> callback, FileChooserParams params) {
                if (pendingPick != null) pendingPick.onReceiveValue(null);
                pendingPick = callback;
                Intent pick = new Intent(Intent.ACTION_GET_CONTENT);
                pick.addCategory(Intent.CATEGORY_OPENABLE);
                pick.setType("*/*"); // .gba, .bps and .sav have no common MIME type
                startActivityForResult(Intent.createChooser(pick, "Choose a file"), PICK_FILE);
                return true;
            }
        });
        if (state != null) web.restoreState(state);
        else web.loadUrl(ORIGIN + "/index.html");
    }

    private WebResourceResponse serveAsset(Uri url) {
        if (!ORIGIN.equals(url.getScheme() + "://" + url.getHost())) return null;
        String path = url.getPath();
        if (path == null || path.equals("/")) path = "/index.html";
        String name = path.substring(1);
        if (name.contains("..")) return notFound();
        int dot = name.lastIndexOf('.');
        String mime = MIME.get(dot < 0 ? "" : name.substring(dot + 1));
        if (mime == null) mime = "application/octet-stream";
        try {
            InputStream in = getAssets().open(name);
            WebResourceResponse res = new WebResourceResponse(mime, null, in);
            Map<String, String> headers = new HashMap<>();
            headers.put("Cache-Control", "no-cache");
            res.setResponseHeaders(headers);
            return res;
        } catch (IOException e) {
            return notFound();
        }
    }

    private static WebResourceResponse notFound() {
        WebResourceResponse res = new WebResourceResponse("text/plain", "utf-8", null);
        res.setStatusCodeAndReasonPhrase(404, "Not Found");
        return res;
    }

    /** Called from the page (Export save): asks where to save the file. */
    void saveFile(String name, byte[] data) {
        pendingSave = data;
        Intent create = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        create.addCategory(Intent.CATEGORY_OPENABLE);
        create.setType("application/octet-stream");
        create.putExtra(Intent.EXTRA_TITLE, name);
        startActivityForResult(create, CREATE_FILE);
    }

    @Override
    protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        Uri uri = result == RESULT_OK && data != null ? data.getData() : null;
        if (request == PICK_FILE && pendingPick != null) {
            pendingPick.onReceiveValue(uri == null ? null : new Uri[] { uri });
            pendingPick = null;
        } else if (request == CREATE_FILE && pendingSave != null) {
            byte[] bytes = pendingSave;
            pendingSave = null;
            if (uri == null) return;
            try (OutputStream out = getContentResolver().openOutputStream(uri)) {
                if (out != null) out.write(bytes);
                toast("Save exported");
            } catch (IOException e) {
                toast("Could not write the file");
            }
        }
    }

    private void toast(String text) {
        android.widget.Toast.makeText(this, text, android.widget.Toast.LENGTH_SHORT).show();
    }

    private void hideSystemBars() {
        if (android.os.Build.VERSION.SDK_INT >= 30) {
            WindowInsetsController c = getWindow().getInsetsController();
            if (c != null) {
                c.hide(WindowInsets.Type.systemBars());
                c.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean focus) {
        super.onWindowFocusChanged(focus);
        if (focus) hideSystemBars();
    }

    @Override
    protected void onPause() {
        // The page saves the game when it is hidden; this hides it.
        web.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        web.onResume();
    }

    @Override
    protected void onSaveInstanceState(Bundle out) {
        super.onSaveInstanceState(out);
        web.saveState(out);
    }

    @Override
    protected void onDestroy() {
        net.closeAll();
        web.destroy();
        super.onDestroy();
    }
}
