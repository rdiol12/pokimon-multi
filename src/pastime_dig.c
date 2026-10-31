#include "global.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "menu.h"
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

// Archaeology digs: an archaeologist in Fallarbor Town has found DIG_SITES
// dig sites around Mt. Chimney, each marked by a bone sticking out of the
// ground. Each site can be worked once a day: DIG_FINDS things are buried
// in a DIG_COLS x DIG_ROWS patch and the player has DIG_TRIES digs. A dug
// square that hits nothing shows how close the nearest buried find is (red
// next to it, then orange, then yellow), and a find comes out once all its
// squares are dug. Fossils are worth taking to the Lilycove Museum's
// curator (src/pastime_museum.c). The archaeologist rewards every few
// finds; R > PASTIMES > DIG SITES lists the sites and the totals.

extern const u8 Dig_EventScript_Archaeologist[];
extern const u8 Dig_EventScript_Site[];

// The dig colors, loaded into the window palette's dynamic text colors
// while digging (put back after).
#define COLOR_DIRT      TEXT_DYNAMIC_COLOR_1
#define COLOR_DIRT_DARK TEXT_DYNAMIC_COLOR_2
#define COLOR_DUG       TEXT_DYNAMIC_COLOR_3
#define COLOR_ORANGE    TEXT_DYNAMIC_COLOR_4
#define COLOR_YELLOW    TEXT_DYNAMIC_COLOR_5
#define COLOR_BONE      TEXT_DYNAMIC_COLOR_6

static const u16 sDigColors[] =
{
    RGB(22, 16, 10), RGB(17, 12, 7), RGB(10, 7, 4), RGB(31, 18, 4), RGB(31, 28, 8), RGB(29, 27, 21),
};

struct DigItem
{
    u16 item;
    u8 width, height;
    u8 color;
};

static const struct DigItem sFossils[] =
{
    {ITEM_ROOT_FOSSIL,  2, 2, COLOR_BONE},
    {ITEM_CLAW_FOSSIL,  2, 2, COLOR_BONE},
    {ITEM_HELIX_FOSSIL, 2, 2, COLOR_BONE},
    {ITEM_DOME_FOSSIL,  2, 2, COLOR_BONE},
    {ITEM_OLD_AMBER,    2, 2, COLOR_ORANGE},
    {ITEM_SKULL_FOSSIL, 2, 2, COLOR_BONE},
    {ITEM_ARMOR_FOSSIL, 2, 2, COLOR_BONE},
    {ITEM_COVER_FOSSIL, 2, 2, COLOR_BONE},
    {ITEM_PLUME_FOSSIL, 2, 2, COLOR_BONE},
    {ITEM_JAW_FOSSIL,   2, 2, COLOR_BONE},
    {ITEM_SAIL_FOSSIL,  2, 2, COLOR_BONE},
};

static const struct DigItem sTreasures[] =
{
    {ITEM_RARE_BONE,     2, 1, COLOR_BONE},
    {ITEM_STAR_PIECE,    1, 1, TEXT_COLOR_LIGHT_RED},
    {ITEM_HEART_SCALE,   1, 1, TEXT_COLOR_LIGHT_RED},
    {ITEM_HARD_STONE,    1, 1, TEXT_COLOR_LIGHT_GRAY},
    {ITEM_EVERSTONE,     1, 1, TEXT_COLOR_LIGHT_GRAY},
    {ITEM_FIRE_STONE,    1, 1, TEXT_COLOR_RED},
    {ITEM_WATER_STONE,   1, 1, TEXT_COLOR_BLUE},
    {ITEM_THUNDER_STONE, 1, 1, COLOR_YELLOW},
    {ITEM_LEAF_STONE,    1, 1, TEXT_COLOR_GREEN},
    {ITEM_MOON_STONE,    1, 1, TEXT_COLOR_DARK_GRAY},
    {ITEM_LIGHT_CLAY,    1, 1, TEXT_COLOR_LIGHT_BLUE},
};

