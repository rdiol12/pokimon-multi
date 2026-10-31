#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "pokemon.h"
#include "random.h"
#include "rtc.h"
#include "string_util.h"
#include "wild_encounter.h"
#include "world.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Bug-catching contest on Route 119: the officer at the north edge of the
// route's big field of tall grass hands out 20 Sport Balls and starts a 10-minute contest (overworld
// time). Meanwhile every wild Pokémon in Route 119's grass is a Bug type.
// Each Bug caught is scored by rarity, level, IVs and shininess; the best
// one counts. Back at the officer (or when time runs out and you return),
// the judges compare it with three rivals' catches. One entry a day; the
// best score goes to the server's BUG CONTEST board.

extern const u8 BugContest_EventScript_Officer[];

#define CONTEST_TICKS ((10 * 60 * 60) / 8) // Pastime_Update ticks every 8 frames
#define BALLS 20

enum { COMMON, UNCOMMON, RARE };

static const struct { u16 species; u8 rarity; } sBugs[] =
{
    {SPECIES_CATERPIE, COMMON}, {SPECIES_WEEDLE, COMMON}, {SPECIES_WURMPLE, COMMON},
    {SPECIES_LEDYBA, COMMON}, {SPECIES_SPINARAK, COMMON}, {SPECIES_KRICKETOT, COMMON},
    {SPECIES_SEWADDLE, COMMON}, {SPECIES_NINCADA, COMMON}, {SPECIES_BLIPBUG, COMMON},
    {SPECIES_VENONAT, UNCOMMON}, {SPECIES_PARAS, UNCOMMON}, {SPECIES_SURSKIT, UNCOMMON},
    {SPECIES_VOLBEAT, UNCOMMON}, {SPECIES_ILLUMISE, UNCOMMON}, {SPECIES_COMBEE, UNCOMMON},
    {SPECIES_YANMA, UNCOMMON}, {SPECIES_CUTIEFLY, UNCOMMON}, {SPECIES_NYMBLE, UNCOMMON},
    {SPECIES_SCYTHER, RARE}, {SPECIES_PINSIR, RARE}, {SPECIES_HERACROSS, RARE},
    {SPECIES_KARRABLAST, RARE}, {SPECIES_SHELMET, RARE}, {SPECIES_LARVESTA, RARE},
};

static const u8 sRarityWeights[] = {[COMMON] = 12, [UNCOMMON] = 5, [RARE] = 1};
static const u8 sRarityPoints[] = {[COMMON] = 20, [UNCOMMON] = 50, [RARE] = 100};

// By place, 1st to 4th.
static const u16 sPrizes[] = {ITEM_SUN_STONE, ITEM_EVERSTONE, ITEM_SITRUS_BERRY, ITEM_ORAN_BERRY};

static const u8 *const sRivalNames[] =
{
    COMPOUND_STRING("BUG CATCHER DON"), COMPOUND_STRING("BUG MANIAC ADA"),
    COMPOUND_STRING("SCHOOL KID PIM"), COMPOUND_STRING("CAMPER ROSS"),
    COMPOUND_STRING("PICNICKER LIV"),
};

struct BugContest
{
    bool8 active;
    bool8 timeUp;
    u16 ticksLeft;
    u16 species; // best catch
    u16 score;
    u16 wildSpecies; // the Pokémon of the battle in progress
    u8 wildLevel;
    u8 wildIvs;
    bool8 wildShiny;
};

// Not static: the screenshot driver shortens the clock.
EWRAM_DATA struct BugContest gBugContest = {0};
#define sContest gBugContest

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gBugContestSpots[1] =
{
    {MAP_ROUTE119, 29, 111, OBJ_EVENT_GFX_POLICEMAN, MOVEMENT_TYPE_FACE_DOWN, 0, BugContest_EventScript_Officer},
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 FindBug(u16 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sBugs); i++)
    {
        if (sBugs[i].species == species)
            return i;
    }
    return ARRAY_COUNT(sBugs);
}

static bool32 InContestArea(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ROUTE119)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ROUTE119);
}

