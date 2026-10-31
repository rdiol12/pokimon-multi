#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "pokemon.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/species.h"

// Alpha taming: ALPHA_COUNT alphas, Pokémon far bigger and stronger than
// the rest of their kind, stand on routes around Hoenn. Each one can be met
// once a day. Talking to one, the player can try to calm it: a timing meter
// of ALPHA_CALM_ROUNDS stops, all of them good to tame it, and then it joins
// the player (with ALPHA_PERFECT_IVS perfect IVs). Miss one and it attacks:
// a wild battle with the same alpha (it can still be caught). Either way it
// leaves until the next day. R > PASTIMES > ALPHAS lists them.

extern const u8 Alpha_EventScript_Alpha[];

struct Alpha
{
    u16 species;
    u8 level;
    const u8 *route;
};

static const struct Alpha sAlphas[ALPHA_COUNT] =
{
    {SPECIES_LOUDRED,  30, COMPOUND_STRING("ROUTE 116")},
    {SPECIES_ZANGOOSE, 36, COMPOUND_STRING("ROUTE 114")},
    {SPECIES_SKARMORY, 38, COMPOUND_STRING("ROUTE 113")},
    {SPECIES_TROPIUS,  42, COMPOUND_STRING("ROUTE 119")},
    {SPECIES_ABSOL,    45, COMPOUND_STRING("ROUTE 120")},
};

#define sSave (gSaveBlock3Ptr->pastime)

// An alpha's sprite is drawn this much bigger (Q8: 0x100 is 1x).
#define ALPHA_SCALE 0xAA // 1.5x
#define MAX_MATRICES_PER_MAP 4

static bool32 IsAlphaShown(u32 alpha);
static u16 GetAlphaGraphicsId(u32 alpha);
static void OnAlphaSpawn(u32 alpha, u8 objectEventId);

#define ALPHA(map, x, y, n) {map, x, y, 0, MOVEMENT_TYPE_FACE_DOWN, n, Alpha_EventScript_Alpha, IsAlphaShown, GetAlphaGraphicsId, OnAlphaSpawn}

const struct PastimeSpot gAlphaSpots[ALPHA_COUNT] =
{
    ALPHA(MAP_ROUTE116, 43, 10, 0),
    ALPHA(MAP_ROUTE114, 32, 13, 1),
    ALPHA(MAP_ROUTE113, 62, 4, 2),
    ALPHA(MAP_ROUTE119, 27, 28, 3),
    ALPHA(MAP_ROUTE120, 19, 7, 4),
};

static EWRAM_DATA u16 sMatrixMap = 0;
static EWRAM_DATA u8 sMatricesUsed = 0;

static u32 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 DoneToday(void)
{
    return sSave.alphaDay == Today() + 1 ? sSave.alphaDoneToday : 0;
}

static void SetDoneToday(u32 alpha)
{
    if (sSave.alphaDay != Today() + 1)
    {
        sSave.alphaDay = Today() + 1;
        sSave.alphaDoneToday = 0;
    }
    sSave.alphaDoneToday |= 1 << alpha;
}

static bool32 IsAlphaShown(u32 alpha)
{
    return !(DoneToday() & (1 << alpha));
}

static u16 GetAlphaGraphicsId(u32 alpha)
{
    return sAlphas[alpha].species + OBJ_EVENT_MON;
}

// Draws the alpha bigger, with an affine matrix of its own. A matrix is
// only taken back when the map's sprites are reset (a warp or a battle),
// so only a few are used per map; past that the alpha is drawn as is. The
// alpha only faces down: a sprite flipped sideways would use the flip bits,
// which an affine sprite reads as its matrix number.
static void OnAlphaSpawn(u32 alpha, u8 objectEventId)
{
    struct Sprite *sprite = &gSprites[gObjectEvents[objectEventId].spriteId];
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u8 matrixNum;

    if (map != sMatrixMap)
    {
        sMatrixMap = map;
        sMatricesUsed = 0;
    }
    if (sMatricesUsed >= MAX_MATRICES_PER_MAP)
        return;
    matrixNum = AllocOamMatrix();
    if (matrixNum >= 8) // the flip bits would clash
    {
        if (matrixNum != 0xFF)
            FreeOamMatrix(matrixNum);
        return;
    }
    sMatricesUsed++;
    SetOamMatrix(matrixNum, ALPHA_SCALE, 0, 0, ALPHA_SCALE);
    sprite->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
    sprite->oam.matrixNum = matrixNum;
    CalcCenterToCornerVec(sprite, sprite->oam.shape, sprite->oam.size, ST_OAM_AFFINE_DOUBLE);
    // Grown from its feet, not its middle.
    sprite->y2 = -8;
}

