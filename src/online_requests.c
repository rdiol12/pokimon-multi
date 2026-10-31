#include "global.h"
#include "online.h"

// Every request the sixth-batch features make (Online_Request, VAR_0x8004).
// The server routes each by its first word to a module in server/online_*.py.
const struct OnlineRequest gOnlineRequests[ONL_REQ_COUNT] =
{
    [ONL_REQ_GOAL_VIEW]  = {"GOAL VIEW"},
    [ONL_REQ_GOAL_CLAIM] = {"GOAL CLAIM"},
    [ONL_REQ_SIGHT_BOARD] = {"SIGHT BOARD"},
};
