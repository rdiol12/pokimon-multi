#include "global.h"
#include "event_data.h"
#include "item.h"
#include "list_menu.h"
#include "malloc.h"
#include "menu.h"
#include "netplay_ext.h"
#include "online.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "constants/items.h"
#include "constants/songs.h"

// Player shops (server/online_shops.py): picking what to put on a shelf
// from the bag, and taking the numbers of a picked shelf.

#define BAG_LIST_MAX   100
#define BAG_LABEL_LEN  24
#define MAX_SHELF      99

struct BagList
{
    struct ListMenuItem items[BAG_LIST_MAX + 1];
    u8 labels[BAG_LIST_MAX][BAG_LABEL_LEN];
    u16 ids[BAG_LIST_MAX];
    u16 counts[BAG_LIST_MAX];
};

static EWRAM_DATA struct BagList *sBagList = NULL;

// The pockets things can be sold from.
static const enum Pocket sSellPockets[] = {POCKET_ITEMS, POCKET_POKE_BALLS, POCKET_BERRIES};

static void Task_ChooseBagItem(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].data[0]);

    if (input == LIST_NOTHING_CHOSEN)
        return;
    PlaySE(SE_SELECT);
    if (input == LIST_CANCEL)
    {
        gSpecialVar_Result = ONLINE_CHOSE_NOTHING;
    }
    else
    {
        gSpecialVar_Result = 0;
        gSpecialVar_0x8005 = sBagList->ids[input];
        gSpecialVar_0x8006 = min(sBagList->counts[input], MAX_SHELF);
        gSpecialVar_0x8007 = GetItemPrice(sBagList->ids[input]);
        CopyItemName(sBagList->ids[input], gStringVar1);
    }
    Online_CloseList(taskId);
    TRY_FREE_AND_SET_NULL(sBagList);
}

// Online_ChooseBagItem (waitstate): a list of what's in the bag's item,
// ball and berry pockets. VAR_RESULT is 0, or ONLINE_CHOSE_NOTHING; then
// VAR_0x8005 is the item, VAR_0x8006 how many the bag has (at most 99),
// VAR_0x8007 its price in the marts, gStringVar1 its name.
void Online_ChooseBagItem(void)
{
    u32 p, i, count = 0;

    sBagList = AllocZeroed(sizeof(*sBagList));
    for (p = 0; p < ARRAY_COUNT(sSellPockets); p++)
    {
        struct BagPocket *pocket = &gBagPockets[sSellPockets[p]];

        for (i = 0; i < pocket->capacity && count < BAG_LIST_MAX; i++)
        {
            struct ItemSlot slot = BagPocket_GetSlotData(pocket, i);
            u8 *end;

            if (slot.itemId == ITEM_NONE || slot.quantity == 0 || GetItemImportance(slot.itemId))
                continue;
            end = StringCopy(sBagList->labels[count], GetItemName(slot.itemId));
            end = StringCopy(end, COMPOUND_STRING(" x"));
            ConvertIntToDecimalStringN(end, slot.quantity, STR_CONV_MODE_LEFT_ALIGN, 3);
            sBagList->items[count].name = sBagList->labels[count];
            sBagList->items[count].id = count;
            sBagList->ids[count] = slot.itemId;
            sBagList->counts[count] = slot.quantity;
            count++;
        }
    }
    sBagList->items[count].name = gText_Cancel;
    sBagList->items[count].id = LIST_CANCEL;
    Online_OpenList(sBagList->items, count + 1, Task_ChooseBagItem);
}

// After a shelf is picked (rows: label, shelf, item, count, price):
// VAR_0x8005 the item, VAR_0x8006 how many are on it, VAR_0x8007 the price
// and gStringVar1 the item's name.
void Online_TakeChosenShelf(void)
{
    gSpecialVar_0x8005 = Npx_ChosenNum(2);
    gSpecialVar_0x8006 = Npx_ChosenNum(3);
    gSpecialVar_0x8007 = Npx_ChosenNum(4);
    CopyItemName(gSpecialVar_0x8005, gStringVar1);
}

// VAR_RESULT of the how-many menu (1, 5, all) to VAR_0x8006, which holds
// how many there are.
void Online_SetShopCount(void)
{
    static const u8 sCounts[] = {1, 5};

    if (gSpecialVar_Result < ARRAY_COUNT(sCounts))
        gSpecialVar_0x8006 = min(gSpecialVar_0x8006, sCounts[gSpecialVar_Result]);
}
