// Quality-of-life options: auto-run, autosave, colour-blind palettes and
// remapped buttons. The OPTION menu (src/option_menu.c) sets them; they are
// kept in SaveBlock2 next to the other options. See docs/qol.md.
#include "global.h"
#include "qol.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "fieldmap.h"
#include "field_screen_effect.h"
#include "link.h"
#include "main.h"
#include "menu.h"
#include "netplay.h"
#include "new_game.h"
#include "overworld.h"
#include "safari_zone.h"
#include "save.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "trainer_hill.h"
#include "window.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"

// Buttons --------------------------------------------------------------------
//
// The key map is a permutation of the six buttons. It is packed in 15 bits:
// for jobs 0-4, three bits of (key - job) mod 6; the last job gets the key
// left over. So 0 is the usual layout, and old saves need nothing.

const u16 gQoLKeyBits[QOL_KEY_COUNT] =
{
    [QOL_KEY_A]      = A_BUTTON,
    [QOL_KEY_B]      = B_BUTTON,
    [QOL_KEY_START]  = START_BUTTON,
    [QOL_KEY_SELECT] = SELECT_BUTTON,
    [QOL_KEY_L]      = L_BUTTON,
    [QOL_KEY_R]      = R_BUTTON,
};

void QoL_DecodeKeyMap(u16 packed, u8 *map)
{
    u32 i, used = 0;

    for (i = 0; i < QOL_KEY_COUNT - 1; i++)
    {
        map[i] = (i + ((packed >> (i * 3)) & 7)) % QOL_KEY_COUNT;
        used |= 1 << map[i];
    }
    for (map[i] = 0; map[i] < QOL_KEY_COUNT && (used & (1 << map[i])); map[i]++)
        ;
    used |= 1 << map[i];
    if (used != (1 << QOL_KEY_COUNT) - 1)
    {
        // Not a permutation (shouldn't happen): fall back to the usual layout.
        for (i = 0; i < QOL_KEY_COUNT; i++)
            map[i] = i;
    }
}

u16 QoL_EncodeKeyMap(const u8 *map)
{
    u32 i;
    u16 packed = 0;

    for (i = 0; i < QOL_KEY_COUNT - 1; i++)
        packed |= ((map[i] + QOL_KEY_COUNT - i) % QOL_KEY_COUNT) << (i * 3);
    return packed;
}

static EWRAM_DATA u16 sKeyMapPacked = 0;
static EWRAM_DATA u8 sKeyMap[QOL_KEY_COUNT] = {0};

// Turns the keys pressed on the pad into the jobs they do.
u16 QoL_RemapKeys(u16 keys)
{
    u32 i;
    u16 packed = gSaveBlock2Ptr->optionsKeyMap, out;

    if (packed == 0)
        return keys;
    if (packed != sKeyMapPacked || sKeyMap[0] == sKeyMap[1])
    {
        QoL_DecodeKeyMap(packed, sKeyMap);
        sKeyMapPacked = packed;
    }
    out = keys & DPAD_ANY;
    for (i = 0; i < QOL_KEY_COUNT; i++)
    {
        if (keys & gQoLKeyBits[sKeyMap[i]])
            out |= gQoLKeyBits[i];
    }
    return out;
}

// Auto-run -------------------------------------------------------------------

bool32 QoL_WantsToRun(u16 heldKeys)
{
    bool32 holdingB = (heldKeys & B_BUTTON) != 0;

    return gSaveBlock2Ptr->optionsAutoRun ? !holdingB : holdingB;
}

// Autosave -------------------------------------------------------------------
//
// With AUTOSAVE on, the game saves after the player walks into a new area (at
// most once a minute) and every five minutes in one area, whenever the player
// stands still with nothing else going on. It never saves over a different
// save file: a new game has to be saved by hand once first.

#define AUTOSAVE_MIN_GAP    (60 * 60)     // frames (one minute) between area saves
#define AUTOSAVE_PERIOD     (5 * 60 * 60) // frames (five minutes) in one area
#define AUTOSAVE_SHOW_FRAMES 90

static EWRAM_DATA struct {
    u8 started;
    u8 mapGroup;
    u8 mapNum;
    u32 lastSave; // gMain.vblankCounter1 at the last save
} sAutosave = {0};

static const struct WindowTemplate sAutosaveWindow =
{
    .bg = 0,
    .tilemapLeft = 21,
    .tilemapTop = 1,
    .width = 8,
    .height = 2,
    .paletteNum = 15,
    .baseBlock = 0x0D0,
};

