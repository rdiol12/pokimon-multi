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
#include "pokemon.h"
#include "random.h"
#include "save_ext.h"
#include "script.h"
#include "string_util.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/map_groups.h"
#include "constants/metatile_labels.h"
#include "constants/species.h"

// Towns with daily routines (docs/alive.md): townsfolk head home in the
// evening and come out again in the morning, Poké Marts close for the
// night, and every town has resident Pokémon going about their day.

// --- Townsfolk going home and coming out ---------------------------------

#define MAX_COMMUTERS 12

enum
{
    COMMUTE_OUT,        // where the map put them
    COMMUTE_GOING_HOME, // walking to a door
    COMMUTE_HOME,       // indoors (hidden on a wall tile)
    COMMUTE_COMING_OUT, // walking from a door to their spot
};

struct Commuter
{
    u8 localId;
    u8 state;
    u8 steps;
    u8 movementType; // the map's, restored once they're back
    s16 homeX, homeY; // their door
    s16 spotX, spotY; // where the map put them
};

static EWRAM_DATA struct
{
    struct Commuter people[MAX_COMMUTERS];
    u8 count;
    u8 scanned;
    u8 timer;
    u8 walking; // index + 1 of the one walking now
} sTown = {0};

static bool32 IsWanderer(u8 movementType)
{
    switch (movementType)
    {
    case MOVEMENT_TYPE_WANDER_AROUND:
    case MOVEMENT_TYPE_WANDER_UP_AND_DOWN:
    case MOVEMENT_TYPE_WANDER_DOWN_AND_UP:
    case MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT:
    case MOVEMENT_TYPE_WANDER_RIGHT_AND_LEFT:
    case MOVEMENT_TYPE_LOOK_AROUND:
    case MOVEMENT_TYPE_WALK_UP_AND_DOWN:
    case MOVEMENT_TYPE_WALK_DOWN_AND_UP:
    case MOVEMENT_TYPE_WALK_LEFT_AND_RIGHT:
    case MOVEMENT_TYPE_WALK_RIGHT_AND_LEFT:
        return TRUE;
    default:
        return FALSE;
    }
}

// The door nearest (x, y), and a wall tile next to it to stand them on
// while they're indoors.
static bool32 NearestDoor(s16 x, s16 y, s16 *doorX, s16 *doorY)
{
    const struct MapEvents *events = gMapHeader.events;
    u32 i, best = 0xFFFF;

    for (i = 0; i < events->warpCount; i++)
    {
        s16 wx = events->warps[i].x + MAP_OFFSET, wy = events->warps[i].y + MAP_OFFSET;
        u32 behavior = MapGridGetMetatileBehaviorAt(wx, wy);
        u32 d = abs(wx - x) + abs(wy - y);
        if (!MetatileBehavior_IsDoor(behavior) && !MetatileBehavior_IsWarpDoor(behavior))
            continue;
        if (d < best)
        {
            best = d;
            *doorX = wx;
            *doorY = wy;
        }
    }
    return best != 0xFFFF && best < 30;
}

static bool32 GoesHome(u8 localId)
{
    // Two in three head in for the night; the rest are night owls.
    return (localId * 7 + gSaveBlock1Ptr->location.mapNum) % 3 != 0;
}

static void ScanTownsfolk(void)
{
    u32 i;

    sTown.scanned = TRUE;
    sTown.count = 0;
    if (!Alive_IsTown())
        return;
    for (i = 0; i < OBJECT_EVENTS_COUNT && sTown.count < MAX_COMMUTERS; i++)
    {
        struct ObjectEvent *obj = &gObjectEvents[i];
        struct Commuter *c;
        if (!Alive_IsTownsperson(obj) || !IsWanderer(obj->movementType) || !GoesHome(obj->localId))
            continue;
        c = &sTown.people[sTown.count];
        if (!NearestDoor(obj->initialCoords.x, obj->initialCoords.y, &c->homeX, &c->homeY))
            continue;
        c->localId = obj->localId;
        c->state = COMMUTE_OUT;
        c->movementType = obj->movementType;
        c->spotX = obj->initialCoords.x;
        c->spotY = obj->initialCoords.y;
        sTown.count++;
    }
}

