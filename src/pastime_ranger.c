#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "event_data.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "overworld.h"
#include "pokemon.h"
#include "region_map.h"
#include "rtc.h"
#include "sound.h"
#include "string_util.h"
#include "world.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/region_map_sections.h"

// Ranger missions: the RANGER in Fortree gives one mission at a time, up to
// three a day, picked from a pool by the day:
//  - RESCUE: a lost Pokémon is hiding on a route (it appears there only
//    while the mission is on); find it and talk to it.
//  - CALM: wild Pokémon are rampaging on a route; win N wild battles there.
//  - SURVEY: catch N Pokémon of a type, anywhere.
// Missions earn Ranger Points; points raise the rank, and each rank up
// comes with a prize. R > PASTIMES > RANGER shows the mission and rank.

extern const u8 Ranger_EventScript_Ranger[];
extern const u8 Ranger_EventScript_LostMon[];

struct Mission
{
    u8 kind;
    u8 target;  // RESCUE: the lost Pokémon (sLostMons), CALM: count, SURVEY: type
    u8 count;   // SURVEY: count
    u16 map;    // CALM: the route
    u8 points;
};

static const struct Mission sMissions[] =
{
    {RANGER_RESCUE, 0, 0, 0, 3},
    {RANGER_CALM, 3, 0, MAP_ROUTE119, 4},
    {RANGER_SURVEY, TYPE_WATER, 2, 0, 3},
    {RANGER_RESCUE, 1, 0, 0, 3},
    {RANGER_CALM, 3, 0, MAP_ROUTE110, 3},
    {RANGER_SURVEY, TYPE_GRASS, 2, 0, 3},
    {RANGER_RESCUE, 2, 0, 0, 3},
    {RANGER_CALM, 4, 0, MAP_ROUTE117, 4},
    {RANGER_SURVEY, TYPE_BUG, 3, 0, 3},
    {RANGER_RESCUE, 3, 0, 0, 3},
    {RANGER_CALM, 3, 0, MAP_ROUTE104, 3},
    {RANGER_SURVEY, TYPE_FLYING, 2, 0, 4},
    {RANGER_RESCUE, 4, 0, 0, 4},
    {RANGER_CALM, 4, 0, MAP_ROUTE113, 4},
    {RANGER_SURVEY, TYPE_ELECTRIC, 1, 0, 5},
};

// RESCUE missions: who's lost, where (the spots below, in the same order).
static const u16 sLostMons[RANGER_LOST_COUNT] =
{
    SPECIES_SKITTY, SPECIES_PLUSLE, SPECIES_AZURILL, SPECIES_WHISMUR, SPECIES_SPINDA,
};

static const u8 *const sRankNames[RANGER_RANK_COUNT] =
{
    COMPOUND_STRING("TRAINEE"), COMPOUND_STRING("RANGER"), COMPOUND_STRING("AREA RANGER"),
    COMPOUND_STRING("TOP RANGER"),
};

static const u16 sRankPoints[RANGER_RANK_COUNT] = {0, 10, 30, 60};
static const u16 sRankPrizes[RANGER_RANK_COUNT] = {ITEM_NONE, ITEM_ULTRA_BALL, ITEM_EXP_SHARE, ITEM_MASTER_BALL};

#define sSave (gSaveBlock3Ptr->pastime)

static bool32 IsLostMonShown(u32 lost)
{
    return sSave.rangerMission != 0 && sMissions[sSave.rangerMission - 1].kind == RANGER_RESCUE
        && sMissions[sSave.rangerMission - 1].target == lost && sSave.rangerProgress == 0;
}

static u16 GetLostMonGraphicsId(u32 lost)
{
    return sLostMons[lost] + OBJ_EVENT_MON;
}

const struct PastimeSpot gRangerSpots[1 + RANGER_LOST_COUNT] =
{
    {MAP_FORTREE_CITY, 23, 14, OBJ_EVENT_GFX_PICNICKER, MOVEMENT_TYPE_FACE_LEFT, 0, Ranger_EventScript_Ranger},
    {MAP_ROUTE104, 23, 50, 0, MOVEMENT_TYPE_LOOK_AROUND, 0, Ranger_EventScript_LostMon, IsLostMonShown, GetLostMonGraphicsId},
    {MAP_ROUTE110, 15, 91, 0, MOVEMENT_TYPE_LOOK_AROUND, 1, Ranger_EventScript_LostMon, IsLostMonShown, GetLostMonGraphicsId},
    {MAP_ROUTE117, 23, 6, 0, MOVEMENT_TYPE_LOOK_AROUND, 2, Ranger_EventScript_LostMon, IsLostMonShown, GetLostMonGraphicsId},
    {MAP_ROUTE116, 16, 17, 0, MOVEMENT_TYPE_LOOK_AROUND, 3, Ranger_EventScript_LostMon, IsLostMonShown, GetLostMonGraphicsId},
    {MAP_ROUTE113, 23, 12, 0, MOVEMENT_TYPE_LOOK_AROUND, 4, Ranger_EventScript_LostMon, IsLostMonShown, GetLostMonGraphicsId},
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 GetRank(void)
{
    u32 rank = RANGER_RANK_COUNT - 1;

    while (sSave.rangerPoints < sRankPoints[rank])
        rank--;
    return rank;
}

static u8 *AppendMapName(u8 *dest, u16 map)
{
    return GetMapName(dest, Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF)->regionMapSectionId, 0);
}

