#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_specials.h"
#include "fieldmap.h"
#include "netplay_ext.h"
#include "online.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/abilities.h"
#include "constants/maps.h"
#include "constants/songs.h"

// Hide and seek (server/online_hide.py): the game takes everyone to the
// arena, counts down, holds the seeker while the hiders hide, and times the
// game. The server decides who the seeker sees and who was found, and its
// clock runs the game: each status says how long is left. The lead Pokémon
// helps (sends its talents with HIDE JOIN; see HideTalents).

enum
{
    HIDE_IDLE,
    HIDE_LOBBY,   // warped to the arena; the lobby script starts once the field is free
    HIDE_WAITING, // the lobby script runs
    HIDE_PLAYING,
    HIDE_ENDED,   // the end script runs
};

#define SEEKER 0
#define HIDERS 5

// Talents, as in server/online_hide.py.
enum
{
    SEEK_NONE,
    SEEK_SHARP_EYES, // sees hiders from farther away
    SEEK_SENSE,      // HOT, WARM or COLD for the closest hider
};

enum
{
    HIDE_NONE,
    HIDE_BLEND_IN, // the seeker has to come closer to see them
};

// How long a game whose clock ran out waits for the server's word that it's
// over (if the server is never heard from again).
#define OVER_WAIT (20 * 60)

struct HideArena
{
    u16 map;
    u8 seeker[2];
    u8 hiders[HIDERS][2];
};

// In the order of server/online_hide.py's ARENAS.
static const struct HideArena sArenas[] =
{
    {MAP_PETALBURG_WOODS, {12, 9}, {{8, 33}, {10, 33}, {12, 33}, {14, 33}, {16, 33}}},
    {MAP_ROUTE117, {30, 8}, {{50, 8}, {51, 8}, {52, 8}, {53, 8}, {54, 8}}},
};

struct Hide
{
    u8 state;
    u8 role;
    u8 arena;
    u16 countdown;  // frames: the lobby
    u16 timer;      // frames left in the game
    u16 seekFrames; // the seeking part of it
    u16 hideFrames;
    u16 gameId;
    u16 overWait;
    s16 serverSeconds; // the last seconds left the server told
    struct WarpData back;
};

static EWRAM_DATA struct Hide sHide = {0};

extern const u8 Online_EventScript_HideLobby[];
extern const u8 Online_EventScript_HideEnd[];

bool32 Online_HideIsOn(void)
{
    return sHide.state != HIDE_IDLE;
}

// The server's status of this game: 0 found (TRUE if the player was), 1 how
// many still hide, 2 over; another game's status is stale.
// 3 the game id, 4 the seconds left, 5 the seeker's sense of the closest
// hider (0 none, 1 COLD, 2 WARM, 3 HOT).
static s32 GameStatus(u32 field)
{
    if (Online_StatusNum("HIDE", 3) != sHide.gameId)
        return 0;
    return Online_StatusNum("HIDE", field);
}

static void HideHudText(u8 *text)
{
    u8 *end;

    if (sHide.state == HIDE_WAITING)
    {
        end = StringCopy(text, COMPOUND_STRING("READY "));
        ConvertIntToDecimalStringN(end, (sHide.countdown + 59) / 60, STR_CONV_MODE_LEFT_ALIGN, 2);
        return;
    }
    if (sHide.role == SEEKER && sHide.timer > sHide.seekFrames)
    {
        end = StringCopy(text, COMPOUND_STRING("COUNT "));
        ConvertIntToDecimalStringN(end, (sHide.timer - sHide.seekFrames + 59) / 60, STR_CONV_MODE_LEFT_ALIGN, 2);
        return;
    }
    if (GameStatus(0) == 1)
    {
        StringCopy(text, COMPOUND_STRING("FOUND!"));
        return;
    }
    if (sHide.timer > sHide.seekFrames)
    {
        end = StringCopy(text, COMPOUND_STRING("HIDE "));
        Online_FormatTime(end, sHide.timer - sHide.seekFrames, FALSE);
    }
    else
    {
        static const u8 *const sSense[] = {NULL, COMPOUND_STRING(" COLD"), COMPOUND_STRING(" WARM"), COMPOUND_STRING(" HOT")};
        u32 sense = sHide.role == SEEKER ? GameStatus(5) : 0;

        end = StringCopy(text, COMPOUND_STRING("SEEK "));
        end = Online_FormatTime(end, sHide.timer, FALSE);
        if (sense > 0 && sense < ARRAY_COUNT(sSense))
            StringCopy(end, sSense[sense]);
    }
}

