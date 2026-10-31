#include "global.h"
#include "event_object_lock.h"
#include "life.h"
#include "menu.h"
#include "netplay.h"
#include "netplay_ext.h"
#include "netplay_menu.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "window.h"
#include "constants/songs.h"

// START > ONLINE: the online screens. New features add a row here.
const struct NetplayMenuEntry gNetplayOnlineMenu[] =
{
    {COMPOUND_STRING("LADDER"),  ShowNetplayLadder},
    {COMPOUND_STRING("SEASON"),  ShowNetplaySeason},
    {COMPOUND_STRING("FRIENDS"), ShowNetplayFriends},
    {COMPOUND_STRING("WATCH"),   ShowNetplayWatch},
    {COMPOUND_STRING("PLAZA"),   NetplayExt_OpenPlazaFromMenu},
    {COMPOUND_STRING("AUCTION"), LifeAuction_OpenFromMenu},
    {NULL, NULL},
};

#define MAX_ENTRIES 8

#define tWindowId data[0]
#define tCount    data[1]

static EWRAM_DATA u8 sCursor = 0;

static void Task_OnlineMenu(u8 taskId);

void ShowNetplayOnlineMenu(void)
{
    static const struct WindowTemplate template =
    {
        .bg = 0,
        .tilemapLeft = 20,
        .tilemapTop = 1,
        .width = 9,
        .height = MAX_ENTRIES * 2,
        .paletteNum = 15,
        .baseBlock = 0x139,
    };
    struct MenuAction items[MAX_ENTRIES];
    struct WindowTemplate window = template;
    u8 taskId = CreateTask(Task_OnlineMenu, 80);
    u32 count;

    for (count = 0; count < MAX_ENTRIES && gNetplayOnlineMenu[count].text != NULL; count++)
        items[count].text = gNetplayOnlineMenu[count].text;
    window.height = count * 2;
    gTasks[taskId].tCount = count;
    gTasks[taskId].tWindowId = AddWindow(&window);
    if (sCursor >= count)
        sCursor = 0;
    DrawStdWindowFrame(gTasks[taskId].tWindowId, FALSE);
    PrintMenuTable(gTasks[taskId].tWindowId, count, items);
    InitMenuInUpperLeftCornerNormal(gTasks[taskId].tWindowId, count, sCursor);
    ScheduleBgCopyTilemapToVram(0);
}

static void Task_OnlineMenu(u8 taskId)
{
    s8 input = Menu_ProcessInputNoWrap();

    if (input == MENU_NOTHING_CHOSEN)
        return;
    PlaySE(SE_SELECT);
    ClearStdWindowAndFrame(gTasks[taskId].tWindowId, TRUE);
    RemoveWindow(gTasks[taskId].tWindowId);
    DestroyTask(taskId);
    if (input == MENU_B_PRESSED)
    {
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        return;
    }
    sCursor = input;
    gNetplayOnlineMenu[input].callback();
}
