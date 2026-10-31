#include "global.h"
#include "event_data.h"
#include "field_weather.h"
#include "overworld.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "region_map.h"
#include "rtc.h"
#include "string_util.h"
#include "strings.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/weather.h"

// Weather lab: a researcher at the Route 119 Weather Institute forecasts
// where the three roaming weather fronts (src/world_weather.c) will be over
// the next hours, and asks the player to chase them. Standing in a front's
// weather takes a reading (one an hour, up to WEATHER_LAB_MAX_READINGS
// waiting); reporting them earns Research Points, which raise the player's
// rank, and logging all four kinds of weather gives a one-off bonus.

extern const u8 Weather_EventScript_Researcher[];

static const u8 *const sKindNames[WEATHER_KIND_COUNT] =
{
    [WEATHER_KIND_RAIN]  = COMPOUND_STRING("Rain"),
    [WEATHER_KIND_STORM] = COMPOUND_STRING("Thunderstorm"),
    [WEATHER_KIND_SAND]  = COMPOUND_STRING("Sandstorm"),
    [WEATHER_KIND_SNOW]  = COMPOUND_STRING("Snow"),
};

static const u8 sKindPoints[WEATHER_KIND_COUNT] = {1, 3, 3, 3};

// The fronts, in RoamingWeather_GetFrontMap's order.
static const u8 *const sFrontNames[] =
{
    COMPOUND_STRING("RAINSTORM"), COMPOUND_STRING("SANDSTORM"), COMPOUND_STRING("SNOW SQUALL"),
};

static const u8 *const sRankNames[WEATHER_RANK_COUNT] =
{
    COMPOUND_STRING("TRAINEE"), COMPOUND_STRING("ASSISTANT"), COMPOUND_STRING("RESEARCHER"),
    COMPOUND_STRING("METEOROLOGIST"),
};

static const u16 sRankPoints[WEATHER_RANK_COUNT] = {0, 10, 30, 60};
static const u16 sRankPrizes[WEATHER_RANK_COUNT] = {ITEM_NONE, ITEM_HEAT_ROCK, ITEM_SMOOTH_ROCK, ITEM_ICY_ROCK};

#define FORECAST_STEPS 3
#define FORECAST_HOURS 2 // between the steps

// Pastime_Update calls (8 frames each) on a map before a reading is taken,
// so the map's name popup is gone and the toast can be seen.
#define READING_DELAY 40

#define sSave (gSaveBlock3Ptr->pastime)

static EWRAM_DATA u16 sReadingMap = 0;
static EWRAM_DATA u8 sReadingTimer = 0;

const struct PastimeSpot gWeatherSpots[1] =
{
    {MAP_ROUTE119_WEATHER_INSTITUTE_1F, 12, 10, OBJ_EVENT_GFX_SCIENTIST_1, MOVEMENT_TYPE_FACE_DOWN, 0, Weather_EventScript_Researcher},
};

static u16 HourNow(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days * 24 + gLocalTime.hours;
}

static s32 KindOf(u32 weather)
{
    switch (weather)
    {
    case WEATHER_RAIN:
        return WEATHER_KIND_RAIN;
    case WEATHER_RAIN_THUNDERSTORM:
        return WEATHER_KIND_STORM;
    case WEATHER_SANDSTORM:
        return WEATHER_KIND_SAND;
    case WEATHER_SNOW:
        return WEATHER_KIND_SNOW;
    }
    return -1;
}

static u32 GetRank(void)
{
    u32 rank = WEATHER_RANK_COUNT - 1;

    while (sSave.weatherPoints < sRankPoints[rank])
        rank--;
    return rank;
}

static void CopyMapName(u8 *dest, u16 map)
{
    GetMapName(dest, Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF)->regionMapSectionId, 0);
}