static void Hide(struct ObjectEvent *obj, struct Commuter *c)
{
    // Out of the way on the wall above their door, out of sight.
    SetTrainerMovementType(obj, MOVEMENT_TYPE_NONE);
    MoveObjectEventToMapCoords(obj, c->homeX, c->homeY - 1);
    obj->invisible = TRUE;
    c->state = COMMUTE_HOME;
}

static void Show(struct ObjectEvent *obj, struct Commuter *c)
{
    MoveObjectEventToMapCoords(obj, c->homeX, c->homeY);
    obj->invisible = FALSE;
    ObjectEventTurn(obj, DIR_SOUTH);
    c->state = COMMUTE_COMING_OUT;
    c->steps = 0;
}

static void Restore(struct ObjectEvent *obj, struct Commuter *c)
{
    obj->invisible = FALSE;
    SetTrainerMovementType(obj, c->movementType);
    c->state = COMMUTE_OUT;
}

// The part of the day the townsfolk keep to.
enum { DAY_OUT, DAY_EVENING, DAY_NIGHT, DAY_MORNING };

static u32 TownPhase(void)
{
    u32 hour = Alive_Hour();
    if (hour >= 21 || hour < 5)
        return DAY_NIGHT;
    if (hour >= 19)
        return DAY_EVENING;
    if (hour < 8)
        return DAY_MORNING;
    return DAY_OUT;
}

static bool32 OnScreen(s16 x, s16 y)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    return abs(x - player->currentCoords.x) <= 8 && abs(y - player->currentCoords.y) <= 6;
}

static void UpdateCommuters(void)
{
    u32 i, phase = TownPhase();

    if (!sTown.scanned)
    {
        ScanTownsfolk();
        // Night already: everyone who goes home is home.
        for (i = 0; i < sTown.count; i++)
        {
            struct ObjectEvent *obj = Alive_Obj(sTown.people[i].localId);
            if (obj == NULL)
                continue;
            if (phase == DAY_NIGHT)
                Hide(obj, &sTown.people[i]);
        }
    }
    for (i = 0; i < sTown.count; i++)
    {
        struct Commuter *c = &sTown.people[i];
        struct ObjectEvent *obj = Alive_Obj(c->localId);

        if (obj == NULL)
            continue;
        // Respawned by the camera at their spot (the map reloads its
        // people as they come into view).
        if (c->state == COMMUTE_HOME && !obj->invisible)
        {
            Hide(obj, c);
            continue;
        }
        switch (c->state)
        {
        case COMMUTE_OUT:
            if (phase == DAY_NIGHT)
            {
                if (!OnScreen(obj->currentCoords.x, obj->currentCoords.y))
                    Hide(obj, c);
            }
            else if (phase == DAY_EVENING && sTown.walking == 0 && Random() % 200 == 0)
            {
                // Off home, one at a time.
                SetTrainerMovementType(obj, MOVEMENT_TYPE_NONE);
                c->state = COMMUTE_GOING_HOME;
                c->steps = 0;
                sTown.walking = i + 1;
            }
            break;
        case COMMUTE_GOING_HOME:
            if (Alive_Busy(obj))
                break;
            if ((obj->currentCoords.x == c->homeX && obj->currentCoords.y == c->homeY + 1)
             || (obj->currentCoords.x == c->homeX && obj->currentCoords.y == c->homeY)
             || ++c->steps > 40 || !Alive_StepToward(obj, c->homeX, c->homeY + 1, FALSE))
            {
                if (obj->currentCoords.x == c->homeX && obj->currentCoords.y == c->homeY + 1)
                    Alive_Do(obj, GetWalkNormalMovementAction(DIR_NORTH));
                Hide(obj, c);
                if (sTown.walking == i + 1)
                    sTown.walking = 0;
            }
            break;
        case COMMUTE_HOME:
            if ((phase == DAY_MORNING || phase == DAY_OUT) && sTown.walking == 0
             && Alive_CanStandAt(c->homeX, c->homeY + 1) && (phase == DAY_OUT || Random() % 120 == 0))
            {
                if (phase == DAY_OUT && !OnScreen(c->spotX, c->spotY) && !OnScreen(c->homeX, c->homeY))
                {
                    MoveObjectEventToMapCoords(obj, c->spotX, c->spotY);
                    Restore(obj, c);
                }
                else
                {
                    Show(obj, c);
                    sTown.walking = i + 1;
                }
            }
            break;
        case COMMUTE_COMING_OUT:
            if (Alive_Busy(obj))
                break;
            if (++c->steps > 40 || !Alive_StepToward(obj, c->spotX, c->spotY, FALSE))
            {
                Restore(obj, c);
                if (sTown.walking == i + 1)
                    sTown.walking = 0;
            }
            break;
        }
    }
}