// Scripts ------------------------------------------------------------------------------

// Script: alpha VAR_0x8004. gStringVar1 and VAR_0x8005 = its species,
// gStringVar2 and VAR_0x8006 = its level.
void Alpha_GetInfo(void)
{
    const struct Alpha *alpha = &sAlphas[gSpecialVar_0x8004];

    StringCopy(gStringVar1, GetSpeciesName(alpha->species));
    ConvertIntToDecimalStringN(gStringVar2, alpha->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_0x8005 = alpha->species;
    gSpecialVar_0x8006 = alpha->level;
}

static void Task_Calm(u8 taskId)
{
    s32 hits = TimingMeter_GetResult();

    if (hits < 0)
        return;
    gSpecialVar_Result = hits >= ALPHA_CALM_ROUNDS;
    SetDoneToday(gSpecialVar_0x8004);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Script: `callnative Alpha_Calm, waitstate=1`: the meter. VAR_RESULT =
// TRUE if alpha VAR_0x8004 is calm. Either way it's met for today.
void Alpha_Calm(void)
{
    CreateTask(Task_Calm, 80);
    TimingMeter_Start(ALPHA_CALM_ROUNDS, ALPHA_ZONE_TILES, ALPHA_SPEED);
}

// ALPHA_PERFECT_IVS different stats at 31.
static void GivePerfectIVs(struct Pokemon *mon)
{
    u32 i, stat, perfect = MAX_PER_STAT_IVS, done = 0;

    for (i = 0; i < ALPHA_PERFECT_IVS; i++)
    {
        do
            stat = Random() % NUM_STATS;
        while (done & (1 << stat));
        done |= 1 << stat;
        SetMonData(mon, MON_DATA_HP_IV + stat, &perfect);
    }
    CalculateMonStats(mon);
}

// Script: the calm alpha VAR_0x8004 joins the player. VAR_RESULT =
// MON_GIVEN_TO_PARTY, MON_GIVEN_TO_PC or MON_CANT_GIVE.
void Alpha_Tame(void)
{
    const struct Alpha *alpha = &sAlphas[gSpecialVar_0x8004];
    struct Pokemon mon;

    CreateRandomMon(&mon, alpha->species, alpha->level);
    GivePerfectIVs(&mon);
    gSpecialVar_Result = GiveScriptedMonToPlayer(&mon, PARTY_SIZE);
    if (gSpecialVar_Result == MON_CANT_GIVE)
        return;
    sSave.alphaTamedMask |= 1 << gSpecialVar_0x8004;
    if (sSave.alphaTamed < 0xFFFF)
        sSave.alphaTamed++;
}

// Script: after a `setwildbattle` (for a single battle): makes the wild
// Pokémon alpha VAR_0x8004.
void Alpha_PrepareBattle(void)
{
    const struct Alpha *alpha = &sAlphas[gSpecialVar_0x8004];

    CreateScriptedWildMon(alpha->species, alpha->level, ITEM_NONE);
    GivePerfectIVs(&gParties[B_TRAINER_OPPONENT_A][0]);
    if (sSave.alphaBattled < 0xFFFF)
        sSave.alphaBattled++;
}

// R > PASTIMES > ALPHAS ----------------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    const struct Alpha *alpha;

    if (index == ALPHA_COUNT)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Tamed{CLEAR_TO 56}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.alphaTamed, STR_CONV_MODE_LEFT_ALIGN, 5);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 96}Battled{CLEAR_TO 152}"));
        ConvertIntToDecimalStringN(dest, sSave.alphaBattled, STR_CONV_MODE_LEFT_ALIGN, 5);
        return;
    }
    alpha = &sAlphas[index];
    dest = StringCopy(dest, GetSpeciesName(alpha->species));
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 64}"));
    dest = StringCopy(dest, alpha->route);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 128}"));
    if (sSave.alphaTamedMask & (1 << index))
        StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}TAMED"));
    else if (DoneToday() & (1 << index))
        StringCopy(dest, COMPOUND_STRING("Gone today"));
    else
        StringCopy(dest, COMPOUND_STRING("{COLOR RED}{SHADOW LIGHT_RED}Roaming"));
}

static void GetDetail(u16 index, u8 *dest)
{
    if (index < ALPHA_COUNT)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Lv. "));
        dest = ConvertIntToDecimalStringN(dest, sAlphas[index].level, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringCopy(dest, COMPOUND_STRING(" - calm it to tame it"));
        return;
    }
    StringCopy(dest, COMPOUND_STRING("Alphas come back every day"));
}

void Alpha_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("ALPHAS"), ALPHA_COUNT + 1, GetLine, GetDetail);
}
