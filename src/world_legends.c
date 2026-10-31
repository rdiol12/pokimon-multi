#include "global.h"
#include "achievements.h"
#include "battle.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "rtc.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "world.h"
#include "constants/battle.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/map_groups.h"
#include "constants/weather.h"

// Legendary quests: official legendaries, at the levels they are met in
// the main series, with their own level-up moves.
//
// 1. Someone tells you the legend (the hint NPC).
// 2. The legendary shows up at the first spot, but only in its weather or at
//    night. Walk up to it and it runs off toward the second spot.
// 3. At the second spot (same condition) it waits. Catch it to finish the
//    quest; if it faints or you run, it comes back.
//
// Stages are kept in gSaveBlock3Ptr->world.legendStage. R > QUESTS lists
// the quests the player has heard of.

extern const u8 World_EventScript_LegendFlee[];
extern const u8 World_EventScript_LegendBattle[];

enum
{
    STAGE_UNKNOWN,
    STAGE_HEARD,
    STAGE_FOLLOWING,
    STAGE_DONE,
};

enum
{
    WHEN_THUNDER, // the center of the rain front
    WHEN_RAIN,    // rain or thunder
    WHEN_NIGHT,
};

struct LegendSpot
{
    u16 map; // MAP_* (group << 8 | num)
    s16 x, y;
};

struct LegendQuest
{
    u16 species;
    u8 level;
    u8 hint;
    u8 when;
    struct LegendSpot spots[2];
    const u8 *where;    // where it is now, for R > QUESTS
    const u8 *fled;     // shown when it runs from the first spot
};

static const struct LegendQuest sQuests[LEGEND_COUNT] =
{
    [LEGEND_RAIKOU] =
    {
        .species = SPECIES_RAIKOU, .level = 40, .hint = LEGEND_HINT_STORMS, .when = WHEN_THUNDER,
        .spots = {{MAP_ROUTE110, 14, 94}, {MAP_ROUTE118, 62, 10}},
        .where = COMPOUND_STRING("Seen in thunderstorms on ROUTE 110."),
        .fled = COMPOUND_STRING("RAIKOU leapt away with a crack\nof thunder… toward ROUTE 118!"),
    },
    [LEGEND_SUICUNE] =
    {
        .species = SPECIES_SUICUNE, .level = 40, .hint = LEGEND_HINT_STORMS, .when = WHEN_RAIN,
        .spots = {{MAP_ROUTE120, 5, 26}, {MAP_ROUTE119, 30, 17}},
        .where = COMPOUND_STRING("Seen in the rain on ROUTE 120."),
        .fled = COMPOUND_STRING("SUICUNE ran off like the north\nwind… into the rain of ROUTE 119!"),
    },
    [LEGEND_ENTEI] =
    {
        .species = SPECIES_ENTEI, .level = 40, .hint = LEGEND_HINT_STARS, .when = WHEN_NIGHT,
        .spots = {{MAP_ROUTE113, 9, 10}, {MAP_ROUTE113, 40, 4}},
        .where = COMPOUND_STRING("Seen at night on ROUTE 113."),
        .fled = COMPOUND_STRING("ENTEI vanished in a burst of\nflame…\pThe ash is still warm, farther\nalong ROUTE 113."),
    },
    [LEGEND_CRESSELIA] =
    {
        .species = SPECIES_CRESSELIA, .level = 50, .hint = LEGEND_HINT_MOON, .when = WHEN_NIGHT,
        .spots = {{MAP_ROUTE121, 12, 9}, {MAP_ROUTE121, 55, 13}},
        .where = COMPOUND_STRING("Seen at night on ROUTE 121."),
        .fled = COMPOUND_STRING("CRESSELIA drifted away like\nmoonlight… toward LILYCOVE!"),
    },
};

