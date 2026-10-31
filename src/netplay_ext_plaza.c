#include "global.h"
#include "netplay_ext.h"
#include "bg.h"
#include "event_data.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "malloc.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The Online Plaza: one screen with the ten features of the second online
// batch. Picking one goes back to the field with VAR_RESULT = PLAZA_*, and
// NetplayExt_EventScript_Plaza runs that feature.

#define ICON_TAG   0x5E30
#define CURSOR_TAG 0x5E31
#define PLAZA_COLUMNS 5

enum
{
    WIN_TITLE,
    WIN_INFO,
};

struct PlazaEntry
{
    const u8 *name;
    const u8 *info;
};

static const struct PlazaEntry sEntries[PLAZA_COUNT] =
{
    [PLAZA_GUILDS - 1]   = {COMPOUND_STRING("GUILDS"), COMPOUND_STRING("Found a guild with a tag, join one, and\nclimb the guild ladder together.")},
    [PLAZA_WONDER - 1]   = {COMPOUND_STRING("WONDER TRADE"), COMPOUND_STRING("Send a POKéMON blind and get a random\none back from another player.")},
    [PLAZA_BASES - 1]    = {COMPOUND_STRING("SECRET BASES"), COMPOUND_STRING("Share your base, visit friends' bases\nand battle their teams, even offline.")},
    [PLAZA_TOWER - 1]    = {COMPOUND_STRING("BATTLE TOWER"), COMPOUND_STRING("Climb floor after floor against other\nplayers' teams. One loss ends the run.")},
    [PLAZA_BOUNTIES - 1] = {COMPOUND_STRING("BOUNTIES"), COMPOUND_STRING("Post a reward for a POKéMON you want,\nor catch one and claim a reward.")},
    [PLAZA_TITLES - 1]   = {COMPOUND_STRING("TITLES"), COMPOUND_STRING("Pick the title and badge shown above\nyour head. Earn more on the ladder.")},
    [PLAZA_REPLAYS - 1]  = {COMPOUND_STRING("REPLAYS"), COMPOUND_STRING("Watch your past online battles again,\nturn by turn.")},
    [PLAZA_DRAFT - 1]    = {COMPOUND_STRING("DRAFT BATTLE"), COMPOUND_STRING("Everyone drafts from the same pool.\nPick three and battle a fair match.")},
    [PLAZA_MAIL - 1]     = {COMPOUND_STRING("MAIL"), COMPOUND_STRING("Send mail with money or an item to a\nfriend, and open what they sent you.")},
    [PLAZA_DEX - 1]      = {COMPOUND_STRING("POKéDEX RACE"), COMPOUND_STRING("Who will complete the POKéDEX first?\nSee where you stand.")},
};

extern const u32 gNpxIconsGfx[];
extern const u16 gNpxIconsPal[];
extern const u32 gNpxPlazaTiles[];
extern const u32 gNpxPlazaTilemap[];
extern const u16 gNpxPlazaPal[];
extern const u32 gNpxCursorGfx[];
extern const u16 gNpxCursorPal[];
extern const u32 gNpxPlazaTilesSize;

bool32 NetplayExt_NewMailWaiting(void);

struct Plaza
{
    u8 cursor;
    u8 iconSprites[PLAZA_COUNT];
    u8 cursorSprite;
    u8 tilemap[BG_SCREEN_SIZE];
};

static EWRAM_DATA struct Plaza *sPlaza = NULL;

static const struct BgTemplate sBgTemplates[] =
{
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .priority = 0},
    {.bg = 3, .charBaseIndex = 2, .mapBaseIndex = 30, .priority = 3},
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_TITLE] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 0, .width = 28, .height = 3, .paletteNum = 15, .baseBlock = 0x001},
    [WIN_INFO]  = {.bg = 0, .tilemapLeft = 2, .tilemapTop = 14, .width = 26, .height = 5, .paletteNum = 15, .baseBlock = 0x055},
    DUMMY_WIN_TEMPLATE,
};

static const struct OamData sOam_32x32 =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
};

