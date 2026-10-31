#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "item.h"
#include "item_icon.h"
#include "life.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "overworld.h"
#include "palette.h"
#include "random.h"
#include "rtc.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Underground mining (docs/life.md): glittering rocks in Hoenn's caves can
// be dug into once a day, after the digger in Rusturf Tunnel hands over
// his tools. The wall is a 13x6 grid of rock, one to three layers deep,
// with two to four treasures buried under it. The pick (A) digs two layers
// where it hits and one around it; the hammer (B) digs a 3x3 area but
// shakes the wall more. Every hit adds cracks, and when the wall gives way
// the dig ends: treasures that were fully uncovered go in the bag.

#define COLS 13
#define ROWS 6
#define CELL 16
#define WALL_X 16 // pixels
#define WALL_Y 24
#define MAX_TREASURES 4
#define CRACK_MAX 70
#define CRACK_PICK 2
#define CRACK_HAMMER 4

#define TAG_CURSOR   0x5B00
#define TAG_TREASURE 0x5B10 // + treasure index, for tiles and palettes

// Tiles in graphics/life/mine_tiles.png, after the blank tile 0.
#define TILE_EARTH 1
#define TILE_BLOCK(layer, variant) (2 + ((layer) - 1) * 8 + (variant) * 4)

enum
{
    WIN_TOP,
    WIN_BOTTOM,
};

static const struct BgTemplate sBgTemplates[] =
{
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .priority = 0},
    {.bg = 1, .charBaseIndex = 2, .mapBaseIndex = 30, .priority = 1},
    {.bg = 2, .charBaseIndex = 2, .mapBaseIndex = 29, .priority = 3},
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_TOP] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 0, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 0x001},
    [WIN_BOTTOM] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 16, .width = 28, .height = 4, .paletteNum = 15, .baseBlock = 0x040},
    DUMMY_WIN_TEMPLATE
};

static const u32 sMineTiles[] = INCBIN_U32("graphics/life/mine_tiles.4bpp");
static const u16 sMinePal[] = INCBIN_U16("graphics/life/mine_tiles.gbapal");
static const u32 sCursorGfx[] = INCBIN_U32("graphics/life/mine_cursor.4bpp");
static const u16 sCursorPal[] = INCBIN_U16("graphics/life/mine_cursor.gbapal");