#define FOSSIL_CHANCE 60 // percent: the biggest find is a fossil, else a Rare Bone

static const u8 *const sSiteNames[DIG_SITES] =
{
    COMPOUND_STRING("ROUTE 114"), COMPOUND_STRING("ROUTE 113"), COMPOUND_STRING("ROUTE 115"), COMPOUND_STRING("ROUTE 111"),
};

// The archaeologist's rewards, by finds.
static const struct { u8 finds; u16 item; } sRewards[] =
{
    {3,  ITEM_RARE_BONE},
    {10, ITEM_OLD_AMBER},
    {20, ITEM_COMET_SHARD},
    {35, ITEM_ABILITY_PATCH},
};

#define CELL_DUG  0x80
#define CELL_FIND 0x7F // the find + 1, or 0

#define WIN_X 10 // tiles
#define WIN_Y 1
#define WIN_W 10
#define WIN_H 8
#define WIN_BASE_BLOCK 0xD0
#define CELL 9 // pixels
#define GRID_X 4
#define GRID_Y 2
#define PIPS_Y 58
#define END_DELAY 50 // frames the last dig stays up

struct Find
{
    const struct DigItem *item;
    u8 x, y;
    bool8 found;
};

struct Dig
{
    u8 cells[DIG_COLS * DIG_ROWS];
    struct Find finds[DIG_FINDS];
    u8 findCount;
    u8 tries;
    u8 cursorX, cursorY;
    u8 site;
    u8 windowId;
    u8 endTimer;
    u16 savedColors[ARRAY_COUNT(sDigColors)];
};

// Not static: the screenshot driver digs with it.
EWRAM_DATA struct Dig gDig = {0};
#define sDig gDig

#define sSave (gSaveBlock3Ptr->pastime)

#define SITE(map, x, y, n) {map, x, y, OBJ_EVENT_GFX_FOSSIL, MOVEMENT_TYPE_NONE, n, Dig_EventScript_Site}

const struct PastimeSpot gDigSpots[1 + DIG_SITES] =
{
    {MAP_FALLARBOR_TOWN, 12, 13, OBJ_EVENT_GFX_HIKER, MOVEMENT_TYPE_FACE_DOWN, 0, Dig_EventScript_Archaeologist},
    SITE(MAP_ROUTE114, 22, 10, 0),
    SITE(MAP_ROUTE113, 52, 3, 1),
    SITE(MAP_ROUTE115, 20, 9, 2),
    SITE(MAP_ROUTE111, 8, 9, 3),
};

static u32 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

// Burying ----------------------------------------------------------------------------

static bool32 Bury(const struct DigItem *item)
{
    u32 attempt, x, y, dx, dy;

    for (attempt = 0; attempt < 40; attempt++)
    {
        bool32 free = TRUE;

        x = Random() % (DIG_COLS - item->width + 1);
        y = Random() % (DIG_ROWS - item->height + 1);
        for (dy = 0; dy < item->height; dy++)
            for (dx = 0; dx < item->width; dx++)
                free &= sDig.cells[(y + dy) * DIG_COLS + x + dx] == 0;
        if (!free)
            continue;
        for (dy = 0; dy < item->height; dy++)
            for (dx = 0; dx < item->width; dx++)
                sDig.cells[(y + dy) * DIG_COLS + x + dx] = sDig.findCount + 1;
        sDig.finds[sDig.findCount].item = item;
        sDig.finds[sDig.findCount].x = x;
        sDig.finds[sDig.findCount].y = y;
        sDig.finds[sDig.findCount].found = FALSE;
        sDig.findCount++;
        return TRUE;
    }
    return FALSE;
}

