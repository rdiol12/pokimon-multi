#include "global.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "menu.h"
#include "player_style.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "window.h"
#include "constants/songs.h"

// The Style menu from the start menu: pick how your trainer looks. The player
// on the map changes as the cursor moves, A keeps the look and B puts the old
// one back. Online, other players see the new look straight away because the
// netplay mailbox sends the player's sprite.

#define tWindowId  data[0]
#define tOldStyle  data[1]
#define tShown     data[2]

static void Task_PlayerStyleMenu(u8 taskId);

static const u8 sText_Brendan[] = _("BRENDAN");
static const u8 sText_May[] = _("MAY");
static const u8 sText_Red[] = _("RED");
static const u8 sText_Leaf[] = _("LEAF");

static const struct WindowTemplate sStyleWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 20,
    .tilemapTop = 1,
    .width = 9,
    .height = PLAYER_STYLE_COUNT * 2,
    .paletteNum = 15,
    .baseBlock = 0x139,
};

static const struct MenuAction sStyleMenuItems[PLAYER_STYLE_COUNT] =
{
    [PLAYER_STYLE_BRENDAN] = {sText_Brendan},
    [PLAYER_STYLE_MAY]     = {sText_May},
    [PLAYER_STYLE_RED]     = {sText_Red},
    [PLAYER_STYLE_LEAF]    = {sText_Leaf},
};

void ShowPlayerStyleMenu(void)
{
    u8 taskId = CreateTask(Task_PlayerStyleMenu, 80);
    struct Task *task = &gTasks[taskId];
    enum PlayerStyle style = GetPlayerStyle();

    task->tWindowId = AddWindow(&sStyleWindowTemplate);
    task->tOldStyle = style;
    task->tShown = style;
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PrintMenuTable(task->tWindowId, PLAYER_STYLE_COUNT, sStyleMenuItems);
    InitMenuInUpperLeftCornerNormal(task->tWindowId, PLAYER_STYLE_COUNT, style);
    ScheduleBgCopyTilemapToVram(0);
}

static void ShowStyle(struct Task *task, enum PlayerStyle style)
{
    if (task->tShown == style)
        return;
    task->tShown = style;
    SetPlayerStyle(style);
    ApplyPlayerStyleToAvatar();
}

static void CloseStyleMenu(u8 taskId)
{
    ClearStdWindowAndFrame(gTasks[taskId].tWindowId, TRUE);
    RemoveWindow(gTasks[taskId].tWindowId);
    DestroyTask(taskId);
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

static void Task_PlayerStyleMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s8 input = Menu_ProcessInputNoWrap();

    switch (input)
    {
    case MENU_NOTHING_CHOSEN:
        ShowStyle(task, Menu_GetCursorPos());
        break;
    case MENU_B_PRESSED:
        PlaySE(SE_SELECT);
        ShowStyle(task, task->tOldStyle);
        CloseStyleMenu(taskId);
        break;
    default:
        ShowStyle(task, input);
        CloseStyleMenu(taskId);
        break;
    }
}
