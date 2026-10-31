# Online hub: the sixth batch of online features

The sixth batch lives under START → ONLINE → **HUB** (`data/scripts/online.inc`).
Each feature has a section below.

## Catch goals

![Catch goals: the board, the notice when a goal is reached on both games, the reward](images/online-goals.png)

Every Pokémon caught online counts towards the week's server-wide goals
(`server/online_goals.py`):

| Goal | Target | Reward |
| --- | --- | --- |
| Catch Pokémon | 300 | 10 Ultra Balls |
| Fish up Pokémon | 100 | 10 Dive Balls |
| Catch the week's featured Pokémon (Zigzagoon, Wingull, Taillow…) | 40 | 3 Rare Candies |
| Find shiny Pokémon | 3 | 1 Master Ball |

When a goal is reached, everyone online gets a notice. Everyone who caught at
least one Pokémon that week can collect the reward from HUB → CATCH GOALS.
Goals start over every week.

## Legend sightings

![TEST1 meets a roaming Latios, TEST2 gets the notice, the sightings board](images/online-sightings.png)

When a player meets a roaming Pokémon or any legend in a wild battle, the
game reports it (`OEV SIGHT species mapGroup mapNum roaming`). Everyone else
online gets a notice ("TEST1 spotted LATIOS at ROUTE 117!"), at most once
every ten minutes per player and species, and HUB → LEGEND SIGHTINGS shows
where each one was last seen, by whom and how long ago
(`server/online_sightings.py`).

## Website: live map and shiny gallery

![The live map with two players and two legends](images/online-live-map.png)
![The shiny gallery](images/online-shiny-gallery.png)
![TEST1 catches a shiny Absol and TEST2 gets the notice](images/online-shiny-catch.png)

The server's web page (`--http-port`) links two new pages
(`server/online_web.py`):

- **/map**: the game's own Hoenn region map with every player online on
  it (red), and the legends seen in the last day (purple). It refreshes
  every five seconds from `/map.json`.
- **/shinies**: every shiny Pokémon caught online, newest first, with who
  caught it, where and when. Each catch also tells everyone else online.
  The pictures are the game's own sprites (`/sprite/<species>.png`): the
  front picture with its shiny palette swapped in.