// Pastime_Update: takes a reading while the player stands in a front's
// weather (not the story's own weather) for a few seconds, once an hour.
void Weather_Update(void)
{
    u8 mapGroup = gSaveBlock1Ptr->location.mapGroup, mapNum = gSaveBlock1Ptr->location.mapNum;
    u16 map = (mapGroup << 8) | mapNum;
    u32 weather;
    s32 kind;
    u16 hour;
    u8 *p;

    if (!(sSave.weatherSeen & WEATHER_LAB_JOINED) || sSave.weatherReadings >= WEATHER_LAB_MAX_READINGS)
        return;
    weather = RoamingWeather_GetForMap(mapGroup, mapNum);
    kind = KindOf(weather);
    if (kind < 0 || GetSavedWeather() != weather || !IsMapTypeOutdoors(gMapHeader.mapType) || sReadingMap != map)
    {
        sReadingMap = map;
        sReadingTimer = 0;
        return;
    }
    if (sReadingTimer < READING_DELAY)
    {
        sReadingTimer++;
        return;
    }
    hour = HourNow() + 1; // + 1, so 0 means never
    if (sSave.weatherHour == hour)
        return;
    sSave.weatherHour = hour;
    sSave.weatherReadings++;
    sSave.weatherPending += sKindPoints[kind];
    sSave.weatherSeen |= 1 << kind;
    p = StringCopy(gStringVar4, sKindNames[kind]);
    p = StringCopy(p, COMPOUND_STRING(" reading taken! ("));
    p = ConvertIntToDecimalStringN(p, sSave.weatherReadings, STR_CONV_MODE_LEFT_ALIGN, 2);
    p = StringCopy(p, COMPOUND_STRING("/"));
    p = ConvertIntToDecimalStringN(p, WEATHER_LAB_MAX_READINGS, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringCopy(p, COMPOUND_STRING(")"));
    World_ShowToast(COMPOUND_STRING("WEATHER LAB"), gStringVar4);
}

// Script: VAR_RESULT = TRUE if the player already helps the lab;
// gStringVar1 = the rank.
void Weather_GetState(void)
{
    StringCopy(gStringVar1, sRankNames[GetRank()]);
    gSpecialVar_Result = (sSave.weatherSeen & WEATHER_LAB_JOINED) != 0;
}

void Weather_Join(void)
{
    sSave.weatherSeen |= WEATHER_LAB_JOINED;
}

// Script: hands in the readings. VAR_RESULT = how many (0: none);
// gStringVar1 = that, gStringVar2 = the points, gStringVar3 = the rank.
// VAR_0x8006 = TRUE if all four kinds are now logged for the first time
// (WEATHER_LAB_FULL_SET_POINTS more); VAR_0x8005 = a prize if the rank went
// up (else ITEM_NONE).
void Weather_Report(void)
{
    u32 oldRank = GetRank(), points = sSave.weatherPending;

    gSpecialVar_Result = sSave.weatherReadings;
    gSpecialVar_0x8005 = ITEM_NONE;
    gSpecialVar_0x8006 = FALSE;
    if (sSave.weatherReadings == 0)
        return;
    if (!(sSave.weatherSeen & WEATHER_LAB_BONUS) && (sSave.weatherSeen & ((1 << WEATHER_KIND_COUNT) - 1)) == (1 << WEATHER_KIND_COUNT) - 1)
    {
        sSave.weatherSeen |= WEATHER_LAB_BONUS;
        points += WEATHER_LAB_FULL_SET_POINTS;
        gSpecialVar_0x8006 = TRUE;
    }
    ConvertIntToDecimalStringN(gStringVar1, sSave.weatherReadings, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, sSave.weatherPending, STR_CONV_MODE_LEFT_ALIGN, 3);
    sSave.weatherPoints = min(sSave.weatherPoints + points, 0xFFFF);
    sSave.weatherTotal = min(sSave.weatherTotal + sSave.weatherReadings, 0xFFFF);
    sSave.weatherReadings = 0;
    sSave.weatherPending = 0;
    StringCopy(gStringVar3, sRankNames[GetRank()]);
    if (GetRank() != oldRank)
        gSpecialVar_0x8005 = sRankPrizes[GetRank()];
}

// The researcher's forecast board -----------------------------------------

static void GetForecastLine(u16 index, u8 *dest)
{
    u32 front = index / FORECAST_STEPS, step = index % FORECAST_STEPS;

    dest = StringCopy(dest, step == 0 ? sFrontNames[front] : gText_EmptyString2);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 72}"));
    if (step == 0)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Now"));
    }
    else
    {
        dest = StringCopy(dest, COMPOUND_STRING("In "));
        dest = ConvertIntToDecimalStringN(dest, step * FORECAST_HOURS, STR_CONV_MODE_LEFT_ALIGN, 1);
        dest = StringCopy(dest, COMPOUND_STRING("h"));
    }
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 112}"));
    CopyMapName(dest, RoamingWeather_GetFrontMap(front, step * FORECAST_HOURS));
}

