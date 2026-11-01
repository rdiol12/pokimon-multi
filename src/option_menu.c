#include "global.h"
#include "option_menu.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "qol.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The OPTION menu. It scrolls: the usual six options, then the
// quality-of-life ones (src/qol.c), then CANCEL. CONTROLS opens a second list
// where each button's job can be moved to another button.

enum
{
    MENUITEM_TEXTSPEED,
    MENUITEM_BATTLESCENE,
    MENUITEM_BATTLESTYLE,
    MENUITEM_SOUND,
    MENUITEM_BUTTONMODE,
    MENUITEM_FRAMETYPE,
    MENUITEM_AUTORUN,
    MENUITEM_AUTOSAVE,
    MENUITEM_COLORS,
    MENUITEM_CONTROLS,
    MENUITEM_CANCEL,
    MENUITEM_COUNT,
};

// The CONTROLS list: one row per job (QOL_KEY_*), then USE DEFAULTS.
#define CONTROLS_ROW_DEFAULTS QOL_KEY_COUNT
#define CONTROLS_ROW_COUNT    (QOL_KEY_COUNT + 1)

#define VISIBLE_ROWS 7
#define VALUE_X      104
#define VALUE_RIGHT  198

#define TAG_SCROLL_ARROWS 0x1E40

enum
{
    WIN_HEADER,
    WIN_OPTIONS
};

static EWRAM_DATA struct
{
    u8 values[MENUITEM_COUNT];
    u8 keyMap[QOL_KEY_COUNT];
    u8 selection;
    u8 controlsSelection;
    bool8 onControls;
    u8 arrowTaskId;
    u16 scroll;
} sOptions = {0};

