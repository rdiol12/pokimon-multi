# Netplay

Online multiplayer for the game: players see each other on the overworld, can
challenge each other to battles or trade, and battle results feed a league
leaderboard.

```
 ROM (gNetplayMailbox in EWRAM)  <->  mGBA Lua bridge  <->  TCP  <->  server/
```

- **ROM** keeps a small shared-memory mailbox up to date (own position, requests,
  outgoing link packets) and reads what the bridge writes into it (other
  players, invites, incoming link packets).
- **Bridge** (`tools/netplay_bridge/netplay_bridge.lua`) runs inside mGBA 0.10+,
  copies the mailbox to and from the server once per frame.
- **Server** (`server/netplay_server.py`) relays positions between players on
  the same map, pairs players for battles and trades, relays their link
  packets, and keeps the leaderboard in SQLite.

## Running

```sh
python3 server/netplay_server.py --port 7777 --db league.sqlite3
```

In mGBA: *Tools → Scripting… → File → Load script*, pick
`tools/netplay_bridge/netplay_bridge.lua`. Edit `HOST`/`PORT` at the top of the
script to point at the server.

Tests: `python3 -m unittest discover -s server` and
`lua5.4 tools/netplay_bridge/test_bridge.lua`.

End-to-end: `python3 tools/netplay_bridge/e2e/run_e2e.py` (needs a built
`pokeemerald.gba`, `libmgba-dev` 0.10 and gcc). It runs two headless games
against a fresh server: they meet in Oldale Town, battle (player 1 forfeits),
trade their lead Pokémon, and then player 2 sells a Pokémon to player 1 for
¥1,000. It checks the leaderboard, both parties and both wallets. Screenshots and
logs go to `--out`.

## Mailbox layout (version 1)

All values little-endian. The bridge finds the mailbox by scanning EWRAM
(`0x02000000`–`0x0203FFFF`, 4-byte aligned) for the magic. "ROM" / "bridge"
says who writes the field; the other side only reads it.

| Offset | Size | Writer | Field |
|-------:|-----:|--------|-------|
| 0x000 | 8 | ROM | magic `"PKMNNET1"` |
| 0x008 | 1 | ROM | version (1) |
| 0x009 | 1 | bridge | state: 0 offline, 1 connecting, 2 online |
| 0x00A | 2 | bridge | local player id assigned by the server |
| 0x00C | 8 | ROM | player name, game charset, `0xFF` terminated; empty until the player is in the overworld |
| 0x014 | 2 | ROM | trainer id (low 16 bits) |
| 0x016 | 1 | ROM | map group |
| 0x017 | 1 | ROM | map number |
| 0x018 | 2 | ROM | x (s16, map coordinates) |
| 0x01A | 2 | ROM | y (s16) |
| 0x01C | 1 | ROM | facing direction (`DIR_*`) |
| 0x01D | 1 | ROM | elevation |
| 0x01E | 2 | ROM | avatar graphics id (`OBJ_EVENT_GFX_*`) |
| 0x020 | 8×20 | bridge | remote players (below) |
| 0x0C0 | 8 | ROM | request (below) |
| 0x0C8 | 8 | bridge | event (below) |
| 0x0D0 | 1 | ROM | event ack: ROM copies the event seq here once handled |
| 0x0D1 | 3 | — | padding |
| 0x0D4 | 4 | ROM/bridge | link TX ring: u8 head (ROM), u8 tail (bridge), 2 pad |
| 0x0D8 | 16×20 | ROM | link TX packets |
| 0x218 | 4 | bridge/ROM | link RX ring: u8 head (bridge), u8 tail (ROM), 2 pad |
| 0x21C | 16×20 | bridge | link RX packets |
| 0x35C | | | end |

The C definition is `struct NetplayMailbox` in `include/netplay.h`.

**Remote player slot (20 bytes):** u16 id (0 = empty slot), u8 map group,
u8 map number, s16 x, s16 y, u8 facing, u8 elevation, u16 graphics id,
u8 name[8]. The bridge only fills slots with players on the same map as the
local player, and keeps each player in the same slot while they stay.

**Request (ROM → bridge, 8 bytes):** u8 seq, u8 type, u16 target id,
u16 session id, u16 arg. The ROM fills the fields, then increments `seq`, at
most once per frame; the bridge acts once per new `seq`.

