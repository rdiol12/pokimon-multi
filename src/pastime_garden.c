#include "global.h"
#include "berry.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "item.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "rtc.h"
#include "string_util.h"
#include "world.h"
#include "constants/berry.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Community garden: eight Berry plots at the north end of Route 104 (berry
// trees GARDEN_TREE_FIRST on, spawned from code), tended by the GARDENER,
// who gives starter Berries. Berries planted side by side can mutate into
// new ones (OW_BERRY_MUTATIONS, e.g. Oran + Pecha = Qualot). Each day the
// GARDEN SHOW asks for some Berries, often a mutated kind; bringing them
// earns Garden Points, which raise the player's gardener rank.

extern const u8 Garden_EventScript_Gardener[];
extern const u8 BerryTreeScript[];

static const struct { u16 item; u8 count; u8 points; } sShows[] =
{
    {ITEM_QUALOT_BERRY, 1, 5},
    {ITEM_ORAN_BERRY, 10, 2},
    {ITEM_HONDEW_BERRY, 1, 5},
    {ITEM_PECHA_BERRY, 8, 2},
    {ITEM_TAMATO_BERRY, 1, 6},
    {ITEM_SITRUS_BERRY, 3, 3},
    {ITEM_KELPSY_BERRY, 1, 5},
    {ITEM_LEPPA_BERRY, 5, 2},
    {ITEM_POMEG_BERRY, 1, 6},
    {ITEM_LUM_BERRY, 2, 3},
    {ITEM_GREPA_BERRY, 1, 6},
    {ITEM_PERSIM_BERRY, 5, 2},
};

static const u8 *const sRankNames[GARDEN_RANK_COUNT] =
{
    COMPOUND_STRING("SPROUT"), COMPOUND_STRING("BLOOM"), COMPOUND_STRING("GREEN THUMB"),
    COMPOUND_STRING("MASTER GARDENER"),
};

static const u16 sRankPoints[GARDEN_RANK_COUNT] = {0, 10, 30, 60};
static const u16 sRankPrizes[GARDEN_RANK_COUNT] = {ITEM_NONE, ITEM_MIRACLE_SEED, ITEM_BIG_ROOT, ITEM_LEAF_STONE};

static const u8 *const sStageNames[] =
{
    [BERRY_STAGE_NO_BERRY]  = COMPOUND_STRING("Empty soil"),
    [BERRY_STAGE_PLANTED]   = COMPOUND_STRING("Planted"),
    [BERRY_STAGE_SPROUTED]  = COMPOUND_STRING("Sprouted"),
    [BERRY_STAGE_TALLER]    = COMPOUND_STRING("Growing"),
    [BERRY_STAGE_FLOWERING] = COMPOUND_STRING("Flowering"),
    [BERRY_STAGE_BERRIES]   = COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Ripe!"),
    [BERRY_STAGE_TRUNK]     = COMPOUND_STRING("Growing"),
    [BERRY_STAGE_BUDDING]   = COMPOUND_STRING("Budding"),
};

#define sSave (gSaveBlock3Ptr->pastime)

static void SetUpPlot(u32 plot, u8 objectEventId)
{
    gObjectEvents[objectEventId].trainerRange_berryTreeId = GARDEN_TREE_FIRST + plot;
}

#define PLOT(x, y, n) {MAP_ROUTE104, x, y, OBJ_EVENT_GFX_BERRY_TREE, MOVEMENT_TYPE_BERRY_TREE_GROWTH, n, BerryTreeScript, NULL, NULL, SetUpPlot}

const struct PastimeSpot gGardenSpots[1 + GARDEN_PLOTS] =
{
    {MAP_ROUTE104, 27, 2, OBJ_EVENT_GFX_WOMAN_2, MOVEMENT_TYPE_FACE_LEFT, 0, Garden_EventScript_Gardener},
    PLOT(22, 1, 0), PLOT(23, 1, 1), PLOT(24, 1, 2), PLOT(25, 1, 3),
    PLOT(22, 3, 4), PLOT(23, 3, 5), PLOT(24, 3, 6), PLOT(25, 3, 7),
};

// The plots' soft soil (the metatile under Route 104's own berry trees).
#define METATILE_SOIL 0x14C