static const struct OamData sCursorOam = {.size = SPRITE_SIZE(16x16), .shape = SPRITE_SHAPE(16x16), .priority = 0};
static const struct SpriteTemplate sCursorTemplate =
{
    .tileTag = TAG_CURSOR, .paletteTag = TAG_CURSOR, .oam = &sCursorOam,
    .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

// Treasures are the items' own icons, scaled up for the 3x3 ones.
static const struct OamData sTreasureOam =
{
    .affineMode = ST_OAM_AFFINE_DOUBLE, .size = SPRITE_SIZE(32x32), .shape = SPRITE_SHAPE(32x32), .priority = 2,
};
static const union AffineAnimCmd sAffineSmall[] = {AFFINEANIMCMD_FRAME(0x140, 0x140, 0, 0), AFFINEANIMCMD_END};
static const union AffineAnimCmd sAffineBig[] = {AFFINEANIMCMD_FRAME(0x1E0, 0x1E0, 0, 0), AFFINEANIMCMD_END};
static const union AffineAnimCmd *const sTreasureAffineAnims[] = {sAffineSmall, sAffineBig};
static const struct SpriteTemplate sTreasureTemplate =
{
    .tileTag = TAG_TREASURE, .paletteTag = TAG_TREASURE, .oam = &sTreasureOam,
    .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = sTreasureAffineAnims,
    .callback = SpriteCallbackDummy,
};

struct Treasure
{
    u16 item;
    u8 size;   // 2 or 3 cells square
    u8 weight; // out of the table's total
};

// Fossils are the two the Devon lab in Rustboro can revive.
static const struct Treasure sTreasures[] =
{
    {ITEM_RED_SHARD, 2, 14}, {ITEM_BLUE_SHARD, 2, 14}, {ITEM_YELLOW_SHARD, 2, 14}, {ITEM_GREEN_SHARD, 2, 14},
    {ITEM_HARD_STONE, 2, 10}, {ITEM_EVERSTONE, 2, 10}, {ITEM_HEART_SCALE, 2, 12}, {ITEM_STAR_PIECE, 2, 6},
    {ITEM_FIRE_STONE, 2, 5}, {ITEM_WATER_STONE, 2, 5}, {ITEM_THUNDER_STONE, 2, 5}, {ITEM_LEAF_STONE, 2, 5},
    {ITEM_MOON_STONE, 2, 4}, {ITEM_SUN_STONE, 2, 4}, {ITEM_ICE_STONE, 2, 3}, {ITEM_DUSK_STONE, 2, 2},
    {ITEM_DAWN_STONE, 2, 2}, {ITEM_SHINY_STONE, 2, 2}, {ITEM_OVAL_STONE, 2, 4},
    {ITEM_FIRE_GEM, 2, 3}, {ITEM_WATER_GEM, 2, 3}, {ITEM_ELECTRIC_GEM, 2, 3}, {ITEM_GRASS_GEM, 2, 3},
    {ITEM_ROOT_FOSSIL, 3, 3}, {ITEM_CLAW_FOSSIL, 3, 3}, {ITEM_RARE_BONE, 3, 3}, {ITEM_NUGGET, 2, 3},
};

struct Buried
{
    u16 item;
    u8 x, y, size;
    u8 spriteId;
    bool8 found;
};

// The screenshot driver reads the start of this (tools/life_shots/mining.lua).
struct Mine
{
    struct Buried buried[MAX_TREASURES];
    u8 count;
    u8 crack;
    u8 depth[ROWS][COLS];
    u8 variant[ROWS][COLS];
    u16 tilemap1[BG_SCREEN_SIZE / 2];
    u16 tilemap2[BG_SCREEN_SIZE / 2];
    s8 cx, cy;
    u8 cursorSpriteId;
    u8 shake;
};

EWRAM_DATA struct Mine *gLifeMine = NULL; // the screenshot driver reads it

static void Task_Mine(u8 taskId);
static void Task_MineResults(u8 taskId);
static void Task_MineExit(u8 taskId);

static const u8 sTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

// --- The wall -----------------------------------------------------------------

static void DrawCell(u32 x, u32 y)
{
    u32 tx = WALL_X / 8 + x * 2, ty = WALL_Y / 8 + y * 2;
    u32 depth = gLifeMine->depth[y][x];
    u32 i;

    for (i = 0; i < 4; i++)
    {
        u32 pos = (ty + i / 2) * 32 + tx + i % 2;
        gLifeMine->tilemap1[pos] = depth == 0 ? 0 : (TILE_BLOCK(depth, gLifeMine->variant[y][x]) + i) | (2 << 12);
    }
}

static void DrawWall(void)
{
    u32 x, y;

    for (y = 0; y < 20; y++)
    {
        for (x = 0; x < 32; x++)
            gLifeMine->tilemap2[y * 32 + x] = TILE_EARTH | (2 << 12);
    }
    for (y = 0; y < ROWS; y++)
    {
        for (x = 0; x < COLS; x++)
            DrawCell(x, y);
    }
    ScheduleBgCopyTilemapToVram(1);
    ScheduleBgCopyTilemapToVram(2);
}

static bool32 Overlaps(u32 x, u32 y, u32 size)
{
    u32 i;
    for (i = 0; i < gLifeMine->count; i++)
    {
        const struct Buried *b = &gLifeMine->buried[i];
        if (x < b->x + b->size && b->x < x + size && y < b->y + b->size && b->y < y + size)
            return TRUE;
    }
    return FALSE;
}

static u32 PickTreasure(void)
{
    u32 total = 0, i, roll;
    for (i = 0; i < ARRAY_COUNT(sTreasures); i++)
        total += sTreasures[i].weight;
    roll = Random() % total;
    for (i = 0; i < ARRAY_COUNT(sTreasures) - 1; i++)
    {
        if (roll < sTreasures[i].weight)
            break;
        roll -= sTreasures[i].weight;
    }
    return i;
}

static void GenerateWall(void)
{
    u32 x, y, i, tries, want;

    // Rock: mostly two layers, with harder and softer patches.
    for (y = 0; y < ROWS; y++)
    {
        for (x = 0; x < COLS; x++)
        {
            gLifeMine->depth[y][x] = 1 + Random() % 3;
            gLifeMine->variant[y][x] = Random() & 1;
        }
    }
    for (i = 0; i < 2; i++)
    {
        u8 smooth[ROWS][COLS];
        for (y = 0; y < ROWS; y++)
        {
            for (x = 0; x < COLS; x++)
            {
                u32 sum = gLifeMine->depth[y][x] * 2, n = 2;
                if (x > 0) sum += gLifeMine->depth[y][x - 1], n++;
                if (x < COLS - 1) sum += gLifeMine->depth[y][x + 1], n++;
                if (y > 0) sum += gLifeMine->depth[y - 1][x], n++;
                if (y < ROWS - 1) sum += gLifeMine->depth[y + 1][x], n++;
                smooth[y][x] = (sum + n / 2) / n;
            }
        }
        memcpy(gLifeMine->depth, smooth, sizeof(smooth));
    }

    want = 2 + Random() % 3;
    gLifeMine->count = 0;
    for (tries = 0; tries < 60 && gLifeMine->count < want; tries++)
    {
        const struct Treasure *t = &sTreasures[PickTreasure()];
        x = Random() % (COLS - t->size + 1);
        y = Random() % (ROWS - t->size + 1);
        if (Overlaps(x, y, t->size))
            continue;
        gLifeMine->buried[gLifeMine->count].item = t->item;
        gLifeMine->buried[gLifeMine->count].x = x;
        gLifeMine->buried[gLifeMine->count].y = y;
        gLifeMine->buried[gLifeMine->count].size = t->size;
        gLifeMine->buried[gLifeMine->count].found = FALSE;
        gLifeMine->count++;
    }
}

static void CreateTreasureSprites(void)
{
    u32 i;
    for (i = 0; i < gLifeMine->count; i++)
    {
        struct Buried *b = &gLifeMine->buried[i];
        u8 spriteId = AddCustomItemIconSprite(&sTreasureTemplate, TAG_TREASURE + i, TAG_TREASURE + i, b->item);
        struct Sprite *sprite = &gSprites[spriteId];

        b->spriteId = spriteId;
        sprite->x = WALL_X + b->x * CELL + b->size * CELL / 2;
        sprite->y = WALL_Y + b->y * CELL + b->size * CELL / 2;
        StartSpriteAffineAnim(sprite, b->size == 3 ? 1 : 0);
    }
}

// --- Text ----------------------------------------------------------------------

static u32 FoundCount(void)
{
    u32 i, n = 0;
    for (i = 0; i < gLifeMine->count; i++)
        n += gLifeMine->buried[i].found;
    return n;
}

static void PrintTop(void)
{
    u32 width = 120, filled = width * gLifeMine->crack / CRACK_MAX;

    FillWindowPixelBuffer(WIN_TOP, PIXEL_FILL(1));
    AddTextPrinterParameterized3(WIN_TOP, FONT_SMALL, 2, 2, sTextColors, TEXT_SKIP_DRAW, COMPOUND_STRING("WALL"));
    FillWindowPixelRect(WIN_TOP, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), 30, 4, width + 2, 8);
    FillWindowPixelRect(WIN_TOP, PIXEL_FILL(TEXT_COLOR_GREEN), 31, 5, width, 6);
    if (filled != 0)
        FillWindowPixelRect(WIN_TOP, PIXEL_FILL(TEXT_COLOR_RED), 31, 5, filled, 6);
    ConvertIntToDecimalStringN(gStringVar1, FoundCount(), STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertIntToDecimalStringN(gStringVar2, gLifeMine->count, STR_CONV_MODE_LEFT_ALIGN, 1);
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("FOUND {STR_VAR_1}/{STR_VAR_2}"));
    AddTextPrinterParameterized3(WIN_TOP, FONT_SMALL, 168, 2, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
    CopyWindowToVram(WIN_TOP, COPYWIN_FULL);
}

static void PrintBottom(const u8 *text)
{
    FillWindowPixelBuffer(WIN_BOTTOM, PIXEL_FILL(1));
    StringExpandPlaceholders(gStringVar4, text);
    AddTextPrinterParameterized3(WIN_BOTTOM, FONT_NORMAL, 2, 1, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
    CopyWindowToVram(WIN_BOTTOM, COPYWIN_FULL);
}

static void PrintHelp(void)
{
    PrintBottom(COMPOUND_STRING("A: PICK  B: HAMMER  START: STOP\nUncover the treasures before it falls!"));
}

// --- Digging -------------------------------------------------------------------

static void DigCell(s32 x, s32 y, u32 amount)
{
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS)
        return;
    gLifeMine->depth[y][x] = gLifeMine->depth[y][x] > amount ? gLifeMine->depth[y][x] - amount : 0;
    DrawCell(x, y);
}

static bool32 IsUncovered(const struct Buried *b)
{
    u32 x, y;
    for (y = b->y; y < b->y + b->size; y++)
    {
        for (x = b->x; x < b->x + b->size; x++)
        {
            if (gLifeMine->depth[y][x] != 0)
                return FALSE;
        }
    }
    return TRUE;
}

static void CheckFinds(void)
{
    u32 i;
    for (i = 0; i < gLifeMine->count; i++)
    {
        struct Buried *b = &gLifeMine->buried[i];
        if (b->found || !IsUncovered(b))
            continue;
        b->found = TRUE;
        gSprites[b->spriteId].oam.priority = 0;
        PlaySE(SE_RG_POKE_JUMP_SUCCESS);
        CopyItemName(b->item, gStringVar1);
        PrintBottom(COMPOUND_STRING("You dug up {STR_VAR_1}!"));
    }
}

static void Hit(bool32 hammer)
{
    s32 x = gLifeMine->cx, y = gLifeMine->cy, dx, dy;

    if (hammer)
    {
        for (dy = -1; dy <= 1; dy++)
        {
            for (dx = -1; dx <= 1; dx++)
                DigCell(x + dx, y + dy, (dx == 0 && dy == 0) ? 2 : 1);
        }
        gLifeMine->crack += CRACK_HAMMER;
        gLifeMine->shake = 8;
        PlaySE(SE_M_ROCK_THROW);
    }
    else
    {
        DigCell(x, y, 2);
        DigCell(x - 1, y, 1);
        DigCell(x + 1, y, 1);
        DigCell(x, y - 1, 1);
        DigCell(x, y + 1, 1);
        gLifeMine->crack += CRACK_PICK;
        gLifeMine->shake = 4;
        PlaySE(SE_BREAKABLE_DOOR);
    }
    if (gLifeMine->crack > CRACK_MAX)
        gLifeMine->crack = CRACK_MAX;
    ScheduleBgCopyTilemapToVram(1);
    CheckFinds();
    PrintTop();
}

static void MoveCursor(void)
{
    struct Sprite *cursor = &gSprites[gLifeMine->cursorSpriteId];
    cursor->x = WALL_X + gLifeMine->cx * CELL + CELL / 2;
    cursor->y = WALL_Y + gLifeMine->cy * CELL + CELL / 2;
}

// Puts what was found in the bag, and its names in gStringVar4.
static void TakeFinds(void)
{
    u32 i, n = 0;
    u8 *end = gStringVar4;

    *end = EOS;
    for (i = 0; i < gLifeMine->count; i++)
    {
        if (!gLifeMine->buried[i].found)
            continue;
        AddBagItem(gLifeMine->buried[i].item, 1);
        if (n != 0)
            end = StringAppend(end, n % 2 ? COMPOUND_STRING(", ") : COMPOUND_STRING(",\n"));
        end = CopyItemName(gLifeMine->buried[i].item, end);
        n++;
    }
    gSaveBlock3Ptr->life.treasuresDug += n;
    gSpecialVar_Result = n;
}

static void Task_Mine(u8 taskId)
{
    if (gLifeMine->shake != 0)
    {
        gLifeMine->shake--;
        SetGpuReg(REG_OFFSET_BG1HOFS, (gLifeMine->shake & 2) ? 1 : 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, (gLifeMine->shake & 2) ? 1 : 0);
    }
    if (gPaletteFade.active)
        return;

    if (JOY_NEW(START_BUTTON) || gLifeMine->crack >= CRACK_MAX || FoundCount() == gLifeMine->count)
    {
        if (gLifeMine->crack >= CRACK_MAX)
        {
            PlaySE(SE_M_EARTHQUAKE);
            gLifeMine->shake = 30;
        }
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].func = Task_MineResults;
        return;
    }
    if (JOY_REPEAT(DPAD_LEFT) && gLifeMine->cx > 0)
        gLifeMine->cx--;
    else if (JOY_REPEAT(DPAD_RIGHT) && gLifeMine->cx < COLS - 1)
        gLifeMine->cx++;
    else if (JOY_REPEAT(DPAD_UP) && gLifeMine->cy > 0)
        gLifeMine->cy--;
    else if (JOY_REPEAT(DPAD_DOWN) && gLifeMine->cy < ROWS - 1)
        gLifeMine->cy++;
    MoveCursor();
    if (JOY_NEW(A_BUTTON))
        Hit(FALSE);
    else if (JOY_NEW(B_BUTTON))
        Hit(TRUE);
}