static bool32 CanAutosaveHere(void)
{
    mapsec_u8_t mapsec = gMapHeader.regionMapSectionId;

    if (gDifferentSaveFile || gSaveFileStatus == SAVE_STATUS_NO_FLASH)
        return FALSE;
    if (gMain.callback2 != CB2_Overworld || !IsPlayerStandingStill())
        return FALSE;
    if (Netplay_InSession() || InUnionRoom() || gReceivedRemoteLinkPlayers)
        return FALSE;
    if (GetSafariZoneFlag() || InBattlePyramid() || InBattlePike() || InTrainerHillChallenge())
        return FALSE;
    if (mapsec == MAPSEC_BATTLE_FRONTIER || mapsec == MAPSEC_TRAINER_HILL)
        return FALSE;
    return TRUE;
}

#define tWindowId data[0]
#define tTimer    data[1]

static void CloseAutosaveNotice(u8 taskId)
{
    ClearStdWindowAndFrameToTransparent(gTasks[taskId].tWindowId, FALSE);
    ClearWindowTilemap(gTasks[taskId].tWindowId);
    RemoveWindow(gTasks[taskId].tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
}

static void Task_AutosaveNotice(u8 taskId)
{
    // Leave the screen to scripts and menus.
    if (--gTasks[taskId].tTimer <= 0 || ArePlayerFieldControlsLocked() || gMain.callback2 != CB2_Overworld)
        CloseAutosaveNotice(taskId);
}

static void PrintAutosaveNotice(u8 windowId, const u8 *text)
{
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(windowId, FONT_NORMAL, text, 6, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_GFX);
}

// "Saving…" goes up before the save, which takes a moment (the game waits for
// the flash chip, while the screen keeps showing); then it turns into "Saved!".
static u8 ShowAutosaveNotice(void)
{
    u8 taskId, windowId;

    if (FuncIsActiveTask(Task_AutosaveNotice))
        CloseAutosaveNotice(FindTaskIdByFunc(Task_AutosaveNotice));
    taskId = CreateTask(Task_AutosaveNotice, 90);
    windowId = AddWindow(&sAutosaveWindow);
    gTasks[taskId].tWindowId = windowId;
    gTasks[taskId].tTimer = AUTOSAVE_SHOW_FRAMES;
    DrawStdWindowFrame(windowId, FALSE);
    PutWindowTilemap(windowId);
    PrintAutosaveNotice(windowId, COMPOUND_STRING("Saving…"));
    CopyWindowToVram(windowId, COPYWIN_FULL);
    return taskId;
}

// Called every overworld frame while the player has control. TRUE if it saved.
bool32 QoL_TryAutosave(void)
{
    u32 now = gMain.vblankCounter1;
    bool32 newArea;
    u8 taskId;

    if (!sAutosave.started)
    {
        // The first area after loading or a new game is where we already are.
        sAutosave.started = TRUE;
        sAutosave.mapGroup = gSaveBlock1Ptr->location.mapGroup;
        sAutosave.mapNum = gSaveBlock1Ptr->location.mapNum;
        sAutosave.lastSave = now;
        return FALSE;
    }
    newArea = sAutosave.mapGroup != gSaveBlock1Ptr->location.mapGroup
           || sAutosave.mapNum != gSaveBlock1Ptr->location.mapNum;
    if (!gSaveBlock2Ptr->optionsAutosave)
        return FALSE;
    if (newArea ? now - sAutosave.lastSave < AUTOSAVE_MIN_GAP : now - sAutosave.lastSave < AUTOSAVE_PERIOD)
        return FALSE;
    if (!CanAutosaveHere())
        return FALSE;

    sAutosave.mapGroup = gSaveBlock1Ptr->location.mapGroup;
    sAutosave.mapNum = gSaveBlock1Ptr->location.mapNum;
    SaveMapView();
    taskId = ShowAutosaveNotice();
    if (TrySavingData(SAVE_NORMAL) == SAVE_STATUS_OK)
        PrintAutosaveNotice(gTasks[taskId].tWindowId, COMPOUND_STRING("Saved!"));
    else
        CloseAutosaveNotice(taskId);
    sAutosave.lastSave = gMain.vblankCounter1; // the save took a moment
    return TRUE;
}

#undef tWindowId
#undef tTimer

// Colour-blind palettes --------------------------------------------------------
//
// tools/qol/make_cvd_tables.py builds one table per mode; palette.c filters
// every colour through it on its way to the screen.

static const u16 sColorTableRedWeak[]   = INCBIN_U16("graphics/qol/cvd_protan.bin");
static const u16 sColorTableGreenWeak[] = INCBIN_U16("graphics/qol/cvd_deutan.bin");
static const u16 sColorTableBlueWeak[]  = INCBIN_U16("graphics/qol/cvd_tritan.bin");

const u16 *QoL_GetColorTable(void)
{
    if (gSaveBlock2Ptr == NULL)
        return NULL;
    switch (gSaveBlock2Ptr->optionsColorMode)
    {
    case COLOR_MODE_RED_WEAK:
        return sColorTableRedWeak;
    case COLOR_MODE_GREEN_WEAK:
        return sColorTableGreenWeak;
    case COLOR_MODE_BLUE_WEAK:
        return sColorTableBlueWeak;
    default:
        return NULL;
    }
}
