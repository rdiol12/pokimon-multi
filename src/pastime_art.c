#include "global.h"
#include "event_data.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "money.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "script.h"
#include "script_menu.h"
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
#include "constants/songs.h"

// Art studio: a painter at the Lilycove Museum runs paint-by-numbers
// classes. The canvas is ART_SIZE x ART_SIZE cells; unpainted cells show a
// dot of the color they should be. The + Control Pad moves the brush, A
// paints (hold it to paint while moving), L and R (or SELECT) change the
// color, START finishes. A painting 90% right or better is hung in the
// gallery and the museum buys it; hanging all ART_PAINTINGS earns a prize.

extern const u8 Art_EventScript_Painter[];

// Picture cells: '.' white, K dark, L light gray, R red, r light red,
// G green, g light green, B blue, b light blue.
struct Painting
{
    const u8 *name;
    const char *cells; // ART_SIZE rows of ART_SIZE
};

static const struct Painting sPaintings[ART_PAINTINGS] =
{
    {COMPOUND_STRING("POKé BALL"),
        "....KKKK...."
        "..KKRRRRKK.."
        ".KRRrRRRRRK."
        ".KRrRRRRRRK."
        "KRRRRKKRRRRK"
        "KKKKK..KKKKK"
        "KKKKK..KKKKK"
        "K....KK....K"
        ".K........K."
        ".K........K."
        "..KK....KK.."
        "....KKKK...."},
    {COMPOUND_STRING("ORAN BERRY"),
        ".....GG....."
        "....GgG....."
        "...KKGKK...."
        "..KBBBBBK..."
        ".KBbbBBBBK.."
        ".KBbBBBBBK.."
        ".KBBBBBBBK.."
        ".KBBBBBBBK.."
        "..KBBBBBK..."
        "...KKKKK...."
        "............"
        "............"},
    {COMPOUND_STRING("HEART"),
        "............"
        "..KKK..KKK.."
        ".KrrrKKrrrK."
        "KrRRrrrrrrrK"
        "KrRrrrrrrrrK"
        "KrrrrrrrrrrK"
        ".KrrrrrrrrK."
        "..KrrrrrrK.."
        "...KrrrrK..."
        "....KrrK...."
        ".....KK....."
        "............"},
    {COMPOUND_STRING("LEAF"),
        "..........GG"
        ".......GGGgG"
        ".....GGggggG"
        "....GgggGggG"
        "...GggGgggG."
        "..GggGgggG.."
        "..GgGgggG..."
        ".GgGggGG...."
        ".GGgGG......"
        ".GGG........"
        "G..........."
        "G..........."},
    {COMPOUND_STRING("MUSHROOM"),
        "....KKKK...."
        "..KKRRRRKK.."
        ".KRR..RRRRK."
        ".KR....RR.K."
        "KRRR..RRR..K"
        "KRRRRRRRRRRK"
        ".KKKKKKKKKK."
        "...KLLLLK..."
        "...KL..LK..."
        "...KL..LK..."
        "...KLLLLK..."
        "....KKKK...."},
};

#define CANVAS_X 10 // tiles
#define CANVAS_Y 1
#define CANVAS_W 10
#define CANVAS_H 8
#define CANVAS_BASE_BLOCK 0xD0
#define CELL 5
#define GRID_X 2 // pixels
#define GRID_Y 2
#define SWATCH_X 66
#define SWATCH_W 12
#define SWATCH_H 8

#define PASS_PERCENT 90
#define PRICE_PER_PERCENT 20 // the museum pays this many ¥ per percent right

struct ArtStudio
{
    u8 canvas[ART_SIZE * ART_SIZE]; // TEXT_COLOR_*
    u8 colors[8];
    u8 colorCount;
    u8 color;     // index in colors
    u8 cursorX, cursorY;
    u8 painting;
    u8 windowId;
    bool8 done;
};

// Not static: the screenshot driver paints with it.
EWRAM_DATA struct ArtStudio gArtStudio = {0};
#define sArt gArtStudio

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gArtSpots[1] =
{
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 5, 8, OBJ_EVENT_GFX_ARTIST, MOVEMENT_TYPE_FACE_DOWN, 0, Art_EventScript_Painter},
};

static u8 CellColor(char c)
{
    switch (c)
    {
    case 'K': return TEXT_COLOR_DARK_GRAY;
    case 'L': return TEXT_COLOR_LIGHT_GRAY;
    case 'R': return TEXT_COLOR_RED;
    case 'r': return TEXT_COLOR_LIGHT_RED;
    case 'G': return TEXT_COLOR_GREEN;
    case 'g': return TEXT_COLOR_LIGHT_GREEN;
    case 'B': return TEXT_COLOR_BLUE;
    case 'b': return TEXT_COLOR_LIGHT_BLUE;
    }
    return TEXT_COLOR_WHITE;
}