static void Task_OptionMenuFadeIn(u8 taskId);
static void Task_OptionMenuProcessInput(u8 taskId);
static void Task_OptionMenuSave(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
static void HighlightOptionMenuItem(u8 row);
static void DrawHeaderText(void);
static void DrawOptionRows(void);
static void DrawBgWindowFrames(void);
static void LoadOptions(void);
static void UpdateScrollArrows(void);

#define GREEN(str) "{COLOR GREEN}{SHADOW LIGHT_GREEN}" str

static const u8 gText_Option[]             = _("OPTION");
static const u8 gText_Controls[]           = _("CONTROLS");
static const u8 gText_BattleSceneOn[]      = _(GREEN("ON"));
static const u8 gText_BattleSceneOff[]     = _(GREEN("OFF"));
static const u8 gText_BattleStyleShift[]   = _(GREEN("SHIFT"));
static const u8 gText_BattleStyleSet[]     = _(GREEN("SET"));
static const u8 gText_SoundMono[]          = _(GREEN("MONO"));
static const u8 gText_SoundStereo[]        = _(GREEN("STEREO"));
static const u8 gText_FrameType[]          = _(GREEN("TYPE"));
static const u8 gText_FrameTypeNumber[]    = _(GREEN(""));
static const u8 gText_ButtonTypeNormal[]   = _(GREEN("NORMAL"));
static const u8 gText_ButtonTypeLR[]       = _(GREEN("LR"));
static const u8 gText_ButtonTypeLEqualsA[] = _(GREEN("L=A"));
static const u8 gText_UseDefaults[]        = _(GREEN("PRESS {A_BUTTON}"));

static const u8 *const sTextSpeedNames[] =
{
    [OPTIONS_TEXT_SPEED_SLOW]    = COMPOUND_STRING(GREEN("SLOW")),
    [OPTIONS_TEXT_SPEED_MID]     = COMPOUND_STRING(GREEN("MID")),
    [OPTIONS_TEXT_SPEED_FAST]    = COMPOUND_STRING(GREEN("FAST")),
    [OPTIONS_TEXT_SPEED_INSTANT] = COMPOUND_STRING(GREEN("INSTANT")),
};

static const u8 *const sColorModeNames[COLOR_MODE_COUNT] =
{
    [COLOR_MODE_NORMAL]     = COMPOUND_STRING(GREEN("NORMAL")),
    [COLOR_MODE_RED_WEAK]   = COMPOUND_STRING(GREEN("RED-WEAK")),
    [COLOR_MODE_GREEN_WEAK] = COMPOUND_STRING(GREEN("GREEN-WEAK")),
    [COLOR_MODE_BLUE_WEAK]  = COMPOUND_STRING(GREEN("BLUE-WEAK")),
};

static const u8 *const sKeyNames[QOL_KEY_COUNT] =
{
    [QOL_KEY_A]      = COMPOUND_STRING(GREEN("A")),
    [QOL_KEY_B]      = COMPOUND_STRING(GREEN("B")),
    [QOL_KEY_START]  = COMPOUND_STRING(GREEN("START")),
    [QOL_KEY_SELECT] = COMPOUND_STRING(GREEN("SELECT")),
    [QOL_KEY_L]      = COMPOUND_STRING(GREEN("L")),
    [QOL_KEY_R]      = COMPOUND_STRING(GREEN("R")),
};

static const u8 *const sJobNames[CONTROLS_ROW_COUNT] =
{
    [QOL_KEY_A]             = COMPOUND_STRING("A BUTTON"),
    [QOL_KEY_B]             = COMPOUND_STRING("B BUTTON"),
    [QOL_KEY_START]         = COMPOUND_STRING("START"),
    [QOL_KEY_SELECT]        = COMPOUND_STRING("SELECT"),
    [QOL_KEY_L]             = COMPOUND_STRING("L BUTTON"),
    [QOL_KEY_R]             = COMPOUND_STRING("R BUTTON"),
    [CONTROLS_ROW_DEFAULTS] = COMPOUND_STRING("USE DEFAULTS"),
};

// Shown in the header while the cursor is on these rows.
static const u8 *const sOptionTips[MENUITEM_COUNT] =
{
    [MENUITEM_TEXTSPEED] = COMPOUND_STRING("INSTANT shows text all at once."),
    [MENUITEM_AUTORUN]   = COMPOUND_STRING("Run without B. Hold B to walk."),
    [MENUITEM_AUTOSAVE]  = COMPOUND_STRING("Save when you enter a new area."),
    [MENUITEM_COLORS]    = COMPOUND_STRING("Colors for color-blind players."),
    [MENUITEM_CONTROLS]  = COMPOUND_STRING("Choose which button does what."),
};

static const u16 sOptionMenuText_Pal[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCGFX_U8("graphics/interface/option_menu_equals_sign.png", ".4bpp");

static const u8 *const sOptionMenuItemsNames[MENUITEM_COUNT] =
{
    [MENUITEM_TEXTSPEED]   = COMPOUND_STRING("TEXT SPEED"),
    [MENUITEM_BATTLESCENE] = COMPOUND_STRING("BATTLE SCENE"),
    [MENUITEM_BATTLESTYLE] = COMPOUND_STRING("BATTLE STYLE"),
    [MENUITEM_SOUND]       = COMPOUND_STRING("SOUND"),
    [MENUITEM_BUTTONMODE]  = COMPOUND_STRING("BUTTON MODE"),
    [MENUITEM_FRAMETYPE]   = COMPOUND_STRING("FRAME"),
    [MENUITEM_AUTORUN]     = COMPOUND_STRING("AUTO-RUN"),
    [MENUITEM_AUTOSAVE]    = COMPOUND_STRING("AUTOSAVE"),
    [MENUITEM_COLORS]      = COMPOUND_STRING("COLORS"),
    [MENUITEM_CONTROLS]    = COMPOUND_STRING("CONTROLS"),
    [MENUITEM_CANCEL]      = COMPOUND_STRING("CANCEL"),
};

static const struct WindowTemplate sOptionMenuWinTemplates[] =
{
    [WIN_HEADER] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [WIN_OPTIONS] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 0x36
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOptionMenuBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_InitOptionMenu(void)
{
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        ChangeBgX(3, 0, BG_COORD_SET);
        ChangeBgY(3, 0, BG_COORD_SET);
        InitWindows(sOptionMenuWinTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        gMain.state++;
        break;
    case 4:
        LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(0), sizeof(sOptionMenuBg_Pal));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 5:
        LoadPalette(sOptionMenuText_Pal, BG_PLTT_ID(1), sizeof(sOptionMenuText_Pal));
        gMain.state++;
        break;
    case 6:
        PutWindowTilemap(WIN_HEADER);
        DrawHeaderText();
        gMain.state++;
        break;
    case 7:
        gMain.state++;
        break;
    case 8:
        PutWindowTilemap(WIN_OPTIONS);
        gMain.state++;
    case 9:
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 10:
        CreateTask(Task_OptionMenuFadeIn, 0);
        LoadOptions();
        HighlightOptionMenuItem(0);
        DrawOptionRows();
        CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
        DrawHeaderText();
        sOptions.arrowTaskId = TASK_NONE;
        UpdateScrollArrows();
        gMain.state++;
        break;
    case 11:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        return;
    }
}

static void Task_OptionMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_OptionMenuProcessInput;
}

static void LoadOptions(void)
{
    sOptions.selection = 0;
    sOptions.controlsSelection = 0;
    sOptions.onControls = FALSE;
    sOptions.scroll = 0;
    sOptions.values[MENUITEM_TEXTSPEED] = gSaveBlock2Ptr->optionsTextSpeed;
    sOptions.values[MENUITEM_BATTLESCENE] = gSaveBlock2Ptr->optionsBattleSceneOff;
    sOptions.values[MENUITEM_BATTLESTYLE] = gSaveBlock2Ptr->optionsBattleStyle;
    sOptions.values[MENUITEM_SOUND] = gSaveBlock2Ptr->optionsSound;
    sOptions.values[MENUITEM_BUTTONMODE] = gSaveBlock2Ptr->optionsButtonMode;
    sOptions.values[MENUITEM_FRAMETYPE] = gSaveBlock2Ptr->optionsWindowFrameType;
    // Shown as ON, OFF like BATTLE SCENE, so 0 is ON.
    sOptions.values[MENUITEM_AUTORUN] = !gSaveBlock2Ptr->optionsAutoRun;
    sOptions.values[MENUITEM_AUTOSAVE] = !gSaveBlock2Ptr->optionsAutosave;
    sOptions.values[MENUITEM_COLORS] = gSaveBlock2Ptr->optionsColorMode;
    if (sOptions.values[MENUITEM_TEXTSPEED] > OPTIONS_TEXT_SPEED_INSTANT)
        sOptions.values[MENUITEM_TEXTSPEED] = OPTIONS_TEXT_SPEED_FAST;
    QoL_DecodeKeyMap(gSaveBlock2Ptr->optionsKeyMap, sOptions.keyMap);
}

static bool32 KeyMapIsDefault(void)
{
    u32 i;

    for (i = 0; i < QOL_KEY_COUNT; i++)
    {
        if (sOptions.keyMap[i] != i)
            return FALSE;
    }
    return TRUE;
}

static u32 RowCount(void)
{
    return sOptions.onControls ? CONTROLS_ROW_COUNT : MENUITEM_COUNT;
}

static u32 Selection(void)
{
    return sOptions.onControls ? sOptions.controlsSelection : sOptions.selection;
}

static void UpdateScrollArrows(void)
{
    if (sOptions.arrowTaskId != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(sOptions.arrowTaskId);
        sOptions.arrowTaskId = TASK_NONE;
    }
    if (!sOptions.onControls)
        sOptions.arrowTaskId = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, DISPLAY_WIDTH / 2, 36, 156,
                                    MENUITEM_COUNT - VISIBLE_ROWS, TAG_SCROLL_ARROWS, TAG_SCROLL_ARROWS, &sOptions.scroll);
}