static void BuryFinds(void)
{
    u32 i;

    if (Random() % 100 < FOSSIL_CHANCE)
        Bury(&sFossils[Random() % ARRAY_COUNT(sFossils)]);
    else
        Bury(&sTreasures[0]);
    for (i = 1; i < DIG_FINDS; i++)
        Bury(&sTreasures[1 + Random() % (ARRAY_COUNT(sTreasures) - 1)]);
}

// How near the closest find still buried is: 1 next to it, up to 3; 0 far.
static u32 Heat(u32 x, u32 y)
{
    u32 i, best = 0xFF;

    for (i = 0; i < DIG_COLS * DIG_ROWS; i++)
    {
        u32 find = sDig.cells[i] & CELL_FIND, dx, dy, d;
        if (find == 0 || sDig.finds[find - 1].found)
            continue;
        dx = abs((s32)(i % DIG_COLS) - (s32)x);
        dy = abs((s32)(i / DIG_COLS) - (s32)y);
        d = max(dx, dy);
        if (d < best)
            best = d;
    }
    return best <= 3 ? best : 0;
}

// The dig ----------------------------------------------------------------------------

static void DrawDig(void)
{
    static const u8 sHeatColors[] = {COLOR_DUG, TEXT_COLOR_RED, COLOR_ORANGE, COLOR_YELLOW};
    u32 i, x, y;

    FillWindowPixelBuffer(sDig.windowId, PIXEL_FILL(COLOR_DIRT_DARK));
    for (i = 0; i < DIG_COLS * DIG_ROWS; i++)
    {
        u32 find = sDig.cells[i] & CELL_FIND;

        x = GRID_X + (i % DIG_COLS) * CELL;
        y = GRID_Y + (i / DIG_COLS) * CELL;
        if (!(sDig.cells[i] & CELL_DUG))
        {
            FillWindowPixelRect(sDig.windowId, PIXEL_FILL(COLOR_DIRT), x, y, CELL - 1, CELL - 1);
            FillWindowPixelRect(sDig.windowId, PIXEL_FILL(COLOR_DIRT_DARK), x + 2 + (i % 3), y + 2 + (i % 4), 1, 1);
        }
        else if (find != 0)
        {
            FillWindowPixelRect(sDig.windowId, PIXEL_FILL(sDig.finds[find - 1].item->color), x, y, CELL - 1, CELL - 1);
        }
        else
        {
            u32 heat = Heat(i % DIG_COLS, i / DIG_COLS);
            FillWindowPixelRect(sDig.windowId, PIXEL_FILL(COLOR_DUG), x, y, CELL - 1, CELL - 1);
            if (heat != 0)
                FillWindowPixelRect(sDig.windowId, PIXEL_FILL(sHeatColors[heat]), x + 2, y + 2, CELL - 5, CELL - 5);
        }
    }
    // A find that's out: framed whole.
    for (i = 0; i < sDig.findCount; i++)
    {
        const struct Find *find = &sDig.finds[i];
        u32 w = find->item->width * CELL, h = find->item->height * CELL;

        if (!find->found)
            continue;
        x = GRID_X + find->x * CELL - 1;
        y = GRID_Y + find->y * CELL - 1;
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), x, y, w + 1, 1);
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), x, y + h, w + 1, 1);
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), x, y, 1, h + 1);
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), x + w, y, 1, h + 1);
    }
    // The shovel: an outline around its square.
    if (sDig.endTimer == 0)
    {
        x = GRID_X + sDig.cursorX * CELL - 1;
        y = GRID_Y + sDig.cursorY * CELL - 1;
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x, y, CELL + 1, 1);
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x, y + CELL, CELL + 1, 1);
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x, y, 1, CELL + 1);
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x + CELL, y, 1, CELL + 1);
    }
    // The digs left.
    for (i = 0; i < DIG_TRIES; i++)
        FillWindowPixelRect(sDig.windowId, PIXEL_FILL(i < sDig.tries ? COLOR_BONE : COLOR_DUG), GRID_X + 1 + i * 5, PIPS_Y, 4, 4);
    CopyWindowToVram(sDig.windowId, COPYWIN_GFX);
}