static u8 TargetColor(u32 cell)
{
    return CellColor(sPaintings[sArt.painting].cells[cell]);
}

// The canvas ---------------------------------------------------------------------

static void DrawCanvas(void)
{
    u32 i, x, y;

    FillWindowPixelBuffer(sArt.windowId, PIXEL_FILL(TEXT_COLOR_LIGHT_GRAY));
    for (i = 0; i < ART_SIZE * ART_SIZE; i++)
    {
        x = GRID_X + (i % ART_SIZE) * CELL;
        y = GRID_Y + (i / ART_SIZE) * CELL;
        FillWindowPixelRect(sArt.windowId, PIXEL_FILL(sArt.canvas[i]), x, y, CELL, CELL);
        // Unpainted: a dot of the color it should be.
        if (sArt.canvas[i] == TEXT_COLOR_WHITE && TargetColor(i) != TEXT_COLOR_WHITE)
            FillWindowPixelRect(sArt.windowId, PIXEL_FILL(TargetColor(i)), x + 2, y + 2, 1, 1);
    }
    // The brush: an outline around its cell.
    x = GRID_X + sArt.cursorX * CELL - 1;
    y = GRID_Y + sArt.cursorY * CELL - 1;
    FillWindowPixelRect(sArt.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x, y, CELL + 2, 1);
    FillWindowPixelRect(sArt.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x, y + CELL + 1, CELL + 2, 1);
    FillWindowPixelRect(sArt.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x, y, 1, CELL + 2);
    FillWindowPixelRect(sArt.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x + CELL + 1, y, 1, CELL + 2);
    // The colors, the one in use framed.
    for (i = 0; i < sArt.colorCount; i++)
    {
        y = GRID_Y + i * (SWATCH_H + 2);
        if (i == sArt.color)
            FillWindowPixelRect(sArt.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), SWATCH_X - 2, y - 1, SWATCH_W + 4, SWATCH_H + 2);
        FillWindowPixelRect(sArt.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), SWATCH_X - 1, y, SWATCH_W + 2, SWATCH_H);
        FillWindowPixelRect(sArt.windowId, PIXEL_FILL(sArt.colors[i]), SWATCH_X, y + 1, SWATCH_W, SWATCH_H - 2);
    }
    CopyWindowToVram(sArt.windowId, COPYWIN_GFX);
}

// The painting's colors, white (the eraser) last.
static void PickColors(void)
{
    u32 i, j;

    sArt.colorCount = 0;
    for (i = 0; i < ART_SIZE * ART_SIZE; i++)
    {
        u8 color = TargetColor(i);
        if (color == TEXT_COLOR_WHITE)
            continue;
        for (j = 0; j < sArt.colorCount && sArt.colors[j] != color; j++)
            ;
        if (j == sArt.colorCount && sArt.colorCount < ARRAY_COUNT(sArt.colors) - 1)
            sArt.colors[sArt.colorCount++] = color;
    }
    sArt.colors[sArt.colorCount++] = TEXT_COLOR_WHITE;
}

static void Paint(void)
{
    u32 cell = sArt.cursorY * ART_SIZE + sArt.cursorX;

    if (sArt.canvas[cell] != sArt.colors[sArt.color])
    {
        sArt.canvas[cell] = sArt.colors[sArt.color];
        PlaySE(SE_BALL_TRAY_BALL);
    }
}