static void DrawOptionRow(u32 row);

// Moves the cursor; scrolls the main list so the cursor stays in view.
static void MoveSelection(s32 delta)
{
    u32 count = RowCount();
    u32 sel = (Selection() + count + delta) % count;
    u16 scroll = sOptions.scroll;

    if (sOptions.onControls)
    {
        sOptions.controlsSelection = sel;
    }
    else
    {
        sOptions.selection = sel;
        if (sel < scroll)
            scroll = sel;
        else if (sel >= scroll + VISIBLE_ROWS)
            scroll = sel - VISIBLE_ROWS + 1;
    }
    if (scroll != sOptions.scroll)
    {
        sOptions.scroll = scroll;
        DrawOptionRows();
    }
    HighlightOptionMenuItem(sel - (sOptions.onControls ? 0 : sOptions.scroll));
    DrawHeaderText();
}

static u8 CycleChoice(u8 value, u8 count)
{
    if (JOY_NEW(DPAD_RIGHT))
        value = (value + 1) % count;
    else if (JOY_NEW(DPAD_LEFT))
        value = (value + count - 1) % count;
    return value;
}

// Left/right on a row of the main list. TRUE if its value changed.
static bool32 ChangeOption(u32 item)
{
    u8 *value = &sOptions.values[item];
    u8 old = *value;

    switch (item)
    {
    case MENUITEM_TEXTSPEED:
        *value = CycleChoice(*value, OPTIONS_TEXT_SPEED_INSTANT + 1);
        break;
    case MENUITEM_BATTLESCENE:
    case MENUITEM_BATTLESTYLE:
    case MENUITEM_AUTORUN:
    case MENUITEM_AUTOSAVE:
        *value = CycleChoice(*value, 2);
        break;
    case MENUITEM_SOUND:
        *value = CycleChoice(*value, 2);
        if (*value != old)
            SetPokemonCryStereo(*value);
        break;
    case MENUITEM_BUTTONMODE:
        *value = CycleChoice(*value, 3);
        break;
    case MENUITEM_FRAMETYPE:
        *value = CycleChoice(*value, WINDOW_FRAMES_COUNT);
        if (*value != old)
        {
            LoadBgTiles(1, GetWindowFrameTilesPal(*value)->tiles, 0x120, 0x1A2);
            LoadPalette(GetWindowFrameTilesPal(*value)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        }
        break;
    case MENUITEM_COLORS:
        *value = CycleChoice(*value, COLOR_MODE_COUNT);
        // Show the new colours right away.
        gSaveBlock2Ptr->optionsColorMode = *value;
        break;
    }
    return *value != old;
}

// Left/right on a row of the CONTROLS list: give this job the next key, and
// the job that had that key gets this job's old one.
static bool32 ChangeControl(u32 job)
{
    u32 i;
    u8 oldKey, newKey;

    if (job >= QOL_KEY_COUNT)
        return FALSE;
    oldKey = sOptions.keyMap[job];
    newKey = CycleChoice(oldKey, QOL_KEY_COUNT);
    if (newKey == oldKey)
        return FALSE;
    for (i = 0; i < QOL_KEY_COUNT; i++)
    {
        if (sOptions.keyMap[i] == newKey)
            sOptions.keyMap[i] = oldKey;
    }
    sOptions.keyMap[job] = newKey;
    return TRUE;
}

static void OpenControls(bool32 open)
{
    sOptions.onControls = open;
    sOptions.controlsSelection = 0;
    PlaySE(SE_SELECT);
    UpdateScrollArrows();
    DrawOptionRows();
    HighlightOptionMenuItem(open ? 0 : sOptions.selection - sOptions.scroll);
    DrawHeaderText();
}

static void Task_OptionMenuProcessInput(u8 taskId)
{
    u32 sel = Selection();

    if (JOY_NEW(A_BUTTON))
    {
        if (sOptions.onControls)
        {
            if (sel == CONTROLS_ROW_DEFAULTS)
            {
                u32 i;

                for (i = 0; i < QOL_KEY_COUNT; i++)
                    sOptions.keyMap[i] = i;
                PlaySE(SE_SELECT);
                DrawOptionRows();
            }
        }
        else if (sel == MENUITEM_CONTROLS)
        {
            OpenControls(TRUE);
        }
        else if (sel == MENUITEM_CANCEL)
        {
            gTasks[taskId].func = Task_OptionMenuSave;
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        if (sOptions.onControls)
            OpenControls(FALSE);
        else
            gTasks[taskId].func = Task_OptionMenuSave;
    }
    else if (JOY_REPEAT(DPAD_UP))
    {
        MoveSelection(-1);
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        MoveSelection(1);
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        bool32 changed = sOptions.onControls ? ChangeControl(sel) : ChangeOption(sel);

        if (changed && sOptions.onControls)
        {
            DrawOptionRows(); // the swap changed another row too
        }
        else if (changed)
        {
            DrawOptionRow(sel - sOptions.scroll);
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        }
    }
}

static void Task_OptionMenuSave(u8 taskId)
{
    gSaveBlock2Ptr->optionsTextSpeed = sOptions.values[MENUITEM_TEXTSPEED];
    gSaveBlock2Ptr->optionsBattleSceneOff = sOptions.values[MENUITEM_BATTLESCENE];
    gSaveBlock2Ptr->optionsBattleStyle = sOptions.values[MENUITEM_BATTLESTYLE];
    gSaveBlock2Ptr->optionsSound = sOptions.values[MENUITEM_SOUND];
    gSaveBlock2Ptr->optionsButtonMode = sOptions.values[MENUITEM_BUTTONMODE];
    gSaveBlock2Ptr->optionsWindowFrameType = sOptions.values[MENUITEM_FRAMETYPE];
    gSaveBlock2Ptr->optionsAutoRun = !sOptions.values[MENUITEM_AUTORUN];
    gSaveBlock2Ptr->optionsAutosave = !sOptions.values[MENUITEM_AUTOSAVE];
    gSaveBlock2Ptr->optionsColorMode = sOptions.values[MENUITEM_COLORS];
    gSaveBlock2Ptr->optionsKeyMap = QoL_EncodeKeyMap(sOptions.keyMap);

    if (sOptions.arrowTaskId != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(sOptions.arrowTaskId);
        sOptions.arrowTaskId = TASK_NONE;
    }
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_OptionMenuFadeOut;
}

static void Task_OptionMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

static void HighlightOptionMenuItem(u8 row)
{
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(row * 16 + 40, row * 16 + 56));
}

static void DrawOptionMenuChoice(const u8 *text, u8 x, u8 y, u8 style)
{
    u8 dst[24];
    u16 i;

    for (i = 0; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);

    if (style != 0)
    {
        dst[2] = TEXT_COLOR_RED;
        dst[5] = TEXT_COLOR_LIGHT_RED;
    }

    dst[i] = EOS;
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, dst, x, y + 1, TEXT_SKIP_DRAW, NULL);
}