// Keeps the game's clock in step with the server's.
static void SyncTimer(void)
{
    s32 seconds;

    if (Online_StatusNum("HIDE", 3) != sHide.gameId || *Online_Status("HIDE", 4) == '\0')
        return;
    seconds = GameStatus(4);
    if (seconds == sHide.serverSeconds)
        return;
    sHide.serverSeconds = seconds;
    sHide.timer = min(seconds * 60, sHide.hideFrames + sHide.seekFrames);
}

// Called every overworld frame. TRUE if it started a script.
bool32 Online_HideUpdate(void)
{
    bool32 free = !ArePlayerFieldControlsLocked();

    switch (sHide.state)
    {
    case HIDE_LOBBY:
        if (!free)
            return FALSE;
        sHide.state = HIDE_WAITING;
        ScriptContext_SetupScript(Online_EventScript_HideLobby);
        return TRUE;
    case HIDE_PLAYING:
        SyncTimer();
        if (sHide.timer != 0)
            sHide.timer--;
        else
            sHide.overWait++;
        if (sHide.timer % 180 == 0)
            Online_PollStatusSoon();
        if (GameStatus(2) == 1 || sHide.overWait > OVER_WAIT)
        {
            sHide.state = HIDE_ENDED;
            return FALSE;
        }
        if (free)
            Online_ShowHud(HideHudText, FALSE);
        return FALSE;
    case HIDE_ENDED:
        if (!free)
            return FALSE;
        ScriptContext_SetupScript(Online_EventScript_HideEnd);
        return TRUE;
    }
    return FALSE;
}

// After HIDE JOIN: the arena is data row 0 (role, arena, lane, hide
// seconds, seek seconds, game id), VAR_0x800B the seconds until hiding starts. Warps
// there.
void Online_HideStart(void)
{
    const struct HideArena *arena;
    const u8 *spot;

    sHide.role = Online_DataNum(0, 0) == SEEKER ? SEEKER : 1;
    sHide.arena = Online_DataNum(0, 1) % ARRAY_COUNT(sArenas);
    sHide.hideFrames = Online_DataNum(0, 3) * 60;
    sHide.seekFrames = Online_DataNum(0, 4) * 60;
    sHide.countdown = max(gSpecialVar_0x800B, 3) * 60;
    sHide.gameId = Online_DataNum(0, 5);
    sHide.timer = 0;
    sHide.overWait = 0;
    sHide.serverSeconds = -1;
    sHide.state = HIDE_LOBBY;
    sHide.back = gSaveBlock1Ptr->location;
    PlayerGetDestCoords(&sHide.back.x, &sHide.back.y);
    sHide.back.x -= MAP_OFFSET;
    sHide.back.y -= MAP_OFFSET;
    Online_SetQuietRules(TRUE);
    arena = &sArenas[sHide.arena];
    spot = sHide.role == SEEKER ? arena->seeker : arena->hiders[Online_DataNum(0, 2) % HIDERS];
    SetWarpDestination(MAP_GROUP(arena->map), MAP_NUM(arena->map), WARP_ID_NONE, spot[0], spot[1]);
    DoWarp();
}

