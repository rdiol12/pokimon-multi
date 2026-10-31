#include "global.h"
#include "event_data.h"
#include "menu.h"
#include "pastime.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "window.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/songs.h"

// Carnival games at the Sunfair Carnival (the map is tools/areas/
// carnival.py; its hosts' scripts go to Pastime_EventScript_<game>).
// Bowling, mini golf and a soccer penalty shootout play on the timing meter
// (src/world_meter.c), with a little picture of the lane, green or goal
// drawn in a window as the game goes. The Ferris wheel lifts the camera
// over the carnival.

// The picture window: 10 x 7 tiles in the top right, clear of the
// multichoice menus on the left and the meter in the middle.
#define COURT_X 19
#define COURT_Y 1
#define COURT_W 10
#define COURT_H 7
#define COURT_BASE_BLOCK 0xD0 // as Pastime_ShowMonPic's box

#define C_WHITE      TEXT_COLOR_WHITE
#define C_DARK       TEXT_COLOR_DARK_GRAY
#define C_LIGHT      TEXT_COLOR_LIGHT_GRAY
#define C_RED        TEXT_COLOR_RED
#define C_GREEN      TEXT_COLOR_GREEN
#define C_LIGHTGREEN TEXT_COLOR_LIGHT_GREEN
#define C_BLUE       TEXT_COLOR_BLUE
#define C_LIGHTBLUE  TEXT_COLOR_LIGHT_BLUE

enum { SHOT_PERFECT, SHOT_GOOD, SHOT_CLOSE, SHOT_MISS };

struct Carnival
{
    u8 round;      // bowling frame, golf hole, soccer kick
    u8 roll;       // bowling roll, golf stroke, soccer: 0 shooting, 1 keeping
    u8 score;      // bowling score, golf strokes, soccer player goals
    u8 cpuScore;   // soccer
    u16 pins;      // bowling: a bit per standing pin
    u16 remaining; // golf: yards to the cup
    u8 ballX, ballY;
    u8 keeperX;
    bool8 holed;
};

// Not static: the screenshot driver reads it.
EWRAM_DATA struct Carnival gCarnival = {0};
#define sGame gCarnival

static EWRAM_DATA u8 sCourtWindowId = 0;
static EWRAM_DATA bool8 sCourtShown = FALSE;

#define sSave (gSaveBlock3Ptr->pastime)

// The picture window ---------------------------------------------------------

static void Rect(u32 color, u32 x, u32 y, u32 w, u32 h)
{
    FillWindowPixelRect(sCourtWindowId, PIXEL_FILL(color), x, y, w, h);
}

static void OpenCourt(void)
{
    struct WindowTemplate template;

    if (sCourtShown)
        return;
    template = CreateWindowTemplate(0, COURT_X, COURT_Y, COURT_W, COURT_H, 15, COURT_BASE_BLOCK);
    sCourtWindowId = AddWindow(&template);
    PutWindowTilemap(sCourtWindowId);
    SetStandardWindowBorderStyle(sCourtWindowId, TRUE);
    sCourtShown = TRUE;
}

