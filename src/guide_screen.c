#include "global.h"
#include "bg.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "guide.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "region_map.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/region_map_sections.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The guide's screens (docs/guide.md): the list (a window over the field)
// and the map (the region map with a marker on every town where something
// waits). Both borrow their memory from the heap while open.

static const u8 sColorsNormal[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sColorsNew[3]    = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};
static const u8 sColorsTodo[3]   = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};
static const u8 sColorsDone[3]   = {TEXT_COLOR_WHITE, TEXT_COLOR_GREEN, TEXT_COLOR_LIGHT_GREEN};
static const u8 sColorsFaint[3]  = {TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_WHITE};

// The list's order: what needs the player first.
static const u8 sStatusOrder[] =
{
    [GUIDE_NEW]    = 0,
    [GUIDE_TODO]   = 1,
    [GUIDE_KNOWN]  = 2,
    [GUIDE_LOCKED] = 3,
    [GUIDE_DONE]   = 4,
};

static void StatusTag(u32 entry, enum GuideStatus status, u8 *dest, const u8 **colors)
{
    switch (status)
    {
    case GUIDE_NEW:
        StringCopy(dest, COMPOUND_STRING("NEW!"));
        *colors = sColorsNew;
        break;
    case GUIDE_TODO:
        StringCopy(dest, COMPOUND_STRING("TO DO"));
        *colors = sColorsTodo;
        break;
    case GUIDE_KNOWN:
        StringCopy(dest, COMPOUND_STRING("VISITED"));
        *colors = sColorsNormal;
        break;
    case GUIDE_DONE:
        StringCopy(dest, COMPOUND_STRING("DONE"));
        *colors = sColorsDone;
        break;
    default:
        if (gGuideEntries[entry].badges == GUIDE_CHAMPION)
        {
            StringCopy(dest, COMPOUND_STRING("CHAMPION"));
        }
        else
        {
            dest = StringCopy(dest, COMPOUND_STRING("BADGE "));
            ConvertIntToDecimalStringN(dest, gGuideEntries[entry].badges, STR_CONV_MODE_LEFT_ALIGN, 1);
        }
        *colors = sColorsFaint;
        break;
    }
}

// List -----------------------------------------------------------------------

#define LIST_ROWS        6
#define LIST_LINE_HEIGHT 14
#define LIST_TOP         16
#define LIST_WIDTH       28

static const struct WindowTemplate sListWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = LIST_WIDTH,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

struct GuideList
{
    u8 order[GUIDE_MAX_ENTRIES];
    u8 status[GUIDE_MAX_ENTRIES];
    u8 count;
    u8 cursor;
    u8 scroll;
    u8 windowId;
};

static EWRAM_DATA struct GuideList *sList = NULL;
static EWRAM_DATA u8 sListCursor = 0; // kept between opens, so A > map > back returns to the same line

static void Task_GuideList(u8 taskId);

static bool32 ComesAfter(u32 a, u32 b)
{
    u32 sa = sStatusOrder[sList->status[a]], sb = sStatusOrder[sList->status[b]];

    if (sa != sb)
        return sa > sb;
    if (sList->status[a] == GUIDE_NEW)
        return gGuideEntries[a].badges < gGuideEntries[b].badges;
    return FALSE;
}

static void SortEntries(void)
{
    u32 i, j;

    sList->count = gGuideEntryCount;
    for (i = 0; i < sList->count; i++)
    {
        sList->status[i] = Guide_GetStatus(i);
        sList->order[i] = i;
    }
    // Insertion sort, stable: by status, then story order, except that new
    // entries that need more badges (just opened) come first.
    for (i = 1; i < sList->count; i++)
    {
        u8 e = sList->order[i];
        for (j = i; j > 0 && ComesAfter(sList->order[j - 1], e); j--)
            sList->order[j] = sList->order[j - 1];
        sList->order[j] = e;
    }
}