static const u8 *const sHints[] =
{
    [LEGEND_HINT_STORMS] = COMPOUND_STRING(
        "One more thing… When the big\n"
        "storm rolls through, a yellow\l"
        "beast runs with the lightning.\p"
        "I saw it on ROUTE 110, right in\n"
        "the thunder!\p"
        "And in the rain on ROUTE 120,\n"
        "a blue one, fast as the north\l"
        "wind…"),
    [LEGEND_HINT_STARS] = COMPOUND_STRING(
        "Some nights, a falling star\n"
        "lands on ROUTE 113, and the\l"
        "ash catches fire.\p"
        "The old folks say it's no star,\n"
        "but ENTEI, born of a volcano."),
    [LEGEND_HINT_MOON] = COMPOUND_STRING(
        "Hehe… A tip, free of charge.\p"
        "At night on ROUTE 121, a Pokémon\n"
        "made of moonlight comes down.\p"
        "Sweet dreams come to those\n"
        "who see it."),
};

// Which quest is standing on this map, and where. -1 if none.
static EWRAM_DATA s8 sShownQuest = 0;
static EWRAM_DATA u8 sShownStage = 0;
static EWRAM_DATA u8 sTimer = 0;

// What the player sees counts, so map weather (Route 119's endless rain,
// Route 120's rain zones) works as well as the roaming fronts.
static bool32 IsWhen(u8 when)
{
    u32 weather = GetCurrentWeather();

    switch (when)
    {
    case WHEN_THUNDER:
        return weather == WEATHER_RAIN_THUNDERSTORM;
    case WHEN_RAIN:
        return weather == WEATHER_RAIN || weather == WEATHER_RAIN_THUNDERSTORM || weather == WEATHER_DOWNPOUR;
    case WHEN_NIGHT:
        return GetTimeOfDay() == TIME_NIGHT;
    }
    return FALSE;
}

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

// The quest that should be standing on the current map right now, or -1.
static s32 WantedQuest(void)
{
    u32 i, stage;
    u16 map = CurrentMap();

    for (i = 0; i < LEGEND_COUNT; i++)
    {
        stage = gSaveBlock3Ptr->world.legendStage[i];
        if (stage != STAGE_HEARD && stage != STAGE_FOLLOWING)
            continue;
        if (sQuests[i].spots[stage - STAGE_HEARD].map == map && IsWhen(sQuests[i].when))
            return i;
    }
    return -1;
}

static bool32 IsShown(void)
{
    return GetObjectEventIdByLocalIdAndMap(LOCALID_WORLD_LEGEND, gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup) != OBJECT_EVENTS_COUNT;
}