static const struct OamData sOam_Cursor =
{
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
    .priority = 2,
};

static const union AnimCmd sAnim_Cursor[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_FRAME(64, 16),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sAnims_Cursor[] = {sAnim_Cursor};

#define ICON_ANIM(n) static const union AnimCmd sAnim_Icon##n[] = {ANIMCMD_FRAME(n * 16, 1), ANIMCMD_END}
ICON_ANIM(0); ICON_ANIM(1); ICON_ANIM(2); ICON_ANIM(3); ICON_ANIM(4);
ICON_ANIM(5); ICON_ANIM(6); ICON_ANIM(7); ICON_ANIM(8); ICON_ANIM(9);
#undef ICON_ANIM

static const union AnimCmd *const sAnims_Icon[PLAZA_COUNT] =
{
    sAnim_Icon0, sAnim_Icon1, sAnim_Icon2, sAnim_Icon3, sAnim_Icon4,
    sAnim_Icon5, sAnim_Icon6, sAnim_Icon7, sAnim_Icon8, sAnim_Icon9,
};

static const struct SpriteTemplate sIconTemplate =
{
    .tileTag = ICON_TAG,
    .paletteTag = ICON_TAG,
    .oam = &sOam_32x32,
    .anims = sAnims_Icon,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sCursorTemplate =
{
    .tileTag = CURSOR_TAG,
    .paletteTag = CURSOR_TAG,
    .oam = &sOam_Cursor,
    .anims = sAnims_Cursor,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const u8 sTitleColors[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};
static const u8 sInfoColors[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static void CB2_InitPlaza(void);
static void Task_Plaza(u8 taskId);

static s16 IconX(u32 i) { return 32 + (i % PLAZA_COLUMNS) * 44; }
static s16 IconY(u32 i) { return 48 + (i / PLAZA_COLUMNS) * 42; }

static void Task_OpenPlaza(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        DestroyTask(taskId);
        SetMainCallback2(CB2_InitPlaza);
    }
}

void NetplayExt_OpenPlaza(void)
{
    LockPlayerFieldControls();
    FadeScreen(FADE_TO_BLACK, 0);
    CreateTask(Task_OpenPlaza, 10);
}

static void VBlankCB_Plaza(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Plaza(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void DrawInfo(void)
{
    const struct PlazaEntry *entry = &sEntries[sPlaza->cursor];

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
    AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 4, 2, sInfoColors, TEXT_SKIP_DRAW, entry->name);
    if (sPlaza->cursor == PLAZA_MAIL - 1 && NetplayExt_NewMailWaiting())
        AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 150, 2, sTitleColors, TEXT_SKIP_DRAW, COMPOUND_STRING("NEW MAIL!"));
    AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 4, 16, sInfoColors, TEXT_SKIP_DRAW, entry->info);
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

static void MoveCursor(void)
{
    struct Sprite *cursor = &gSprites[sPlaza->cursorSprite];
    u32 i;

    cursor->x = IconX(sPlaza->cursor);
    cursor->y = IconY(sPlaza->cursor);
    for (i = 0; i < PLAZA_COUNT; i++)
        gSprites[sPlaza->iconSprites[i]].y2 = 0;
    DrawInfo();
}

static void CB2_InitPlaza(void)
{
    u32 i;

    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        CpuFill32(0, (void *)VRAM, VRAM_SIZE);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(3, 0, BG_COORD_SET);
        ChangeBgY(3, 0, BG_COORD_SET);
        ResetPaletteFade();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ScanlineEffect_Stop();
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        sPlaza = AllocZeroed(sizeof(*sPlaza));
        gMain.state++;
        break;
    case 1:
        LoadBgTiles(3, gNpxPlazaTiles, gNpxPlazaTilesSize, 0);
        memcpy(sPlaza->tilemap, gNpxPlazaTilemap, BG_SCREEN_SIZE);
        SetBgTilemapBuffer(3, sPlaza->tilemap);
        CopyBgTilemapBufferToVram(3);
        LoadPalette(gNpxPlazaPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 2:
    {
        struct SpriteSheet icons = {(const u8 *)gNpxIconsGfx, PLAZA_COUNT * 0x200, ICON_TAG};
        struct SpritePalette iconPal = {gNpxIconsPal, ICON_TAG};
        struct SpriteSheet cursor = {(const u8 *)gNpxCursorGfx, 2 * 0x800, CURSOR_TAG};
        struct SpritePalette cursorPal = {gNpxCursorPal, CURSOR_TAG};

        LoadSpriteSheet(&icons);
        LoadSpritePalette(&iconPal);
        LoadSpriteSheet(&cursor);
        LoadSpritePalette(&cursorPal);
        for (i = 0; i < PLAZA_COUNT; i++)
        {
            sPlaza->iconSprites[i] = CreateSprite(&sIconTemplate, IconX(i), IconY(i), 1);
            StartSpriteAnim(&gSprites[sPlaza->iconSprites[i]], i);
        }
        sPlaza->cursorSprite = CreateSprite(&sCursorTemplate, IconX(0), IconY(0), 2);
        gMain.state++;
        break;
    }
    case 3:
        FillWindowPixelBuffer(WIN_TITLE, PIXEL_FILL(0));
        AddTextPrinterParameterized3(WIN_TITLE, FONT_NORMAL, 4, 4, sTitleColors, TEXT_SKIP_DRAW, COMPOUND_STRING("ONLINE PLAZA"));
        AddTextPrinterParameterized3(WIN_TITLE, FONT_SMALL, 150, 6, sTitleColors, TEXT_SKIP_DRAW, COMPOUND_STRING("{A_BUTTON} OPEN  {B_BUTTON} EXIT"));
        PutWindowTilemap(WIN_TITLE);
        CopyWindowToVram(WIN_TITLE, COPYWIN_FULL);
        PutWindowTilemap(WIN_INFO);
        sPlaza->cursor = 0;
        MoveCursor();
        gMain.state++;
        break;
    case 4:
        ShowBg(0);
        ShowBg(3);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_BG0_ON | DISPCNT_BG3_ON | DISPCNT_OBJ_1D_MAP);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Plaza);
        CreateTask(Task_Plaza, 0);
        SetMainCallback2(CB2_Plaza);
        gMain.state = 0;
        break;
    }
}

#define tState data[0]
#define tTimer data[1]

static void ClosePlaza(u8 taskId, u16 result)
{
    gSpecialVar_Result = result;
    gTasks[taskId].tState = 1;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

static void Task_Plaza(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s32 cursor = sPlaza->cursor;

    if (task->tState == 1)
    {
        if (!gPaletteFade.active)
        {
            FreeAllWindowBuffers();
            FREE_AND_SET_NULL(sPlaza);
            DestroyTask(taskId);
            SetMainCallback2(CB2_ReturnToFieldContinueScript);
        }
        return;
    }
    if (gPaletteFade.active)
        return;

    // The picked icon bobs up and down.
    task->tTimer++;
    gSprites[sPlaza->iconSprites[cursor]].y2 = (task->tTimer / 8) % 2 ? -1 : 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        ClosePlaza(taskId, cursor + 1);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        ClosePlaza(taskId, PLAZA_EXIT);
    }
    else
    {
        if (JOY_REPEAT(DPAD_LEFT))
            cursor = (cursor % PLAZA_COLUMNS == 0) ? cursor + PLAZA_COLUMNS - 1 : cursor - 1;
        else if (JOY_REPEAT(DPAD_RIGHT))
            cursor = (cursor % PLAZA_COLUMNS == PLAZA_COLUMNS - 1) ? cursor - PLAZA_COLUMNS + 1 : cursor + 1;
        else if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
            cursor = (cursor + PLAZA_COLUMNS) % PLAZA_COUNT;
        if (cursor != sPlaza->cursor)
        {
            PlaySE(SE_SELECT);
            sPlaza->cursor = cursor;
            MoveCursor();
        }
    }
}

#undef tState
#undef tTimer