// World_ModifyWildSpecies: during the contest, Route 119's grass only has Bugs.
u16 BugContest_ModifyWildSpecies(u16 species)
{
    u32 total = 0, roll, i;

    if (!sContest.active || sContest.timeUp || !InContestArea() || gIsFishingEncounter || gIsSurfingEncounter)
        return species;
    for (i = 0; i < ARRAY_COUNT(sBugs); i++)
        total += sRarityWeights[sBugs[i].rarity];
    roll = Random() % total;
    for (i = 0; i < ARRAY_COUNT(sBugs); i++)
    {
        if (roll < sRarityWeights[sBugs[i].rarity])
            return sBugs[i].species;
        roll -= sRarityWeights[sBugs[i].rarity];
    }
    return species;
}

// Remembers the wild Pokémon as the battle starts (the enemy party may be
// gone by the time the catch is reported).
void BugContest_OnBattleStart(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u32 ivs = 0, stat;

    if (!sContest.active || (gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK)))
        return;
    sContest.wildSpecies = GetMonData(mon, MON_DATA_SPECIES);
    sContest.wildLevel = GetMonData(mon, MON_DATA_LEVEL);
    for (stat = 0; stat < NUM_STATS; stat++)
        ivs += GetMonData(mon, MON_DATA_HP_IV + stat);
    sContest.wildIvs = ivs;
    sContest.wildShiny = IsMonShiny(mon);
}