static void DrawList(void)
{
    u8 text[48];
    u32 i, y, width;
    u8 win = sList->windowId;
    const u8 *colors;

    FillWindowPixelBuffer(win, PIXEL_FILL(1));
    AddTextPrinterParameterized3(win, FONT_NORMAL, 4, 0, sColorsTodo, TEXT_SKIP_DRAW, COMPOUND_STRING("GUIDE"));
    ConvertIntToDecimalStringN(gStringVar1, Guide_CountStatus(GUIDE_NEW), STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, Guide_CountStatus(GUIDE_TODO), STR_CONV_MODE_LEFT_ALIGN, 2);
    StringExpandPlaceholders(text, COMPOUND_STRING("{STR_VAR_1} NEW, {STR_VAR_2} TO DO"));
    width = GetStringWidth(FONT_SMALL, text, 0);
    AddTextPrinterParameterized3(win, FONT_SMALL, LIST_WIDTH * 8 - 6 - width, 2, sColorsNormal, TEXT_SKIP_DRAW, text);

    for (i = 0; i < LIST_ROWS && sList->scroll + i < sList->count; i++)
    {
        u32 row = sList->scroll + i;
        u32 entry = sList->order[row];
        y = LIST_TOP + i * LIST_LINE_HEIGHT;
        if (row == sList->cursor)
            AddTextPrinterParameterized(win, FONT_NORMAL, gText_SelectorArrow2, 0, y, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized3(win, FONT_NORMAL, 10, y,
                                     sList->status[entry] == GUIDE_LOCKED ? sColorsFaint : sColorsNormal,
                                     TEXT_SKIP_DRAW, gGuideEntries[entry].name);
        StatusTag(entry, sList->status[entry], text, &colors);
        width = GetStringWidth(FONT_SMALL, text, 0);
        AddTextPrinterParameterized3(win, FONT_SMALL, LIST_WIDTH * 8 - 6 - width, y + 2, colors, TEXT_SKIP_DRAW, text);
    }

    // The picked entry: how to get there, and what it needs.
    if (sList->count != 0)
    {
        u32 entry = sList->order[sList->cursor];
        y = LIST_TOP + LIST_ROWS * LIST_LINE_HEIGHT + 2;
        AddTextPrinterParameterized3(win, FONT_SMALL, 4, y, sColorsNormal, TEXT_SKIP_DRAW, gGuideEntries[entry].where);
        if (sList->status[entry] == GUIDE_LOCKED)
            AddTextPrinterParameterized3(win, FONT_SMALL, 4, y + 24, sColorsNew, TEXT_SKIP_DRAW, Guide_GetNeedsText(entry));
    }
    CopyWindowToVram(win, COPYWIN_FULL);
}

void Guide_ShowList(void)
{
    sList = AllocZeroed(sizeof(*sList));
    if (sList == NULL)
    {
        gSpecialVar_Result = GUIDE_NONE;
        ScriptContext_Enable();
        return;
    }
    SortEntries();
    sList->cursor = sListCursor < sList->count ? sListCursor : 0;
    if (sList->cursor >= LIST_ROWS)
        sList->scroll = sList->cursor - LIST_ROWS + 1;
    sList->windowId = AddWindow(&sListWindowTemplate);
    DrawStdWindowFrame(sList->windowId, FALSE);
    PutWindowTilemap(sList->windowId);
    DrawList();
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_GuideList, 80);
}

static void CloseList(u8 taskId, u16 result)
{
    sListCursor = sList->cursor;
    ClearStdWindowAndFrameToTransparent(sList->windowId, FALSE);
    ClearWindowTilemap(sList->windowId);
    RemoveWindow(sList->windowId);
    ScheduleBgCopyTilemapToVram(0);
    TRY_FREE_AND_SET_NULL(sList);
    DestroyTask(taskId);
    gSpecialVar_Result = result;
    ScriptContext_Enable();
}

