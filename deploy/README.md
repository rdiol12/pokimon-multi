# Running the netplay server

For playing with friends, `make play` is enough: the host's machine runs the
server next to the game (see docs/netplay.md). This page is for a server that
stays up on its own.

One process runs everything players need:

- **Game port** (TCP 7777): the mGBA bridge script connects here. It handles
  presence, invites, battles, trades, deals and the league.
- **Web port** (HTTP 8080): the league leaderboard page, `/leaderboard.json`,
  `/health`, and `/netplay_bridge.lua`, the bridge script with this server's
  address already filled in.

The league is one SQLite file. Back it up by copying it while the server is
stopped, or with `sqlite3 league.sqlite3 ".backup copy.sqlite3"` while it
runs.

## With Docker

```sh
cd deploy
NETPLAY_PUBLIC_HOST=play.example.org docker compose up -d --build
```

The league is kept in the `league-data` volume. `NETPLAY_PUBLIC_HOST` is the
name or IP players use to reach the server; it goes into the downloaded bridge
script. Without it, the script uses whatever address the page was opened with.

Update after pulling new code: `docker compose up -d --build`.

## Without Docker (systemd)

Needs Python 3.10 or newer, no other packages.

```sh
sudo useradd --system --home-dir /var/lib/pokimon-netplay pokimon
sudo git clone https://github.com/rdiol12/pokimon-multi /opt/pokimon-multi
sudo cp /opt/pokimon-multi/deploy/pokimon-netplay.service /etc/systemd/system/
sudo systemctl enable --now pokimon-netplay
journalctl -u pokimon-netplay -f
```

Set `NETPLAY_PUBLIC_HOST` in the unit file if players reach the server by a
name the page can't guess.

## Firewall

Open TCP 7777 (game) and 8080 (web). Nothing else is needed.

## Settings

Each setting is a command line flag or an environment variable.

| Flag | Variable | Default | |
|------|----------|---------|---|
| `--host` | `NETPLAY_HOST` | `0.0.0.0` | address to listen on |
| `--port` | `NETPLAY_PORT` | `7777` | game port |
| `--http-port` | `NETPLAY_HTTP_PORT` | `0` (off; 8080 in Docker and systemd) | web port |
| `--public-host` | `NETPLAY_PUBLIC_HOST` | from the request | address written into the bridge download |
| `--db` | `NETPLAY_DB` | `league.sqlite3` | league database file |

## Players

`make play JOIN=<server>` opens the game connected to it. Without the source:

1. Open `http://<server>:8080/` and download `netplay_bridge.lua`.
2. Open the game in mGBA 0.10 or newer.
3. *Tools → Scripting… → File → Load script*, and pick the downloaded file.
