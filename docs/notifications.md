# Phone notifications

The phone apps tell players about things that happen online while they aren't
looking: trade offers, friends coming online and tournaments starting. They
work while the app is in the background and, with a periodic check, while it
is closed. No push service (Firebase, APNs) is involved: the app asks our own
netplay server.

## Kinds

| Kind      | When                                                         | Default |
| --------- | ------------------------------------------------------------ | ------- |
| `trade`   | someone invites you to a trade or a trade deal               | on      |
| `friend`  | a player on your friend list comes online (at most every 30 minutes per friend) | on |
| `tour`    | a community cup you signed up for starts (with your first opponent) | on |
| `fishing` | a fishing tournament starts (every 2 hours)                  | off     |

Players pick them in the app: ☰ > **Notifications**. The choice is stored on
the phone; the server keeps every note and the app decides which to show.

## Server (server/notify.py)

Notes live in the league database (`notify_devices`, `notify_notes`,
`notify_state`). A note is only kept for a player who registered a phone, for
7 days and at most 50 per player. A background check every 30 seconds starts
cups that are due (so their note goes out on time) and notices fishing
tournaments.

Protocol, on the normal game connection:

```
NOTIFY ON [token]        after HELLO; registers this phone
  -> NOTIFYKEY <name> <tid> <token> <last note id>
NOTE <id> <kind> <text>  sent live to a registered connection
NOTIFY OFF               stop live notes on this connection

NOTIFYPOLL <name> <tid> <token> <since>   on a fresh connection, no HELLO
  -> NOTE lines newer than <since>, then NOTEEND <last id>
  -> NOTEEND -1 for an unknown token (the app registers again next time it plays)
```

A player can have up to 4 phones; the oldest token goes first.

Environment: `NETPLAY_NOTIFY_TICK` (seconds between checks, default 30),
`NETPLAY_NOTIFY_FRIEND_QUIET` (seconds between two "is online" notes for the
same friend, default 1800).

Tests: `python3 server/test_notify.py`.

## Android (mobile/android)

- `NetBridge` looks at every line from the server. After `WELCOME` it sends
  `NOTIFY ON <token>`; `NOTIFYKEY` saves the name, trainer id and token;
  `NOTE` lines are shown when the app is in the background.
- `Notifier` keeps the settings and last seen note id in SharedPreferences
  and posts the notifications (channel "Online play").
- `NotifyJob` (JobScheduler, about every 15 minutes, needs network, survives
  reboots) connects, sends `NOTIFYPOLL` and shows new notes.
- Android 13+ asks for the notification permission the first time the game
  starts.

## iOS (mobile/ios)

- `NetBridge.swift` reads the lines the same way and passes them to
  `Notifier.swift`, which uses UserDefaults and UNUserNotificationCenter.
- A `BGAppRefreshTask` (`io.github.rdiol12.pokimonmulti.notify`) polls with `NOTIFYPOLL`
  when iOS gives the app time; iOS decides how often, so notes can arrive
  late while the app is closed.
- The web page sends the settings through the `pokimonNotify` message
  handler.