static void Task_Paint(u8 taskId)
{
    bool32 moved = FALSE, changed = FALSE;

    if (JOY_NEW(START_BUTTON) || JOY_NEW(B_BUTTON) || sArt.done)
    {
        PlaySE(SE_SELECT);
        ClearStdWindowAndFrameToTransparent(sArt.windowId, TRUE);
        RemoveWindow(sArt.windowId);
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    if (JOY_REPEAT(DPAD_UP) && sArt.cursorY > 0)
        sArt.cursorY--, moved = TRUE;
    else if (JOY_REPEAT(DPAD_DOWN) && sArt.cursorY < ART_SIZE - 1)
        sArt.cursorY++, moved = TRUE;
    else if (JOY_REPEAT(DPAD_LEFT) && sArt.cursorX > 0)
        sArt.cursorX--, moved = TRUE;
    else if (JOY_REPEAT(DPAD_RIGHT) && sArt.cursorX < ART_SIZE - 1)
        sArt.cursorX++, moved = TRUE;
    if (JOY_NEW(R_BUTTON) || JOY_NEW(SELECT_BUTTON))
    {
        sArt.color = (sArt.color + 1) % sArt.colorCount;
        changed = TRUE;
    }
    else if (JOY_NEW(L_BUTTON))
    {
        sArt.color = (sArt.color + sArt.colorCount - 1) % sArt.colorCount;
        changed = TRUE;
    }
    if (JOY_NEW(A_BUTTON) || (moved && JOY_HELD(A_BUTTON)))
    {
        Paint();
        changed = TRUE;
    }
    if (moved || changed)
        DrawCanvas();
}

// Scripts ------------------------------------------------------------------------

static void PushMenuItem(const u8 *name, u32 id)
{
    struct ListMenuItem item;
    u8 *buffer = Alloc(StringLength(name) + 1);

    StringCopy(buffer, name);
    item.name = buffer;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

// Script: pushes the paintings onto the dynmultichoice stack (hung ones
// marked).
void Art_PushPaintingMenu(void)
{
    u8 text[32];
    u32 i;

    for (i = 0; i < ART_PAINTINGS; i++)
    {
        u8 *p = StringCopy(text, sPaintings[i].name);
        if (sSave.artBest[i] >= PASS_PERCENT)
            StringCopy(p, COMPOUND_STRING(" {COLOR GREEN}{SHADOW LIGHT_GREEN}HUNG"));
        PushMenuItem(text, i);
    }
    PushMenuItem(COMPOUND_STRING("CANCEL"), ART_MENU_CANCEL);
}

// Script: `callnative Art_Paint, waitstate=1` paints picture VAR_0x8005.
void Art_Paint(void)
{
    struct WindowTemplate template = CreateWindowTemplate(0, CANVAS_X, CANVAS_Y, CANVAS_W, CANVAS_H, 15, CANVAS_BASE_BLOCK);

    memset(&sArt, 0, sizeof(sArt));
    memset(sArt.canvas, TEXT_COLOR_WHITE, sizeof(sArt.canvas));
    sArt.painting = gSpecialVar_0x8005;
    sArt.cursorX = ART_SIZE / 2;
    sArt.cursorY = ART_SIZE / 2;
    PickColors();
    sArt.windowId = AddWindow(&template);
    PutWindowTilemap(sArt.windowId);
    SetStandardWindowBorderStyle(sArt.windowId, TRUE);
    DrawCanvas();
    CopyWindowToVram(sArt.windowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Paint, 80);
}

// Script: judges the painting. gStringVar1 = percent right, gStringVar2 =
// the name, gStringVar3 = what the museum pays; VAR_RESULT = TRUE if it's
// hung (and paid for); VAR_0x8005 = a prize for hanging every
// painting (else ITEM_NONE).
void Art_Judge(void)
{
    u32 i, right = 0, percent;
    bool32 wasComplete = TRUE, complete = TRUE;

    for (i = 0; i < ART_SIZE * ART_SIZE; i++)
    {
        if (sArt.canvas[i] == TargetColor(i))
            right++;
    }
    percent = right * 100 / (ART_SIZE * ART_SIZE);
    for (i = 0; i < ART_PAINTINGS; i++)
        wasComplete &= sSave.artBest[i] >= PASS_PERCENT;
    if (percent > sSave.artBest[sArt.painting])
        sSave.artBest[sArt.painting] = percent;
    for (i = 0; i < ART_PAINTINGS; i++)
        complete &= sSave.artBest[i] >= PASS_PERCENT;
    if (sSave.artPaintings < 0xFFFF)
        sSave.artPaintings++;
    ConvertIntToDecimalStringN(gStringVar1, percent, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(gStringVar2, sPaintings[sArt.painting].name);
    gSpecialVar_Result = percent >= PASS_PERCENT;
    if (gSpecialVar_Result)
        AddMoney(&gSaveBlock1Ptr->money, percent * PRICE_PER_PERCENT);
    ConvertIntToDecimalStringN(gStringVar3, percent * PRICE_PER_PERCENT, STR_CONV_MODE_LEFT_ALIGN, 5);
    gSpecialVar_0x8005 = (complete && !wasComplete) ? ITEM_SHINY_STONE : ITEM_NONE;
}

// R > PASTIMES > ART STUDIO -------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < ART_PAINTINGS)
    {
        dest = StringCopy(dest, sPaintings[index].name);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 96}"));
        if (sSave.artBest[index] == 0)
        {
            StringCopy(dest, COMPOUND_STRING("-"));
            return;
        }
        dest = ConvertIntToDecimalStringN(dest, sSave.artBest[index], STR_CONV_MODE_LEFT_ALIGN, 3);
        dest = StringCopy(dest, COMPOUND_STRING("%{CLEAR_TO 136}"));
        if (sSave.artBest[index] >= PASS_PERCENT)
            StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}HUNG"));
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Paintings made{CLEAR_TO 96}"));
    ConvertIntToDecimalStringN(dest, sSave.artPaintings, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void GetDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("Paint at the LILYCOVE MUSEUM"));
}

void Art_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("ART STUDIO"), ART_PAINTINGS + 1, GetLine, GetDetail);
}
