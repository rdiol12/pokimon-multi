#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "main.h"
#include "menu.h"
#include "netplay_ext.h"
#include "online.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "world.h"
#include "constants/maps.h"
#include "constants/songs.h"

// Mount racing (server/online_racing.py): racers who join a course within a
// few seconds of each other race it together, each from their own lane at
// the start line. The game times the ride from GO! to the finish and checks
// the gates in between; wild Pokémon and trainers leave racers alone.

enum
{
    RACE_IDLE,
    RACE_COUNTDOWN, // warped to the start; the countdown script starts once the field is free
    RACE_READY,     // the countdown script runs
    RACE_RUNNING,
    RACE_FINISHED,  // the finish script runs
};

enum
{
    GATE_X_BELOW, // x <= value on the map
    GATE_Y_BELOW, // y <= value on the map
    GATE_MAP,     // on the map at all
};

struct RaceGate
{
    u16 map;
    u8 kind;
    u8 value;
};

struct RaceCourse
{
    u16 map;
    u8 facing;
    u8 lanes[RACE_LANES][2];
    struct RaceGate gates[RACE_GATES]; // the last is the finish
};

// In the order of server/online_racing.py's COURSES.
static const struct RaceCourse sCourses[] =
{
    { // PETALBURG RUN: Route 102, east to west
        .map = MAP_ROUTE102,
        .facing = DIR_WEST,
        .lanes = {{49, 10}, {49, 11}, {47, 12}, {47, 13}},
        .gates = {
            {MAP_ROUTE102, GATE_X_BELOW, 36},
            {MAP_ROUTE102, GATE_X_BELOW, 20},
            {MAP_ROUTE102, GATE_X_BELOW, 8},
            {MAP_PETALBURG_CITY, GATE_MAP},
        },
    },
    { // SLATEPORT SPRINT: Route 110, south to north
        .map = MAP_ROUTE110,
        .facing = DIR_NORTH,
        .lanes = {{15, 98}, {17, 98}, {19, 98}, {21, 98}},
        .gates = {
            {MAP_ROUTE110, GATE_Y_BELOW, 75},
            {MAP_ROUTE110, GATE_Y_BELOW, 50},
            {MAP_ROUTE110, GATE_Y_BELOW, 25},
            {MAP_MAUVILLE_CITY, GATE_MAP},
        },
    },
    { // ASHEN DASH: Route 113, east to west
        .map = MAP_ROUTE113,
        .facing = DIR_WEST,
        .lanes = {{99, 7}, {99, 8}, {99, 9}, {99, 10}},
        .gates = {
            {MAP_ROUTE113, GATE_X_BELOW, 75},
            {MAP_ROUTE113, GATE_X_BELOW, 50},
            {MAP_ROUTE113, GATE_X_BELOW, 25},
            {MAP_FALLARBOR_TOWN, GATE_MAP},
        },
    },
};

STATIC_ASSERT(ARRAY_COUNT(sCourses) == RACE_COURSES, RaceCourseCount);

struct Race
{
    u8 state;
    u8 course;
    u8 gate;
    u16 countdown; // frames
    u16 frames;
    struct WarpData back;
};

static EWRAM_DATA struct Race sRace = {0};

#define RACE_PASS_LOCALID_FIRST 200
#define RACE_PASS_LOCALID_LAST  250

extern const u8 Online_EventScript_RaceCountdown[];
extern const u8 Online_EventScript_RaceFinish[];

static bool32 OnMap(u16 map)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(map) && gSaveBlock1Ptr->location.mapNum == MAP_NUM(map);
}

// The engine's own switches (include/config/overworld.h and
// wild_encounter.h) leave racers and hide-and-seek players alone. EWRAM
// starts idle, so a game saved halfway is cleared on the next boot.
void Online_SetQuietRules(bool32 on)
{
    if (on)
    {
        FlagSet(OW_FLAG_NO_TRAINER_SEE);
        FlagSet(WE_FLAG_NO_ENCOUNTER);
    }
    else
    {
        FlagClear(OW_FLAG_NO_TRAINER_SEE);
        FlagClear(WE_FLAG_NO_ENCOUNTER);
    }
}

bool32 Online_QuietRulesOn(void)
{
    return FlagGet(WE_FLAG_NO_ENCOUNTER);
}

// The HUD's text.
static void RaceHudText(u8 *text)
{
    u8 *end;

    if (sRace.state == RACE_RUNNING || sRace.state == RACE_FINISHED)
    {
        end = Online_FormatTime(text, sRace.frames, TRUE);
        *end++ = CHAR_SPACE;
        end = ConvertIntToDecimalStringN(end, sRace.gate, STR_CONV_MODE_LEFT_ALIGN, 1);
        *end++ = CHAR_SLASH;
        ConvertIntToDecimalStringN(end, RACE_GATES, STR_CONV_MODE_LEFT_ALIGN, 1);
    }
    else if (sRace.countdown > 60)
    {
        end = StringCopy(text, COMPOUND_STRING("READY "));
        ConvertIntToDecimalStringN(end, (sRace.countdown + 59) / 60, STR_CONV_MODE_LEFT_ALIGN, 2);
    }
    else
    {
        StringCopy(text, COMPOUND_STRING("GO!"));
    }
}

// The race --------------------------------------------------------------------