static void DrawTwoChoices(const u8 *left, const u8 *right, u8 selection, u8 y)
{
    DrawOptionMenuChoice(left, VALUE_X, y, selection == 0);
    DrawOptionMenuChoice(right, GetStringRightAlignXOffset(FONT_NORMAL, right, VALUE_RIGHT), y, selection == 1);
}

static void DrawThreeChoices(const u8 *left, const u8 *mid, const u8 *right, u8 selection, u8 y)
{
    s32 widthLeft = GetStringWidth(FONT_NORMAL, left, 0);
    s32 widthMid = GetStringWidth(FONT_NORMAL, mid, 0) - 94;
    s32 widthRight = GetStringWidth(FONT_NORMAL, right, 0);

    DrawOptionMenuChoice(left, VALUE_X, y, selection == 0);
    DrawOptionMenuChoice(mid, (widthLeft - widthMid - widthRight) / 2 + VALUE_X, y, selection == 1);
    DrawOptionMenuChoice(right, GetStringRightAlignXOffset(FONT_NORMAL, right, VALUE_RIGHT), y, selection == 2);
}

static void DrawFrameType(u8 selection, u8 y)
{
    u8 text[16] = {EOS};
    u8 n = selection + 1;
    u16 i;

    for (i = 0; gText_FrameTypeNumber[i] != EOS && i <= 5; i++)
        text[i] = gText_FrameTypeNumber[i];

    if (n / 10 != 0)
    {
        text[i++] = n / 10 + CHAR_0;
        text[i++] = n % 10 + CHAR_0;
    }
    else
    {
        text[i++] = n % 10 + CHAR_0;
        text[i++] = CHAR_SPACER;
    }
    text[i] = EOS;

    DrawOptionMenuChoice(gText_FrameType, VALUE_X, y, 0);
    DrawOptionMenuChoice(text, VALUE_X + 24, y, 1);
}

