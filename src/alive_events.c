#include "global.h"
#include "alive.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "data.h"
#include "follower_helper.h"
#include "item.h"
#include "metatile_behavior.h"
#include "money.h"
#include "netplay_ext.h"
#include "pokemon.h"
#include "random.h"
#include "save_ext.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "string_util.h"
#include "wild_encounter.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/songs.h"

// Surprise events on the routes (docs/alive.md): a stampede, a lost child
// to walk to town, a Pokémon stuck up a ledge, a crate washed up on the
// shore, a hurt wild Pokémon that needs a Potion. At most a few a day.

#define EVENTS_PER_DAY 5
#define STAMPEDE_SIZE  3

enum
{
    SLOT_A, // the main person or Pokémon
    SLOT_B, // a second one (the stuck Pokémon's owner)
    SLOT_C,
    SLOT_DROP, // something left behind
};

static EWRAM_DATA struct
{
    u8 kind;          // ALIVE_INCIDENT_* on this map
    u8 started;
    u8 done;
    u16 timer;
    u16 species;
    u8 level;
    u8 steps;
    s16 goalX, goalY; // where the stampede runs to
    u8 childFollowing;  // carried over from map to map
    u8 childGfx;
    u8 childHomeMsg;  // the child just got to town (the script is running)
} sEvent = {0};

static bool32 PickWild(u16 *species, u8 *level)
{
    u16 header = GetCurrentMapWildMonHeaderId();
    const struct WildPokemonInfo *info;
    const struct WildPokemon *mon;

    if (header == HEADER_NONE)
        return FALSE;
    info = gWildMonHeaders[header].encounterTypes[GetTimeOfDayForEncounters(header, WILD_AREA_LAND)].landMonsInfo;
    if (info == NULL || info->wildPokemon == NULL)
        return FALSE;
    mon = &info->wildPokemon[Random() % NUM_LAND_MONS_ENCOUNTER_SLOTS];
    if (mon->species == SPECIES_NONE)
        return FALSE;
    *species = mon->species;
    *level = mon->minLevel;
    return TRUE;
}

static u8 Slot(u32 slot)
{
    return LOCALID_ALIVE_INCIDENT + slot;
}

bool32 AliveEvents_IsIncidentObject(struct ObjectEvent *obj)
{
    return (obj->localId >= LOCALID_ALIVE_INCIDENT && obj->localId < LOCALID_ALIVE_INCIDENT + ALIVE_INCIDENT_OBJECTS)
        || obj->localId == LOCALID_ALIVE_LOST_CHILD;
}

// The server puts good deeds in the news (server/alive.py AEV).
static EWRAM_DATA struct
{
    u8 kind;
    u8 waiting;
    u8 mapGroup, mapNum;
} sReport = {0};

static void Report(u32 kind)
{
    if (!Npx_IsOnline() || sReport.kind != 0)
        return;
    sReport.kind = kind;
    sReport.mapGroup = gSaveBlock1Ptr->location.mapGroup;
    sReport.mapNum = gSaveBlock1Ptr->location.mapNum;
}

static void UpdateReport(void)
{
    if (sReport.waiting)
    {
        if (NpxRpc_Poll() == NPX_RPC_WAITING)
            return;
        NpxRpc_Release();
        sReport.waiting = FALSE;
        sReport.kind = 0;
        return;
    }
    if (sReport.kind == 0 || !Npx_IsOnline() || ArePlayerFieldControlsLocked() || !NpxRpc_Begin("AEV"))
        return;
    NpxRpc_Num(sReport.kind);
    NpxRpc_Num(sReport.mapGroup);
    NpxRpc_Num(sReport.mapNum);
    NpxRpc_Send();
    sReport.waiting = TRUE;
}

static void Helped(void)
{
    if (gSaveBlockExt.alive.helped < 0xFFFF)
        gSaveBlockExt.alive.helped++;
}

// --- Starting one -----------------------------------------------------------

static bool32 Allowed(void)
{
    struct AliveSave *save = &gSaveBlockExt.alive;
    u16 day = Alive_Day() + 1;
    if (save->eventDay != day)
    {
        save->eventDay = day;
        save->eventsToday = 0;
    }
    return save->eventsToday < EVENTS_PER_DAY;
}