static void Task_GuideList(u8 taskId)
{
    u8 old = sList->cursor;

    if (JOY_NEW(A_BUTTON) && sList->count != 0)
    {
        u32 entry = sList->order[sList->cursor];
        PlaySE(SE_SELECT);
        Guide_MarkSeen(entry);
        CloseList(taskId, entry);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseList(taskId, GUIDE_NONE);
        return;
    }
    if (JOY_REPEAT(DPAD_DOWN) && sList->cursor + 1 < sList->count)
        sList->cursor++;
    else if (JOY_REPEAT(DPAD_UP) && sList->cursor > 0)
        sList->cursor--;
    else if (JOY_REPEAT(DPAD_RIGHT))
        sList->cursor = min(sList->cursor + LIST_ROWS, sList->count - 1);
    else if (JOY_REPEAT(DPAD_LEFT))
        sList->cursor = sList->cursor > LIST_ROWS ? sList->cursor - LIST_ROWS : 0;
    if (sList->cursor != old)
    {
        if (sList->cursor < sList->scroll)
            sList->scroll = sList->cursor;
        else if (sList->cursor >= sList->scroll + LIST_ROWS)
            sList->scroll = sList->cursor - LIST_ROWS + 1;
        PlaySE(SE_SELECT);
        DrawList();
    }
}

// Map ------------------------------------------------------------------------

enum {
    WIN_INFO,
    WIN_TITLE,
};

enum {
    TAG_PLAYER_ICON = 0x6A00,
    TAG_CURSOR,
    TAG_MARKER,
};

enum {
    MARKER_NEW,
    MARKER_TODO,
};

#define MAX_MARKERS 24
#define MAP_BORDER_TILE 71 // after the two windows (56 + 14 tiles from 1)
#define MARKER_BOB_FRAMES 24

static const u16 sMarkerPal[] = INCGFX_U16("graphics/guide/markers.png", ".gbapal");
static const u32 sMarkerGfx[] = INCGFX_U32("graphics/guide/markers.png", ".4bpp");

static const struct SpriteSheet sMarkerSheet = {sMarkerGfx, sizeof(sMarkerGfx), TAG_MARKER};
static const struct SpritePalette sMarkerPalette = {sMarkerPal, TAG_MARKER};

static const struct OamData sMarkerOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static const union AnimCmd sAnim_New[] = {ANIMCMD_FRAME(0, 30), ANIMCMD_FRAME(4, 30), ANIMCMD_JUMP(0)};
static const union AnimCmd sAnim_Todo[] = {ANIMCMD_FRAME(8, 30), ANIMCMD_END};
static const union AnimCmd *const sMarkerAnims[] = {[MARKER_NEW] = sAnim_New, [MARKER_TODO] = sAnim_Todo};

static void SpriteCB_Marker(struct Sprite *sprite);

static const struct SpriteTemplate sMarkerTemplate =
{
    .tileTag = TAG_MARKER,
    .paletteTag = TAG_MARKER,
    .oam = &sMarkerOam,
    .anims = sMarkerAnims,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Marker,
};

static const struct BgTemplate sMapBgTemplates[] = {
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }, {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 28,
        .screenSize = 2,
        .paletteMode = 1,
        .priority = 2,
        .baseTile = 0
    }
};

static const struct WindowTemplate sMapWindowTemplates[] =
{
    [WIN_INFO] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 17,
        .width = 28,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1
    },
    [WIN_TITLE] = {
        .bg = 0,
        .tilemapLeft = 22,
        .tilemapTop = 1,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 57
    },
    DUMMY_WIN_TEMPLATE
};

struct GuideMap
{
    struct RegionMap regionMap;
    u16 state;
    u16 lastMapSec;
    u8 startEntry;
    u8 shown;           // which of the cursor's section's entries the info line shows
};

static EWRAM_DATA struct GuideMap *sMap = NULL;

static void CB2_GuideMapInit(void);
static void CB2_GuideMap(void);
static void VBlankCB_GuideMap(void);

void Guide_ShowMap(void)
{
    sMap = AllocZeroed(sizeof(*sMap));
    if (sMap == NULL)
    {
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        return;
    }
    sMap->startEntry = gSpecialVar_0x8004;
    sMap->lastMapSec = MAPSEC_NONE;
    SetVBlankCallback(NULL);
    SetMainCallback2(CB2_GuideMapInit);
}