static void Hide(void)
{
    RemoveObjectEventByLocalIdAndMap(LOCALID_WORLD_LEGEND, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

static void Show(u32 quest)
{
    u32 stage = gSaveBlock3Ptr->world.legendStage[quest];
    const struct LegendSpot *spot = &sQuests[quest].spots[stage - STAGE_HEARD];
    s16 x = spot->x + MAP_OFFSET, y = spot->y + MAP_OFFSET;

    SpawnSpecialObjectEventParameterized(OBJ_EVENT_MON + sQuests[quest].species, MOVEMENT_TYPE_FACE_DOWN,
                                         LOCALID_WORLD_LEGEND, x, y, MapGridGetElevationAt(x, y));
    sShownQuest = quest;
    sShownStage = stage;
}

// Called every overworld frame; checks a few times a second whether the
// legendary should appear on (or leave) the current map.
void Legends_Update(void)
{
    s32 wanted;
    bool32 shown;

    if (++sTimer < 16 || ArePlayerFieldControlsLocked())
        return;
    sTimer = 0;
    wanted = WantedQuest();
    shown = IsShown();
    if (shown && (wanted != sShownQuest || (wanted >= 0 && gSaveBlock3Ptr->world.legendStage[wanted] != sShownStage)))
    {
        Hide();
        shown = FALSE;
    }
    if (!shown && wanted >= 0)
        Show(wanted);
}

// Called when the player presses A facing (x, y) (map coordinates with
// MAP_OFFSET). Starts the legendary's script if it is standing there.
bool32 Legends_TryInteract(s16 x, s16 y)
{
    const struct LegendSpot *spot;
    u32 stage;

    if (!IsShown())
        return FALSE;
    stage = gSaveBlock3Ptr->world.legendStage[sShownQuest];
    if (stage != STAGE_HEARD && stage != STAGE_FOLLOWING)
        return FALSE;
    spot = &sQuests[sShownQuest].spots[stage - STAGE_HEARD];
    if (spot->x + MAP_OFFSET != x || spot->y + MAP_OFFSET != y)
        return FALSE;
    gSpecialVar_0x8004 = sShownQuest;
    gSpecialVar_0x8005 = sQuests[sShownQuest].species;
    StringCopy(gStringVar1, GetSpeciesName(sQuests[sShownQuest].species));
    ScriptContext_SetupScript(stage == STAGE_HEARD ? World_EventScript_LegendFlee : World_EventScript_LegendBattle);
    return TRUE;
}

// Script: after the run-off movement. gStringVar4 = where it went.
void Legends_Flee(void)
{
    u32 quest = gSpecialVar_0x8004;

    Hide();
    gSaveBlock3Ptr->world.legendStage[quest] = STAGE_FOLLOWING;
    StringCopy(gStringVar4, sQuests[quest].fled);
}

// Script: before dowildbattle.
void Legends_SetupBattle(void)
{
    u32 quest = gSpecialVar_0x8004;

    CreateScriptedWildMon(sQuests[quest].species, sQuests[quest].level, ITEM_NONE);
}

// Script: after the battle. VAR_RESULT = TRUE if it was caught.
void Legends_AfterBattle(void)
{
    u32 i, quest = gSpecialVar_0x8004;

    gSpecialVar_Result = (gBattleOutcome == B_OUTCOME_CAUGHT);
    if (!gSpecialVar_Result)
        return;
    gSaveBlock3Ptr->world.legendStage[quest] = STAGE_DONE;
    Hide();
    Achievements_Unlock(ACH_LEGEND);
    for (i = 0; i < LEGEND_COUNT; i++)
    {
        if (gSaveBlock3Ptr->world.legendStage[i] != STAGE_DONE)
            return;
    }
    Achievements_Unlock(ACH_ALL_LEGENDS);
}

// Script: VAR_0x8004 = LEGEND_HINT_*. Starts the quests that NPC knows
// about; gStringVar4 = what they say.
void Legends_Hint(void)
{
    u32 i;

    for (i = 0; i < LEGEND_COUNT; i++)
    {
        if (sQuests[i].hint == gSpecialVar_0x8004 && gSaveBlock3Ptr->world.legendStage[i] == STAGE_UNKNOWN)
            gSaveBlock3Ptr->world.legendStage[i] = STAGE_HEARD;
    }
    StringCopy(gStringVar4, sHints[gSpecialVar_0x8004]);
}

// R > QUESTS --------------------------------------------------------------

static void GetQuestLine(u16 i, u8 *dest)
{
    u32 stage = gSaveBlock3Ptr->world.legendStage[i];

    if (stage == STAGE_UNKNOWN)
    {
        StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}??? An untold legend"));
        return;
    }
    if (stage == STAGE_DONE)
        dest = StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}"));
    dest = StringCopy(dest, GetSpeciesName(sQuests[i].species));
    if (stage == STAGE_HEARD)
        StringCopy(dest, COMPOUND_STRING(": a rumor"));
    else if (stage == STAGE_FOLLOWING)
        StringCopy(dest, COMPOUND_STRING(": on its trail"));
    else
        StringCopy(dest, COMPOUND_STRING(": caught!"));
}

static void GetQuestDetail(u16 i, u8 *dest)
{
    const struct LegendQuest *quest = &sQuests[i];

    switch (gSaveBlock3Ptr->world.legendStage[i])
    {
    case STAGE_UNKNOWN:
        StringCopy(dest, COMPOUND_STRING("Someone in HOENN knows this story."));
        break;
    case STAGE_HEARD:
        StringCopy(dest, quest->where);
        break;
    case STAGE_FOLLOWING:
        StringCopy(dest, COMPOUND_STRING("It ran off. Follow where it went."));
        break;
    default:
        StringCopy(dest, COMPOUND_STRING("The legend is yours."));
        break;
    }
}

void Legends_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("LEGENDARY QUESTS"), LEGEND_COUNT, GetQuestLine, GetQuestDetail);
}