static bool32 StartStampede(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 i, n = 0;
    bool32 fromWest = Random() & 1;
    s16 x, y;

    if (!PickWild(&sEvent.species, &sEvent.level))
        return FALSE;
    for (i = 0; i < STAMPEDE_SIZE; i++)
    {
        if (!Alive_FindSpotNear(player->currentCoords.x + (fromWest ? -6 : 6), player->currentCoords.y - 2 + i * 2, 1, &x, &y))
            continue;
        if (Alive_Spawn(Slot(n), Alive_MonGfx(sEvent.species), x, y, fromWest ? DIR_EAST : DIR_WEST) < OBJECT_EVENTS_COUNT)
            n++;
    }
    if (n == 0)
        return FALSE;
    sEvent.goalX = player->currentCoords.x + (fromWest ? 12 : -12);
    sEvent.goalY = player->currentCoords.y;
    StringCopy(gStringVar1, GetSpeciesName(sEvent.species));
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("A herd of wild {STR_VAR_1} stampedes past!"));
    Alive_ShowCaption(NULL, gStringVar4);
    PlayCry_ByMode(sEvent.species, 0, CRY_MODE_ENCOUNTER);
    Report(3);
    return TRUE;
}

static bool32 StartLostChild(void)
{
    s16 x, y;
    if (sEvent.childFollowing || !Alive_FindSpotOnScreen(3, &x, &y))
        return FALSE;
    sEvent.childGfx = (Random() & 1) ? OBJ_EVENT_GFX_LITTLE_GIRL : OBJ_EVENT_GFX_LITTLE_BOY;
    if (Alive_Spawn(LOCALID_ALIVE_LOST_CHILD, sEvent.childGfx, x, y, DIR_SOUTH) >= OBJECT_EVENTS_COUNT)
        return FALSE;
    Alive_Emote(Alive_Obj(LOCALID_ALIVE_LOST_CHILD), FOLLOWER_EMOTION_SAD);
    return TRUE;
}

// Up on a ledge or a rock, next to where its owner stands.
static bool32 StartStuckMon(void)
{
    static const u16 sPets[] = {SPECIES_SKITTY, SPECIES_ZIGZAGOON, SPECIES_AZURILL, SPECIES_PLUSLE, SPECIES_MINUN,
                                SPECIES_PICHU, SPECIES_EEVEE, SPECIES_TEDDIURSA};
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 tries;

    for (tries = 0; tries < 40; tries++)
    {
        s16 x = player->currentCoords.x - 6 + Random() % 13;
        s16 y = player->currentCoords.y - 4 + Random() % 9;
        if (Alive_PlayerDistance(x, y) < 3 || !Alive_CanStandAt(x, y))
            continue;
        // The tile above them is blocked: up there is the Pokémon.
        if (MapGridGetCollisionAt(x, y - 1) == 0 || GetObjectEventIdByXY(x, y - 1) != OBJECT_EVENTS_COUNT
         || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y - 1)))
            continue;
        sEvent.species = sPets[Random() % ARRAY_COUNT(sPets)];
        if (Alive_Spawn(Slot(SLOT_B), OBJ_EVENT_GFX_WOMAN_2, x, y, DIR_NORTH) >= OBJECT_EVENTS_COUNT)
            return FALSE;
        if (Alive_Spawn(Slot(SLOT_A), Alive_MonGfx(sEvent.species), x, y - 1, DIR_SOUTH) >= OBJECT_EVENTS_COUNT)
        {
            Alive_Remove(Slot(SLOT_B));
            return FALSE;
        }
        Alive_Emote(Alive_Obj(Slot(SLOT_B)), FOLLOWER_EMOTION_UPSET);
        return TRUE;
    }
    return FALSE;
}

static bool32 NextToWater(s16 x, s16 y)
{
    return MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x + 1, y))
        || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x - 1, y))
        || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y + 1))
        || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y - 1));
}

static bool32 StartCrate(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 tries;

    for (tries = 0; tries < 80; tries++)
    {
        s16 x = player->currentCoords.x - 7 + Random() % 15;
        s16 y = player->currentCoords.y - 4 + Random() % 9;
        if (Alive_PlayerDistance(x, y) < 2 || !Alive_CanStandAt(x, y) || !NextToWater(x, y))
            continue;
        return Alive_Spawn(Slot(SLOT_A), OBJ_EVENT_GFX_MOVING_BOX, x, y, DIR_SOUTH) < OBJECT_EVENTS_COUNT;
    }
    return FALSE;
}

