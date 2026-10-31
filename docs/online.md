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

## How it talks to the server

These features use the Online Plaza's request channel (`NpxRpc_*` in
`src/netplay_ext.c`, `X` lines on the server, see `docs/netplay_ext.md`) with
their own request words, so they need no new mailbox or bridge code.

In the game, `src/online.c` sends a request from the table in
`src/online_requests.c` (`Online_Request`, VAR_0x8004 = `ONL_REQ_*`) and
handles every answer the same way:

| Rows of the answer | What the game does |
| --- | --- |
| row 0: `message\|code\|var1\|var2\|var3` | message to `gStringVar4` (`~` new line, `^` new page, `#` é), code to `VAR_0x800B`, vars to `gStringVar1`–`3` |
| `+ITEM\|id\|n`, `-ITEM\|id\|n`, `+MONEY\|n`, `-MONEY\|n`, `+MON\|species\|level\|shiny` | applied right away |
| `N\|text` | a notice, shown once the player is free |
| `S\|key\|...` | the status of a live feature (`Online_Status`) |
| anything else | data rows (their count is in `VAR_0x800A`) for `Online_ShowBoard` and `Online_Choose` |

A board's data rows are its title, its columns (`x:LABEL` each), then its
lines; a line starting with `*` is the player's own (red), and
`%|percent|text|right text|count` is a progress bar.

In the background the game reports what happens (`OEV CATCH species shiny
mapGroup mapNum fished`) and asks for the live features' status (`OSTAT`)
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
- `python3 tools/online_shots/run.py tools/online_shots/goals.lua OUT 60000 2 tools/online_shots/goals_seed.py`
  runs two games against a fresh server; the seed plays other players first.
  `run.py` is `tools/world_shots/run.py` plus the seed and the bridge
  extensions; `tools/online_shots/lib.lua` has `Driver.openHub`,
  `Driver.catchWild` and `Driver.waitNotice`.