static bool32 AllFound(void)
{
    u32 i;

    for (i = 0; i < sDig.findCount; i++)
    {
        if (!sDig.finds[i].found)
            return FALSE;
    }
    return TRUE;
}

static void DigHere(void)
{
    u32 cell = sDig.cursorY * DIG_COLS + sDig.cursorX, find, dx, dy;
    struct Find *f;
    bool32 out = TRUE;

    if (sDig.cells[cell] & CELL_DUG)
        return;
    sDig.cells[cell] |= CELL_DUG;
    sDig.tries--;
    find = sDig.cells[cell] & CELL_FIND;
    if (find == 0)
    {
        PlaySE(SE_M_DIG);
        return;
    }
    f = &sDig.finds[find - 1];
    for (dy = 0; dy < f->item->height; dy++)
        for (dx = 0; dx < f->item->width; dx++)
            out &= (sDig.cells[(f->y + dy) * DIG_COLS + f->x + dx] & CELL_DUG) != 0;
    if (out)
    {
        f->found = TRUE;
        PlaySE(SE_SUCCESS);
    }
    else
    {
        PlaySE(SE_M_ROCK_THROW);
    }
}

static void EndDig(u8 taskId)
{
    ClearStdWindowAndFrameToTransparent(sDig.windowId, TRUE);
    RemoveWindow(sDig.windowId);
    LoadPalette(sDig.savedColors, BG_PLTT_ID(15) + TEXT_DYNAMIC_COLOR_1, sizeof(sDig.savedColors));
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Dig(u8 taskId)
{
    bool32 moved = FALSE;

    if (sDig.endTimer != 0)
    {
        if (--sDig.endTimer == 0)
            EndDig(taskId);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        EndDig(taskId);
        return;
    }
    if (JOY_REPEAT(DPAD_UP) && sDig.cursorY > 0)
        sDig.cursorY--, moved = TRUE;
    else if (JOY_REPEAT(DPAD_DOWN) && sDig.cursorY < DIG_ROWS - 1)
        sDig.cursorY++, moved = TRUE;
    else if (JOY_REPEAT(DPAD_LEFT) && sDig.cursorX > 0)
        sDig.cursorX--, moved = TRUE;
    else if (JOY_REPEAT(DPAD_RIGHT) && sDig.cursorX < DIG_COLS - 1)
        sDig.cursorX++, moved = TRUE;
    if (JOY_NEW(A_BUTTON))
    {
        DigHere();
        if (sDig.tries == 0 || AllFound())
            sDig.endTimer = END_DELAY;
        moved = TRUE;
    }
    if (moved)
        DrawDig();
}

// Scripts ----------------------------------------------------------------------------

// Script: site VAR_0x8004. VAR_RESULT = DIG_RESULT_DUG_TODAY or
// DIG_RESULT_DIG; gStringVar1 = the site.
void Dig_CheckSite(void)
{
    StringCopy(gStringVar1, sSiteNames[gSpecialVar_0x8004]);
    gSpecialVar_Result = sSave.digDay[gSpecialVar_0x8004] == Today() + 1 ? DIG_RESULT_DUG_TODAY : DIG_RESULT_DIG;
}

// Script: `callnative Dig_Start, waitstate=1` digs at site VAR_0x8004 (B
// stops early).
void Dig_Start(void)
{
    struct WindowTemplate template = CreateWindowTemplate(0, WIN_X, WIN_Y, WIN_W, WIN_H, 15, WIN_BASE_BLOCK);

    memset(&sDig, 0, sizeof(sDig));
    sDig.site = gSpecialVar_0x8004;
    sDig.tries = DIG_TRIES;
    sDig.cursorX = DIG_COLS / 2;
    sDig.cursorY = DIG_ROWS / 2;
    sSave.digDay[sDig.site] = Today() + 1;
    BuryFinds();
    memcpy(sDig.savedColors, &gPlttBufferUnfaded[BG_PLTT_ID(15) + TEXT_DYNAMIC_COLOR_1], sizeof(sDig.savedColors));
    LoadPalette(sDigColors, BG_PLTT_ID(15) + TEXT_DYNAMIC_COLOR_1, sizeof(sDigColors));
    sDig.windowId = AddWindow(&template);
    PutWindowTilemap(sDig.windowId);
    SetStandardWindowBorderStyle(sDig.windowId, TRUE);
    DrawDig();
    CopyWindowToVram(sDig.windowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Dig, 80);
}

// Script: puts what was dug up in the bag. gPastimeText = what came out;
// VAR_RESULT = how many finds.
void Dig_Finish(void)
{
    u8 *p = gPastimeText;
    u32 i, count = 0;

    for (i = 0; i < sDig.findCount; i++)
    {
        u16 item = sDig.finds[i].item->item;

        if (!sDig.finds[i].found)
            continue;
        if (count != 0)
            p = StringCopy(p, COMPOUND_STRING("\p"));
        p = StringCopy(p, COMPOUND_STRING("{PLAYER} dug up "));
        p = StringCopy(p, GetItemName(item));
        if (AddBagItem(item, 1))
        {
            p = StringCopy(p, COMPOUND_STRING("!"));
        }
        else
        {
            p = StringCopy(p, COMPOUND_STRING(",\nbut the BAG is full…"));
            continue;
        }
        count++;
        if (sSave.digFinds < 0xFFFF)
            sSave.digFinds++;
        if (sDig.finds[i].item->width == 2 && sDig.finds[i].item->height == 2 && sSave.digFossils < 0xFFFF)
            sSave.digFossils++;
    }
    if (p == gPastimeText)
        StringCopy(p, COMPOUND_STRING("Nothing came out this time…"));
    gSpecialVar_Result = count;
}

// Script: gStringVar3 = finds for the archaeologist's next reward
// (VAR_RESULT = FALSE if there's none left).
void Dig_GetNextReward(void)
{
    gSpecialVar_Result = sSave.digRewards < ARRAY_COUNT(sRewards);
    if (gSpecialVar_Result)
        ConvertIntToDecimalStringN(gStringVar3, sRewards[sSave.digRewards].finds, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Script: the archaeologist. gStringVar1 = finds, gStringVar2 = fossils;
// VAR_0x8005 = a reward now due (else ITEM_NONE); then as
// Dig_GetNextReward.
void Dig_TalkToArchaeologist(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sSave.digFinds, STR_CONV_MODE_LEFT_ALIGN, 5);
    ConvertIntToDecimalStringN(gStringVar2, sSave.digFossils, STR_CONV_MODE_LEFT_ALIGN, 5);
    gSpecialVar_0x8005 = ITEM_NONE;
    if (sSave.digRewards < ARRAY_COUNT(sRewards) && sSave.digFinds >= sRewards[sSave.digRewards].finds)
        gSpecialVar_0x8005 = sRewards[sSave.digRewards++].item;
    Dig_GetNextReward();
}

// R > PASTIMES > DIG SITES ---------------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < DIG_SITES)
    {
        dest = StringCopy(dest, sSiteNames[index]);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 96}"));
        if (sSave.digDay[index] == Today() + 1)
            StringCopy(dest, COMPOUND_STRING("Dug today"));
        else
            StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Ready to dig"));
        return;
    }
    if (index == DIG_SITES)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Finds{CLEAR_TO 96}"));
        ConvertIntToDecimalStringN(dest, sSave.digFinds, STR_CONV_MODE_LEFT_ALIGN, 5);
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Fossils{CLEAR_TO 96}"));
    ConvertIntToDecimalStringN(dest, sSave.digFossils, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void GetDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("Each site can be dug once a day"));
}

void Dig_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("DIG SITES"), DIG_SITES + 2, GetLine, GetDetail);
}
