#include "global.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "palette.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "window.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Lighthouse keeper: the old keeper of Dewford's harbor light hires the
// player for a night shift, once a night (19:00 to 5:00). Ships come in out
// of the dark in LIGHTHOUSE_LANES lanes, heading for the rocks; the beam
// (LEFT and RIGHT) has to stay on a ship for a moment before its crew sees
// the rocks and steers clear. A ship nobody lit runs aground. A shift is
// LIGHTHOUSE_SHIPS ships, paid LIGHTHOUSE_PAY each, with a Big Pearl for a
// perfect one. R > PASTIMES > LIGHTHOUSE has the records.

extern const u8 Lighthouse_EventScript_Keeper[];

// Loaded into the window palette's dynamic text colors during the shift.
#define COLOR_SEA      TEXT_DYNAMIC_COLOR_1
#define COLOR_SEA_LIT  TEXT_DYNAMIC_COLOR_2
#define COLOR_BEAM     TEXT_DYNAMIC_COLOR_3
#define COLOR_ROCK     TEXT_DYNAMIC_COLOR_4
#define COLOR_HULL     TEXT_DYNAMIC_COLOR_5
#define COLOR_WAVE     TEXT_DYNAMIC_COLOR_6

static const u16 sColors[] =
{
    RGB(2, 4, 10), RGB(8, 11, 17), RGB(31, 29, 14), RGB(12, 12, 13), RGB(17, 10, 5), RGB(5, 8, 16),
};

#define WIN_X 10 // tiles
#define WIN_Y 1
#define WIN_W 10
#define WIN_H 8
#define WIN_BASE_BLOCK 0xD0
#define FIELD_W (WIN_W * 8)
#define LANE_W (FIELD_W / LIGHTHOUSE_LANES)
#define ROCK_Y 46    // a ship's bow here hits the rocks
#define TOWER_Y 52   // the lighthouse's lamp
#define SHIP_W 8
#define SHIP_H 6
#define SEEN_FRAMES 36 // in the beam this long, a ship turns away
#define MAX_SHIPS 3

enum
{
    SHIP_NONE,
    SHIP_SAILING,
    SHIP_TURNING,   // seen the light: steers off
    SHIP_WRECKED,
};

struct Ship
{
    u8 state;
    u8 lane;
    u8 lit;
    u8 timer;
    u16 y; // Q4 pixels
    u8 speed; // Q4 pixels a frame
};

struct Lighthouse
{
    struct Ship ships[MAX_SHIPS];
    u8 beam;     // lane
    u8 launched;
    u8 guided;
    u8 wrecked;
    u8 nextShip; // frames until the next ship
    u8 windowId;
    u8 endTimer;
    u8 frame;
    u16 savedColors[ARRAY_COUNT(sColors)];
};

// Not static: the screenshot driver reads it.
EWRAM_DATA struct Lighthouse gLighthouse = {0};
#define sGame gLighthouse

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gLighthouseSpots[1] =
{
    {MAP_DEWFORD_TOWN, 17, 10, OBJ_EVENT_GFX_SAILOR, MOVEMENT_TYPE_FACE_UP, 0, Lighthouse_EventScript_Keeper},
};

static bool32 IsNight(void)
{
    RtcCalcLocalTime();
    return gLocalTime.hours >= 19 || gLocalTime.hours < 5;
}

// Nights + 1, so a night's early hours count with the evening before.
static u16 Tonight(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days - (gLocalTime.hours < 5) + 1;
}

// Drawing ------------------------------------------------------------------------------

static u32 LaneX(u32 lane)
{
    return lane * LANE_W + LANE_W / 2;
}

static void DrawBeam(void)
{
    s32 y, cx = FIELD_W / 2, tx = LaneX(sGame.beam);

    for (y = 0; y < TOWER_Y; y++)
    {
        s32 t = TOWER_Y - y; // pixels from the lamp
        s32 x = cx + (tx - cx) * t / TOWER_Y;
        s32 half = 1 + t / 6;
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(COLOR_SEA_LIT), x - half, y, half * 2, 1);
    }
}

static void DrawShip(const struct Ship *ship)
{
    s32 x = LaneX(ship->lane) - SHIP_W / 2, y = ship->y >> 4;

    if (ship->state == SHIP_TURNING)
        x += ship->timer / 2 * (ship->lane < LIGHTHOUSE_LANES / 2 ? -1 : 1);
    if (ship->state == SHIP_WRECKED && (ship->timer / 4) % 2)
        return;
    FillWindowPixelRect(sGame.windowId, PIXEL_FILL(COLOR_HULL), x, y + 2, SHIP_W, SHIP_H - 2);
    FillWindowPixelRect(sGame.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), x + 3, y, 2, 3); // the sail
    if (ship->state == SHIP_SAILING && ship->lit != 0)
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(COLOR_BEAM), x + 1, y + 3, (SHIP_W - 2) * ship->lit / SEEN_FRAMES, 1);
    if (ship->state == SHIP_WRECKED)
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(TEXT_COLOR_RED), x + 1, y + 3, SHIP_W - 2, 2);
}