static bool32 StartHurtMon(void)
{
    s16 x, y;
    if (!PickWild(&sEvent.species, &sEvent.level) || !Alive_FindSpotOnScreen(3, &x, &y))
        return FALSE;
    if (Alive_Spawn(Slot(SLOT_A), Alive_MonGfx(sEvent.species), x, y, DIR_SOUTH) >= OBJECT_EVENTS_COUNT)
        return FALSE;
    Alive_Emote(Alive_Obj(Slot(SLOT_A)), FOLLOWER_EMOTION_SAD);
    return TRUE;
}

static void TryStart(void)
{
    u32 kind;
    bool32 ok = FALSE;

    sEvent.started = TRUE;
    if (gAliveShowcase.scene == ALIVE_SHOW_INCIDENT)
    {
        kind = gAliveShowcase.arg;
        gAliveShowcase.scene = ALIVE_SHOW_NONE;
    }
    else
    {
        if (!Alive_IsRoute() || Alive_IsQuiet() || Alive_IsNight() || Random() % 5 != 0 || !Allowed())
            return;
        kind = 1 + Random() % (ALIVE_INCIDENT_COUNT - 1);
    }
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) || !Alive_RoomForObjects(4))
        return;
    switch (kind)
    {
    case ALIVE_INCIDENT_STAMPEDE:   ok = StartStampede(); break;
    case ALIVE_INCIDENT_LOST_CHILD: ok = StartLostChild(); break;
    case ALIVE_INCIDENT_STUCK_MON:  ok = StartStuckMon(); break;
    case ALIVE_INCIDENT_CRATE:      ok = StartCrate(); break;
    case ALIVE_INCIDENT_HURT_MON:   ok = StartHurtMon(); break;
    }
    if (ok)
    {
        sEvent.kind = kind;
        sEvent.timer = 0;
        sEvent.done = FALSE;
        gSaveBlockExt.alive.eventsToday++;
    }
}

// --- Running ----------------------------------------------------------------

static void UpdateStampede(void)
{
    u32 i, left = 0;
    if (sEvent.done)
        return;
    for (i = 0; i < STAMPEDE_SIZE; i++)
    {
        struct ObjectEvent *obj = Alive_Obj(Slot(i));
        if (obj == NULL)
            continue;
        left++;
        if (Alive_Busy(obj))
            continue;
        if (sEvent.steps > 30 || !Alive_StepToward(obj, sEvent.goalX, sEvent.goalY + (s16)i - 1, TRUE))
        {
            // Gone over the hill. The first leaves a berry behind.
            if (i == 0 && Alive_Obj(Slot(SLOT_DROP)) == NULL && Alive_CanStandAt(obj->currentCoords.x, obj->currentCoords.y + 1))
                Alive_Spawn(Slot(SLOT_DROP), OBJ_EVENT_GFX_ITEM_BALL, obj->currentCoords.x, obj->currentCoords.y + 1, DIR_SOUTH);
            Alive_Remove(Slot(i));
        }
    }
    sEvent.steps++;
    if (left == 0)
        sEvent.done = TRUE;
}

static void UpdateChild(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *child = Alive_Obj(LOCALID_ALIVE_LOST_CHILD);
    extern const u8 Alive_EventScript_ChildHome[];

    if (!sEvent.childFollowing)
        return;
    if (child == NULL)
    {
        // A new map: they're right behind the player.
        s16 x = player->previousCoords.x, y = player->previousCoords.y;
        if (!Alive_CanStandAt(x, y) && !Alive_FindSpotNear(player->currentCoords.x, player->currentCoords.y, 1, &x, &y))
            return;
        Alive_Spawn(LOCALID_ALIVE_LOST_CHILD, sEvent.childGfx, x, y, player->facingDirection);
        return;
    }
    if (Alive_IsTown() && !sEvent.childHomeMsg)
    {
        sEvent.childHomeMsg = TRUE;
        ScriptContext_SetupScript(Alive_EventScript_ChildHome);
        return;
    }
    if (Alive_Busy(child))
        return;
    // Keep up.
    if (Alive_PlayerDistance(child->currentCoords.x, child->currentCoords.y) > 1)
    {
        if (!Alive_StepToward(child, player->previousCoords.x, player->previousCoords.y, TRUE)
         && Alive_PlayerDistance(child->currentCoords.x, child->currentCoords.y) > 4)
            Alive_Remove(LOCALID_ALIVE_LOST_CHILD); // respawns behind the player
    }
}