static void CB2_GuideMapInit(void)
{
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(1, sMapBgTemplates, ARRAY_COUNT(sMapBgTemplates));
    InitWindows(sMapWindowTemplates);
    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(0, MAP_BORDER_TILE, BG_PLTT_ID(13));
    ClearScheduledBgCopiesToVram();
    SetMainCallback2(CB2_GuideMap);
    SetVBlankCallback(VBlankCB_GuideMap);
}

static void VBlankCB_GuideMap(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

// Is the entry worth a marker, and which one.
static bool32 MarkerFor(u32 entry, u8 *kind)
{
    switch (Guide_GetStatus(entry))
    {
    case GUIDE_NEW:
        *kind = MARKER_NEW;
        return TRUE;
    case GUIDE_TODO:
        *kind = MARKER_TODO;
        return TRUE;
    default:
        return FALSE;
    }
}

static void SpriteCB_Marker(struct Sprite *sprite)
{
    // The NEW marker bobs; its animation swaps the two frames.
    if (sprite->animNum == MARKER_NEW)
        sprite->y2 = (sprite->animCmdIndex == 0) ? 0 : -1;
}

static void CreateMarkers(void)
{
    u16 sections[MAX_MARKERS];
    u8 kinds[MAX_MARKERS];
    u32 i, j, count = 0;

    LoadSpriteSheet(&sMarkerSheet);
    LoadSpritePalette(&sMarkerPalette);
    for (i = 0; i < gGuideEntryCount; i++)
    {
        u8 kind;
        u16 mapsec = gGuideEntries[i].mapsec;
        if (!MarkerFor(i, &kind) || gRegionMapEntries[mapsec].width == 0)
            continue;
        for (j = 0; j < count && sections[j] != mapsec; j++)
            ;
        if (j == count)
        {
            if (count == MAX_MARKERS)
                continue;
            sections[count] = mapsec;
            kinds[count++] = kind;
        }
        else if (kind == MARKER_NEW)
        {
            kinds[j] = MARKER_NEW;
        }
    }
    for (i = 0; i < count; i++)
    {
        const struct RegionMapLocation *loc = &gRegionMapEntries[sections[i]];
        // Just above the middle of the section, pointing down at it.
        s16 x = 8 * (loc->x + loc->width / 2 + 1) + 4;
        s16 y = 8 * (loc->y + loc->height / 2 + 2) - 1;
        u8 spriteId = CreateSprite(&sMarkerTemplate, x, y, 10); // behind the cursor
        if (spriteId != MAX_SPRITES)
            StartSpriteAnim(&gSprites[spriteId], kinds[i]);
    }
}

static u32 EntriesHere(u16 mapsec, u32 nth, u32 *entry)
{
    u32 i, n = 0;

    for (i = 0; i < gGuideEntryCount; i++)
    {
        if (gGuideEntries[i].mapsec != mapsec || Guide_GetStatus(i) == GUIDE_LOCKED)
            continue;
        if (n == nth)
            *entry = i;
        n++;
    }
    return n;
}

static void PrintInfo(void)
{
    static const u8 sColorsHeader[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
    u8 text[48];
    u16 mapsec = sMap->regionMap.mapSecId;
    u32 entry = 0, count;
    const u8 *colors;

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    if (sMap->regionMap.mapSecType == MAPSECTYPE_NONE)
    {
        AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 0, 1, sColorsFaint, TEXT_SKIP_DRAW,
                                     COMPOUND_STRING("Markers show where something waits."));
    }
    else
    {
        count = EntriesHere(mapsec, sMap->shown, &entry);
        if (count == 0)
        {
            AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 0, 1, sColorsHeader, TEXT_SKIP_DRAW, sMap->regionMap.mapSecName);
        }
        else
        {
            u32 x;
            enum GuideStatus status = Guide_GetStatus(entry);
            Guide_MarkSeen(entry);
            AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 0, 1, sColorsHeader, TEXT_SKIP_DRAW, gGuideEntries[entry].name);
            x = GetStringWidth(FONT_NORMAL, gGuideEntries[entry].name, 0) + 6;
            StatusTag(entry, status, text, &colors);
            AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, x, 3, colors, TEXT_SKIP_DRAW, text);
            if (count > 1)
            {
                ConvertIntToDecimalStringN(gStringVar1, sMap->shown + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
                ConvertIntToDecimalStringN(gStringVar2, count, STR_CONV_MODE_LEFT_ALIGN, 2);
                StringExpandPlaceholders(text, COMPOUND_STRING("{A_BUTTON} {STR_VAR_1}/{STR_VAR_2}"));
                x = 28 * 8 - GetStringWidth(FONT_SMALL, text, 0) - 2;
                AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, x, 3, sColorsNormal, TEXT_SKIP_DRAW, text);
            }
        }
    }
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