static void Task_MineResults(u8 taskId)
{
    s16 *timer = &gTasks[taskId].data[0];

    if (gLifeMine->shake != 0)
    {
        gLifeMine->shake--;
        SetGpuReg(REG_OFFSET_BG1VOFS, (gLifeMine->shake & 2) ? 2 : 0);
        return;
    }
    if (*timer == 0)
    {
        u32 found = FoundCount();
        gSprites[gLifeMine->cursorSpriteId].invisible = TRUE;
        if (found == 0)
        {
            PrintBottom(gLifeMine->crack >= CRACK_MAX ? COMPOUND_STRING("The wall gave way…\nNothing was dug up.")
                                                  : COMPOUND_STRING("You stopped digging.\nNothing was dug up."));
            TakeFinds();
        }
        else
        {
            TakeFinds();
            if (found == gLifeMine->count)
                PlayFanfare(MUS_OBTAIN_ITEM);
            FillWindowPixelBuffer(WIN_BOTTOM, PIXEL_FILL(1));
            AddTextPrinterParameterized3(WIN_BOTTOM, FONT_SMALL, 2, 0, sTextColors, TEXT_SKIP_DRAW,
                                         found == gLifeMine->count ? COMPOUND_STRING("Everything dug up! In the bag:") : COMPOUND_STRING("In the bag:"));
            AddTextPrinterParameterized3(WIN_BOTTOM, FONT_SMALL, 2, 9, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
            CopyWindowToVram(WIN_BOTTOM, COPYWIN_FULL);
        }
    }
    if (++*timer > 30 && JOY_NEW(A_BUTTON | B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_MineExit;
    }
}

static void Task_MineExit(u8 taskId)
{
    if (gPaletteFade.active || IsFanfareTaskInactive() == FALSE)
        return;
    FreeAllWindowBuffers();
    Free(gLifeMine);
    gLifeMine = NULL;
    DestroyTask(taskId);
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

// --- Setup ---------------------------------------------------------------------

static void VBlankCB_Mine(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Mine(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void CB2_InitMine(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        ScanlineEffect_Stop();
        ResetPaletteFade();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ResetTasks();
        gMain.state++;
        break;
    case 1:
        ResetVramOamAndBgCntRegs();
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        SetBgTilemapBuffer(1, gLifeMine->tilemap1);
        SetBgTilemapBuffer(2, gLifeMine->tilemap2);
        ResetAllBgsCoordinates();
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        LoadUserWindowBorderGfx(WIN_TOP, 0x200, BG_PLTT_ID(14));
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 2:
        LoadBgTiles(1, sMineTiles, sizeof(sMineTiles), 1);
        LoadPalette(sMinePal, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
        LoadPalette(sMinePal, BG_PLTT_ID(0), PLTT_SIZE_4BPP); // backdrop: the outline colour
        GenerateWall();
        DrawWall();
        gMain.state++;
        break;
    case 3:
    {
        struct SpriteSheet sheet = {sCursorGfx, sizeof(sCursorGfx), TAG_CURSOR};
        struct SpritePalette pal = {sCursorPal, TAG_CURSOR};
        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&pal);
        CreateTreasureSprites();
        gLifeMine->cx = COLS / 2;
        gLifeMine->cy = ROWS / 2;
        gLifeMine->cursorSpriteId = CreateSprite(&sCursorTemplate, 0, 0, 0);
        MoveCursor();
        gMain.state++;
        break;
    }
    case 4:
        DrawStdFrameWithCustomTileAndPalette(WIN_TOP, FALSE, 0x200, 14);
        DrawStdFrameWithCustomTileAndPalette(WIN_BOTTOM, FALSE, 0x200, 14);
        PutWindowTilemap(WIN_TOP);
        PutWindowTilemap(WIN_BOTTOM);
        PrintTop();
        PrintHelp();
        ScheduleBgCopyTilemapToVram(0);
        gMain.state++;
        break;
    case 5:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Mine);
        CreateTask(Task_Mine, 0);
        SetMainCallback2(CB2_Mine);
        break;
    }
}

// --- Scripts ---------------------------------------------------------------------

static void ResetDailySpots(void)
{
    struct LifeSaveData *life = &gSaveBlock3Ptr->life;
    RtcCalcLocalTime();
    if (life->digDay != gLocalTime.days)
    {
        life->digDay = gLocalTime.days;
        life->digSpotsToday = 0;
    }
}

// VAR_0x8005: the dig spot. VAR_RESULT: LIFE_DIG_NO_KIT, LIFE_DIG_DONE_TODAY
// or LIFE_DIG_READY.
void LifeMining_CheckSpot(void)
{
    ResetDailySpots();
    if (!gSaveBlock3Ptr->life.hasDigKit)
        gSpecialVar_Result = LIFE_DIG_NO_KIT;
    else if (gSaveBlock3Ptr->life.digSpotsToday & (1 << gSpecialVar_0x8005))
        gSpecialVar_Result = LIFE_DIG_DONE_TODAY;
    else
        gSpecialVar_Result = LIFE_DIG_READY;
}

// Digs at spot VAR_0x8005. Back in the script, VAR_RESULT is the number of
// treasures found.
void LifeMining_Start(void)
{
    ResetDailySpots();
    gSaveBlock3Ptr->life.digSpotsToday |= 1 << gSpecialVar_0x8005;
    gLifeMine = AllocZeroed(sizeof(*gLifeMine));
    gMain.state = 0;
    SetMainCallback2(CB2_InitMine);
}

// The digger in Rusturf Tunnel. VAR_RESULT: TRUE if the player already had
// the tools.
void LifeMining_GiveKit(void)
{
    gSpecialVar_Result = gSaveBlock3Ptr->life.hasDigKit;
    gSaveBlock3Ptr->life.hasDigKit = TRUE;
    ConvertIntToDecimalStringN(gStringVar1, gSaveBlock3Ptr->life.treasuresDug, STR_CONV_MODE_LEFT_ALIGN, 5);
}