static void Draw(void)
{
    u32 i;

    FillWindowPixelBuffer(sGame.windowId, PIXEL_FILL(COLOR_SEA));
    // A few waves, drifting.
    for (i = 0; i < 6; i++)
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(COLOR_WAVE), (i * 29 + sGame.frame / 8) % (FIELD_W - 6), 6 + i * 7, 5, 1);
    DrawBeam();
    // The rocks along the shore.
    for (i = 0; i < LIGHTHOUSE_LANES; i++)
    {
        u32 x = i * LANE_W + 2;
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(COLOR_ROCK), x, ROCK_Y + 2, 5, 3);
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(COLOR_ROCK), x + 9, ROCK_Y + 1, 4, 4);
    }
    for (i = 0; i < MAX_SHIPS; i++)
    {
        if (sGame.ships[i].state != SHIP_NONE)
            DrawShip(&sGame.ships[i]);
    }
    // The lighthouse, and the ships still to come as pips.
    FillWindowPixelRect(sGame.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), FIELD_W / 2 - 3, TOWER_Y, 6, 12);
    FillWindowPixelRect(sGame.windowId, PIXEL_FILL(TEXT_COLOR_RED), FIELD_W / 2 - 3, TOWER_Y + 4, 6, 2);
    FillWindowPixelRect(sGame.windowId, PIXEL_FILL(COLOR_BEAM), FIELD_W / 2 - 2, TOWER_Y - 2, 4, 3);
    for (i = 0; i < LIGHTHOUSE_SHIPS - sGame.launched; i++)
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(COLOR_HULL), 2 + i * 4, 59, 3, 3);
    for (i = 0; i < sGame.guided; i++)
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(TEXT_COLOR_LIGHT_GREEN), FIELD_W - 5 - i * 4, 59, 3, 3);
    CopyWindowToVram(sGame.windowId, COPYWIN_GFX);
}

// The shift ----------------------------------------------------------------------------

static void Launch(void)
{
    u32 i;

    for (i = 0; i < MAX_SHIPS; i++)
    {
        struct Ship *ship = &sGame.ships[i];
        if (ship->state != SHIP_NONE)
            continue;
        memset(ship, 0, sizeof(*ship));
        ship->state = SHIP_SAILING;
        ship->lane = Random() % LIGHTHOUSE_LANES;
        ship->speed = 3 + sGame.launched / 3 + Random() % 2;
        sGame.launched++;
        sGame.nextShip = 70 + Random() % 60 - sGame.launched * 4;
        return;
    }
    sGame.nextShip = 10; // every ship slot is out: soon
}

static void UpdateShip(struct Ship *ship)
{
    switch (ship->state)
    {
    case SHIP_SAILING:
        ship->y += ship->speed;
        if (sGame.beam == ship->lane && ++ship->lit >= SEEN_FRAMES)
        {
            ship->state = SHIP_TURNING;
            ship->timer = 0;
            sGame.guided++;
            PlaySE(SE_SHIP);
        }
        else if ((ship->y >> 4) + SHIP_H >= ROCK_Y)
        {
            ship->state = SHIP_WRECKED;
            ship->timer = 0;
            sGame.wrecked++;
            PlaySE(SE_M_ROCK_THROW);
        }
        break;
    case SHIP_TURNING:
        if (++ship->timer > 40)
            ship->state = SHIP_NONE;
        break;
    case SHIP_WRECKED:
        if (++ship->timer > 40)
            ship->state = SHIP_NONE;
        break;
    }
}

static bool32 AnyShipOut(void)
{
    u32 i;

    for (i = 0; i < MAX_SHIPS; i++)
    {
        if (sGame.ships[i].state != SHIP_NONE)
            return TRUE;
    }
    return FALSE;
}

static void Task_Shift(u8 taskId)
{
    u32 i;

    if (sGame.endTimer != 0)
    {
        if (--sGame.endTimer == 0)
        {
            ClearStdWindowAndFrameToTransparent(sGame.windowId, TRUE);
            RemoveWindow(sGame.windowId);
            LoadPalette(sGame.savedColors, BG_PLTT_ID(15) + TEXT_DYNAMIC_COLOR_1, sizeof(sGame.savedColors));
            DestroyTask(taskId);
            ScriptContext_Enable();
        }
        return;
    }
    sGame.frame++;
    if (JOY_REPEAT(DPAD_LEFT) && sGame.beam > 0)
        sGame.beam--;
    else if (JOY_REPEAT(DPAD_RIGHT) && sGame.beam < LIGHTHOUSE_LANES - 1)
        sGame.beam++;
    if (sGame.launched < LIGHTHOUSE_SHIPS && --sGame.nextShip == 0)
        Launch();
    for (i = 0; i < MAX_SHIPS; i++)
        UpdateShip(&sGame.ships[i]);
    if (sGame.launched == LIGHTHOUSE_SHIPS && !AnyShipOut())
        sGame.endTimer = 30;
    if (sGame.frame % 2 == 0)
        Draw();
}