static void DrawOptionValue(u32 item, u8 y)
{
    u8 value = sOptions.values[item];

    switch (item)
    {
    case MENUITEM_TEXTSPEED:
        DrawOptionMenuChoice(sTextSpeedNames[value], VALUE_X, y, 1);
        break;
    case MENUITEM_BATTLESCENE:
    case MENUITEM_AUTORUN:
    case MENUITEM_AUTOSAVE:
        DrawTwoChoices(gText_BattleSceneOn, gText_BattleSceneOff, value, y);
        break;
    case MENUITEM_BATTLESTYLE:
        DrawTwoChoices(gText_BattleStyleShift, gText_BattleStyleSet, value, y);
        break;
    case MENUITEM_SOUND:
        DrawTwoChoices(gText_SoundMono, gText_SoundStereo, value, y);
        break;
    case MENUITEM_BUTTONMODE:
        DrawThreeChoices(gText_ButtonTypeNormal, gText_ButtonTypeLR, gText_ButtonTypeLEqualsA, value, y);
        break;
    case MENUITEM_FRAMETYPE:
        DrawFrameType(value, y);
        break;
    case MENUITEM_COLORS:
        DrawOptionMenuChoice(sColorModeNames[value], VALUE_X, y, 1);
        break;
    case MENUITEM_CONTROLS:
        DrawOptionMenuChoice(KeyMapIsDefault() ? COMPOUND_STRING(GREEN("DEFAULT")) : COMPOUND_STRING(GREEN("CUSTOM")), VALUE_X, y, 1);
        break;
    }
}