// gPastimeText = what the mission asks.
static void DescribeMission(const struct Mission *mission, bool32 briefing)
{
    u8 *p = gPastimeText;

    switch (mission->kind)
    {
    case RANGER_RESCUE:
        p = StringCopy(p, briefing ? COMPOUND_STRING("A lost ") : COMPOUND_STRING("Find the lost "));
        p = StringCopy(p, GetSpeciesName(sLostMons[mission->target]));
        p = StringCopy(p, briefing ? COMPOUND_STRING(" was seen on\n") : COMPOUND_STRING(" on\n"));
        p = AppendMapName(p, gRangerSpots[1 + mission->target].map);
        StringCopy(p, briefing ? COMPOUND_STRING(". Please find it!") : COMPOUND_STRING("."));
        break;
    case RANGER_CALM:
        p = StringCopy(p, briefing ? COMPOUND_STRING("Wild Pokémon are rampaging on\n") : COMPOUND_STRING("Calm the wild Pokémon on\n"));
        p = AppendMapName(p, mission->map);
        p = StringCopy(p, briefing ? COMPOUND_STRING("! Win ") : COMPOUND_STRING(": win "));
        p = ConvertIntToDecimalStringN(p, mission->target, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringCopy(p, COMPOUND_STRING(" battles there."));
        break;
    default:
        p = StringCopy(p, briefing ? COMPOUND_STRING("We're surveying the region.\nCatch ") : COMPOUND_STRING("Catch "));
        p = ConvertIntToDecimalStringN(p, mission->count, STR_CONV_MODE_LEFT_ALIGN, 1);
        *p++ = CHAR_SPACE;
        p = StringCopy(p, gTypesInfo[mission->target].name);
        StringCopy(p, COMPOUND_STRING("-type Pokémon."));
        break;
    }
}

static u32 MissionGoal(const struct Mission *mission)
{
    switch (mission->kind)
    {
    case RANGER_RESCUE:
        return 1;
    case RANGER_CALM:
        return mission->target;
    default:
        return mission->count;
    }
}

// Script: VAR_RESULT = RANGER_STATE_*. gPastimeText = the mission.
// gStringVar1 = the rank.
void Ranger_GetState(void)
{
    StringCopy(gStringVar1, sRankNames[GetRank()]);
    if (sSave.rangerDay != Today() + 1) // + 1, so 0 means never
    {
        sSave.rangerDay = Today() + 1;
        sSave.rangerToday = 0;
    }
    if (sSave.rangerMission != 0)
    {
        const struct Mission *mission = &sMissions[sSave.rangerMission - 1];
        DescribeMission(mission, FALSE);
        gSpecialVar_Result = sSave.rangerProgress >= MissionGoal(mission) ? RANGER_STATE_COMPLETE : RANGER_STATE_ACTIVE;
    }
    else if (sSave.rangerToday >= RANGER_MISSIONS_PER_DAY)
    {
        gSpecialVar_Result = RANGER_STATE_DONE_TODAY;
    }
    else
    {
        gSpecialVar_Result = RANGER_STATE_OPEN;
    }
}

// Script: gives today's next mission. gPastimeText = the briefing.
void Ranger_StartMission(void)
{
    u32 i = (Today() * RANGER_MISSIONS_PER_DAY + sSave.rangerToday) % ARRAY_COUNT(sMissions);

    sSave.rangerMission = i + 1;
    sSave.rangerProgress = 0;
    sSave.rangerToday++;
    DescribeMission(&sMissions[i], TRUE);
}

// Script: hands in the finished mission. gStringVar2 = points won; VAR_RESULT
// = TRUE if the rank went up, and then VAR_0x8005 = the prize and
// gStringVar1 = the new rank.
void Ranger_CompleteMission(void)
{
    u32 oldRank = GetRank(), points = sMissions[sSave.rangerMission - 1].points;

    sSave.rangerPoints = min(sSave.rangerPoints + points, 0xFFFF);
    if (sSave.rangerDone < 0xFFFF)
        sSave.rangerDone++;
    sSave.rangerMission = 0;
    sSave.rangerProgress = 0;
    ConvertIntToDecimalStringN(gStringVar2, points, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringCopy(gStringVar1, sRankNames[GetRank()]);
    gSpecialVar_Result = GetRank() != oldRank;
    gSpecialVar_0x8005 = sRankPrizes[GetRank()];
}

// Script: talked to a lost Pokémon (VAR_0x8004 = which). gStringVar1 = its name.
void Ranger_RescueLostMon(void)
{
    StringCopy(gStringVar1, GetSpeciesName(sLostMons[gSpecialVar_0x8004]));
    if (IsLostMonShown(gSpecialVar_0x8004))
        sSave.rangerProgress = 1;
}

// Script: VAR_RESULT = TRUE if the lost Pokémon (VAR_0x8004) is still lost.
void Ranger_IsLostMonWaiting(void)
{
    gSpecialVar_Result = IsLostMonShown(gSpecialVar_0x8004);
    PlayCry_Normal(sLostMons[gSpecialVar_0x8004], 0);
}

static void AddProgress(void)
{
    const struct Mission *mission = &sMissions[sSave.rangerMission - 1];

    if (sSave.rangerProgress >= MissionGoal(mission))
        return;
    sSave.rangerProgress++;
    if (sSave.rangerProgress >= MissionGoal(mission))
        World_ShowToast(COMPOUND_STRING("MISSION COMPLETE!"), COMPOUND_STRING("Report back to the RANGER in Fortree."));
    else
    {
        ConvertIntToDecimalStringN(gStringVar1, sSave.rangerProgress, STR_CONV_MODE_LEFT_ALIGN, 1);
        ConvertIntToDecimalStringN(gStringVar2, MissionGoal(mission), STR_CONV_MODE_LEFT_ALIGN, 1);
        StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("{STR_VAR_1} of {STR_VAR_2} done."));
        World_ShowToast(COMPOUND_STRING("RANGER MISSION"), gStringVar4);
    }
}

void Ranger_OnCatch(u16 species)
{
    const struct Mission *mission;

    if (sSave.rangerMission == 0)
        return;
    mission = &sMissions[sSave.rangerMission - 1];
    if (mission->kind == RANGER_SURVEY
     && (gSpeciesInfo[species].types[0] == mission->target || gSpeciesInfo[species].types[1] == mission->target))
        AddProgress();
}

void Ranger_OnWildBattleEnd(void)
{
    const struct Mission *mission;

    if (sSave.rangerMission == 0 || gBattleOutcome != B_OUTCOME_WON)
        return;
    mission = &sMissions[sSave.rangerMission - 1];
    if (mission->kind == RANGER_CALM
     && gSaveBlock1Ptr->location.mapGroup == (mission->map >> 8)
     && gSaveBlock1Ptr->location.mapNum == (mission->map & 0xFF))
        AddProgress();
}

// R > PASTIMES > RANGER ---------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    static const u8 *const labels[] =
    {
        COMPOUND_STRING("Rank"),
        COMPOUND_STRING("Ranger Points"),
        COMPOUND_STRING("Missions done"),
        COMPOUND_STRING("Today's missions"),
        COMPOUND_STRING("Mission"),
    };

    dest = StringCopy(dest, labels[index]);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 112}"));
    switch (index)
    {
    case 0:
        StringCopy(dest, sRankNames[GetRank()]);
        break;
    case 1:
        ConvertIntToDecimalStringN(dest, sSave.rangerPoints, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    case 2:
        ConvertIntToDecimalStringN(dest, sSave.rangerDone, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    case 3:
        dest = ConvertIntToDecimalStringN(dest, sSave.rangerDay == Today() + 1 ? sSave.rangerToday : 0, STR_CONV_MODE_LEFT_ALIGN, 1);
        dest = StringCopy(dest, COMPOUND_STRING(" of "));
        ConvertIntToDecimalStringN(dest, RANGER_MISSIONS_PER_DAY, STR_CONV_MODE_LEFT_ALIGN, 1);
        break;
    default:
        if (sSave.rangerMission == 0)
        {
            StringCopy(dest, COMPOUND_STRING("None"));
        }
        else
        {
            const struct Mission *mission = &sMissions[sSave.rangerMission - 1];
            dest = ConvertIntToDecimalStringN(dest, sSave.rangerProgress, STR_CONV_MODE_LEFT_ALIGN, 1);
            dest = StringCopy(dest, COMPOUND_STRING(" of "));
            ConvertIntToDecimalStringN(dest, MissionGoal(mission), STR_CONV_MODE_LEFT_ALIGN, 1);
        }
        break;
    }
}

static void GetDetail(u16 index, u8 *dest)
{
    u32 rank = GetRank();

    if (index == 4 && sSave.rangerMission != 0)
    {
        u8 *p;
        DescribeMission(&sMissions[sSave.rangerMission - 1], FALSE);
        for (p = gPastimeText; *p != EOS; p++)
        {
            if (*p == CHAR_NEWLINE)
                *p = CHAR_SPACE;
        }
        StringCopyN(dest, gPastimeText, 60);
        dest[60] = EOS;
    }
    else if (rank + 1 < RANGER_RANK_COUNT)
    {
        dest = StringCopy(dest, sRankNames[rank + 1]);
        dest = StringCopy(dest, COMPOUND_STRING(" at "));
        dest = ConvertIntToDecimalStringN(dest, sRankPoints[rank + 1], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringCopy(dest, COMPOUND_STRING(" points"));
    }
    else
    {
        StringCopy(dest, COMPOUND_STRING("The RANGER is in Fortree City"));
    }
}

void Ranger_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("RANGER MISSIONS"), 5, GetLine, GetDetail);
}