`tools/online/gen_ids.py` draws `server/static/hoenn.png` from the region
map's tiles and finds each species' picture and shiny palette. The Docker
image now copies `graphics/pokemon`, `server/static` and every bridge
script (the Online Plaza's `netplay_ext.lua` was missing from it).

## Fishing tournaments

![Start notice, both players' live boards, the results notice and board, a prize](images/online-fishing.png)

Every two hours, for half an hour, the server runs a fishing tournament
(`server/online_fishing.py`, `src/online_fishing.c`). Everyone online is
told when one starts. While it's on:

- a third of the Pokémon that bite are one of the tournament's three rare
  species (Feebas, Relicanth, Luvdisc, Lapras, Dratini, Mantine…, a
  different three each time), at the level of the Pokémon they replace;
- every Pokémon landed with a rod scores its size in centimetres (the same
  size as R → RECORDS), plus 50 for a rare one, and the player is told how
  many points it got.

HUB → FISHING TOURNEY shows the live standings, or the last results and when
the next one starts. When it ends everyone online hears the top three, who
win 5 Rare Candies and ¥20,000, 3 Rare Candies and ¥10,000, and 10 Ultra
Balls and ¥5,000; everyone else who entered gets 3 Dive Balls. Prizes are
collected from the same board.

`NETPLAY_FISHT_PERIOD`, `NETPLAY_FISHT_LENGTH` and `NETPLAY_FISHT_START`
(seconds) change the schedule.

## Co-op fishing

![TEST1 invites TEST2, both reel, both land a giant Avalugg, the giant catch board](images/online-coop-fishing.png)

Two players on the same map can reel in a giant Pokémon together
(`server/online_coopfish.py`, `src/online_fishing.c`,
`data/scripts/online/coop_fishing.inc`). One faces the water and picks
HUB → CO-OP FISHING → FISH TOGETHER, then a player nearby; the other gets an
invitation as soon as they're free. Once both say yes, each plays three
rounds of the reeling meter. With four hits or more between them the giant
(Wailord, Gyarados, Lapras, Milotic, Kingdra, Dondozo, Avalugg…) is landed
and both get one at level 45 (shiny one time in 64). The biggest giants
landed are on HUB → CO-OP FISHING → GIANT CATCHES.

## Player shops

![TEST1 visits TEST2's shop and buys; TEST2 stocks a shelf, hears of the sale and collects](images/online-shops.png)

Every player can run a shop with up to six shelves of items
(`server/online_shops.py`, `src/online_shops.c`,
`data/scripts/online/shops.inc`). HUB → PLAYER SHOPS:

- **VISIT A SHOP** lists the other players' shops, best sellers first.
  Opening one shows its shelves; pick a shelf, then 1, 5 or ALL. The money
  is taken and the items go in the bag (if there's room).
- **MY SHOP** → **STOCK AN ITEM** picks something from the bag's item,
  ball or berry pockets, how many, and a price: half, the same as, one and
  a half or twice the mart price. The items leave the bag. **TAKE BACK**
  empties a shelf, **COLLECT MONEY** pays out what has sold.

Shops stay open while their owner is offline; the money waits on the
server. An owner who is online hears of each sale. The market board and
the auction house sell Pokémon; shops sell items.

## Pokémon pageants

![TEST1 looks through the entries and votes; TEST2 enters a shiny Gardevoir, hears of the vote, sees the standings and collects last week's prize](images/online-pageant.png)

Every week there's a pageant with a theme after the contest categories
(Cutest, Coolest, Most Beautiful, Smartest, Toughest)
(`server/online_pageant.py`, `src/online_pageant.c`,
`data/scripts/online/pageant.inc`). HUB → POKéMON PAGEANT:

- **ENTER A POKéMON** enters one from the party (entering again swaps it,
  and its votes start over). Everyone online hears of it.
- **VOTE** goes through the other players' entries one at a time, each
  with its picture (shiny colours too), and **VOTE FOR IT** gives it one of
  your three votes for the week. Its owner hears of each vote.
- **STANDINGS** shows the week's board. When a week is over, its top three
  win 3, 2 or 1 Rare Candies and ¥15,000, ¥8,000 or ¥4,000, collected here.

## Duo quests

![TEST1 asks TEST2 to team up, both see the quest, one catch each finishes it, both collect the reward and the next quest starts](images/online-duo.png)

Two players team up and work on quests together (`server/online_duo.py`,
`data/scripts/online/duo.inc`). In HUB → DUO QUESTS a player without a
partner picks someone online; the other gets a notice and answers in the
same place. The pair's catches count together: catch 10 Pokémon, land 5
with a rod, catch 6 different species, catch in 4 different places,
catch 20, then round again with bigger targets. Both players hear when
they're halfway and one away. When a quest is done both collect its reward
(Ultra, Dive, Timer or Nest Balls, or Rare Candies, and money) and the next
one starts. The board shows each partner's share; SPLIT UP ends the pair.

## Mentors

![TEST1 joins the mentor list, TEST2 picks TEST1, TEST1 cheers TEST2 on, TEST2 wins a badge and both collect gifts](images/online-mentors.png)

Trainers with six badges or more can mentor newer ones
(`server/online_mentors.py`, `src/online_mentors.c`,
`data/scripts/online/mentors.inc`). In HUB → MENTORS:

- A mentor joins (or leaves) the mentor list and sees their apprentices'
  badges. Once a day they can **CHEER ON** each apprentice, which sends
  3 Super Potions with the notice.
- A trainer with fewer than six badges picks a mentor from the list (up to
  four apprentices each), and can **SAY GOODBYE** later.
- Each badge the apprentice wins brings both a gift: Super Potions, Hyper
  Potions or Ultra Balls for the apprentice, Balls or a Rare Candy for the
  mentor, plus money. At eight badges the apprentice graduates: 5 Rare
  Candies for them, 3 for the mentor, and everyone online hears of it.

The game sends its badge count with every MENTOR request and reports it
(`OEV BADGES n`) whenever it changes.

## Player-made quests

![TEST1 hides 5 Rare Candies on a Lilycove path; TEST2 hears of it, takes the quest, is told it's on this map, that something glints, and finds it](images/online-player-quests.png)

Players hide treasure for each other (`server/online_pquests.py`,
`src/online_pquests.c`, `data/scripts/online/pquests.inc`). In HUB →
PLAYER QUESTS:

- **HIDE A TREASURE** puts items from the bag (1, 5 or all of one kind) at
  the spot where the player stands, with a hint picked from a list ("On
  the path", "By a tree", "Near the water"…). Up to three at once;
  everyone online hears where.
- **FIND A TREASURE** lists the others' treasures by place; taking one
  shows who hid what, where, and the hint. Once the player is on that map
  the game says so, then that something glints when they're three steps
  away or closer, and on the very spot the treasure is theirs. The one who
  hid it hears who found it.
- **MY TREASURES** shows how many were found, and digs one up again.

The spot is the server's copy of the hider's position (the POS lines);
the quest the player took is in the status, so the game checks the spot
itself.

## Spectator betting

![Gary and Mia battle: both games hear of it, TEST1 bets 100 COINS on Gary and TEST2 50 on Mia, Gary wins, TEST1 collects 650 COINS, the best bettors board](images/online-betting.png)

When a PvP battle starts, everyone else online hears of it and can bet
Game Corner COINS on either side for the first two minutes
(`server/online_betting.py`, `data/scripts/online/betting.inc`). In HUB →
BETTING → BET ON A BATTLE the player picks the battle, sees what's been
bet on each side with the payout per coin, picks a side and 10, 50, 100 or
500 COINS. One side per battle; more bets on the same side add up.

When the battle ends, those who backed the winner share the whole pool in
proportion to their bets, and collect it from COLLECT WINNINGS; the others
hear what they lost. After a draw, a dispute or when nobody backed the
winner, every bet comes back. A player with no COINS gets 100 from the
house once a day. BEST BETTORS ranks everyone by COINS won or lost. The
battle itself can be watched from ONLINE → WATCH as before.

The module wraps the plaza server's `cmd_accept` and `finish_battle`, so it
hears of every battle start and result without changing them.

## Ranked doubles

![TEST2 challenges TEST1 to a ranked double battle, TEST1 sent out Mudkip and Poochyena, the battle, everyone hears who won, the RANKED DOUBLES ladder](images/online-ranked-doubles.png)

ONLINE → challenge a player → RANKED DOUBLE BATTLE starts a two-on-two
link battle: each player uses the first two Pokémon of their party, as in a
cable club double battle. It's a new plaza session kind, `double`
(`NETPLAY_KIND_DOUBLE`, bridge `REQ double`), linked the same way as single
battles; `Task_Linkup` now gives each game its battler id from the link
order, which the double battle intro needs for the opponent's name.

Results go to their own Elo ladder (`server/online_doubles.py`, table
`online_doubles`), separate from the singles league; the server announces
each winner, and HUB → DOUBLES LADDER shows the board. Spectator betting
takes double battles too.

## How it talks to the server

These features use the Online Plaza's request channel (`NpxRpc_*` in
`src/netplay_ext.c`, `X` lines on the server, see `docs/netplay_ext.md`) with
their own request words, so they need no new mailbox or bridge code.

In the game, `src/online.c` sends a request from the table in
`src/online_requests.c` (`Online_Request`, VAR_0x8004 = `ONL_REQ_*`) and
handles every answer the same way:

| Rows of the answer | What the game does |
| --- | --- |
| row 0: `message\|code\|var1\|var2\|var3` | message to `gStringVar4` (`~` new line, `^` new page, `#` é), code to `VAR_0x800B`, vars to `gStringVar1`–`3`; `$` is the money sign, `>` itself |
| `+ITEM\|id\|n`, `-ITEM\|id\|n`, `+MONEY\|n`, `-MONEY\|n`, `+COINS\|n`, `-COINS\|n`, `+MON\|species\|level\|shiny` | applied right away |
| `N\|text` | a notice, shown once the player is free |
| `S\|key\|...` | the status of a live feature (`Online_Status`) |
| anything else | data rows (their count is in `VAR_0x800A`) for `Online_ShowBoard` and `Online_Choose` |

A board's data rows are its title, its columns (`x:LABEL` each), then its
lines; a line starting with `*` is the player's own (red), and
`%|percent|text|right text|count` is a progress bar.

In the background the game reports what happens (`OEV CATCH species shiny
mapGroup mapNum fished`, `OEV SIGHT ...`, `OEV FISH species size`) and asks for the live features' status (`OSTAT`)
every ten seconds; both answers bring any notices.

On the server, `server/online.py` adds these words to the plaza's request
handlers. Each feature is a module `server/online_*.py` with `COMMANDS`,
and optionally `setup(db)`, `EVENTS` (reports it listens to) and
`status(ctx)`. `server/online_ids.py` has the game's item, species and map
numbers; `tools/online/gen_ids.py` writes it from the headers.

Numbers this batch owns (agreed with the other feature threads): flags
`0x4C0`–`0x4DF`, vars `0x40BB` and `0x40DB`, object local ids 241–248 on
shared maps.

## Tests and screenshots

- `python3 -m unittest server/test_online.py`
- `website.lua` with `website_seed.py` also screenshots the web pages with
  Playwright (`tools/online_shots/web_shot.js`) while both games are online.
- `python3 tools/online_shots/run.py tools/online_shots/goals.lua OUT 60000 2 tools/online_shots/goals_seed.py`
  runs two games against a fresh server; the seed plays other players first.
  `run.py` is `tools/world_shots/run.py` plus the seed and the bridge
  extensions; `tools/online_shots/lib.lua` has `Driver.openHub`,
  `Driver.catchWild` and `Driver.waitNotice`.