void BugContest_OnCatch(u16 species)
{
    u32 bug = FindBug(species), score;

    if (!sContest.active || sContest.timeUp || bug == ARRAY_COUNT(sBugs) || species != sContest.wildSpecies)
        return;
    // Up to 100 for rarity, 3 a level, half the IV total (up to 93), 50 if shiny.
    score = sRarityPoints[sBugs[bug].rarity] + sContest.wildLevel * 3 + sContest.wildIvs / 2;
    if (sContest.wildShiny)
        score += 50;
    if (score <= sContest.score)
        return;
    sContest.score = score;
    sContest.species = species;
    ConvertIntToDecimalStringN(gStringVar1, score, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Your best catch: {STR_VAR_1} points."));
    World_ShowToast(COMPOUND_STRING("NEW BEST BUG!"), gStringVar4);
}

// Pastime_Update, every 8 frames in the overworld.
void BugContest_Update(void)
{
    if (!sContest.active || sContest.timeUp)
        return;
    if (sContest.ticksLeft != 0)
        sContest.ticksLeft--;
    if (sContest.ticksLeft == 60 * 60 / 8)
        World_ShowToast(COMPOUND_STRING("BUG CONTEST"), COMPOUND_STRING("One minute left!"));
    if (sContest.ticksLeft == 0)
    {
        sContest.timeUp = TRUE;
        World_ShowToast(COMPOUND_STRING("TIME'S UP!"), COMPOUND_STRING("Go back to the contest officer."));
    }
}

// Script: VAR_RESULT = BUG_CONTEST_OPEN, _RUNNING or _DONE_TODAY.
void BugContest_GetState(void)
{
    if (sContest.active)
        gSpecialVar_Result = BUG_CONTEST_RUNNING;
    else if (sSave.bugDay == Today() + 1) // + 1, so 0 means never
        gSpecialVar_Result = BUG_CONTEST_DONE_TODAY;
    else
        gSpecialVar_Result = BUG_CONTEST_OPEN;
    gSpecialVar_0x8005 = sContest.timeUp;
}

// Script: starts the contest and gives the Sport Balls.
void BugContest_Start(void)
{
    memset(&sContest, 0, sizeof(sContest));
    sContest.active = TRUE;
    sContest.ticksLeft = CONTEST_TICKS;
    sSave.bugDay = Today() + 1;
    if (sSave.bugEntries < 0xFFFF)
        sSave.bugEntries++;
    AddBagItem(ITEM_SPORT_BALL, BALLS);
    gLastThrownBall = gBallToDisplay = ITEM_SPORT_BALL; // R throws it in battle
}

// Script: ends the contest and judges it. gPastimeText = the results;
// VAR_RESULT = the place (1 to 4), VAR_0x8005 = the prize.
void BugContest_Judge(void)
{
    u32 rivals[3], first = Random() % ARRAY_COUNT(sRivalNames), place = 1, i;
    u8 *p = gPastimeText;

    sContest.active = FALSE;
    RemoveBagItem(ITEM_SPORT_BALL, CountTotalItemQuantityInBag(ITEM_SPORT_BALL));
    for (i = 0; i < ARRAY_COUNT(rivals); i++)
    {
        rivals[i] = 90 + Random() % 120;
        if (rivals[i] >= sContest.score)
            place++;
    }
    if (sContest.score == 0)
    {
        p = StringCopy(p, COMPOUND_STRING("You didn't catch a Bug Pokémon?\nThat's too bad…"));
        place = 4;
    }
    else
    {
        p = StringCopy(p, COMPOUND_STRING("Your "));
        p = StringCopy(p, GetSpeciesName(sContest.species));
        p = StringCopy(p, COMPOUND_STRING(" scored "));
        p = ConvertIntToDecimalStringN(p, sContest.score, STR_CONV_MODE_LEFT_ALIGN, 3);
        p = StringCopy(p, COMPOUND_STRING(" points!"));
    }
    for (i = 0; i < ARRAY_COUNT(rivals); i++)
    {
        p = StringCopy(p, COMPOUND_STRING("\p"));
        p = StringCopy(p, sRivalNames[(first + i) % ARRAY_COUNT(sRivalNames)]);
        p = StringCopy(p, COMPOUND_STRING("\nscored "));
        p = ConvertIntToDecimalStringN(p, rivals[i], STR_CONV_MODE_LEFT_ALIGN, 3);
        p = StringCopy(p, COMPOUND_STRING(" points."));
    }
    if (sContest.score > sSave.bugBest)
    {
        sSave.bugBest = sContest.score;
        sSave.bugBestSpecies = sContest.species;
    }
    if (place == 1 && sSave.bugWins < 0xFFFF)
        sSave.bugWins++;
    gSpecialVar_Result = place;
    gSpecialVar_0x8005 = sPrizes[place - 1];
    if (sContest.score != 0)
        World_SubmitScore(WORLD_BOARD_BUG_CONTEST, sContest.score, sContest.species);
}

// R > PASTIMES > BUG CONTEST ----------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    static const u8 *const labels[] =
    {
        COMPOUND_STRING("Today's contest"),
        COMPOUND_STRING("Best catch now"),
        COMPOUND_STRING("Best ever"),
        COMPOUND_STRING("Contests won"),
        COMPOUND_STRING("Contests entered"),
    };

    dest = StringCopy(dest, labels[index]);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 112}"));
    switch (index)
    {
    case 0:
        if (sContest.active && !sContest.timeUp)
        {
            u32 seconds = sContest.ticksLeft * 8 / 60;
            dest = ConvertIntToDecimalStringN(dest, seconds / 60, STR_CONV_MODE_LEFT_ALIGN, 2);
            dest = StringCopy(dest, COMPOUND_STRING("m "));
            dest = ConvertIntToDecimalStringN(dest, seconds % 60, STR_CONV_MODE_LEFT_ALIGN, 2);
            StringCopy(dest, COMPOUND_STRING("s left"));
        }
        else if (sContest.active)
            StringCopy(dest, COMPOUND_STRING("Time's up: see the officer"));
        else
            StringCopy(dest, sSave.bugDay == Today() + 1 ? COMPOUND_STRING("Done") : COMPOUND_STRING("Open"));
        break;
    case 1:
    case 2:
    {
        u16 species = index == 1 ? sContest.species : sSave.bugBestSpecies;
        u16 score = index == 1 ? sContest.score : sSave.bugBest;
        if (score == 0)
        {
            StringCopy(dest, COMPOUND_STRING("-"));
            break;
        }
        dest = ConvertIntToDecimalStringN(dest, score, STR_CONV_MODE_LEFT_ALIGN, 3);
        *dest++ = CHAR_SPACE;
        StringCopy(dest, GetSpeciesName(species));
        break;
    }
    case 3:
        ConvertIntToDecimalStringN(dest, sSave.bugWins, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    default:
        ConvertIntToDecimalStringN(dest, sSave.bugEntries, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    }
}

static void GetDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("The officer is by the tall grass of Route 119"));
}

void BugContest_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("BUG-CATCHING CONTEST"), 5, GetLine, GetDetail);
}
