#include "global.h"
#include "online.h"

// Every request the sixth-batch features make (Online_Request, VAR_0x8004).
// The server routes each by its first word to a module in server/online_*.py.
const struct OnlineRequest gOnlineRequests[ONL_REQ_COUNT] =
{
    [ONL_REQ_GOAL_VIEW]  = {"GOAL VIEW"},
    [ONL_REQ_GOAL_CLAIM] = {"GOAL CLAIM"},
    [ONL_REQ_SIGHT_BOARD] = {"SIGHT BOARD"},
    [ONL_REQ_FISHT_BOARD] = {"FISHT BOARD"},
    [ONL_REQ_FISHT_CLAIM] = {"FISHT CLAIM"},
    [ONL_REQ_COFISH_WHO]    = {"COFISH WHO"},
    [ONL_REQ_COFISH_ASK]    = {"COFISH ASK", .flags = ONL_ARG_CHOSEN},
    [ONL_REQ_COFISH_JOIN]   = {"COFISH JOIN"},
    [ONL_REQ_COFISH_NO]     = {"COFISH NO"},
    [ONL_REQ_COFISH_WAIT]   = {"COFISH WAIT"},
    [ONL_REQ_COFISH_REEL]   = {"COFISH REEL", 1},
    [ONL_REQ_COFISH_RESULT] = {"COFISH RESULT"},
    [ONL_REQ_COFISH_BOARD]  = {"COFISH BOARD"},
    [ONL_REQ_SHOP_LIST]    = {"SHOP LIST"},
    [ONL_REQ_SHOP_OPEN]    = {"SHOP OPEN", .flags = ONL_ARG_CHOSEN},
    [ONL_REQ_SHOP_ITEMS]   = {"SHOP ITEMS"},
    [ONL_REQ_SHOP_BUY]     = {"SHOP BUY", 2, ONL_ARG_CHOSEN | ONL_ARG_MONEY},
    [ONL_REQ_SHOP_MINE]    = {"SHOP MINE"},
    [ONL_REQ_SHOP_PRICES]  = {"SHOP PRICES", 3},
    [ONL_REQ_SHOP_STOCK]   = {"SHOP STOCK", 2, ONL_ARG_CHOSEN},
    [ONL_REQ_SHOP_TAKE]    = {"SHOP TAKE", .flags = ONL_ARG_CHOSEN},
    [ONL_REQ_SHOP_COLLECT] = {"SHOP COLLECT"},
    [ONL_REQ_PAGEANT_ENTER] = {"PAGEANT ENTER", .flags = ONL_ARG_MON},
    [ONL_REQ_PAGEANT_SHOW]  = {"PAGEANT SHOW", 1},
    [ONL_REQ_PAGEANT_VOTE]  = {"PAGEANT VOTE", 2},
    [ONL_REQ_PAGEANT_BOARD] = {"PAGEANT BOARD"},
    [ONL_REQ_PAGEANT_CLAIM] = {"PAGEANT CLAIM"},
};
