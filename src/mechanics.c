#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "mechanics.h"
#include "script.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/layouts.h"

// People the mechanics features stand on existing maps, spawned from C so
// no map file changes: the move tutor in every Pokémon Center, and so on.
// Each has its own local id, LOCALID_MECH_FIRST + its index.

extern const u8 Mechanics_EventScript_MoveTutor[];

struct MechNpc
{
    bool32 (*isHere)(void);
    s16 x, y;
    u16 gfx;
    u8 movement;
    const u8 *script;
};

static bool32 InPokemonCenter(void)
{
    return gMapHeader.mapLayoutId == LAYOUT_POKEMON_CENTER_1F
        || gMapHeader.mapLayoutId == LAYOUT_LAVARIDGE_TOWN_POKEMON_CENTER_1F;
}

static const struct MechNpc sMechNpcs[] =
{
    {InPokemonCenter, 13, 5, OBJ_EVENT_GFX_EXPERT_M, MOVEMENT_TYPE_FACE_LEFT, Mechanics_EventScript_MoveTutor},
};

static EWRAM_DATA u8 sTimer = 0;

static bool32 IsShown(u32 i)
{
    return GetObjectEventIdByLocalIdAndMap(LOCALID_MECH_FIRST + i, gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup) != OBJECT_EVENTS_COUNT;
}

static void UpdateNpcs(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMechNpcs); i++)
    {
        const struct MechNpc *npc = &sMechNpcs[i];
        bool32 wanted = npc->isHere();

        if (wanted == IsShown(i))
            continue;
        if (wanted)
        {
            s16 x = npc->x + MAP_OFFSET, y = npc->y + MAP_OFFSET;
            SpawnSpecialObjectEventParameterized(npc->gfx, npc->movement, LOCALID_MECH_FIRST + i,
                                                 x, y, MapGridGetElevationAt(x, y));
        }
        else
        {
            RemoveObjectEventByLocalIdAndMap(LOCALID_MECH_FIRST + i, gSaveBlock1Ptr->location.mapNum,
                                             gSaveBlock1Ptr->location.mapGroup);
        }
    }
}

void Mechanics_Update(void)
{
    if (++sTimer < 8 || ArePlayerFieldControlsLocked())
        return;
    sTimer = 0;
    UpdateNpcs();
}

bool32 Mechanics_TryInteract(s16 x, s16 y)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMechNpcs); i++)
    {
        const struct MechNpc *npc = &sMechNpcs[i];
        u8 objectId;

        if (npc->x + MAP_OFFSET != x || npc->y + MAP_OFFSET != y || !IsShown(i))
            continue;
        objectId = GetObjectEventIdByLocalIdAndMap(LOCALID_MECH_FIRST + i, gSaveBlock1Ptr->location.mapNum,
                                                   gSaveBlock1Ptr->location.mapGroup);
        gSelectedObjectEvent = objectId;
        gSpecialVar_LastTalked = LOCALID_MECH_FIRST + i;
        gSpecialVar_Facing = GetPlayerFacingDirection();
        ScriptContext_SetupScript(npc->script);
        return TRUE;
    }
    return FALSE;
}