// Scripts ----------------------------------------------------------------------------

// Script: VAR_RESULT = LIGHTHOUSE_RESULT_DAY (not night yet),
// LIGHTHOUSE_RESULT_DONE (worked tonight) or LIGHTHOUSE_RESULT_WORK.
void Lighthouse_CheckShift(void)
{
    if (!IsNight())
        gSpecialVar_Result = LIGHTHOUSE_RESULT_DAY;
    else if (sSave.lighthouseNight == Tonight())
        gSpecialVar_Result = LIGHTHOUSE_RESULT_DONE;
    else
        gSpecialVar_Result = LIGHTHOUSE_RESULT_WORK;
}

// Script: `callnative Lighthouse_Play, waitstate=1`: the shift.
void Lighthouse_Play(void)
{
    struct WindowTemplate template = CreateWindowTemplate(0, WIN_X, WIN_Y, WIN_W, WIN_H, 15, WIN_BASE_BLOCK);

    memset(&sGame, 0, sizeof(sGame));
    sGame.beam = LIGHTHOUSE_LANES / 2;
    sGame.nextShip = 30;
    sSave.lighthouseNight = Tonight();
    memcpy(sGame.savedColors, &gPlttBufferUnfaded[BG_PLTT_ID(15) + TEXT_DYNAMIC_COLOR_1], sizeof(sGame.savedColors));
    LoadPalette(sColors, BG_PLTT_ID(15) + TEXT_DYNAMIC_COLOR_1, sizeof(sColors));
    sGame.windowId = AddWindow(&template);
    PutWindowTilemap(sGame.windowId);
    SetStandardWindowBorderStyle(sGame.windowId, TRUE);
    Draw();
    CopyWindowToVram(sGame.windowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Shift, 80);
}

// Script: pays for the shift. gStringVar1 = ships guided, gStringVar2 =
// the pay; VAR_RESULT = TRUE if it was a new best; VAR_0x8005 = a Big Pearl
// for a perfect shift (else ITEM_NONE).
void Lighthouse_Finish(void)
{
    u32 pay = sGame.guided * LIGHTHOUSE_PAY;

    AddMoney(&gSaveBlock1Ptr->money, pay);
    ConvertIntToDecimalStringN(gStringVar1, sGame.guided, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, pay, STR_CONV_MODE_LEFT_ALIGN, 5);
    gSpecialVar_Result = sGame.guided > sSave.lighthouseBest;
    if (gSpecialVar_Result)
        sSave.lighthouseBest = sGame.guided;
    if (sSave.lighthouseShifts < 0xFFFF)
        sSave.lighthouseShifts++;
    sSave.lighthouseShips = min(sSave.lighthouseShips + sGame.guided, 0xFFFF);
    gSpecialVar_0x8005 = sGame.guided == LIGHTHOUSE_SHIPS ? ITEM_BIG_PEARL : ITEM_NONE;
}

// R > PASTIMES > LIGHTHOUSE ----------------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    switch (index)
    {
    case 0:
        dest = StringCopy(dest, COMPOUND_STRING("Shifts worked{CLEAR_TO 112}"));
        ConvertIntToDecimalStringN(dest, sSave.lighthouseShifts, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    case 1:
        dest = StringCopy(dest, COMPOUND_STRING("Ships guided{CLEAR_TO 112}"));
        ConvertIntToDecimalStringN(dest, sSave.lighthouseShips, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    case 2:
        dest = StringCopy(dest, COMPOUND_STRING("Best shift{CLEAR_TO 112}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.lighthouseBest, STR_CONV_MODE_LEFT_ALIGN, 2);
        dest = StringCopy(dest, COMPOUND_STRING(" / "));
        ConvertIntToDecimalStringN(dest, LIGHTHOUSE_SHIPS, STR_CONV_MODE_LEFT_ALIGN, 2);
        break;
    default:
        dest = StringCopy(dest, COMPOUND_STRING("Tonight{CLEAR_TO 112}"));
        if (!IsNight())
            StringCopy(dest, COMPOUND_STRING("From 19:00"));
        else if (sSave.lighthouseNight == Tonight())
            StringCopy(dest, COMPOUND_STRING("Worked"));
        else
            StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}On duty!"));
        break;
    }
}

static void GetDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("The keeper is at DEWFORD's harbor"));
}

void Lighthouse_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("LIGHTHOUSE"), 4, GetLine, GetDetail);
}
