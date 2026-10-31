#include "global.h"
#include "guide.h"
#include "online.h"

// The guide's online requests (server/guide_social.py). They go through
// Online_Request like the HUB's, with VAR_0x8004 = ONL_REQ_GUIDE_*.
const struct OnlineRequest gGuideRequests[ONL_REQ_GUIDE_COUNT] =
{
    [ONL_REQ_GUIDE_INVITE - ONL_REQ_GUIDE_FIRST] = {"GUIDE INVITE"},
    [ONL_REQ_GUIDE_NEARBY - ONL_REQ_GUIDE_FIRST] = {"GUIDE NEARBY"},
    [ONL_REQ_GUIDE_FRIEND - ONL_REQ_GUIDE_FIRST] = {"GUIDE FRIEND", .flags = ONL_ARG_CHOSEN},
};