static void PrintTitle(void)
{
    static const u8 sColorsTitle[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
    const u8 *title = COMPOUND_STRING("GUIDE");

    FillWindowPixelBuffer(WIN_TITLE, PIXEL_FILL(1));
    AddTextPrinterParameterized3(WIN_TITLE, FONT_NORMAL, GetStringCenterAlignXOffset(FONT_NORMAL, title, 0x38), 1,
                                 sColorsTitle, TEXT_SKIP_DRAW, title);
    CopyWindowToVram(WIN_TITLE, COPYWIN_FULL);
}

static void UpdateGuideMap(void)
{
    switch (sMap->state)
    {
    case 0:
        InitRegionMap(&sMap->regionMap, FALSE);
        CreateRegionMapPlayerIcon(TAG_PLAYER_ICON, TAG_PLAYER_ICON);
        CreateRegionMapCursor(TAG_CURSOR, TAG_CURSOR);
        if (sMap->startEntry < gGuideEntryCount)
            SetRegionMapCursorToMapSec(gGuideEntries[sMap->startEntry].mapsec);
        CreateMarkers();
        sMap->state++;
        break;
    case 1:
        DrawStdFrameWithCustomTileAndPalette(WIN_TITLE, FALSE, MAP_BORDER_TILE, 0xd);
        PrintTitle();
        DrawStdFrameWithCustomTileAndPalette(WIN_INFO, FALSE, MAP_BORDER_TILE, 0xd);
        // Start on the picked entry, if its section has several.
        if (sMap->startEntry < gGuideEntryCount)
        {
            u32 n, entry = 0;
            for (n = 0; n < EntriesHere(sMap->regionMap.mapSecId, 0, &entry); n++)
            {
                EntriesHere(sMap->regionMap.mapSecId, n, &entry);
                if (entry == sMap->startEntry)
                    sMap->shown = n;
            }
        }
        PrintInfo();
        ScheduleBgCopyTilemapToVram(0);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        sMap->state++;
        break;
    case 2:
        SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(0);
        ShowBg(2);
        sMap->state++;
        break;
    case 3:
        if (!gPaletteFade.active)
            sMap->state++;
        break;
    case 4:
        switch (DoRegionMapInputCallback())
        {
        case MAP_INPUT_MOVE_END:
            if (sMap->regionMap.mapSecId != sMap->lastMapSec)
            {
                sMap->lastMapSec = sMap->regionMap.mapSecId;
                sMap->shown = 0;
                PrintInfo();
            }
            break;
        case MAP_INPUT_A_BUTTON:
        {
            u32 entry = 0, count = EntriesHere(sMap->regionMap.mapSecId, 0, &entry);
            if (count > 1)
            {
                PlaySE(SE_SELECT);
                sMap->shown = (sMap->shown + 1) % count;
                PrintInfo();
            }
            break;
        }
        case MAP_INPUT_B_BUTTON:
            PlaySE(SE_SELECT);
            sMap->state++;
            break;
        }
        break;
    case 5:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        sMap->state++;
        break;
    case 6:
        if (!gPaletteFade.active)
        {
            FreeRegionMapIconResources();
            ResetSpriteData();
            FreeAllSpritePalettes();
            FreeAllWindowBuffers();
            TRY_FREE_AND_SET_NULL(sMap);
            SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        }
        break;
    }
}

static void CB2_GuideMap(void)
{
    UpdateGuideMap();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}
