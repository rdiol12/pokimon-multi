#include "global.h"
#include "frontier_online.h"
#include "event_data.h"
#include "netplay.h"
#include "string_util.h"
#include "trainer_level.h"
#include "world.h"
#include "constants/battle_frontier.h"

// The online Battle Frontier (docs/adventure.md): the Battle Factory,
// Arena and Pyramid send their best streaks to the server, which ranks
// every player on the FACTORY, ARENA and PYRAMID boards; wins there give
// trainer XP online. A records keeper outside the Ranking Hall and
// R > ADVENTURE > FRONTIER RECORDS show the boards.

#define sSave (gSaveBlock3Ptr->adventure)
#define sFrontier (gSaveBlock2Ptr->frontier)

#define XP_PER_WIN 5
#define CHECK_FRAMES 60

enum {
    FACILITY_FACTORY,
    FACILITY_ARENA,
    FACILITY_PYRAMID,
};

static const u8 sBoards[FRONTIER_ONLINE_FACILITIES] =
{
    [FACILITY_FACTORY] = WORLD_BOARD_FACTORY,
    [FACILITY_ARENA]   = WORLD_BOARD_ARENA,
    [FACILITY_PYRAMID] = WORLD_BOARD_PYRAMID,
};

static EWRAM_DATA u8 sTimer = 0;
static EWRAM_DATA u16 sSubmitted[FRONTIER_ONLINE_FACILITIES] = {0};

// The best streak in any mode (level 50 or open, and singles or doubles at
// the Factory), current or record. extra: the mode it was set in.
static u32 BestStreak(u32 facility, u32 *mode)
{
    u32 lvl, battle, best = 0;

    *mode = 0;
    for (lvl = 0; lvl < FRONTIER_LVL_MODE_COUNT; lvl++)
    {
        switch (facility)
        {
        case FACILITY_FACTORY:
            for (battle = 0; battle < 2; battle++)
            {
                u32 streak = max(sFrontier.factoryWinStreaks[battle][lvl], sFrontier.factoryRecordWinStreaks[battle][lvl]);
                if (streak > best)
                {
                    best = streak;
                    *mode = lvl | (battle << 1);
                }
            }
            break;
        case FACILITY_ARENA:
            if (max(sFrontier.arenaWinStreaks[lvl], sFrontier.arenaRecordStreaks[lvl]) > best)
            {
                best = max(sFrontier.arenaWinStreaks[lvl], sFrontier.arenaRecordStreaks[lvl]);
                *mode = lvl;
            }
            break;
        case FACILITY_PYRAMID:
            if (max(sFrontier.pyramidWinStreaks[lvl], sFrontier.pyramidRecordStreaks[lvl]) > best)
            {
                best = max(sFrontier.pyramidWinStreaks[lvl], sFrontier.pyramidRecordStreaks[lvl]);
                *mode = lvl;
            }
            break;
        }
    }
    return best;
}

// The current streaks added up: when they grow, the player won there.
static u32 CurrentWins(u32 facility)
{
    u32 lvl, wins = 0;

    for (lvl = 0; lvl < FRONTIER_LVL_MODE_COUNT; lvl++)
    {
        if (facility == FACILITY_FACTORY)
            wins += sFrontier.factoryWinStreaks[0][lvl] + sFrontier.factoryWinStreaks[1][lvl];
        else if (facility == FACILITY_ARENA)
            wins += sFrontier.arenaWinStreaks[lvl];
        else
            wins += sFrontier.pyramidWinStreaks[lvl];
    }
    return wins;
}

// Called once per overworld frame; looks at the streaks once a second.
void FrontierOnline_Update(void)
{
    u32 i, best, mode, wins;

    if (++sTimer < CHECK_FRAMES)
        return;
    sTimer = 0;
    for (i = 0; i < FRONTIER_ONLINE_FACILITIES; i++)
    {
        wins = CurrentWins(i);
        // The streaks a save already had when it first got here are old news.
        if (sSave.frontierStarted && wins > sSave.frontierWins[i])
            TrainerLevel_AddXp((wins - sSave.frontierWins[i]) * XP_PER_WIN);
        sSave.frontierWins[i] = wins;

        if (!Netplay_IsOnline())
            continue;
        best = BestStreak(i, &mode);
        if (best > sSubmitted[i])
        {
            sSubmitted[i] = best;
            World_SubmitScore(sBoards[i], best, mode);
        }
    }
    sSave.frontierStarted = TRUE;
}

// Board text: "{streak} wins" or "{floors} floors", and the mode.
void FrontierOnline_FormatScore(u8 *dest, u8 board, u32 value, u32 extra)
{
    static const u8 *const sLevels[] = {COMPOUND_STRING("Lv. 50"), COMPOUND_STRING("Open")};

    ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 4);
    StringCopy(gStringVar2, sLevels[extra & 1]);
    if (board == WORLD_BOARD_FACTORY)
        StringAppend(gStringVar2, (extra & 2) ? COMPOUND_STRING(" doubles") : COMPOUND_STRING(" singles"));
    if (board == WORLD_BOARD_PYRAMID)
        StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} floors, {STR_VAR_2}"));
    else
        StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} wins, {STR_VAR_2}"));
}

// Screenshots (src/adventure.c): sets the level 50 singles streaks.
void FrontierOnline_Showcase(u32 factory, u32 arena, u32 pyramid)
{
    sFrontier.factoryRecordWinStreaks[0][FRONTIER_LVL_50] = factory;
    sFrontier.arenaRecordStreaks[FRONTIER_LVL_50] = arena;
    sFrontier.pyramidRecordStreaks[FRONTIER_LVL_50] = pyramid;
}
