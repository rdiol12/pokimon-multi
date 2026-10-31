#include "global.h"
#include "main.h"
#include "menu.h"
#include "online.h"
#include "overworld.h"
#include "script.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"

// A small window at the top right for the live features (mount races, hide
// and seek). Its owner gives the text. It closes itself when something takes
// over the screen, and opens again on the owner's next Online_ShowHud.

static const struct WindowTemplate sHudWindow =
{
    .bg = 0,
    .tilemapLeft = 19,
    .tilemapTop = 1,
    .width = 10,
    .height = 2,
    .paletteNum = 15,
    .baseBlock = 0x2C0,
};

static EWRAM_DATA OnlineHudText sHudText = NULL;
static EWRAM_DATA u8 sHudTaskId = 0;
static EWRAM_DATA bool8 sHudWhileLocked = FALSE;

#define tWindowId data[0]
#define tTimer    data[1]

static void DrawHud(u8 windowId)
{
    u8 text[32];

    sHudText(text);
    // Another window closing (an achievement's banner) may have cleared ours.
    DrawStdWindowFrame(windowId, FALSE);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(windowId, FONT_NORMAL, text, 2, 1, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void CloseHud(u8 taskId)
{
    ClearStdWindowAndFrameToTransparent(gTasks[taskId].tWindowId, FALSE);
    ClearWindowTilemap(gTasks[taskId].tWindowId);
    RemoveWindow(gTasks[taskId].tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
}

static void Task_Hud(u8 taskId)
{
    if (sHudText == NULL || gMain.callback2 != CB2_Overworld || (ArePlayerFieldControlsLocked() && !sHudWhileLocked))
    {
        CloseHud(taskId);
        return;
    }
    if (++gTasks[taskId].tTimer % 6 == 0)
        DrawHud(gTasks[taskId].tWindowId);
}

// Shows the HUD with text from `text`; whileLocked: also while a script has
// the player (a countdown).
void Online_ShowHud(OnlineHudText text, bool32 whileLocked)
{
    sHudText = text;
    sHudWhileLocked = whileLocked;
    if (FuncIsActiveTask(Task_Hud))
        return;
    if (ArePlayerFieldControlsLocked() && !whileLocked)
        return;
    sHudTaskId = CreateTask(Task_Hud, 90);
    gTasks[sHudTaskId].tWindowId = AddWindow(&sHudWindow);
    DrawHud(gTasks[sHudTaskId].tWindowId);
}

void Online_HideHud(void)
{
    sHudText = NULL;
}

// m:ss.cc of a time in frames (cs: with hundredths).
u8 *Online_FormatTime(u8 *dest, u32 frames, bool32 cs)
{
    u32 hundredths = frames * 100 / 60;

    dest = ConvertIntToDecimalStringN(dest, hundredths / 6000, STR_CONV_MODE_LEFT_ALIGN, 2);
    *dest++ = CHAR_COLON;
    dest = ConvertIntToDecimalStringN(dest, hundredths / 100 % 60, STR_CONV_MODE_LEADING_ZEROS, 2);
    if (!cs)
        return dest;
    *dest++ = CHAR_PERIOD;
    return ConvertIntToDecimalStringN(dest, hundredths % 100, STR_CONV_MODE_LEADING_ZEROS, 2);
}
