#include "global.h"
#include "achievements.h"
#include "battle.h"
#include "berry.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "item.h"
#include "pokemon.h"
#include "script.h"
#include "string_util.h"
#include "task.h"
#include "world.h"
#include "constants/battle.h"

// Fishing and berry records.
//
// Fishing: every hooked Pokémon gets a size (its official Pokédex height,
// 75%–125% by personality, so the same Pokémon always measures the same).
// Landing it (catching it) counts it, and the biggest one is the record.
// Berry picking: before a tree is picked, a three-stop timing meter adds a
// berry per hit. R > RECORDS shows the player's records; online the best
// ones go to the server's leaderboards (netplay_world.c).

#define BIG_FISH_MM 1500

static EWRAM_DATA bool8 sFishBattle = FALSE;
static EWRAM_DATA u16 sHookedSpecies = 0;
static EWRAM_DATA u16 sHookedSize = 0;

static u32 SizeOf(u16 species, u32 personality)
{
    u32 heightMm = gSpeciesInfo[species].height * 100;
    u32 scale = 75 + ((personality >> 8) & 0xFF) * 50 / 255;

    return heightMm * scale / 100;
}

static void FormatSize(u8 *dest, u32 mm)
{
    if (mm >= 1000)
    {
        ConvertIntToDecimalStringN(dest, mm / 1000, STR_CONV_MODE_LEFT_ALIGN, 2);
        dest = StringAppend(dest, COMPOUND_STRING("."));
        ConvertIntToDecimalStringN(dest + StringLength(dest), (mm % 1000) / 10, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringAppend(dest, COMPOUND_STRING(" m"));
    }
    else
    {
        ConvertIntToDecimalStringN(dest, mm / 10, STR_CONV_MODE_LEFT_ALIGN, 3);
        dest = StringAppend(dest, COMPOUND_STRING("."));
        ConvertIntToDecimalStringN(dest + StringLength(dest), mm % 10, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(dest, COMPOUND_STRING(" cm"));
    }
}

// Called right after a fishing encounter's Pokémon is created.
void Records_OnFishHooked(void)
{
    struct Pokemon *mon = &gEnemyParty[0];

    sHookedSpecies = GetMonData(mon, MON_DATA_SPECIES);
    sHookedSize = SizeOf(sHookedSpecies, GetMonData(mon, MON_DATA_PERSONALITY));
    sFishBattle = TRUE;
}

void Records_OnWildBattleEnd(void)
{
    struct WorldSaveData *world = &gSaveBlock3Ptr->world;
    bool32 record;

    if (!sFishBattle)
        return;
    sFishBattle = FALSE;
    if (gBattleOutcome != B_OUTCOME_CAUGHT)
        return;
    if (world->fishLanded < 0xFFFF)
        world->fishLanded++;
    record = sHookedSize > world->bestFishSize;
    if (record)
    {
        world->bestFishSize = sHookedSize;
        world->bestFishSpecies = sHookedSpecies;
        World_SubmitScore(WORLD_BOARD_FISH, sHookedSize, sHookedSpecies);
    }
    if (sHookedSize >= BIG_FISH_MM)
        Achievements_Unlock(ACH_BIG_FISH);

    StringCopy(gStringVar1, GetSpeciesName(sHookedSpecies));
    FormatSize(gStringVar2, sHookedSize);
    StringExpandPlaceholders(gStringVar3, COMPOUND_STRING("LANDED! {STR_VAR_1}, {STR_VAR_2}"));
    World_ShowToast(gStringVar3, record ? COMPOUND_STRING("A new personal record!") : COMPOUND_STRING("Not your biggest yet."));
}

void Records_OnBerriesPicked(u32 count)
{
    struct WorldSaveData *world = &gSaveBlock3Ptr->world;

    world->berriesHarvested = min(world->berriesHarvested + count, 0xFFFF);
    if (count > world->bestBerryHarvest)
        world->bestBerryHarvest = count;
    World_SubmitScore(WORLD_BOARD_BERRIES, world->berriesHarvested, world->bestBerryHarvest);
}

// Berry picking ----------------------------------------------------------
//
// Script: `callnative BerryGame_Start, waitstate=1` before the tree is
// picked. Each hit adds a berry to the tree (up to its 31-berry limit).
// VAR_0x8005 = hits.

#define tTreeId data[0]

static void Task_BerryGame(u8 taskId)
{
    struct BerryTree *tree;
    s32 hits = TimingMeter_GetResult();

    if (hits < 0)
        return;
    tree = GetBerryTreeInfo(gTasks[taskId].tTreeId);
    tree->berryYield = min(tree->berryYield + hits, 31);
    Records_OnBerriesPicked(tree->berryYield);
    // The tree script already wrote "N <berry>" for its message; redo it.
    gSpecialVar_0x8005 = hits;
    gSpecialVar_0x8006 = tree->berryYield;
    ConvertIntToDecimalStringN(gStringVar2, tree->berryYield, STR_CONV_MODE_LEFT_ALIGN, 2);
    CopyItemNameHandlePlural(BerryTypeToItemId(tree->berry), gStringVar1, tree->berryYield);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

void BerryGame_Start(void)
{
    u8 taskId = CreateTask(Task_BerryGame, 80);

    gTasks[taskId].tTreeId = GetObjectEventBerryTreeId(gSelectedObjectEvent);
    TimingMeter_Start(3, 3, 3);
}

#undef tTreeId

// R > RECORDS ------------------------------------------------------------

enum
{
    RECORD_BEST_FISH,
    RECORD_FISH_LANDED,
    RECORD_BERRIES,
    RECORD_BEST_HARVEST,
    RECORD_CHAIN,
    RECORD_HARDCORE,
    RECORD_PHOTOS,
    RECORD_HUNTS,
    RECORD_COUNT,
};

static void GetRecordLine(u16 i, u8 *dest)
{
    const struct WorldSaveData *world = &gSaveBlock3Ptr->world;

    switch (i)
    {
    case RECORD_BEST_FISH:
        if (world->bestFishSize == 0)
        {
            StringCopy(dest, COMPOUND_STRING("Biggest fish: none yet"));
            break;
        }
        StringCopy(gStringVar1, GetSpeciesName(world->bestFishSpecies));
        FormatSize(gStringVar2, world->bestFishSize);
        StringExpandPlaceholders(dest, COMPOUND_STRING("Biggest fish: {STR_VAR_1} {STR_VAR_2}"));
        break;
    case RECORD_FISH_LANDED:
        ConvertIntToDecimalStringN(gStringVar1, world->fishLanded, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringExpandPlaceholders(dest, COMPOUND_STRING("Fish landed: {STR_VAR_1}"));
        break;
    case RECORD_BERRIES:
        ConvertIntToDecimalStringN(gStringVar1, world->berriesHarvested, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringExpandPlaceholders(dest, COMPOUND_STRING("Berries picked: {STR_VAR_1}"));
        break;
    case RECORD_BEST_HARVEST:
        ConvertIntToDecimalStringN(gStringVar1, world->bestBerryHarvest, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringExpandPlaceholders(dest, COMPOUND_STRING("Best harvest from one tree: {STR_VAR_1}"));
        break;
    case RECORD_CHAIN:
        if (world->chainCount == 0)
        {
            StringCopy(dest, COMPOUND_STRING("Shiny chain: none"));
            break;
        }
        StringCopy(gStringVar1, GetSpeciesName(world->chainSpecies));
        ConvertIntToDecimalStringN(gStringVar2, world->chainCount, STR_CONV_MODE_LEFT_ALIGN, 4);
        StringExpandPlaceholders(dest, COMPOUND_STRING("Shiny chain: {STR_VAR_1} x{STR_VAR_2}"));
        break;
    case RECORD_HARDCORE:
        ConvertIntToDecimalStringN(gStringVar1, world->hardcoreDeaths, STR_CONV_MODE_LEFT_ALIGN, 3);
        if (world->hardcore == HARDCORE_ON)
            StringExpandPlaceholders(dest, COMPOUND_STRING("Hardcore run: on, {STR_VAR_1} lost"));
        else if (world->hardcore == HARDCORE_FAILED)
            StringExpandPlaceholders(dest, COMPOUND_STRING("Hardcore run: over, {STR_VAR_1} lost"));
        else
            StringCopy(dest, COMPOUND_STRING("Hardcore run: not started"));
        break;
    case RECORD_PHOTOS:
        ConvertIntToDecimalStringN(gStringVar1, world->photosTaken, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringExpandPlaceholders(dest, COMPOUND_STRING("Photos taken: {STR_VAR_1}"));
        break;
    case RECORD_HUNTS:
        ConvertIntToDecimalStringN(gStringVar1, world->huntsEntered, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringExpandPlaceholders(dest, COMPOUND_STRING("Safari hunts entered: {STR_VAR_1}"));
        break;
    }
}

void Records_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("RECORDS"), RECORD_COUNT, GetRecordLine, NULL);
}
