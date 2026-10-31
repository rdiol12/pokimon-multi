#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "main.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "script.h"
#include "constants/event_object_movement.h"

// Minigames and side activities (docs/pastimes.md): shared parts.
//
// Activities place their NPCs and objects from code (struct PastimeSpot)
// instead of editing map.json, so they need no event flags and don't clash
// with other features' map edits. Each frame a few times a second the spots
// of the current map are spawned or removed by their isShown() check.

// Where struct PastimeSaveData is, for the screenshot driver
// (tools/pastime_shots/), which writes scenes straight into it.
EWRAM_DATA struct PastimeSaveData *gPastimeSavePtr = NULL;

// Text an activity builds for a script to show (msgbox gPastimeText). Unlike
// gStringVar4, other messages don't overwrite it.
EWRAM_DATA u8 gPastimeText[PASTIME_TEXT_SIZE] = {0};

static const struct PastimeSpotList sSpotLists[] =
{
    {gLostFoundSpots, LOST_FOUND_COUNT * 2},
};

#define MAX_SHOWN (LOCALID_PASTIME_LAST - LOCALID_PASTIME_FIRST + 1)

static EWRAM_DATA u8 sTimer = 0;

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

// Calls func for every spot on the current map with the local id it gets.
// Stops early if func returns TRUE; returns that spot.
static const struct PastimeSpot *ForEachSpotHere(bool32 (*func)(const struct PastimeSpot *spot, u8 localId, void *data), void *data)
{
    u32 list, i, localId = LOCALID_PASTIME_FIRST;
    u16 map = CurrentMap();

    for (list = 0; list < ARRAY_COUNT(sSpotLists); list++)
    {
        for (i = 0; i < sSpotLists[list].count; i++)
        {
            const struct PastimeSpot *spot = &sSpotLists[list].spots[i];
            if (spot->map != map)
                continue;
            if (localId > LOCALID_PASTIME_LAST)
                return NULL;
            if (func(spot, localId, data))
                return spot;
            localId++;
        }
    }
    return NULL;
}

static bool32 IsSpotShown(u8 localId)
{
    return GetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup) != OBJECT_EVENTS_COUNT;
}

// Same area as RemoveObjectEventsOutsideView, so spawned spots stay.
static bool32 IsInView(s16 x, s16 y)
{
    return x >= gSaveBlock1Ptr->pos.x - 2 && x <= gSaveBlock1Ptr->pos.x + 17
        && y >= gSaveBlock1Ptr->pos.y && y <= gSaveBlock1Ptr->pos.y + 16;
}

static bool32 UpdateSpot(const struct PastimeSpot *spot, u8 localId, void *data)
{
    s16 x = spot->x + MAP_OFFSET, y = spot->y + MAP_OFFSET;
    bool32 wanted = (spot->isShown == NULL || spot->isShown(spot->arg)) && IsInView(x, y);
    bool32 shown = IsSpotShown(localId);

    if (shown && !wanted)
        RemoveObjectEventByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    else if (!shown && wanted)
        SpawnSpecialObjectEventParameterized(spot->graphicsId, spot->movementType, localId, x, y, MapGridGetElevationAt(x, y));
    return FALSE;
}

static void UpdateSpots(void)
{
    ForEachSpotHere(UpdateSpot, NULL);
}

static bool32 MatchLocalId(const struct PastimeSpot *spot, u8 localId, void *data)
{
    return localId == *(u8 *)data;
}

bool32 Pastime_IsPastimeObjectEvent(const struct ObjectEvent *objectEvent)
{
    u8 localId = objectEvent->localId;

    if (localId < LOCALID_PASTIME_FIRST || localId > LOCALID_PASTIME_LAST)
        return FALSE;
    return objectEvent->mapNum == gSaveBlock1Ptr->location.mapNum
        && objectEvent->mapGroup == gSaveBlock1Ptr->location.mapGroup
        && ForEachSpotHere(MatchLocalId, &localId) != NULL;
}

// Talking to a spot: its script, with VAR_0x8004 = its arg.
const u8 *Pastime_GetTalkScript(u8 objectEventId)
{
    u8 localId = gObjectEvents[objectEventId].localId;
    const struct PastimeSpot *spot = ForEachSpotHere(MatchLocalId, &localId);

    if (spot == NULL)
        return NULL;
    gSpecialVar_0x8004 = spot->arg;
    return spot->script;
}

// Called once per overworld frame.
void Pastime_Update(void)
{
    gPastimeSavePtr = &gSaveBlock3Ptr->pastime;
    if (++sTimer < 8 || ArePlayerFieldControlsLocked())
        return;
    sTimer = 0;
    UpdateSpots();
}

// A pressed facing (x, y), map coordinates with MAP_OFFSET.
bool32 Pastime_TryInteract(s16 x, s16 y)
{
    return FALSE;
}

// Called when the player caught a wild Pokémon.
void Pastime_OnCatch(void)
{
}

// Back from a trainer battle.
void Pastime_OnTrainerBattleEnd(void)
{
}