| type | meaning | fields used |
|-----:|---------|-------------|
| 1 | challenge to a battle | target id |
| 2 | offer a trade | target id |
| 3 | accept invite | session id |
| 4 | decline invite | session id |
| 5 | report battle result | session id, arg: 1 won, 2 lost, 3 draw |
| 6 | leave or cancel a session | session id |
| 7 | offer a deal (buy/sell) | target id |

**Event (bridge → ROM, 8 bytes):** u8 seq, u8 type, u16 peer id,
u16 session id, u8 kind (1 battle, 2 trade, 3 deal), u8 role (0 host, 1 guest). The
bridge writes a new event only after the ROM has acked the previous one.

| type | meaning |
|-----:|---------|
| 1 | invite received from peer |
| 2 | session started; open the link with `role` |
| 3 | our invite was declined (or the player was busy or gone) |
| 4 | session ended (peer left, canceled, or disconnected) |
| 5 | our invite was sent; this is its session id |

**Link rings:** a packet is 20 bytes: u8 kind (1 data, 2 handshake,
3 handshake ack, 4 deal offer chunk, 5 deal answer), u8 chunk index (deal
offers only), 2 bytes padding, then 16 bytes: one link command (`CMD_LENGTH`
= 8 u16), a chunk of a deal offer, or a deal answer (first u16: 1 yes, 0 no). A ring is empty when `head == tail`; indices wrap at 16. The writer
fills `packets[head]`, then advances `head`; the reader consumes
`packets[tail]`, then advances `tail`. On `START` the bridge empties both rings.

## Link over the network

While a session is running, `link.c` uses the rings instead of the serial port
(`Netplay_LinkHandshake` and `Netplay_LinkTransfer` in `src/netplay.c`):

- **Handshake.** When the link opens, the host sends a handshake packet and the
  guest answers with a handshake ack; each side is connected once it has seen
  the other's. Packets that arrive before that are left over from an earlier
  link and are dropped.
- **Transfer.** Every frame each game sends exactly one data packet: its next
  queued link command, or zeros. The k-th packet from each side is delivered
  together as one entry of the receive queue, which is what a serial transfer
  does. Network delay only delays entries; it never reorders them.

Battles and trades then run on Emerald's own link code: a battle uses each
player's whole party (healed, and restored afterwards like a cable club
battle), and a trade uses the normal trade screen.

## Deals (buy/sell)

"Buy/Sell" in the talk menu starts a deal session. It doesn't open a link.
Each player picks a Pokémon to offer (or none) and an amount of money (or
none). Each game sends its offer as 7 deal offer chunks: u32 money, u8 has
Pokémon, 3 padding, then the 100-byte `struct Pokemon`. Both players see both
offers and answer yes or no with a deal answer packet. When both say yes,
each game removes what it gave, adds what it got (marking the Pokémon as
caught in the Pokédex), and saves. A deal that would leave a party empty or
with more than 6 Pokémon is refused. Deals never touch the leaderboard.

## Server protocol

TCP, one command per line, fields separated by single spaces.

Client → server:

```
HELLO <name> <trainer id>
POS <map group> <map num> <x> <y> <facing> <elevation> <gfx>
REQ <battle|trade|deal> <target id>      (answered with PENDING, then START or DECLINED)
ACCEPT <session id>
DECLINE <session id>
LINK <session id> <hex>
RESULT <session id> <win|loss|draw>
QUIT <session id>                   (leave a session, or cancel an invite)
BOARD
```

Server → client:

```
WELCOME <id>
PENDING <session id>
PLAYER <id> <name> <map group> <map num> <x> <y> <facing> <elevation> <gfx>
GONE <id>
INVITE <session id> <battle|trade|deal> <from id>
START <session id> <battle|trade|deal> <peer id> <0 host|1 guest>
DECLINED <session id>
ENDED <session id>
LINK <session id> <hex>
RANK <position> <name> <rating> <wins> <losses> <draws>
BOARDEND
ERR <message>
```

## League rules

- Players are identified by name + trainer id. New players start at 1000 Elo.
- A battle counts once **both** players report matching results (win/loss or
  draw/draw); rating moves with K = 32.
- Mismatched reports are recorded as disputed and do not change ratings.
- If a player disconnects during a battle, the remaining player's report is
  accepted on its own, so leaving a losing battle counts as a loss.
- Trades and deals never touch the leaderboard.