static void ShowCourt(void)
{
    CopyWindowToVram(sCourtWindowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

void Carnival_CloseCourt(void)
{
    if (!sCourtShown)
        return;
    ClearStdWindowAndFrameToTransparent(sCourtWindowId, TRUE);
    RemoveWindow(sCourtWindowId);
    sCourtShown = FALSE;
}

// How well the needle was stopped, from the meter's last round.
static u32 ShotQuality(void)
{
    s32 offset = TimingMeter_GetLastOffset();

    if (offset < 0)
        return SHOT_MISS;
    if (offset <= CARNIVAL_PERFECT_PIXELS)
        return SHOT_PERFECT;
    if (TimingMeter_GetResult() > 0)
        return SHOT_GOOD;
    if (offset <= CARNIVAL_CLOSE_PIXELS)
        return SHOT_CLOSE;
    return SHOT_MISS;
}

static void StartMeter(TaskFunc func, u32 zoneTiles, u32 speed)
{
    CreateTask(func, 80);
    TimingMeter_Start(1, zoneTiles, speed);
}

static void WriteNumber(u8 *dest, u32 n)
{
    ConvertIntToDecimalStringN(dest, n, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Bowling ---------------------------------------------------------------------

#define ALL_PINS ((1 << BOWLING_PINS) - 1)

// Pin 0 is the head pin; rows of 1, 2, 3 and 4 going back.
static const struct { u8 x, y; } sPinSpots[BOWLING_PINS] =
{
    {39, 26}, {36, 20}, {42, 20}, {33, 14}, {39, 14}, {45, 14}, {30, 8}, {36, 8}, {42, 8}, {48, 8},
};

static void DrawLane(void)
{
    u32 i;

    Rect(C_DARK, 0, 0, COURT_W * 8, COURT_H * 8);
    Rect(C_LIGHT, 22, 0, 4, COURT_H * 8);  // gutters
    Rect(C_LIGHT, 54, 0, 4, COURT_H * 8);
    Rect(C_WHITE, 26, 0, 28, COURT_H * 8); // the lane
    for (i = 0; i < 3; i++)
        Rect(C_LIGHTBLUE, 32 + i * 8, 40, 2, 3); // aiming arrows
    for (i = 0; i < BOWLING_PINS; i++)
    {
        if (!(sGame.pins & (1 << i)))
        {
            Rect(C_LIGHT, sPinSpots[i].x, sPinSpots[i].y + 3, 3, 1); // a fallen pin's spot
            continue;
        }
        Rect(C_DARK, sPinSpots[i].x, sPinSpots[i].y, 3, 5);
        Rect(C_RED, sPinSpots[i].x, sPinSpots[i].y + 1, 3, 1);
    }
    Rect(C_BLUE, sGame.ballX, sGame.ballY, 4, 4);
    ShowCourt();
}

static u32 CountPins(u32 pins)
{
    u32 count = 0;

    for (; pins != 0; pins >>= 1)
        count += pins & 1;
    return count;
}

// Knocks over n standing pins, the front ones first.
static void KnockPins(u32 n)
{
    static const u8 rowStarts[] = {0, 1, 3, 6, BOWLING_PINS};
    u32 row, i, tries;

    for (row = 0; row < ARRAY_COUNT(rowStarts) - 1 && n > 0; row++)
    {
        for (tries = 0; tries < 8 && n > 0; tries++)
        {
            i = rowStarts[row] + Random() % (rowStarts[row + 1] - rowStarts[row]);
            if (sGame.pins & (1 << i))
            {
                sGame.pins &= ~(1 << i);
                n--;
            }
        }
        for (i = rowStarts[row]; i < rowStarts[row + 1] && n > 0; i++)
        {
            if (sGame.pins & (1 << i))
            {
                sGame.pins &= ~(1 << i);
                n--;
            }
        }
    }
}

void Bowl_Start(void)
{
    memset(&sGame, 0, sizeof(sGame));
    OpenCourt();
}

// Script: sets up the next roll and draws the lane. VAR_RESULT = FALSE once
// the game is over; gPastimeText = what to say.
void Bowl_Prepare(void)
{
    u8 *p = gPastimeText;

    if (sGame.roll == 0)
        sGame.pins = ALL_PINS;
    sGame.ballX = 38;
    sGame.ballY = 48;
    DrawLane();
    if (sGame.round >= BOWLING_FRAMES)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    p = StringCopy(p, COMPOUND_STRING("FRAME "));
    p = ConvertIntToDecimalStringN(p, sGame.round + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, sGame.roll == 0 ? COMPOUND_STRING(", first ball.\n") : COMPOUND_STRING(", second ball.\n"));
    StringCopy(p, COMPOUND_STRING("Stop the needle in the green!"));
    gSpecialVar_Result = TRUE;
}

static void Task_BowlRoll(u8 taskId)
{
    u32 standing, knocked, quality;
    u8 *p = gPastimeText;

    if (TimingMeter_GetResult() < 0)
        return;
    DestroyTask(taskId);
    quality = ShotQuality();
    standing = CountPins(sGame.pins);
    switch (quality)
    {
    case SHOT_PERFECT:
        knocked = standing;
        break;
    case SHOT_GOOD:
        knocked = sGame.roll == 0 ? 7 + Random() % 3 : standing;
        break;
    case SHOT_CLOSE:
        knocked = sGame.roll == 0 ? 4 + Random() % 3 : standing / 2;
        break;
    default:
        knocked = Random() % 3;
        break;
    }
    knocked = min(knocked, standing);
    KnockPins(knocked);
    sGame.ballX = 37 + Random() % 4;
    sGame.ballY = 2;
    sGame.score += knocked;
    if (sGame.pins == 0 && sGame.roll == 0)
    {
        sGame.score += BOWLING_STRIKE_BONUS;
        StringCopy(p, COMPOUND_STRING("STRIKE!! All ten pins!"));
        PlaySE(SE_BANG);
        sGame.round++;
    }
    else if (sGame.pins == 0)
    {
        sGame.score += BOWLING_SPARE_BONUS;
        StringCopy(p, COMPOUND_STRING("SPARE! Not a pin left!"));
        PlaySE(SE_BANG);
        sGame.roll = 0;
        sGame.round++;
    }
    else
    {
        if (knocked == 0)
        {
            p = StringCopy(p, COMPOUND_STRING("Into the gutter…"));
        }
        else
        {
            p = ConvertIntToDecimalStringN(p, knocked, STR_CONV_MODE_LEFT_ALIGN, 2);
            p = StringCopy(p, knocked == 1 ? COMPOUND_STRING(" pin down.") : COMPOUND_STRING(" pins down."));
            PlaySE(SE_BALL_BOUNCE_1);
        }
        if (sGame.roll == 0)
        {
            sGame.roll = 1;
        }
        else
        {
            sGame.roll = 0;
            sGame.round++;
        }
    }
    DrawLane();
    ScriptContext_Enable();
}

// Script: `callnative Bowl_Roll, waitstate=1`. gPastimeText = how it went.
void Bowl_Roll(void)
{
    StartMeter(Task_BowlRoll, 2, 3);
}

// Prizes: VAR_0x8005 = the item (else ITEM_NONE), only the first time for
// the bigger one (bit in carnivalPrizes).
static void GivePrize(u32 bit, bool32 great, u16 greatItem, bool32 good, u16 goodItem)
{
    gSpecialVar_0x8005 = ITEM_NONE;
    if (great && !(sSave.carnivalPrizes & bit))
    {
        sSave.carnivalPrizes |= bit;
        gSpecialVar_0x8005 = greatItem;
    }
    else if (good || great)
    {
        gSpecialVar_0x8005 = goodItem;
    }
    if (sSave.carnivalPlays < 0xFFFF)
        sSave.carnivalPlays++;
}

// Script: the game is over. gStringVar1 = the score, gStringVar2 = the best;
// VAR_RESULT = TRUE if it's a new best; VAR_0x8005 = a prize.
void Bowl_Finish(void)
{
    gSpecialVar_Result = sGame.score > sSave.bowlBest;
    if (gSpecialVar_Result)
        sSave.bowlBest = sGame.score;
    WriteNumber(gStringVar1, sGame.score);
    WriteNumber(gStringVar2, sSave.bowlBest);
    GivePrize(1 << 0, sGame.score >= BOWLING_GREAT, ITEM_MUSCLE_BAND, sGame.score >= BOWLING_GOOD, ITEM_PROTEIN);
}

// Mini golf -------------------------------------------------------------------

static const u16 sHoleYards[GOLF_HOLES] = {120, 160, 200};

#define GREEN_TEE_X 6
#define GREEN_CUP_X 70
#define GREEN_Y     26

static void DrawGreen(void)
{
    u32 length = sHoleYards[sGame.round], x;

    Rect(C_GREEN, 0, 0, COURT_W * 8, COURT_H * 8);
    Rect(C_LIGHTGREEN, 0, GREEN_Y - 8, COURT_W * 8, 18); // the fairway
    Rect(C_LIGHTGREEN, GREEN_CUP_X - 8, GREEN_Y - 12, 18, 26); // the green
    if (sGame.round == GOLF_POND_HOLE)
        Rect(C_BLUE, 40, 36, 14, 10);
    Rect(C_DARK, GREEN_CUP_X, GREEN_Y, 3, 2); // the cup
    Rect(C_DARK, GREEN_CUP_X + 1, GREEN_Y - 12, 1, 12); // the flag
    Rect(C_RED, GREEN_CUP_X + 2, GREEN_Y - 12, 5, 4);
    x = GREEN_TEE_X + (GREEN_CUP_X - GREEN_TEE_X) * (length - sGame.remaining) / length;
    if (!sGame.holed)
    {
        Rect(C_DARK, x + 1, sGame.ballY + 1, 3, 3); // its shadow
        Rect(C_WHITE, x, sGame.ballY, 3, 3);
    }
    ShowCourt();
}

void Golf_Start(void)
{
    memset(&sGame, 0, sizeof(sGame));
    sGame.remaining = sHoleYards[0];
    sGame.ballY = GREEN_Y;
    OpenCourt();
}

// Script: the next stroke. VAR_RESULT = FALSE once all holes are played;
// gPastimeText = what to say.
void Golf_Prepare(void)
{
    u8 *p = gPastimeText;

    if (sGame.holed)
    {
        sGame.holed = FALSE;
        sGame.round++;
        sGame.roll = 0;
        if (sGame.round < GOLF_HOLES)
            sGame.remaining = sHoleYards[sGame.round];
        sGame.ballY = GREEN_Y;
    }
    if (sGame.round >= GOLF_HOLES)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    DrawGreen();
    p = StringCopy(p, COMPOUND_STRING("HOLE "));
    p = ConvertIntToDecimalStringN(p, sGame.round + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, COMPOUND_STRING(", stroke "));
    p = ConvertIntToDecimalStringN(p, sGame.roll + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, COMPOUND_STRING(". "));
    p = ConvertIntToDecimalStringN(p, sGame.remaining, STR_CONV_MODE_LEFT_ALIGN, 3);
    p = StringCopy(p, COMPOUND_STRING(" yards\n"));
    StringCopy(p, sGame.remaining <= GOLF_PUTT_YARDS ? COMPOUND_STRING("to the cup. Time to putt!") : COMPOUND_STRING("to the cup. Swing!"));
    gSpecialVar_Result = TRUE;
}

static void Task_GolfSwing(u8 taskId)
{
    static const u8 sFraction[] = {[SHOT_PERFECT] = 100, [SHOT_GOOD] = 80, [SHOT_CLOSE] = 55, [SHOT_MISS] = 25};
    u32 quality, covered;
    u8 *p = gPastimeText;
    bool32 putt = sGame.remaining <= GOLF_PUTT_YARDS;

    if (TimingMeter_GetResult() < 0)
        return;
    DestroyTask(taskId);
    quality = ShotQuality();
    sGame.roll++;
    sGame.score++;
    if (sGame.round == GOLF_POND_HOLE && !putt && quality == SHOT_MISS)
    {
        // Into the pond: a stroke's penalty, and play again from here.
        sGame.score++;
        PlaySE(SE_M_SURF);
        StringCopy(p, COMPOUND_STRING("Splash! Into the pond…\nThat's a penalty stroke."));
        DrawGreen();
        ScriptContext_Enable();
        return;
    }
    if (putt)
        covered = (quality <= SHOT_GOOD) ? sGame.remaining : sGame.remaining / 2;
    else if (quality == SHOT_PERFECT && sGame.remaining <= GOLF_ACE_YARDS)
        covered = sGame.remaining;
    else
        covered = max(sGame.remaining * sFraction[quality] / 100, 1);
    if (!putt && covered >= sGame.remaining && sGame.remaining > GOLF_ACE_YARDS)
        covered = sGame.remaining - 1 - Random() % 3; // stops just short
    sGame.remaining -= min(covered, sGame.remaining);
    sGame.ballY = GREEN_Y + (sGame.remaining == 0 ? 0 : (s32)(Random() % 7) - 3);
    if (sGame.remaining == 0 || sGame.roll >= GOLF_MAX_STROKES)
    {
        sGame.score += (sGame.remaining != 0); // picked up: one more
        sGame.remaining = 0;
        sGame.holed = TRUE;
        if (sGame.roll == 1)
            p = StringCopy(p, COMPOUND_STRING("HOLE IN ONE!!"));
        else if (sGame.roll < GOLF_PAR)
            p = StringCopy(p, COMPOUND_STRING("In the cup! Under par!"));
        else
            p = StringCopy(p, COMPOUND_STRING("In the cup!"));
        PlaySE(sGame.roll < GOLF_PAR ? SE_SUCCESS : SE_BALL_BOUNCE_1);
    }
    else
    {
        p = StringCopy(p, quality == SHOT_PERFECT ? COMPOUND_STRING("A perfect shot! ") : quality == SHOT_MISS ? COMPOUND_STRING("A poor shot… ") : COMPOUND_STRING("Nice shot! "));
        p = ConvertIntToDecimalStringN(p, sGame.remaining, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringCopy(p, COMPOUND_STRING("\nyards to go."));
        PlaySE(SE_BALL_BOUNCE_1);
    }
    DrawGreen();
    ScriptContext_Enable();
}

// Script: `callnative Golf_Swing, waitstate=1`. gPastimeText = how it went.
void Golf_Swing(void)
{
    if (sGame.remaining <= GOLF_PUTT_YARDS)
        StartMeter(Task_GolfSwing, 3, 2);
    else
        StartMeter(Task_GolfSwing, 2, 3);
}

// Script: gStringVar1 = strokes, gStringVar2 = the best round; VAR_RESULT
// = TRUE if it's a new best; VAR_0x8005 = a prize.
void Golf_Finish(void)
{
    gSpecialVar_Result = sSave.golfBest == 0 || sGame.score < sSave.golfBest;
    if (gSpecialVar_Result)
        sSave.golfBest = sGame.score;
    WriteNumber(gStringVar1, sGame.score);
    WriteNumber(gStringVar2, sSave.golfBest);
    GivePrize(1 << 1, sGame.score < GOLF_PAR * GOLF_HOLES, ITEM_SCOPE_LENS, sGame.score <= GOLF_PAR * GOLF_HOLES, ITEM_IRON);
}

// Soccer penalty shootout -------------------------------------------------------

static const u8 sGoalX[SOCCER_SIDES] = {20, 38, 56}; // left, middle, right
#define GOAL_Y 12

static void DrawGoal(bool32 ballAtSpot)
{
    u32 i;

    Rect(C_GREEN, 0, 0, COURT_W * 8, COURT_H * 8);
    for (i = 0; i < 6; i++)
        Rect(C_LIGHTGREEN, 0, 30 + i * 4, COURT_W * 8, 2); // mown stripes
    Rect(C_LIGHT, 16, 4, 48, 20); // the net
    Rect(C_WHITE, 14, 2, 52, 2);  // crossbar and posts
    Rect(C_WHITE, 14, 2, 2, 24);
    Rect(C_WHITE, 64, 2, 2, 24);
    Rect(C_WHITE, 4, 26, 72, 1);  // goal line
    Rect(C_WHITE, 39, 46, 2, 1);  // penalty spot
    Rect(C_RED, sGame.keeperX, 10, 6, 12); // the keeper
    Rect(C_DARK, sGame.keeperX + 1, 8, 4, 3);
    if (ballAtSpot)
        Rect(C_WHITE, 38, 43, 4, 4);
    else
        Rect(C_WHITE, sGame.ballX, sGame.ballY, 4, 4);
    ShowCourt();
}

void Soccer_Start(void)
{
    memset(&sGame, 0, sizeof(sGame));
    sGame.keeperX = sGoalX[1] - 1;
    OpenCourt();
}

// Script: the next kick. VAR_RESULT = SOCCER_OVER, SOCCER_SHOOT or
// SOCCER_KEEP; gPastimeText = what to say.
void Soccer_Prepare(void)
{
    u8 *p = gPastimeText;

    sGame.keeperX = sGoalX[1] - 1;
    DrawGoal(TRUE);
    if (sGame.round >= SOCCER_KICKS)
    {
        gSpecialVar_Result = SOCCER_OVER;
        return;
    }
    p = StringCopy(p, COMPOUND_STRING("KICK "));
    p = ConvertIntToDecimalStringN(p, sGame.round + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, COMPOUND_STRING(" of 5. "));
    p = ConvertIntToDecimalStringN(p, sGame.score, STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, COMPOUND_STRING(" - "));
    p = ConvertIntToDecimalStringN(p, sGame.cpuScore, STR_CONV_MODE_LEFT_ALIGN, 1);
    if (sGame.roll == 0)
    {
        StringCopy(p, COMPOUND_STRING("\nYour shot! Aim where?"));
        gSpecialVar_Result = SOCCER_SHOOT;
    }
    else
    {
        StringCopy(p, COMPOUND_STRING("\nIn goal! Dive which way?"));
        gSpecialVar_Result = SOCCER_KEEP;
    }
}

static void NextKick(void)
{
    if (sGame.roll == 0)
    {
        sGame.roll = 1;
    }
    else
    {
        sGame.roll = 0;
        sGame.round++;
    }
}

static void Task_SoccerShoot(u8 taskId)
{
    u32 quality, aim = gSpecialVar_0x8005, dive = Random() % SOCCER_SIDES;

    if (TimingMeter_GetResult() < 0)
        return;
    DestroyTask(taskId);
    quality = ShotQuality();
    sGame.keeperX = sGoalX[dive] - 1;
    sGame.ballX = sGoalX[aim];
    sGame.ballY = GOAL_Y;
    if (quality == SHOT_MISS || (quality == SHOT_CLOSE && Random() % 2))
    {
        sGame.ballY = 0; // over the bar
        StringCopy(gPastimeText, COMPOUND_STRING("Over the bar! No goal…"));
        PlaySE(SE_FAILURE);
    }
    else if (dive == aim && quality != SHOT_PERFECT)
    {
        sGame.ballX = sGoalX[aim] + 2;
        sGame.ballY = GOAL_Y + 12;
        StringCopy(gPastimeText, COMPOUND_STRING("Saved! The keeper guessed right!"));
        PlaySE(SE_FAILURE);
    }
    else
    {
        sGame.score++;
        StringCopy(gPastimeText, dive == aim ? COMPOUND_STRING("GOAL! Right past the keeper's\nfingertips!") : COMPOUND_STRING("GOAL!!"));
        PlaySE(SE_SUCCESS);
    }
    DrawGoal(FALSE);
    NextKick();
    ScriptContext_Enable();
}

// Script: `callnative Soccer_Shoot, waitstate=1` with VAR_0x8005 = the side
// aimed at. gPastimeText = how it went.
void Soccer_Shoot(void)
{
    StartMeter(Task_SoccerShoot, 2, 3);
}

// Script: the player dives to side VAR_0x8005. gPastimeText = how it went.
void Soccer_Keep(void)
{
    u32 dive = gSpecialVar_0x8005, aim = Random() % SOCCER_SIDES;

    sGame.keeperX = sGoalX[dive] - 1;
    sGame.ballX = sGoalX[aim];
    sGame.ballY = GOAL_Y;
    if (Random() % 100 < SOCCER_CPU_MISS_PERCENT)
    {
        sGame.ballX = aim == 0 ? 6 : 70; // wide
        StringCopy(gPastimeText, COMPOUND_STRING("Wide of the post! Phew!"));
        PlaySE(SE_SUCCESS);
    }
    else if (dive == aim)
    {
        sGame.ballX = sGoalX[aim] + 2;
        sGame.ballY = GOAL_Y + 12;
        StringCopy(gPastimeText, COMPOUND_STRING("SAVED! What a dive!"));
        PlaySE(SE_SUCCESS);
    }
    else
    {
        sGame.cpuScore++;
        StringCopy(gPastimeText, COMPOUND_STRING("They score…"));
        PlaySE(SE_FAILURE);
    }
    DrawGoal(FALSE);
    NextKick();
}

// Script: gStringVar1 = your goals, gStringVar2 = theirs; VAR_RESULT =
// SOCCER_WON, _DRAW or _LOST; VAR_0x8005 = a prize.
void Soccer_Finish(void)
{
    WriteNumber(gStringVar1, sGame.score);
    WriteNumber(gStringVar2, sGame.cpuScore);
    if (sGame.score > sGame.cpuScore)
    {
        gSpecialVar_Result = SOCCER_WON;
        if (sSave.soccerWins < 0xFF)
            sSave.soccerWins++;
    }
    else
    {
        gSpecialVar_Result = sGame.score == sGame.cpuScore ? SOCCER_DRAW : SOCCER_LOST;
    }
    GivePrize(1 << 2, gSpecialVar_Result == SOCCER_WON, ITEM_QUICK_CLAW, gSpecialVar_Result == SOCCER_DRAW, ITEM_CARBOS);
}

// Ferris wheel -------------------------------------------------------------------

static const u8 *const sSights[] =
{
    COMPOUND_STRING("Way out east, something sparkles at\nthe far end of the bowling lanes!"),
    COMPOUND_STRING("Down in the south-west corner,\nsomething shiny is lying in the grass!"),
    COMPOUND_STRING("Far to the south-east, past the golf\ngreen, something glints!"),
    COMPOUND_STRING("You can see all the tents from up\nhere. What a crowd today!"),
};

// Script: one ride. gPastimeText = what the player sees from the top.
void Ferris_Ride(void)
{
    u8 *p = gPastimeText;

    if (sSave.ferrisRides < 0xFFFF)
        sSave.ferrisRides++;
    RtcCalcLocalTime();
    if (gLocalTime.hours >= 19 || gLocalTime.hours < 5)
        p = StringCopy(p, COMPOUND_STRING("The carnival lights twinkle below!\p"));
    StringCopy(p, sSights[Random() % ARRAY_COUNT(sSights)]);
}

// R > PASTIMES > CARNIVAL ------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    switch (index)
    {
    case 0:
        dest = StringCopy(dest, COMPOUND_STRING("Bowling best{CLEAR_TO 120}"));
        WriteNumber(dest, sSave.bowlBest);
        break;
    case 1:
        dest = StringCopy(dest, COMPOUND_STRING("Golf best round{CLEAR_TO 120}"));
        if (sSave.golfBest == 0)
        {
            StringCopy(dest, COMPOUND_STRING("-"));
            break;
        }
        dest = ConvertIntToDecimalStringN(dest, sSave.golfBest, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringCopy(dest, COMPOUND_STRING(" strokes"));
        break;
    case 2:
        dest = StringCopy(dest, COMPOUND_STRING("Shootouts won{CLEAR_TO 120}"));
        WriteNumber(dest, sSave.soccerWins);
        break;
    case 3:
        dest = StringCopy(dest, COMPOUND_STRING("Ferris wheel rides{CLEAR_TO 120}"));
        ConvertIntToDecimalStringN(dest, sSave.ferrisRides, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    default:
        dest = StringCopy(dest, COMPOUND_STRING("Games played{CLEAR_TO 120}"));
        ConvertIntToDecimalStringN(dest, sSave.carnivalPlays, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    }
}

static void GetDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("Games at the SUNFAIR CARNIVAL"));
}

void Carnival_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("SUNFAIR CARNIVAL"), 5, GetLine, GetDetail);
}