// Redraws one visible row (0 = the top of the window).
static void DrawOptionRow(u32 row)
{
    u8 y = row * 16;

    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 0, y, 208, 16);
    if (sOptions.onControls)
    {
        if (row >= CONTROLS_ROW_COUNT)
            return;
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sJobNames[row], 8, y + 1, TEXT_SKIP_DRAW, NULL);
        if (row == CONTROLS_ROW_DEFAULTS)
            DrawOptionMenuChoice(gText_UseDefaults, VALUE_X, y, 0);
        else
            DrawOptionMenuChoice(sKeyNames[sOptions.keyMap[row]], VALUE_X, y, 1);
    }
    else
    {
        u32 item = sOptions.scroll + row;

        if (item >= MENUITEM_COUNT)
            return;
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[item], 8, y + 1, TEXT_SKIP_DRAW, NULL);
        DrawOptionValue(item, y);
    }
}

static void DrawOptionRows(void)
{
    u32 row;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (row = 0; row < VISIBLE_ROWS; row++)
        DrawOptionRow(row);
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
}

static void DrawHeaderText(void)
{
    const u8 *text = gText_Option;

    if (sOptions.onControls)
        text = sOptions.controlsSelection == CONTROLS_ROW_DEFAULTS ? gText_Controls
             : COMPOUND_STRING("{DPAD_LEFTRIGHT} picks the button for this.");
    else if (sOptionTips[sOptions.selection] != NULL)
        text = sOptionTips[sOptions.selection];
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, text, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA

static void DrawBgWindowFrames(void)
{
    //                     bg, tile,              x, y, width, height, palNum
    // Draw title window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  0, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1,  3,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2,  3, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28,  3,  1,  1,  7);

    // Draw options list window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  4, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 19,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 19, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 19,  1,  1,  7);

    CopyBgTilemapBufferToVram(1);
}