void AliveEvents_OnMapTransition(void)
{
    u8 following = sEvent.childFollowing, gfx = sEvent.childGfx;
    memset(&sEvent, 0, sizeof(sEvent));
    sEvent.childFollowing = following;
    sEvent.childGfx = gfx;
}

void AliveEvents_Update(void)
{
    UpdateReport();
    if (!sEvent.started)
    {
        // Give the map a moment to settle first.
        if (++sEvent.timer < 30)
            return;
        TryStart();
    }
    if (sEvent.kind == ALIVE_INCIDENT_STAMPEDE)
        UpdateStampede();
    UpdateChild();
}

// --- Talking ----------------------------------------------------------------

const u8 *AliveEvents_GetTalkScript(struct ObjectEvent *obj)
{
    extern const u8 Alive_EventScript_LostChild[];
    extern const u8 Alive_EventScript_Stuck[];
    extern const u8 Alive_EventScript_Crate[];
    extern const u8 Alive_EventScript_Dropped[];
    extern const u8 Alive_EventScript_HurtMon[];

    if (!AliveEvents_IsIncidentObject(obj))
        return NULL;
    if (obj->localId == LOCALID_ALIVE_LOST_CHILD)
        return Alive_EventScript_LostChild;
    if (obj->localId == Slot(SLOT_DROP))
        return Alive_EventScript_Dropped;
    switch (sEvent.kind)
    {
    case ALIVE_INCIDENT_STUCK_MON: return Alive_EventScript_Stuck;
    case ALIVE_INCIDENT_CRATE:     return Alive_EventScript_Crate;
    case ALIVE_INCIDENT_HURT_MON:  return Alive_EventScript_HurtMon;
    }
    return NULL;
}

// Script: VAR_RESULT 1 if the child already follows the player.
void AliveEvents_ChildState(void)
{
    gSpecialVar_Result = sEvent.childFollowing;
}

void AliveEvents_ChildFollow(void)
{
    sEvent.childFollowing = TRUE;
    sEvent.childHomeMsg = FALSE;
}

// Script: the child is home. Money for the player; VAR_0x8004 a gift.
void AliveEvents_ChildReward(void)
{
    static const u16 sGifts[] = {ITEM_SUPER_POTION, ITEM_GREAT_BALL, ITEM_REVIVE, ITEM_SOOTHE_BELL, ITEM_RARE_CANDY};
    u32 money = 1000 + 500 * (Random() % 5);
    AddMoney(&gSaveBlock1Ptr->money, money);
    ConvertIntToDecimalStringN(gStringVar2, money, STR_CONV_MODE_LEFT_ALIGN, 5);
    gSpecialVar_0x8004 = sGifts[Random() % (Random() % 8 == 0 ? ARRAY_COUNT(sGifts) : 3)];
    sEvent.childFollowing = FALSE;
    Alive_Remove(LOCALID_ALIVE_LOST_CHILD);
    Helped();
    Report(1);
    AliveNews_AddLocal(COMPOUND_STRING("Lost child walked home safe by a kind TRAINER."));
}

// Script: STR_VAR_1 the stuck Pokémon, STR_VAR_2 the player's lead; VAR_RESULT 1
// if it's down already.
void AliveEvents_BufferStuck(void)
{
    StringCopy(gStringVar1, GetSpeciesName(sEvent.species));
    StringCopy(gStringVar2, GetSpeciesName(Alive_LeadSpecies()));
    gSpecialVar_Result = sEvent.done;
}

// Script: the Pokémon jumps down to its owner. VAR_0x8004 the thank-you gift.
void AliveEvents_RescueStuck(void)
{
    static const u16 sGifts[] = {ITEM_LEMONADE, ITEM_FRESH_WATER, ITEM_SODA_POP, ITEM_MOOMOO_MILK, ITEM_LAVA_COOKIE, ITEM_SOOTHE_BELL};
    struct ObjectEvent *mon = Alive_Obj(Slot(SLOT_A)), *owner = Alive_Obj(Slot(SLOT_B));
    if (mon != NULL && owner != NULL)
    {
        // Down it hops, to the owner's side.
        s16 x = owner->currentCoords.x + 1, y = owner->currentCoords.y;
        if (!Alive_CanStandAt(x, y))
            x = owner->currentCoords.x - 1;
        MoveObjectEventToMapCoords(mon, x, y);
        Alive_Do(mon, GetJumpInPlaceMovementAction(DIR_SOUTH));
        Alive_Emote(mon, FOLLOWER_EMOTION_HAPPY);
        Alive_Emote(owner, FOLLOWER_EMOTION_LOVE);
        PlayCry_ByMode(sEvent.species, 0, CRY_MODE_NORMAL);
    }
    gSpecialVar_0x8004 = sGifts[Random() % ARRAY_COUNT(sGifts)];
    sEvent.done = TRUE;
    Helped();
    Report(2);
}

