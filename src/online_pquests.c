#include "global.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "online.h"
#include "script.h"
#include "constants/event_objects.h"

// Player-made quests (server/online_pquests.py). The status has the quest
// the player took: "S|PQUEST|id|map group|map num|x|y". Once the player is
// on that map the game says so, then that something glints when they're
// close, and on the very spot it asks the server for the treasure.

#define GLINT_DISTANCE 3

enum
{
    STAGE_NONE,
    STAGE_ON_MAP,
    STAGE_CLOSE,
    STAGE_FOUND,
};

struct QuestTrack
{
    u16 quest;
    u8 stage;
    u8 filler;
};

static EWRAM_DATA struct QuestTrack sTrack = {0};

extern const u8 Online_EventScript_PQuestOnMap[];
extern const u8 Online_EventScript_PQuestClose[];
extern const u8 Online_EventScript_PQuestSpot[];

bool32 Online_PlayerQuestUpdate(void)
{
    struct ObjectEvent *player;
    s32 quest = Online_StatusNum("PQUEST", 0);
    s32 dx, dy, distance;

    if (quest <= 0)
        return FALSE;
    if (quest != sTrack.quest)
    {
        sTrack.quest = quest;
        sTrack.stage = STAGE_NONE;
    }
    if (sTrack.stage == STAGE_FOUND
     || Online_StatusNum("PQUEST", 1) != gSaveBlock1Ptr->location.mapGroup
     || Online_StatusNum("PQUEST", 2) != gSaveBlock1Ptr->location.mapNum)
        return FALSE;

    player = &gObjectEvents[gPlayerAvatar.objectEventId];
    dx = player->currentCoords.x - MAP_OFFSET - Online_StatusNum("PQUEST", 3);
    dy = player->currentCoords.y - MAP_OFFSET - Online_StatusNum("PQUEST", 4);
    distance = max(abs(dx), abs(dy));

    if (distance == 0)
    {
        sTrack.stage = STAGE_FOUND;
        ScriptContext_SetupScript(Online_EventScript_PQuestSpot);
        return TRUE;
    }
    if (distance <= GLINT_DISTANCE && sTrack.stage < STAGE_CLOSE)
    {
        sTrack.stage = STAGE_CLOSE;
        ScriptContext_SetupScript(Online_EventScript_PQuestClose);
        return TRUE;
    }
    if (sTrack.stage < STAGE_ON_MAP)
    {
        sTrack.stage = STAGE_ON_MAP;
        ScriptContext_SetupScript(Online_EventScript_PQuestOnMap);
        return TRUE;
    }
    return FALSE;
}

// After a refused FIND (somebody else was quicker, or the server's idea of
// where the player stands is behind): look again on the next step.
void Online_PlayerQuestRetry(void)
{
    sTrack.stage = STAGE_CLOSE;
}
