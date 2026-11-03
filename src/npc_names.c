#include "global.h"
#include "npc_names.h"
#include "event_object_movement.h"
#include "field_name_box.h"
#include "rovers.h"
#include "constants/characters.h"

// Every person on every map has a name of their own: story characters their
// story name, trainers their trainer name (unless several share it, like the
// GRUNTs), everyone else a first name and surname nobody else has. It's
// shown in the name box above everything they say, and the living NPC
// trainers show their own names the same way.

#include "data/npc_names.h"

static EWRAM_DATA const u8 *sTalkSpeaker = NULL;

static const u8 *LookUp(u32 group, u32 num, u32 localId)
{
    u32 key = (group << 8) | num;
    s32 lo = 0, hi = ARRAY_COUNT(sNpcNames) - 1;

    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;
        const struct NpcName *e = &sNpcNames[mid];
        if (e->map == key && e->localId == localId)
            return e->name;
        if (e->map < key || (e->map == key && e->localId < localId))
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL;
}

const u8 *NpcNames_ForObject(u32 objectEventId)
{
    struct ObjectEvent *obj;
    const u8 *name;

    if (objectEventId >= OBJECT_EVENTS_COUNT)
        return NULL;
    obj = &gObjectEvents[objectEventId];
    if (!obj->active || obj->isPlayer)
        return NULL;
    name = Rovers_SpeakerName(obj);
    if (name != NULL)
        return name;
    // Only people from the map's own list (not Pokémon or things the game
    // spawns, whose local ids aren't on it).
    if (GetObjectEventTemplateByLocalIdAndMap(obj->localId, obj->mapNum, obj->mapGroup) == NULL)
        return NULL;
    return LookUp(obj->mapGroup, obj->mapNum, obj->localId);
}

void NpcNames_OnTalk(u32 objectEventId)
{
    sTalkSpeaker = NpcNames_ForObject(objectEventId);
}

void NpcNames_ApplyToMessage(const u8 *raw)
{
    if (sTalkSpeaker == NULL || gSpeakerName != NULL)
        return;
    // "{PLAYER} received ..." and the like are the game talking, not them.
    if (raw[0] == PLACEHOLDER_BEGIN && raw[1] == PLACEHOLDER_ID_PLAYER)
        return;
    gSpeakerName = sTalkSpeaker;
}

void NpcNames_OnScriptEnd(void)
{
    sTalkSpeaker = NULL;
}
