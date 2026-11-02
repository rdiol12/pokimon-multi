package io.github.rdiol12.pokimonmulti;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

/**
 * Phone notifications for online play (docs/notifications.md): trade offers,
 * friends coming online and tournaments starting.
 *
 * While the game is connected, the server sends each note on the game's own
 * connection ({@link NetBridge} hands the lines here) and it is shown when the
 * app is in the background. When the app isn't running, {@link NotifyJob}
 * asks the server for new notes about every 15 minutes.
 */
final class Notifier {
    private static final String PREFS = "notify";
    private static final String CHANNEL = "online";
    private static final int JOB_ID = 1;
    private static final int CONNECT_TIMEOUT_MS = 8000;
    private static final int READ_TIMEOUT_MS = 10000;
    private static final String[] KINDS = { "trade", "friend", "tour", "fishing" };

    /** The game is on screen: notes then show in the game, not as notifications. */
    static volatile boolean foreground;

    private Notifier() { }

    private static SharedPreferences prefs(Context c) {
        return c.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }

    static boolean enabled(Context c) {
        return prefs(c).getBoolean("on", true);
    }

    private static boolean kindOn(Context c, String kind) {
        // Fishing tournaments happen every two hours, so they are opt-in
        return prefs(c).getBoolean("kind." + kind, !kind.equals("fishing"));
    }

    /** The page's settings: {"on": true, "trade": true, "friend": true, "tour": true, "fishing": false}. */
    static String settingsJson(Context c) {
        JSONObject o = new JSONObject();
        try {
            o.put("on", enabled(c));
            for (String kind : KINDS) o.put(kind, kindOn(c, kind));
        } catch (JSONException ignored) { }
        return o.toString();
    }

    static void setSettings(Context c, String json) {
        try {
            JSONObject o = new JSONObject(json);
            SharedPreferences.Editor e = prefs(c).edit();
            if (o.has("on")) e.putBoolean("on", o.getBoolean("on"));
            for (String kind : KINDS) {
                if (o.has(kind)) e.putBoolean("kind." + kind, o.getBoolean(kind));
            }
            e.apply();
        } catch (JSONException ignored) { }
        schedule(c);
    }

    /** The line to send after the server's WELCOME, or null when notifications are off. */
    static String helloLine(Context c) {
        if (!enabled(c)) return null;
        return ("NOTIFY ON " + prefs(c).getString("token", "")).trim() + "\n";
    }

    /**
     * Called for each line the server sends on the game connection.
     * Returns true when the activity should ask for the notification permission.
     */
    static boolean onServerLine(Context c, String line, String host, int port) {
        if (line.startsWith("NOTIFYKEY ")) {
            String[] f = line.split(" ");
            if (f.length < 5) return false;
            SharedPreferences p = prefs(c);
            long last = Math.max(parseLong(f[4]), p.getString("token", "").equals(f[3]) ? p.getLong("last", 0) : 0);
            p.edit()
                    .putString("host", host).putInt("port", port)
                    .putString("name", f[1]).putInt("tid", (int) parseLong(f[2]))
                    .putString("token", f[3]).putLong("last", last)
                    .apply();
            schedule(c);
            boolean ask = !p.getBoolean("asked", false);
            if (ask) p.edit().putBoolean("asked", true).apply();
            return ask;
        }
        if (line.startsWith("NOTE ")) {
            String[] f = line.split(" ", 4);
            if (f.length == 4) note(c, parseLong(f[1]), f[2], f[3]);
        }
        return false;
    }

    private static void note(Context c, long id, String kind, String text) {
        SharedPreferences p = prefs(c);
        if (id <= p.getLong("last", 0)) return;
        p.edit().putLong("last", id).apply();
        if (foreground || !enabled(c) || !kindOn(c, kind)) return;
        show(c, (int) id, kind, text);
    }

    private static String title(String kind) {
        switch (kind) {
            case "trade": return "Trade offer";
            case "friend": return "Friend online";
            case "tour": return "Tournament";
            case "fishing": return "Fishing tournament";
            default: return "Pokimon Multi";
        }
    }

    private static void show(Context c, int id, String kind, String text) {
        if (Build.VERSION.SDK_INT >= 33
                && c.checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        NotificationManager nm = c.getSystemService(NotificationManager.class);
        if (nm == null) return;
        if (nm.getNotificationChannel(CHANNEL) == null) {
            NotificationChannel ch = new NotificationChannel(CHANNEL, "Online play", NotificationManager.IMPORTANCE_DEFAULT);
            ch.setDescription("Trade offers, friends coming online and tournaments");
            nm.createNotificationChannel(ch);
        }
        Intent open = new Intent(c, MainActivity.class).setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        PendingIntent tap = PendingIntent.getActivity(c, 0, open, PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_UPDATE_CURRENT);
        Notification n = new Notification.Builder(c, CHANNEL)
                .setSmallIcon(R.drawable.ic_notify)
                .setContentTitle(title(kind))
                .setContentText(text)
                .setStyle(new Notification.BigTextStyle().bigText(text))
                .setContentIntent(tap)
                .setAutoCancel(true)
                .build();
        nm.notify(id, n);
    }

    /** Polls about every 15 minutes while a player is registered and notifications are on. */
    static void schedule(Context c) {
        JobScheduler js = c.getSystemService(JobScheduler.class);
        if (js == null) return;
        if (!enabled(c) || prefs(c).getString("token", "").isEmpty()) {
            js.cancel(JOB_ID);
            return;
        }
        if (js.getPendingJob(JOB_ID) != null) return;
        JobInfo job = new JobInfo.Builder(JOB_ID, new ComponentName(c, NotifyJob.class))
                .setPeriodic(15 * 60 * 1000L)
                .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                .setPersisted(true)
                .build();
        js.schedule(job);
    }

    /** Asks the server for notes since the last one seen. Blocks; run it off the main thread. */
    static void poll(Context c) {
        SharedPreferences p = prefs(c);
        String host = p.getString("host", "");
        String token = p.getString("token", "");
        if (!enabled(c) || foreground || host.isEmpty() || token.isEmpty()) return;
        String request = "NOTIFYPOLL " + p.getString("name", "") + " " + p.getInt("tid", 0) + " " + token + " "
                + p.getLong("last", 0) + "\n";
        try (Socket s = new Socket()) {
            s.connect(new InetSocketAddress(host, p.getInt("port", 0)), CONNECT_TIMEOUT_MS);
            s.setSoTimeout(READ_TIMEOUT_MS);
            OutputStream out = s.getOutputStream();
            out.write(request.getBytes(StandardCharsets.ISO_8859_1));
            out.flush();
            BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream(), StandardCharsets.ISO_8859_1));
            String line;
            while ((line = in.readLine()) != null) {
                if (line.startsWith("NOTEEND")) {
                    if (line.trim().equals("NOTEEND -1")) {
                        // The server no longer knows this phone: register again next time online
                        p.edit().remove("token").apply();
                        schedule(c);
                    }
                    break;
                }
                onServerLine(c, line, host, p.getInt("port", 0));
            }
        } catch (IOException ignored) {
            // Offline or the server is down: try again next time
        }
    }

    private static long parseLong(String s) {
        try {
            return Long.parseLong(s.trim());
        } catch (NumberFormatException e) {
            return 0;
        }
    }
}
