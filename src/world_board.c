#include "global.h"
#include "event_object_movement.h"
#include "main.h"
#include "menu.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "world.h"
#include "constants/songs.h"

// A full-screen list window for the world features: achievements, records,
// leaderboards. The caller gives a title, a line count and a function that
// writes line i; the board scrolls with the D-pad, shows the selected line's
// detail (if any) at the bottom, and closes with A or B. Opened from a script
// with `callnative ..., waitstate=1`; closing resumes the script.

#define BOARD_ROWS        8
#define BOARD_LINE_HEIGHT 14
#define BOARD_TOP         16

static const struct WindowTemplate sBoardWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

struct WorldBoard
{
    const u8 *title;
    WorldBoardLineFunc getLine;
    WorldBoardLineFunc getDetail;
    u16 count;
    u16 cursor;
    u16 scroll;
    u8 windowId;
    bool8 dirty;
};

static EWRAM_DATA struct WorldBoard sBoard = {0};

static void Task_Board(u8 taskId);

static void DrawBoard(void)
{
    u8 line[64];
    u32 i, y;

    FillWindowPixelBuffer(sBoard.windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(sBoard.windowId, FONT_NORMAL, sBoard.title, 4, 0, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < BOARD_ROWS && sBoard.scroll + i < sBoard.count; i++)
    {
        u32 index = sBoard.scroll + i;
        y = BOARD_TOP + i * BOARD_LINE_HEIGHT;
        line[0] = EOS;
        sBoard.getLine(index, line);
        if (sBoard.getDetail != NULL && index == sBoard.cursor)
            AddTextPrinterParameterized(sBoard.windowId, FONT_NORMAL, gText_SelectorArrow2, 0, y, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(sBoard.windowId, FONT_NORMAL, line, 10, y, TEXT_SKIP_DRAW, NULL);
    }
    if (sBoard.count == 0)
        AddTextPrinterParameterized(sBoard.windowId, FONT_NORMAL, COMPOUND_STRING("Nothing here yet."), 10, BOARD_TOP, TEXT_SKIP_DRAW, NULL);
    if (sBoard.getDetail != NULL && sBoard.count != 0)
    {
        line[0] = EOS;
        sBoard.getDetail(sBoard.cursor, line);
        AddTextPrinterParameterized(sBoard.windowId, FONT_SMALL, line, 4, BOARD_TOP + BOARD_ROWS * BOARD_LINE_HEIGHT + 2, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(sBoard.windowId, COPYWIN_FULL);
}

void WorldBoard_Show(const u8 *title, u16 count, WorldBoardLineFunc getLine, WorldBoardLineFunc getDetail)
{
    sBoard.title = title;
    sBoard.count = count;
    sBoard.getLine = getLine;
    sBoard.getDetail = getDetail;
    sBoard.cursor = 0;
    sBoard.scroll = 0;
    sBoard.windowId = AddWindow(&sBoardWindowTemplate);
    DrawStdWindowFrame(sBoard.windowId, FALSE);
    PutWindowTilemap(sBoard.windowId);
    DrawBoard();
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Board, 80);
}

// Redraws an open board with new contents (for boards filled by the server).
void WorldBoard_Refresh(u16 count)
{
    sBoard.count = count;
    if (sBoard.cursor >= count)
        sBoard.cursor = sBoard.scroll = 0;
    sBoard.dirty = TRUE;
}

static void Task_Board(u8 taskId)
{
    u16 old = sBoard.cursor;

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        ClearStdWindowAndFrameToTransparent(sBoard.windowId, FALSE);
        ClearWindowTilemap(sBoard.windowId);
        RemoveWindow(sBoard.windowId);
        ScheduleBgCopyTilemapToVram(0);
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    if (sBoard.count != 0)
    {
        u16 step = sBoard.getDetail != NULL ? 1 : BOARD_ROWS;
        if (JOY_REPEAT(DPAD_DOWN) && sBoard.cursor + step < sBoard.count)
            sBoard.cursor += step;
        else if (JOY_REPEAT(DPAD_UP) && sBoard.cursor >= step)
            sBoard.cursor -= step;
    }
    if (sBoard.cursor != old || sBoard.dirty)
    {
        if (sBoard.cursor < sBoard.scroll)
            sBoard.scroll = sBoard.cursor;
        else if (sBoard.cursor >= sBoard.scroll + BOARD_ROWS)
            sBoard.scroll = sBoard.cursor - BOARD_ROWS + 1;
        if (sBoard.cursor != old)
            PlaySE(SE_SELECT);
        sBoard.dirty = FALSE;
        DrawBoard();
    }
}