bool32 Alive_IsShopClosed(void)
{
    u32 hour;
    if (gMapHeader.mapLayoutId != LAYOUT_MART || Alive_IsQuiet())
        return FALSE;
    hour = Alive_Hour();
    return hour >= 22 || hour < 6;
}

// --- Town Pokémon ----------------------------------------------------------

enum
{
    ANCHOR_CENTER, // by the Pokémon Center's door
    ANCHOR_MART,   // by the Poké Mart's door
    ANCHOR_SHORE,  // on the shore nearest the middle of town
    ANCHOR_XY,     // at x, y
};

enum
{
    HABIT_WANDER, // ambles about
    HABIT_PACE,   // walks to and fro, east-west (busy at work)
    HABIT_SIT,    // stays put, looking around
    HABIT_HOP,    // bounces in place
};

struct TownMon
{
    u16 map;
    u16 species;
    u16 gfx;      // 0: the species' own
    u8 anchor;
    u8 habit;
    s8 x, y;      // ANCHOR_XY: the spot; otherwise an offset from the anchor
    u8 gift;      // 1: finds things (once a day)
    const u8 *text;
};

#define TOWN(m) MAP_##m

static const struct TownMon sTownMons[] =
{
    {TOWN(LITTLEROOT_TOWN), SPECIES_ZIGZAGOON, 0, ANCHOR_XY, HABIT_WANDER, 10, 14, 1,
        COMPOUND_STRING("ZIGZAGOON is nosing through the\nflower beds. It looks very busy.")},
    {TOWN(LITTLEROOT_TOWN), SPECIES_TAILLOW, 0, ANCHOR_XY, HABIT_SIT, 6, 6, 0,
        COMPOUND_STRING("TAILLOW is perched here, watching\nthe road to ROUTE 101.")},
    {TOWN(OLDALE_TOWN), SPECIES_ZIGZAGOON, 0, ANCHOR_CENTER, HABIT_WANDER, 2, 2, 1,
        COMPOUND_STRING("This ZIGZAGOON lives by the\nPOKéMON CENTER. It loves digging.")},
    {TOWN(OLDALE_TOWN), SPECIES_POOCHYENA, 0, ANCHOR_MART, HABIT_SIT, 2, 2, 0,
        COMPOUND_STRING("POOCHYENA is waiting for its\ntrainer outside the MART.")},
    {TOWN(PETALBURG_CITY), SPECIES_SLAKOTH, 0, ANCHOR_CENTER, HABIT_SIT, -3, 2, 0,
        COMPOUND_STRING("SLAKOTH hasn't moved all day.\nIt yawns at you.")},
    {TOWN(PETALBURG_CITY), SPECIES_WINGULL, 0, ANCHOR_SHORE, HABIT_SIT, 0, 0, 0,
        COMPOUND_STRING("WINGULL is resting by the water.")},
    {TOWN(RUSTBORO_CITY), SPECIES_VIGOROTH, OBJ_EVENT_GFX_VIGOROTH_CARRYING_BOX, ANCHOR_CENTER, HABIT_PACE, 3, 3, 0,
        COMPOUND_STRING("VIGOROTH is hauling parts for the\nDEVON CORPORATION. Hard worker!")},
    {TOWN(RUSTBORO_CITY), SPECIES_NOSEPASS, 0, ANCHOR_MART, HABIT_SIT, -2, 2, 0,
        COMPOUND_STRING("NOSEPASS is pointing north.\nIt always points north.")},
    {TOWN(DEWFORD_TOWN), SPECIES_WINGULL, 0, ANCHOR_SHORE, HABIT_WANDER, 0, 0, 1,
        COMPOUND_STRING("WINGULL picks things up on the\nbeach and brings them to town.")},
    {TOWN(DEWFORD_TOWN), SPECIES_MAKUHITA, 0, ANCHOR_CENTER, HABIT_PACE, 2, 3, 0,
        COMPOUND_STRING("MAKUHITA is training for the GYM.\nHuff! Huff!")},
    {TOWN(SLATEPORT_CITY), SPECIES_VIGOROTH, OBJ_EVENT_GFX_VIGOROTH_CARRYING_BOX, ANCHOR_MART, HABIT_PACE, 2, 3, 0,
        COMPOUND_STRING("VIGOROTH carries crates for the\nshipyard. It looks proud of it!")},
    {TOWN(SLATEPORT_CITY), SPECIES_WINGULL, 0, ANCHOR_SHORE, HABIT_SIT, 0, 0, 0,
        COMPOUND_STRING("WINGULL is waiting for the\nfishing boats to come in.")},
    {TOWN(SLATEPORT_CITY), SPECIES_PELIPPER, 0, ANCHOR_CENTER, HABIT_SIT, -3, 2, 0,
        COMPOUND_STRING("PELIPPER delivers the mail here.\nIts bill is full of letters.")},
    {TOWN(MAUVILLE_CITY), SPECIES_ELECTRIKE, 0, ANCHOR_CENTER, HABIT_WANDER, 3, 2, 0,
        COMPOUND_STRING("ELECTRIKE crackles happily. The\nstreetlights flicker as it passes.")},
    {TOWN(MAUVILLE_CITY), SPECIES_MAGNEMITE, 0, ANCHOR_MART, HABIT_HOP, -2, 2, 0,
        COMPOUND_STRING("MAGNEMITE hums. It's checking the\nwiring of the city.")},
    {TOWN(VERDANTURF_TOWN), SPECIES_WHISMUR, 0, ANCHOR_CENTER, HABIT_SIT, 2, 2, 0,
        COMPOUND_STRING("WHISMUR is whispering very\nquietly. The town is so peaceful.")},
    {TOWN(VERDANTURF_TOWN), SPECIES_SKITTY, 0, ANCHOR_MART, HABIT_WANDER, 2, 2, 1,
        COMPOUND_STRING("SKITTY is chasing its own tail.\nIt found something shiny!")},
    {TOWN(FALLARBOR_TOWN), SPECIES_SPINDA, 0, ANCHOR_CENTER, HABIT_WANDER, 2, 3, 0,
        COMPOUND_STRING("SPINDA wobbles around in circles.\nIt seems happy that way.")},
    {TOWN(FALLARBOR_TOWN), SPECIES_NUMEL, 0, ANCHOR_MART, HABIT_SIT, 2, 2, 0,
        COMPOUND_STRING("NUMEL is warming its back in the\nfalling ash.")},
    {TOWN(LAVARIDGE_TOWN), SPECIES_TORKOAL, 0, ANCHOR_CENTER, HABIT_SIT, 3, 2, 0,
        COMPOUND_STRING("TORKOAL puffs out a little smoke\nring. It's enjoying the hot springs.")},
    {TOWN(LAVARIDGE_TOWN), SPECIES_NUMEL, 0, ANCHOR_MART, HABIT_WANDER, 2, 2, 0,
        COMPOUND_STRING("NUMEL is ambling between the\nhot springs.")},
    {TOWN(FORTREE_CITY), SPECIES_TROPIUS, 0, ANCHOR_CENTER, HABIT_SIT, 3, 2, 1,
        COMPOUND_STRING("TROPIUS grows fruit around its neck.\nIt shares some with visitors!")},
    {TOWN(FORTREE_CITY), SPECIES_SWABLU, 0, ANCHOR_MART, HABIT_HOP, 2, 2, 0,
        COMPOUND_STRING("SWABLU is fluffing its cotton\nwings. It hums a little tune.")},
    {TOWN(LILYCOVE_CITY), SPECIES_MEOWTH, 0, ANCHOR_CENTER, HABIT_WANDER, 2, 2, 1,
        COMPOUND_STRING("This MEOWTH lives off coins people\ndrop near the DEPARTMENT STORE.")},
    {TOWN(LILYCOVE_CITY), SPECIES_WINGULL, 0, ANCHOR_SHORE, HABIT_SIT, 0, 0, 0,
        COMPOUND_STRING("WINGULL is watching the ferry.")},
    {TOWN(MOSSDEEP_CITY), SPECIES_SPOINK, 0, ANCHOR_CENTER, HABIT_HOP, 2, 2, 0,
        COMPOUND_STRING("SPOINK is bouncing on its tail.\nIf it stops, its heart stops!")},
    {TOWN(MOSSDEEP_CITY), SPECIES_WINGULL, 0, ANCHOR_SHORE, HABIT_WANDER, 0, 0, 1,
        COMPOUND_STRING("WINGULL brings shells up from\nthe beach.")},
    {TOWN(SOOTOPOLIS_CITY), SPECIES_AZUMARILL, 0, ANCHOR_CENTER, HABIT_SIT, 2, 2, 0,
        COMPOUND_STRING("AZUMARILL is listening to the\nwater in the crater.")},
    {TOWN(SOOTOPOLIS_CITY), SPECIES_SPHEAL, 0, ANCHOR_MART, HABIT_HOP, 2, 2, 0,
        COMPOUND_STRING("SPHEAL claps its flippers at you!")},
    {TOWN(PACIFIDLOG_TOWN), SPECIES_CORPHISH, 0, ANCHOR_CENTER, HABIT_WANDER, 2, 1, 1,
        COMPOUND_STRING("CORPHISH clacks its claws. It\nfishes things out of the sea.")},
    {TOWN(EVER_GRANDE_CITY), SPECIES_ALTARIA, 0, ANCHOR_CENTER, HABIT_SIT, 3, 2, 0,
        COMPOUND_STRING("ALTARIA hums softly. Trainers\ncome here before the LEAGUE.")},
};