// Script: VAR_0x8004 what was in the crate (or dropped by the herd).
void AliveEvents_OpenCrate(void)
{
    static const u16 sCrate[] = {ITEM_PEARL, ITEM_STARDUST, ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_SHOAL_SALT,
                                 ITEM_HEART_SCALE, ITEM_MAX_ETHER, ITEM_BIG_PEARL, ITEM_STAR_PIECE, ITEM_NUGGET};
    static const u16 sDrop[] = {ITEM_ORAN_BERRY, ITEM_PECHA_BERRY, ITEM_CHERI_BERRY, ITEM_SITRUS_BERRY, ITEM_LEPPA_BERRY};
    bool32 crate = (gSpecialVar_LastTalked != Slot(SLOT_DROP));

    if (crate)
    {
        u32 roll = Random() % 100;
        gSpecialVar_0x8004 = sCrate[roll < 85 ? roll % 7 : 7 + roll % 3];
        Report(4);
    }
    else
    {
        gSpecialVar_0x8004 = sDrop[Random() % ARRAY_COUNT(sDrop)];
    }
    Alive_Remove(gSpecialVar_LastTalked);
}

// Script: STR_VAR_1 the hurt Pokémon; VAR_0x8004 its species (for the cry);
// VAR_RESULT TRUE if the player has something to heal it (VAR_0x8005).
void AliveEvents_BufferHurt(void)
{
    static const u16 sHeals[] = {ITEM_POTION, ITEM_ORAN_BERRY, ITEM_SITRUS_BERRY, ITEM_SUPER_POTION, ITEM_HYPER_POTION,
                                 ITEM_FRESH_WATER, ITEM_BERRY_JUICE};
    u32 i;

    StringCopy(gStringVar1, GetSpeciesName(sEvent.species));
    gSpecialVar_0x8004 = sEvent.species;
    gSpecialVar_Result = FALSE;
    if (sEvent.done)
        return;
    for (i = 0; i < ARRAY_COUNT(sHeals); i++)
    {
        if (CheckBagHasItem(sHeals[i], 1))
        {
            gSpecialVar_0x8005 = sHeals[i];
            gSpecialVar_Result = TRUE;
            return;
        }
    }
}

// Script: uses VAR_0x8005 on it. VAR_RESULT 1 if it wants to join the player,
// else VAR_0x8004 the item it leaves behind.
void AliveEvents_HealHurt(void)
{
    static const u16 sGifts[] = {ITEM_ORAN_BERRY, ITEM_PECHA_BERRY, ITEM_TINY_MUSHROOM, ITEM_PRETTY_FEATHER, ITEM_SILK_SCARF};
    struct ObjectEvent *mon = Alive_Obj(Slot(SLOT_A));

    RemoveBagItem(gSpecialVar_0x8005, 1);
    if (mon != NULL)
    {
        Alive_Do(mon, GetJumpInPlaceMovementAction(DIR_SOUTH));
        Alive_Emote(mon, FOLLOWER_EMOTION_HAPPY);
    }
    PlayCry_ByMode(sEvent.species, 0, CRY_MODE_NORMAL);
    sEvent.done = TRUE;
    Helped();
    Report(2);
    gSpecialVar_Result = (Random() % 100 < 50);
    gSpecialVar_0x8004 = sGifts[Random() % ARRAY_COUNT(sGifts)];
}

// Script: the rescued Pokémon joins. VAR_RESULT MON_GIVEN_TO_*.
void AliveEvents_HurtJoins(void)
{
    gSpecialVar_Result = ScriptGiveMon(sEvent.species, max(sEvent.level, 2), ITEM_NONE);
    if (gSpecialVar_Result != MON_CANT_GIVE)
        Alive_Remove(Slot(SLOT_A));
}

// Script: the healed Pokémon runs off into the grass.
void AliveEvents_HurtLeaves(void)
{
    Alive_Remove(Slot(SLOT_A));
}