static void Task_Countdown(u8 taskId)
{
    if (sHide.countdown % 60 == 0 && sHide.countdown <= 3 * 60 && sHide.countdown != 0)
        PlaySE(SE_BALL_BOUNCE_1);
    if (sHide.countdown == 0)
    {
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    sHide.countdown--;
}

// The lobby script: counts down to the start (waitstate).
void Online_HideLobbyCountdown(void)
{
    Online_ShowHud(HideHudText, TRUE);
    CreateTask(Task_Countdown, 80);
}

// After HIDE GO: the game is on. VAR_RESULT: TRUE for the seeker, who
// counts (Online_HideSeekerCount) before the script lets them go.
void Online_HideBegin(void)
{
    sHide.timer = sHide.hideFrames + sHide.seekFrames;
    sHide.state = HIDE_PLAYING;
    gSpecialVar_Result = sHide.role == SEEKER;
}

// The seeker's count, till the game's clock says seeking starts.
static void Task_SeekerCount(u8 taskId)
{
    u32 left = sHide.timer > sHide.seekFrames ? sHide.timer - sHide.seekFrames : 0;

    if (left % 60 == 0 && left <= 3 * 60 && left != 0)
        PlaySE(SE_BALL_BOUNCE_1);
    if (left == 0 || sHide.state != HIDE_PLAYING)
    {
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    Online_ShowHud(HideHudText, TRUE);
}

void Online_HideSeekerCount(void)
{
    Online_ShowHud(HideHudText, TRUE);
    CreateTask(Task_SeekerCount, 80);
}

// The talk-to-player script: VAR_RESULT is TRUE if the player is seeking
// now, so talking to someone finds them.
void Online_HideIsSeeking(void)
{
    gSpecialVar_Result = sHide.state == HIDE_PLAYING && sHide.role == SEEKER && sHide.timer <= sHide.seekFrames;
}

// HIDE JOIN's build: what the lead Pokémon does for a seeker, and for a
// hider (from its ability, else its type), and its species.
void Online_AddHideTalents(void)
{
    struct Pokemon *mon = &gPlayerParty[GetLeadMonIndex()];
    u32 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 seek = SEEK_NONE, hide = HIDE_NONE;

    if (!GetMonData(mon, MON_DATA_IS_EGG) && species != SPECIES_NONE)
    {
        switch (GetMonAbility(mon))
        {
        case ABILITY_KEEN_EYE:
        case ABILITY_COMPOUND_EYES:
        case ABILITY_ILLUMINATE:
        case ABILITY_FRISK:
            seek = SEEK_SHARP_EYES;
            break;
        case ABILITY_FOREWARN:
        case ABILITY_ANTICIPATION:
        case ABILITY_TELEPATHY:
            seek = SEEK_SENSE;
            break;
        case ABILITY_SAND_VEIL:
        case ABILITY_SNOW_CLOAK:
        case ABILITY_ILLUSION:
        case ABILITY_INFILTRATOR:
            hide = HIDE_BLEND_IN;
            break;
        default:
            break;
        }
        if (seek == SEEK_NONE && hide == HIDE_NONE)
        {
            if (GetSpeciesType(species, 0) == TYPE_PSYCHIC || GetSpeciesType(species, 1) == TYPE_PSYCHIC)
                seek = SEEK_SENSE;
            if (GetSpeciesType(species, 0) == TYPE_GHOST || GetSpeciesType(species, 1) == TYPE_GHOST)
                hide = HIDE_BLEND_IN;
        }
    }
    else
    {
        species = SPECIES_NONE;
    }
    NpxRpc_Num(seek);
    NpxRpc_Num(hide);
    NpxRpc_Num(species);
}

// HIDE TAG's build: who the seeker talked to.
void Online_AddTalkedPlayer(void)
{
    NpxRpc_Num(gSpecialVar_0x8007);
}

// The seeker found everyone: the game is over.
void Online_HideTagged(void)
{
    if (gSpecialVar_0x800B == 0)
        sHide.timer = 1;
}

// Ends the game (over, given up or called off) and warps back.
void Online_HideEnd(void)
{
    sHide.state = HIDE_IDLE;
    Online_HideHud();
    Online_SetQuietRules(FALSE);
    SetWarpDestination(sHide.back.mapGroup, sHide.back.mapNum, WARP_ID_NONE, sHide.back.x, sHide.back.y);
    DoWarp();
}

// VAR_RESULT: TRUE if a game is on.
void Online_HideIsOnScript(void)
{
    gSpecialVar_Result = Online_HideIsOn();
}