// Nocturnal residents stay up.
static bool32 IsNightOwl(u16 species)
{
    return species == SPECIES_MEOWTH || species == SPECIES_ELECTRIKE || species == SPECIES_MAGNEMITE;
}

static EWRAM_DATA struct
{
    u8 index[ALIVE_TOWN_MONS]; // + 1 into sTownMons
    s16 homeX[ALIVE_TOWN_MONS], homeY[ALIVE_TOWN_MONS];
    u8 timer[ALIVE_TOWN_MONS];
    u8 paceDir[ALIVE_TOWN_MONS];
    u8 spawned;
    u8 talked; // the one just talked to (index + 1)
} sMons = {0};

static bool32 FindDoorMetatile(u32 metatile, s16 *x, s16 *y)
{
    const struct MapEvents *events = gMapHeader.events;
    u32 i;
    for (i = 0; i < events->warpCount; i++)
    {
        s16 wx = events->warps[i].x + MAP_OFFSET, wy = events->warps[i].y + MAP_OFFSET;
        if (MapGridGetMetatileIdAt(wx, wy) == metatile)
        {
            *x = wx;
            *y = wy;
            return TRUE;
        }
    }
    return FALSE;
}

// A walkable tile next to water, nearest the middle of the map.
static bool32 FindShore(s16 *outX, s16 *outY)
{
    s32 w = gMapHeader.mapLayout->width, h = gMapHeader.mapLayout->height;
    s32 cx = w / 2 + MAP_OFFSET, cy = h / 2 + MAP_OFFSET, r, x, y;

    for (r = 0; r < max(w, h); r++)
    {
        for (y = cy - r; y <= cy + r; y++)
        {
            for (x = cx - r; x <= cx + r; x++)
            {
                if (abs(x - cx) != r && abs(y - cy) != r)
                    continue;
                if (!Alive_CanStandAt(x, y))
                    continue;
                if (MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x + 1, y))
                 || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x - 1, y))
                 || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y + 1))
                 || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y - 1)))
                {
                    *outX = x;
                    *outY = y;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

static bool32 AnchorOf(const struct TownMon *t, s16 *x, s16 *y)
{
    switch (t->anchor)
    {
    case ANCHOR_CENTER:
        if (!FindDoorMetatile(METATILE_General_Door_PokeCenter, x, y))
            return FALSE;
        break;
    case ANCHOR_MART:
        if (!FindDoorMetatile(METATILE_General_Door_PokeMart, x, y))
            return FALSE;
        break;
    case ANCHOR_SHORE:
        if (!FindShore(x, y))
            return FALSE;
        break;
    default:
        *x = t->x + MAP_OFFSET;
        *y = t->y + MAP_OFFSET;
        return TRUE;
    }
    *x += t->x;
    *y += t->y;
    return TRUE;
}

static u16 TownMonGfx(const struct TownMon *t)
{
    return t->gfx != 0 ? t->gfx : Alive_MonGfx(t->species);
}

static void SpawnTownMons(void)
{
    u32 i, n = 0;
    u16 here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;

    sMons.spawned = TRUE;
    for (i = 0; i < ARRAY_COUNT(sTownMons) && n < ALIVE_TOWN_MONS; i++)
    {
        const struct TownMon *t = &sTownMons[i];
        s16 ax, ay, x, y;
        if (t->map != here || !AnchorOf(t, &ax, &ay) || !Alive_FindSpotNear(ax, ay, 3, &x, &y))
            continue;
        if (!Alive_RoomForObjects(1))
            break;
        if (Alive_Spawn(LOCALID_ALIVE_TOWN_MON + n, TownMonGfx(t), x, y, DIR_SOUTH) >= OBJECT_EVENTS_COUNT)
            continue;
        sMons.index[n] = i + 1;
        sMons.homeX[n] = x;
        sMons.homeY[n] = y;
        sMons.timer[n] = Random() % 60;
        sMons.paceDir[n] = DIR_EAST;
        n++;
    }
}

static void UpdateTownMon(u32 n)
{
    const struct TownMon *t = &sTownMons[sMons.index[n] - 1];
    struct ObjectEvent *obj = Alive_Obj(LOCALID_ALIVE_TOWN_MON + n);

    if (obj == NULL || Alive_Busy(obj))
        return;
    if (Alive_IsNight() && !IsNightOwl(t->species))
    {
        // Curled up for the night.
        if (obj->facingDirection != DIR_SOUTH)
            ObjectEventTurn(obj, DIR_SOUTH);
        if (t->gfx == 0)
            Alive_ShowZzz(obj);
        return;
    }
    if (sMons.timer[n] != 0)
    {
        sMons.timer[n]--;
        return;
    }
    switch (t->habit)
    {
    case HABIT_WANDER:
    {
        u32 dir = Random() % 4 + 1;
        s16 x = obj->currentCoords.x, y = obj->currentCoords.y;
        MoveCoords(dir, &x, &y);
        if (abs(x - sMons.homeX[n]) + abs(y - sMons.homeY[n]) <= 3 && Alive_CanStandAt(x, y))
            Alive_Do(obj, GetWalkNormalMovementAction(dir));
        else
            Alive_Do(obj, GetFaceDirectionMovementAction(dir));
        sMons.timer[n] = 30 + Random() % 60;
        break;
    }
    case HABIT_PACE:
    {
        s16 x = obj->currentCoords.x, y = obj->currentCoords.y;
        MoveCoords(sMons.paceDir[n], &x, &y);
        if (abs(x - sMons.homeX[n]) > 3 || !Alive_CanStandAt(x, y))
        {
            sMons.paceDir[n] = sMons.paceDir[n] == DIR_EAST ? DIR_WEST : DIR_EAST;
            Alive_Do(obj, GetFaceDirectionMovementAction(sMons.paceDir[n]));
            sMons.timer[n] = 40; // put the crate down, pick it up
        }
        else
        {
            Alive_Do(obj, GetWalkNormalMovementAction(sMons.paceDir[n]));
            sMons.timer[n] = 4;
        }
        break;
    }
    case HABIT_HOP:
        Alive_Do(obj, GetJumpInPlaceMovementAction(obj->facingDirection));
        sMons.timer[n] = 20 + Random() % 80;
        break;
    default:
        Alive_Do(obj, GetFaceDirectionMovementAction(Random() % 4 + 1));
        if (Random() % 4 == 0)
            Alive_Emote(obj, Random() & 1 ? FOLLOWER_EMOTION_MUSIC : FOLLOWER_EMOTION_HAPPY);
        sMons.timer[n] = 90 + Random() % 120;
        break;
    }
}

bool32 AliveTown_IsResident(struct ObjectEvent *obj)
{
    return obj->localId >= LOCALID_ALIVE_TOWN_MON && obj->localId < LOCALID_ALIVE_TOWN_MON + ALIVE_TOWN_MONS
        && sMons.index[obj->localId - LOCALID_ALIVE_TOWN_MON] != 0;
}

const u8 *AliveTown_GetTalkScript(struct ObjectEvent *obj)
{
    extern const u8 Alive_EventScript_TownMon[];
    if (!AliveTown_IsResident(obj))
        return NULL;
    sMons.talked = obj->localId - LOCALID_ALIVE_TOWN_MON + 1;
    return Alive_EventScript_TownMon;
}

// Script: STR_VAR_4 what the Pokémon is up to; VAR_0x8004 its species (for
// the cry); VAR_RESULT an item it found for the player today (or 0).
void AliveTown_BufferMonText(void)
{
    static const u16 sFinds[] = {ITEM_ORAN_BERRY, ITEM_PECHA_BERRY, ITEM_POTION, ITEM_TINY_MUSHROOM, ITEM_PRETTY_FEATHER,
                                 ITEM_POKE_BALL, ITEM_SITRUS_BERRY, ITEM_HEART_SCALE, ITEM_STARDUST, ITEM_SHOAL_SHELL};
    const struct TownMon *t;
    u32 n = sMons.talked ? sMons.talked - 1 : 0;
    u32 bit, day = Alive_Day() + 1;
    struct AliveSave *save = &gSaveBlockExt.alive;

    gSpecialVar_Result = ITEM_NONE;
    if (sMons.index[n] == 0)
    {
        StringCopy(gStringVar4, COMPOUND_STRING("It's looking at you."));
        gSpecialVar_0x8004 = SPECIES_ZIGZAGOON;
        return;
    }
    t = &sTownMons[sMons.index[n] - 1];
    gSpecialVar_0x8004 = t->species;
    if (Alive_IsNight() && !IsNightOwl(t->species))
    {
        StringCopy(gStringVar4, COMPOUND_STRING("It's fast asleep. Zzz…"));
        return;
    }
    StringCopy(gStringVar4, t->text);
    if (!t->gift)
        return;
    bit = 1u << ((sMons.index[n] - 1) % 32);
    if (save->giftDay != (u16)day)
    {
        save->giftDay = day;
        save->giftTowns = 0;
    }
    if (save->giftTowns & bit)
        return;
    save->giftTowns |= bit;
    gSpecialVar_Result = sFinds[(Random() % 100 < 4) ? ARRAY_COUNT(sFinds) - 3 + Random() % 3 : Random() % 7];
}

// --- Hooks -----------------------------------------------------------------

void AliveTown_OnMapTransition(void)
{
    memset(&sTown, 0, sizeof(sTown));
    memset(&sMons, 0, sizeof(sMons));
}

void AliveTown_Update(void)
{
    u32 n;

    if (!Alive_IsTown() || Alive_IsQuiet())
        return;
    UpdateCommuters();
    if (!sMons.spawned)
        SpawnTownMons();
    for (n = 0; n < ALIVE_TOWN_MONS; n++)
    {
        if (sMons.index[n] != 0)
            UpdateTownMon(n);
    }
}