// Pastime_Update: lays the plots' soil, which the map itself doesn't have.
void Garden_Update(void)
{
    u32 i;

    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_ROUTE104) || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_ROUTE104))
        return;
    for (i = 1; i <= GARDEN_PLOTS; i++)
    {
        s32 x = gGardenSpots[i].x + MAP_OFFSET, y = gGardenSpots[i].y + MAP_OFFSET;
        if ((MapGridGetMetatileIdAt(x, y) & MAPGRID_METATILE_ID_MASK) == METATILE_SOIL)
            continue;
        MapGridSetMetatileIdAt(x, y, METATILE_SOIL);
        CurrentMapDrawMetatileAt(x, y);
    }
}

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 GetRank(void)
{
    u32 rank = GARDEN_RANK_COUNT - 1;

    while (sSave.gardenPoints < sRankPoints[rank])
        rank--;
    return rank;
}

static u32 TodaysShow(void)
{
    return (Today() * 5) % ARRAY_COUNT(sShows);
}

// Script: gStringVar1 = the rank, gStringVar2 = today's show ("2 SITRUS
// BERRIES"). VAR_RESULT = FALSE if the starter Berries are still to give.
void Garden_GetShow(void)
{
    u32 show = TodaysShow();
    u8 *p;

    StringCopy(gStringVar1, sRankNames[GetRank()]);
    p = ConvertIntToDecimalStringN(gStringVar2, sShows[show].count, STR_CONV_MODE_LEFT_ALIGN, 2);
    *p++ = CHAR_SPACE;
    CopyItemNameHandlePlural(sShows[show].item, p, sShows[show].count);
    gSpecialVar_Result = sSave.gardenGift;
}

// Script: the starter Berries (VAR_0x8005, VAR_0x8006 = item, count).
void Garden_GiveStarterBerries(void)
{
    sSave.gardenGift = TRUE;
}

// Script: VAR_RESULT = GARDEN_SHOW_OK (the Berries are handed in), _DONE
// (already today) or _MISSING. gStringVar3 = points won; VAR_0x8005 = a
// prize if the rank went up (else ITEM_NONE).
void Garden_CheckShow(void)
{
    u32 show = TodaysShow(), oldRank = GetRank();

    gSpecialVar_0x8005 = ITEM_NONE;
    if (sSave.gardenDay == Today() + 1) // + 1, so 0 means never
    {
        gSpecialVar_Result = GARDEN_SHOW_DONE;
        return;
    }
    if (!CheckBagHasItem(sShows[show].item, sShows[show].count))
    {
        gSpecialVar_Result = GARDEN_SHOW_MISSING;
        return;
    }
    RemoveBagItem(sShows[show].item, sShows[show].count);
    sSave.gardenDay = Today() + 1;
    sSave.gardenPoints = min(sSave.gardenPoints + sShows[show].points, 0xFFFF);
    if (sSave.gardenShows < 0xFFFF)
        sSave.gardenShows++;
    ConvertIntToDecimalStringN(gStringVar3, sShows[show].points, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringCopy(gStringVar1, sRankNames[GetRank()]);
    if (GetRank() != oldRank)
        gSpecialVar_0x8005 = sRankPrizes[GetRank()];
    gSpecialVar_Result = GARDEN_SHOW_OK;
}

// R > PASTIMES > GARDEN ---------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < GARDEN_PLOTS)
    {
        u32 tree = GARDEN_TREE_FIRST + index, stage = GetStageByBerryTreeId(tree);

        dest = StringCopy(dest, COMPOUND_STRING("Plot "));
        dest = ConvertIntToDecimalStringN(dest, index + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 40}"));
        if (stage != BERRY_STAGE_NO_BERRY)
        {
            CopyItemName(BerryTypeToItemId(GetBerryTypeByBerryTreeId(tree)), dest);
            dest += StringLength(dest);
        }
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 136}"));
        StringCopy(dest, stage < ARRAY_COUNT(sStageNames) ? sStageNames[stage] : sStageNames[BERRY_STAGE_TALLER]);
        return;
    }
    if (index == GARDEN_PLOTS)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Rank{CLEAR_TO 40}"));
        dest = StringCopy(dest, sRankNames[GetRank()]);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 136}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.gardenPoints, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringCopy(dest, COMPOUND_STRING(" pts"));
        return;
    }
    Garden_GetShow();
    dest = StringCopy(dest, COMPOUND_STRING("Show{CLEAR_TO 40}"));
    dest = StringCopy(dest, gStringVar2);
    if (sSave.gardenDay == Today() + 1)
        StringCopy(dest, COMPOUND_STRING(" (done)"));
}

static void GetDetail(u16 index, u8 *dest)
{
    if (index < GARDEN_PLOTS)
        StringCopy(dest, COMPOUND_STRING("Side-by-side Berries may mutate!"));
    else
        StringCopy(dest, COMPOUND_STRING("The garden is at Route 104's north end"));
}

void Garden_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("COMMUNITY GARDEN"), GARDEN_PLOTS + 2, GetLine, GetDetail);
}