static void GetForecastDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("Stand in a front's weather to take a reading"));
}

void Weather_ShowForecast(void)
{
    WorldBoard_Show(COMPOUND_STRING("WEATHER FORECAST"), ARRAY_COUNT(sFrontNames) * FORECAST_STEPS, GetForecastLine, GetForecastDetail);
}

// R > PASTIMES > WEATHER LAB ----------------------------------------------

enum
{
    LINE_RANK,
    LINE_READINGS,
    LINE_KINDS,
    LINE_FRONTS = LINE_KINDS + WEATHER_KIND_COUNT,
    LINE_TOTAL = LINE_FRONTS + ARRAY_COUNT(sFrontNames),
    LINE_COUNT,
};

static void GetLine(u16 index, u8 *dest)
{
    if (index == LINE_RANK)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Rank{CLEAR_TO 56}"));
        dest = StringCopy(dest, sRankNames[GetRank()]);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 152}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.weatherPoints, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringCopy(dest, COMPOUND_STRING(" pts"));
    }
    else if (index == LINE_READINGS)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Readings to report{CLEAR_TO 152}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.weatherReadings, STR_CONV_MODE_LEFT_ALIGN, 2);
        dest = StringCopy(dest, COMPOUND_STRING("/"));
        ConvertIntToDecimalStringN(dest, WEATHER_LAB_MAX_READINGS, STR_CONV_MODE_LEFT_ALIGN, 2);
    }
    else if (index < LINE_FRONTS)
    {
        u32 kind = index - LINE_KINDS;
        dest = StringCopy(dest, sKindNames[kind]);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 152}"));
        StringCopy(dest, (sSave.weatherSeen & (1 << kind)) ? COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}LOGGED") : COMPOUND_STRING("-"));
    }
    else if (index < LINE_TOTAL)
    {
        u32 front = index - LINE_FRONTS;
        dest = StringCopy(dest, sFrontNames[front]);
        dest = StringCopy(dest, COMPOUND_STRING(" now{CLEAR_TO 112}"));
        CopyMapName(dest, RoamingWeather_GetFrontMap(front, 0));
    }
    else
    {
        dest = StringCopy(dest, COMPOUND_STRING("Readings reported{CLEAR_TO 152}"));
        ConvertIntToDecimalStringN(dest, sSave.weatherTotal, STR_CONV_MODE_LEFT_ALIGN, 5);
    }
}

static void GetDetail(u16 index, u8 *dest)
{
    if (!(sSave.weatherSeen & WEATHER_LAB_JOINED))
        StringCopy(dest, COMPOUND_STRING("Visit the WEATHER INSTITUTE on Route 119"));
    else if (index >= LINE_KINDS && index < LINE_FRONTS)
        StringCopy(dest, COMPOUND_STRING("Log all four kinds for a bonus!"));
    else
        StringCopy(dest, COMPOUND_STRING("Report readings at the WEATHER INSTITUTE"));
}

void Weather_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("WEATHER LAB"), LINE_COUNT, GetLine, GetDetail);
}