static bool32 PassedGate(const struct RaceGate *gate)
{
    s16 x, y;

    if (!OnMap(gate->map))
        return FALSE;
    PlayerGetDestCoords(&x, &y);
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;
    switch (gate->kind)
    {
    case GATE_X_BELOW:
        return x <= gate->value;
    case GATE_Y_BELOW:
        return y <= gate->value;
    default:
        return TRUE;
    }
}

bool32 Online_RaceIsOn(void)
{
    return sRace.state != RACE_IDLE;
}

// Called every overworld frame. TRUE if it started a script.
bool32 Online_RaceUpdate(void)
{
    bool32 free = !ArePlayerFieldControlsLocked();

    switch (sRace.state)
    {
    case RACE_IDLE:
        if (FlagGet(WE_FLAG_NO_ENCOUNTER) && !Online_HideIsOn())
            Online_SetQuietRules(FALSE);
        return FALSE;
    case RACE_COUNTDOWN:
        if (!free)
            return FALSE;
        sRace.state = RACE_READY;
        ScriptContext_SetupScript(Online_EventScript_RaceCountdown);
        return TRUE;
    case RACE_READY:
        return FALSE;
    case RACE_RUNNING:
        if (sRace.frames < 0xFFFF)
            sRace.frames++;
        if (PassedGate(&sCourses[sRace.course].gates[sRace.gate]))
        {
            sRace.gate++;
            PlaySE(sRace.gate == RACE_GATES ? SE_SUCCESS : SE_PIN);
            if (sRace.gate == RACE_GATES)
                sRace.state = RACE_FINISHED;
        }
        else if (free)
        {
            Online_ShowHud(RaceHudText, FALSE);
        }
        return FALSE;
    case RACE_FINISHED:
        if (!free)
            return FALSE;
        ScriptContext_SetupScript(Online_EventScript_RaceFinish);
        return TRUE;
    }
    return FALSE;
}

// Racers ride through what the game's extra features put on the map (local
// ids 200 and up: other players and their followers, legends, ghosts...), so
// other racers and visitors can't block the course. The route's own people
// still stand in the way.
bool32 Online_RacePassesThrough(const struct ObjectEvent *mover, const struct ObjectEvent *other)
{
    return sRace.state == RACE_RUNNING && mover->isPlayer
        && other->localId >= RACE_PASS_LOCALID_FIRST && other->localId <= RACE_PASS_LOCALID_LAST;
}

// VAR_RESULT: TRUE if a race is on.
void Online_RaceIsOnScript(void)
{
    gSpecialVar_Result = Online_RaceIsOn();
}

// VAR_RESULT: TRUE if the party has a Pokémon to ride.
void Online_HasMount(void)
{
    u32 i;

    gSpecialVar_Result = FALSE;
    for (i = 0; i < gPlayerPartyCount; i++)
    {
        if (!GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0
         && Mount_IsRideable(GetMonData(&gPlayerParty[i], MON_DATA_SPECIES)))
            gSpecialVar_Result = TRUE;
    }
}

// After RACE JOIN: the course is the row picked from RACE LIST, VAR_0x800B
// the seconds until the start, data row 0 the lane. Warps to the start line.
void Online_RaceStart(void)
{
    const struct RaceCourse *course;
    u32 lane = Online_DataNum(0, 0) % RACE_LANES;

    sRace.course = Npx_ChosenNum(1) % RACE_COURSES;
    sRace.countdown = max(gSpecialVar_0x800B, 3) * 60;
    sRace.frames = 0;
    sRace.gate = 0;
    sRace.state = RACE_COUNTDOWN;
    sRace.back = gSaveBlock1Ptr->location;
    PlayerGetDestCoords(&sRace.back.x, &sRace.back.y);
    sRace.back.x -= MAP_OFFSET;
    sRace.back.y -= MAP_OFFSET;
    sRace.back.warpId = WARP_ID_NONE;
    Online_SetQuietRules(TRUE);
    course = &sCourses[sRace.course];
    SetWarpDestination(MAP_GROUP(course->map), MAP_NUM(course->map), WARP_ID_NONE,
                       course->lanes[lane][0], course->lanes[lane][1]);
    DoWarp();
}

// The countdown script: faces the course, rides, and counts down
// (waitstate).
static void Task_Countdown(u8 taskId)
{
    if (sRace.countdown % 60 == 0 && sRace.countdown <= 3 * 60 && sRace.countdown != 0)
        PlaySE(SE_BALL_BOUNCE_1);
    if (sRace.countdown == 0)
    {
        PlaySE(SE_M_ENCORE2);
        sRace.state = RACE_RUNNING;
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    sRace.countdown--;
}

void Online_RaceCountdown(void)
{
    ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], sCourses[sRace.course].facing);
    if (!Mount_IsRiding())
        Mount_Toggle();
    Online_ShowHud(RaceHudText, TRUE);
    CreateTask(Task_Countdown, 80);
}

// The finish script: VAR_0x8005 the course, VAR_0x8006 the time in frames.
void Online_RaceSetResult(void)
{
    gSpecialVar_0x8005 = sRace.course;
    gSpecialVar_0x8006 = sRace.frames;
    Online_FormatTime(gStringVar1, sRace.frames, TRUE);
}

// Ends the race (finished or given up) and warps back to where the player
// joined it.
void Online_RaceEnd(void)
{
    sRace.state = RACE_IDLE;
    Online_HideHud();
    Online_SetQuietRules(FALSE);
    SetWarpDestination(sRace.back.mapGroup, sRace.back.mapNum, WARP_ID_NONE, sRace.back.x, sRace.back.y);
    DoWarp();
}
