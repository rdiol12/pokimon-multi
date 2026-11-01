"""Other trainers for hoenn.lua (docs/hoenn_multi.md):
- ASH, MISTY and BROCK cleared gyms before (the record boards),
- MAY earns the KNUCKLE BADGE right after TEST1's first badge (a notice),
- GARY waits at the quick-match desks for a battle (and says no),
- LANCE is fighting ROXANNE (WATCH LIVE),
- IRIS has lent TEST1 her team (TEAM UP)."""
import os
import sqlite3
import struct
import threading
import time

CHARS = {c: 0xBB + i for i, c in enumerate("ABCDEFGHIJKLMNOPQRSTUVWXYZ")}


def game_name(name):
    return bytes(CHARS[c] for c in name) + b"\xff" * (7 - len(name))


def team_record(name, female, mons):
    """A struct SecretBase like Online_AddTeamRecord sends: [(species, level, moves)]."""
    rec = bytearray(160)
    rec[0] = 1
    rec[1] = 0x10 if female else 0
    rec[2:9] = game_name(name)
    party = 0x34
    for i, (species, level, moves) in enumerate(mons):
        struct.pack_into("<I", rec, party + i * 4, 1000 + i * 7)
        for j, move in enumerate(moves):
            struct.pack_into("<H", rec, party + 24 + (i * 4 + j) * 2, move)
        struct.pack_into("<H", rec, party + 72 + i * 2, species)
        rec[party + 96 + i] = level
    return bytes(rec)


def seed(port, out):
    keep = []
    for name, tid, mask, champ in (("ASH", 5001, 0xFF, 1), ("MISTY", 5002, 0x0F, 0), ("BROCK", 5003, 0x01, 0)):
        f = Fake(port, name, tid)
        f.ask(f"OEV MILESTONE {mask} {champ} 0")
        keep.append(f)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"), timeout=10)
    team = team_record("IRIS", True, [(260, 50, [57, 89, 58, 182]), (282, 50, [94, 585, 182, 0])])
    db.execute("INSERT OR REPLACE INTO hoenn_teams VALUES ('TEST1', 4097, 'IRIS', 5006, ?, ?)", (team, time.time()))
    db.commit()
    db.close()

    gary = Fake(port, "GARY", 5007)
    gary.ask("QM JOIN 0")
    keep.append(gary)

    lance = Fake(port, "LANCE", 5008)
    lance.say("SOLO ROXANNE")
    # Treecko Lv15 at 31/40 HP, 2 of 2 alive; Nosepass Lv15 at 12/41, 1 of 2.
    lance.say("BSTATE 0 252 15 31 40 2 2 299 15 12 41 1 2")
    lance.say("BTEXT 0 TREECKO used ABSORB!")
    keep.append(lance)

    may = Fake(port, "MAY", 5009)
    may.ask("OEV MILESTONE 0 0 0")
    keep.append(may)

    def after(port, out):
        # GARY says no to any challenge.
        def decline():
            for line in gary.f:
                if line.startswith("INVITE "):
                    time.sleep(0.3)
                    gary.say(f"DECLINE {line.split()[1]}")
        threading.Thread(target=decline, daemon=True).start()
        # MAY earns her badges just after TEST1's first one.
        db = sqlite3.connect(os.path.join(out, "league.sqlite3"), timeout=10)
        for _ in range(2400):
            if db.execute("SELECT 1 FROM hoenn_clears WHERE name = 'TEST1'").fetchone():
                break
            time.sleep(0.25)
        time.sleep(2)
        may.ask("OEV MILESTONE 3 0 1")
        while True:
            time.sleep(5)
            lance.say("BTEXT 0 Foe NOSEPASS used ROCK TOMB!")
            time.sleep(5)
            lance.say("BTEXT 0 TREECKO used ABSORB!")
    after.keep = keep
    return after
