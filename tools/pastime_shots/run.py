#!/usr/bin/env python3
"""Headless in-game screenshots of the minigames and side activities
(docs/pastimes.md).

Usage: run.py <scenario.lua> <out dir> [frames] [players]

The same runner as tools/world_shots/run.py (Driver.scene, two players
with a server, ...), plus helpers that set up a scene by writing straight
into struct PastimeSaveData (include/pastime_save.h):
    Driver.pastime8/16/32(offset, value), Driver.pastimeRead8/32(offset),
    Driver.pastimeMenu(row).
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
import run as world_run  # noqa: E402

world_run.SYMBOLS += ["gPastimeSavePtr", "gLocalTime", "gQuizRound", "gBugContest", "gSchoolTest", "gCarnival", "gOrchestra"]
world_run.HELPERS += """
-- struct PastimeSaveData (include/pastime_save.h) in SaveBlock3.
function Driver.pastimeAddr(offset)
    return E.read32(Driver.SYM.gPastimeSavePtr) + offset
end
function Driver.pastime8(offset, v) E.write8(Driver.pastimeAddr(offset), v) end
function Driver.pastime16(offset, v)
    E.write8(Driver.pastimeAddr(offset), v & 0xFF)
    E.write8(Driver.pastimeAddr(offset) + 1, (v >> 8) & 0xFF)
end
function Driver.pastime32(offset, v) E.write32(Driver.pastimeAddr(offset), v) end
function Driver.pastimeRead8(offset) return E.read8(Driver.pastimeAddr(offset)) end
function Driver.pastimeRead32(offset) return E.read32(Driver.pastimeAddr(offset)) end

-- R menu: RIDE, ACHIEVEMENTS, RECORDS, QUESTS, LEADERBOARDS, SAFARI HUNT,
-- PHOTO, ADVENTURE, PASTIMES, CLOSE. Opens R > PASTIMES and picks row.
-- Builds with more R menu rows (other features merged in) set
-- Driver.PASTIMES_ROW.
Driver.PASTIMES_ROW = 8
function Driver.pastimeMenu(row)
    Driver.press(Driver.KEY.R, 40)
    for _ = 1, Driver.PASTIMES_ROW do Driver.press(Driver.KEY.DOWN, 8) end
    Driver.press(Driver.KEY.A, 40)
    for _ = 1, row do Driver.press(Driver.KEY.DOWN, 8) end
    Driver.press(Driver.KEY.A, 40)
end
"""

if __name__ == "__main__":
    world_run.main()
