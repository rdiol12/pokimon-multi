#include "global.h"
#include "breed_fusion.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "injury.h"
#include "life.h"
#include "load_save.h"
#include "main.h"
#include "metatile_behavior.h"
#include "money.h"
#include "netplay_ext.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "rovers.h"
#include "netplay.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "wild_encounter.h"
#include "world.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/script_commands.h"
#include "constants/wild_encounter.h"
#include "constants/battle_ai.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/map_types.h"
#include "constants/songs.h"
#include "constants/trainer_types.h"
#include "constants/trainers.h"

// Roaming trainers ("rovers"). 150 trainers live their own lives all over
// Hoenn. The server (server/rovers.py) keeps them going while nobody
// watches: travel, training, catching, gyms, the Day Care. When the player
// walks onto a map, the game asks who is there and plays them out right in
// front of the player:
//
//  - each one walks around with its lead Pokémon following it;
//  - in grass (or anywhere in a cave) a wild Pokémon jumps out next to it,
//    the two Pokémon fight, and the trainer knocks it out (and may grow a
//    level, learn a move or evolve) or throws a Poké Ball at it;
//  - two rovers on one map may be in the middle of a battle, or a rover may
//    be battling one of the map's own trainers. Talk to either to watch it
//    as a real battle, both sides played by the AI;
//  - talk to a wandering rover to see its team, or to battle it yourself.
//
// What the player sees happen goes back to the server and becomes the
// rover's real progress. Offline, a couple of rovers still show up, with
// teams matched to the player's badges, and nothing is kept.
//
// Also here: wild Pokémon that burst out of the grass and charge whoever is
// nearest, and the quest boards' rampaging Pokémon (server MAJOR requests),
// with a quest board next to every town's Pokémon Center.

#define ACTOR_NONE 0xFF
#define FOE_TRAINER 0x80 // partner value: a fight with the map's own trainer

enum
{
    RS_NONE,
    RS_WANDER,
    RS_WILD_APPEAR,
    RS_WILD_FIGHT,
    RS_THROW,
    RS_SHAKE,
    RS_KO,
    RS_DUEL,
    RS_DUEL_OVER,
    RS_LEAVING,
    RS_BUDDY,          // exploring a cave floor with the player
};

enum
{
    EV_NONE,
    EV_CAUGHT,
    EV_EVOLVED,
    EV_LEVEL,
    EV_HEALED,
    EV_TRADED,
};

enum
{
    REQ_NONE,
    REQ_HERE,
    REQ_SYNC,
    REQ_FIGHT,
    REQ_BEAT,
    REQ_LEAVE,
    REQ_BOARD,
    REQ_MAJORS,
    REQ_MAJOR_CLEAR,
    REQ_GIFT,
    REQ_EXPLORED,
    REQ_LEAGUE_RESULT,
    REQ_LEAGUE_SEEN,
};

enum
{
    CHARGER_NONE,
    CHARGER_CHASE,
    CHARGER_CALM,
    CHARGER_FIGHTING, // a rover took it on
};

// One Pokémon of a rover, as it goes over the wire (server/rovers.py).
struct RoverMon
{
    u16 species;
    u8 level;
    u8 movesLevel; // moves were last worked out at this level
    u16 moves[MAX_MON_MOVES];
    u8 injuryPart;
    u8 injuryDamage; // 2% units
    u8 lostParts;
    u8 injuryFlags;
    u16 fusion;      // a fusion's father species (docs/breed_fusion.md), or 0
} __attribute__((packed, aligned(2))); // 18 bytes (packed: GBA structs round up to 4)

struct RoverWire
{
    u8 id;
    u8 count;
    u8 badges;
    u8 hurt;
    u16 wins;
    u16 losses;
    struct RoverMon mons[PARTY_SIZE];
    // From server/rovers_life.py bag_wire(): battle medicine, balls, the
    // ranch and money.
    u8 potions, superPotions, fullHeals, revives;
    u8 balls;
    u8 ranch;
    u16 money;
    // What it thinks of this player (server/rovers.py bond_wire()).
    u8 bond;       // ROVER_BOND_*
    u8 friendship;
} __attribute__((packed, aligned(2))); // 126 bytes
STATIC_ASSERT(sizeof(struct RoverMon) == 18, RoverMonSize);
STATIC_ASSERT(sizeof(struct RoverWire) == 126, RoverWireSize);

#define ROVER_BOND_RIVAL  (1 << 0) // the player beat it; it wants a rematch
#define ROVER_BOND_FRIEND (1 << 1)
#define ROVER_BOND_GIFT   (1 << 2) // a present is waiting

struct RoverActor
{
    struct RoverWire w;
    s16 x, y;          // where the trainer stands (map coords with offset)
    u8 state;
    u8 timer;
    u8 sub;
    u8 partner;        // the other actor in a duel, or FOE_TRAINER
    u8 out;            // team slot fighting in a duel
    u8 foeOut;         // the other side's slot
    u8 foeLeft;        // the game trainer's Pokémon still standing
    u8 hits;
    u16 wildSpecies;
    u8 wildLevel;
    u8 wildDir;        // from the trainer to the wild Pokémon
    u8 missing;        // frames the object has been gone
    u8 leaveTimer;     // minutes-ish on this map
    bool8 duelLeft;    // TRUE: this side of the duel won
    u8 rival;          // 0, 1 spotted the player, 2 challenged them (this map)
};

struct RoverRequest
{
    u8 type;
    u8 a, b, c;
    u16 d;
};

#define QUEUE_SIZE 6

struct RoverState
{
    struct RoverActor actors[ROVERS_MAX_HERE];
    struct RoverRequest queue[QUEUE_SIZE];
    u8 queued;
    u8 waiting;        // REQ_* in flight
    bool8 wantHere;
    bool8 spawned;
    u8 mapGroup, mapNum;
    u8 talk;           // actor talked to
    u16 foeTrainer;    // trainer id of the map's trainer in a duel
    u8 foeLocalId;
    // Chargers and the rampaging Pokémon.
    u8 chargerState;
    u8 chargerTimer;
    u16 chargerSpecies;
    u8 chargerLevel;
    bool8 chargerIsMajor;
    u16 chargeCountdown;
    u16 majorId;
    u16 majorSpecies;
    u8 majorLevel;
    bool8 majorSpawned;
    // A board read in progress.
    bool8 boardWaiting;
    bool8 boardOk;
    // A trade with the player.
    bool8 trading;
    u8 tradeSlot;
    u8 startRival;     // 1 + the actor walking up to challenge the player
    u8 tagA, tagB;     // the two rovers a tag battle is against
    // A cave floor explored together.
    u8 buddy;          // 1 + the actor exploring with the player
    u8 farewellId;     // 1 + the rover id that just parted ways
    u16 farewellItem;
    u8 farewellDelay;  // frames, so the map's name shows first
    // The weekly Rover League (server/rovers_league.py).
    u16 leagueMatch;   // the player's match
    u16 leagueWatch;   // a match to watch
    u8 leagueRound;
    bool8 leagueFighting;
    bool8 leagueWon;
    u8 leagueAsk;      // 1 + a LEAGUE request waiting for the channel
    u16 leagueAskArg;
    u16 leagueAskWait; // frames it has waited
    // Watching.
    bool8 watching;
    u8 duelResult;     // a watched duel's result, 1 + left won, for the map
    bool8 nearPlayer;  // screenshots: put a duel next to the player
    u8 talkCountdown;  // screenshots: talk to an actor after this many frames
    u8 watchA, watchB;
    u32 savedMoney;
    u16 savedPartner;
    u8 savedGender;
    u8 savedName[PLAYER_NAME_LENGTH + 1];
};

static EWRAM_DATA struct RoverState sRovers = {0};
static EWRAM_DATA struct TrainerMon sRoverParty[PARTY_SIZE] = {0};
static EWRAM_DATA struct Trainer sRoverTrainer = {0};
// Tag battles: allocated for the battle (struct RoverTag), NULL otherwise.
EWRAM_DATA struct Trainer *gRoverTagTrainers = NULL;

// A tag battle's three trainers: partner, opponent A, opponent B. Not on
// the heap, which battle start resets. Their parties are sRoverParty's two
// halves and sTagParty.
static EWRAM_DATA struct Trainer sTagTrainers[3] = {0};
static EWRAM_DATA struct TrainerMon sTagParty[MULTI_PARTY_SIZE] = {0};
EWRAM_DATA struct RoverShowcase gRoverShowcase = {0};

#include "data/rovers_cast.h"

extern const u8 Rovers_EventScript_Talk[];
extern const u8 Rovers_EventScript_Charger[];
extern const u8 Rovers_EventScript_Board[];
extern const u8 Rovers_EventScript_Rival[];
extern const u8 Rovers_EventScript_BuddyBye[];
enum Direction GetDirectionToFace(s16 x1, s16 y1, s16 x2, s16 y2);

static void Actor_Remove(u32 i);
static void QueueRequest(u8 type, u8 a, u8 b, u8 c, u16 d);

// Helpers ----------------------------------------------------------------------

static const struct RoverCast *Cast(const struct RoverActor *a)
{
    return &gRoverCast[a->w.id % ROVER_COUNT];
}

static const struct RoverLook *Look(const struct RoverActor *a)
{
    return &sRoverLooks[Cast(a)->look % ARRAY_COUNT(sRoverLooks)];
}

static struct ObjectEvent *ObjByLocalId(u8 localId)
{
    u8 id = GetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    return id < OBJECT_EVENTS_COUNT ? &gObjectEvents[id] : NULL;
}

static struct ObjectEvent *TrainerObj(u32 i)
{
    return ObjByLocalId(LOCALID_ROVER_TRAINER + i);
}

static struct ObjectEvent *MonObj(u32 i)
{
    return ObjByLocalId(LOCALID_ROVER_MON + i);
}

static void RemoveLocal(u8 localId)
{
    RemoveObjectEventByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

static bool32 Busy(struct ObjectEvent *obj)
{
    // A finished held movement stays "active" until the next one replaces it.
    return obj != NULL && (obj->singleMovementActive || (obj->heldMovementActive && !obj->heldMovementFinished));
}

static void Do(struct ObjectEvent *obj, u8 action)
{
    if (obj != NULL)
        ObjectEventForceSetHeldMovement(obj, action);
}

static void Step(s16 *x, s16 *y, u32 dir)
{
    switch (dir)
    {
    case DIR_SOUTH: (*y)++; break;
    case DIR_NORTH: (*y)--; break;
    case DIR_WEST:  (*x)--; break;
    case DIR_EAST:  (*x)++; break;
    }
}

static u32 Opposite(u32 dir)
{
    static const u8 sOpp[] = {[DIR_SOUTH] = DIR_NORTH, [DIR_NORTH] = DIR_SOUTH, [DIR_WEST] = DIR_EAST, [DIR_EAST] = DIR_WEST};
    return dir <= DIR_EAST ? sOpp[dir] : DIR_SOUTH;
}

static bool32 IsOutdoorsOrCave(void)
{
    switch (gMapHeader.mapType)
    {
    case MAP_TYPE_TOWN:
    case MAP_TYPE_CITY:
    case MAP_TYPE_ROUTE:
    case MAP_TYPE_UNDERGROUND:
        return TRUE;
    default:
        return FALSE;
    }
}

// The automated tests (Netplay_TryTestBoot) walk set routes: no rovers,
// chargers, rampaging Pokémon or boards turn up to get in their way, unless
// a screenshot scene asked for rovers (gRoverShowcase).
static bool32 QuietForTests(void)
{
    return Netplay_IsTestBoot() && gRoverShowcase.scene == 0;
}

static bool32 IsRift(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ADVENTURE_DUNGEON)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ADVENTURE_DUNGEON);
}

static bool32 IsTown(void)
{
    return gMapHeader.mapType == MAP_TYPE_TOWN || gMapHeader.mapType == MAP_TYPE_CITY;
}

// A tile someone can walk onto: no wall, nobody there, not water or a door,
// on the player's level.
static bool32 CanStandAt(s16 x, s16 y, bool32 grassOk)
{
    u32 behavior;
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    if (x == player->currentCoords.x && y == player->currentCoords.y)
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) || MetatileBehavior_IsWarpDoor(behavior)
     || MetatileBehavior_IsDoor(behavior) || MetatileBehavior_IsNonAnimDoor(behavior))
        return FALSE;
    if (!grassOk && MetatileBehavior_IsLongGrass(behavior))
        return FALSE;
    return MapGridGetElevationAt(x, y) == player->currentElevation || MapGridGetElevationAt(x, y) == 0
        || player->currentElevation == 0;
}

static bool32 InGrass(s16 x, s16 y)
{
    u32 behavior = MapGridGetMetatileBehaviorAt(x, y);
    if (gMapHeader.mapType == MAP_TYPE_UNDERGROUND)
        return TRUE;
    return MetatileBehavior_IsTallGrass(behavior) || MetatileBehavior_IsLongGrass(behavior);
}

// A free tile somewhere on screen, at least minDist steps from the player.
static bool32 FindSpot(s16 *outX, s16 *outY, u32 minDist, bool32 wantGrass)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 tries;

    for (tries = 0; tries < 60; tries++)
    {
        s16 x = player->currentCoords.x - 6 + Random() % 13;
        s16 y = player->currentCoords.y - 4 + Random() % 9;
        u32 d = abs(x - player->currentCoords.x) + abs(y - player->currentCoords.y);
        if (d < minDist || !CanStandAt(x, y, TRUE))
            continue;
        if (wantGrass && tries < 45 && !InGrass(x, y))
            continue;
        *outX = x;
        *outY = y;
        return TRUE;
    }
    return FALSE;
}

static u8 Spawn(u8 localId, u16 gfx, s16 x, s16 y, u32 facing)
{
    struct ObjectEventTemplate template = {0};
    static const u8 sFace[] =
    {
        [DIR_SOUTH] = MOVEMENT_TYPE_FACE_DOWN, [DIR_NORTH] = MOVEMENT_TYPE_FACE_UP,
        [DIR_WEST] = MOVEMENT_TYPE_FACE_LEFT, [DIR_EAST] = MOVEMENT_TYPE_FACE_RIGHT,
    };
    u8 objectId;

    template.localId = localId;
    template.graphicsId = gfx;
    template.kind = OBJ_KIND_NORMAL;
    template.x = x - MAP_OFFSET;
    template.y = y - MAP_OFFSET;
    template.elevation = MapGridGetElevationAt(x, y);
    template.movementType = sFace[facing <= DIR_EAST ? facing : DIR_SOUTH];
    template.trainerType = TRAINER_TYPE_NONE;
    objectId = SpawnSpecialObjectEvent(&template);
    if (objectId < OBJECT_EVENTS_COUNT)
    {
        // Held movements only from here on.
        gObjectEvents[objectId].movementType = MOVEMENT_TYPE_NONE;
        ObjectEventTurn(&gObjectEvents[objectId], facing);
    }
    return objectId;
}

static u16 MonGfx(u16 species)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        species = SPECIES_PIKACHU;
    return species + OBJ_EVENT_MON;
}

static u32 CountFreeObjects(void)
{
    u32 i, n = 0;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (!gObjectEvents[i].active)
            n++;
    }
    return n;
}

// Busy maps (the Haven Reserve's roamers, crowds) are left alone.
static bool32 RoomForAMon(void)
{
    return CountFreeObjects() >= 4 && CountFreePaletteSlots() >= 2;
}

static void Toast(const u8 *title, const u8 *fmt)
{
    u8 text[64];
    StringExpandPlaceholders(text, fmt);
    World_ShowToast(title, text);
}

static bool32 IsWildState(u32 state)
{
    return state == RS_WILD_APPEAR || state == RS_WILD_FIGHT || state == RS_THROW || state == RS_SHAKE || state == RS_KO;
}

// Teams --------------------------------------------------------------------------

static u32 LeadSlot(const struct RoverWire *w)
{
    u32 i, best = 0;
    for (i = 1; i < w->count; i++)
    {
        if (w->mons[i].level > w->mons[best].level)
            best = i;
    }
    return best;
}

static s32 MoveScore(const struct RoverMon *m, u16 move, u8 favType, s32 skipSlot)
{
    s32 score, i;
    enum Type type;

    if (move == MOVE_NONE || move >= MOVES_COUNT)
        return -1000;
    type = GetMoveType(move);
    score = GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS ? 35 : GetMovePower(move);
    if (score <= 1)
        score = 60; // variable power
    if (type == favType)
        score += 25;
    if (type == GetSpeciesType(m->species, 0) || type == GetSpeciesType(m->species, 1))
        score += 20;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (i != skipSlot && m->moves[i] != move && m->moves[i] != MOVE_NONE && GetMoveType(m->moves[i]) == type)
            score -= 20;
    }
    return score;
}

// The trainer decides whether a newly learnable move is worth a slot: its
// favourite type and the Pokémon's own types count for a lot, and two moves
// of one type count against each other.
static bool32 ConsiderMove(struct RoverMon *m, u16 move, u8 favType)
{
    s32 i, worst = 0, worstScore = 0x7FFF, newScore;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (m->moves[i] == move)
            return FALSE;
    }
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (m->moves[i] == MOVE_NONE)
        {
            m->moves[i] = move;
            return TRUE;
        }
    }
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        s32 s = MoveScore(m, m->moves[i], favType, i);
        if (s < worstScore)
        {
            worstScore = s;
            worst = i;
        }
    }
    newScore = MoveScore(m, move, favType, worst);
    if (newScore <= worstScore)
        return FALSE;
    m->moves[worst] = move;
    return TRUE;
}

static void LearnMoves(struct RoverMon *m, u32 fromLevel, u32 toLevel, u8 favType)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(m->species);
    u32 i;

    if (learnset == NULL)
        return;
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].level > fromLevel && learnset[i].level <= toLevel)
            ConsiderMove(m, learnset[i].move, favType);
    }
}

// What a rover's Pokémon evolves into at its level. Level evolutions as in
// the games; anything else (stones, trades, friendship) the rover manages
// by level 32.
static u16 Evolution(u16 species, u8 level)
{
    const struct Evolution *evo = GetSpeciesEvolutions(species);
    u32 i;

    if (evo == NULL)
        return SPECIES_NONE;
    for (i = 0; evo[i].method != EVOLUTIONS_END; i++)
    {
        if (evo[i].targetSpecies == SPECIES_NONE || evo[i].targetSpecies >= NUM_SPECIES)
            continue;
        if (evo[i].method == EVO_LEVEL || evo[i].method == EVO_LEVEL_BATTLE_ONLY)
        {
            if (evo[i].param != 0 ? level >= evo[i].param : level >= 32)
                return evo[i].targetSpecies;
        }
        else if (evo[i].method != EVO_NONE && evo[i].method != EVO_SPLIT_FROM_EVO && level >= 32)
        {
            return evo[i].targetSpecies;
        }
    }
    return SPECIES_NONE;
}

// Brings a Pokémon's moves and evolution up to its level. Returns the
// species it evolved into, if it did.
static u16 Normalize(struct RoverMon *m, u8 favType)
{
    u16 evolved = SPECIES_NONE, next;
    u32 guard = 0;

    if (m->species == SPECIES_NONE || m->species >= NUM_SPECIES)
        m->species = SPECIES_ZIGZAGOON;
    m->level = max(1, min(m->level, MAX_LEVEL));
    while ((next = Evolution(m->species, m->level)) != SPECIES_NONE && guard++ < 3)
    {
        m->species = next;
        evolved = next;
    }
    if (m->movesLevel < m->level || evolved)
    {
        LearnMoves(m, evolved ? 0 : m->movesLevel, m->level, 0xFF);
        m->movesLevel = m->level;
    }
    if (m->moves[0] == MOVE_NONE)
        m->moves[0] = MOVE_TACKLE;
    return evolved;
}

static bool32 NormalizeTeam(struct RoverActor *a)
{
    u32 i;
    bool32 changed = FALSE;
    a->w.count = max(1, min(a->w.count, PARTY_SIZE));
    for (i = 0; i < a->w.count; i++)
    {
        u8 before = a->w.mons[i].movesLevel;
        u16 species = a->w.mons[i].species;
        Normalize(&a->w.mons[i], Cast(a)->favType);
        if (before != a->w.mons[i].movesLevel || species != a->w.mons[i].species)
            changed = TRUE;
    }
    return changed;
}

static void NewMon(struct RoverMon *m, u16 species, u8 level, u8 favType)
{
    memset(m, 0, sizeof(*m));
    m->species = species;
    m->level = level;
    LearnMoves(m, 0, level, favType);
    m->movesLevel = level;
    Normalize(m, favType);
}

static void AddToTeam(struct RoverActor *a, u16 species, u8 level)
{
    u32 slot = a->w.count;

    if (slot >= PARTY_SIZE)
    {
        u32 i;
        slot = 0;
        for (i = 1; i < PARTY_SIZE; i++)
        {
            if (a->w.mons[i].level < a->w.mons[slot].level)
                slot = i;
        }
        if (slot == LeadSlot(&a->w))
            slot = (slot + 1) % PARTY_SIZE;
    }
    else
    {
        a->w.count++;
    }
    NewMon(&a->w.mons[slot], species, level, Cast(a)->favType);
}

// The map's land Pokémon -----------------------------------------------------

static const struct WildPokemonInfo *LandInfo(void)
{
    u16 header = GetCurrentMapWildMonHeaderId();
    enum TimeOfDay time;

    if (header == HEADER_NONE)
        return NULL;
    time = GetTimeOfDayForEncounters(header, WILD_AREA_LAND);
    return gWildMonHeaders[header].encounterTypes[time].landMonsInfo;
}

static bool32 PickWild(u16 *species, u8 *level)
{
    const struct WildPokemonInfo *info = LandInfo();
    const struct WildPokemon *mon;

    if (info == NULL || info->wildPokemon == NULL)
        return FALSE;
    mon = &info->wildPokemon[Random() % NUM_LAND_MONS_ENCOUNTER_SLOTS];
    if (mon->species == SPECIES_NONE)
        return FALSE;
    *species = mon->species;
    *level = mon->minLevel + (mon->maxLevel > mon->minLevel ? Random() % (mon->maxLevel - mon->minLevel + 1) : 0);
    return TRUE;
}

// Offline rovers --------------------------------------------------------------

static u32 PlayerBadges(void)
{
    u32 i, n = 0;
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        n += FlagGet(i);
    return n;
}

static void MakeOfflineRover(struct RoverActor *a, u32 id, u32 level, u32 teamSize)
{
    const struct RoverCast *c = &gRoverCast[id % ROVER_COUNT];
    u32 i;

    memset(a, 0, sizeof(*a));
    a->w.id = id;
    a->w.badges = min(PlayerBadges() + (id % 3) - 1, 8);
    if (a->w.badges > 8)
        a->w.badges = 0;
    a->w.wins = 3 + (id * 7) % 40;
    a->w.losses = 1 + (id * 5) % 25;
    a->w.count = 1;
    a->w.potions = 1 + a->w.badges / 2;
    a->w.superPotions = a->w.badges / 3;
    a->w.balls = 3 + id % 6;
    a->w.money = 1000 + 500 * a->w.badges + (id * 37) % 900;
    NewMon(&a->w.mons[0], c->starter, level, c->favType);
    for (i = 1; i < teamSize; i++)
    {
        u16 species;
        u8 wildLevel;
        if (!PickWild(&species, &wildLevel))
            break;
        a->w.count++;
        NewMon(&a->w.mons[i], species, max(level - 3 - Random() % 4, 2), c->favType);
    }
}

static u32 OfflineLevel(void)
{
    return 6 + PlayerBadges() * 6 + Random() % 4;
}

// Placing them -------------------------------------------------------------------

static bool32 SpawnActor(u32 i, s16 x, s16 y, u32 facing)
{
    struct RoverActor *a = &sRovers.actors[i];
    s16 mx = x, my = y;
    u32 back = Opposite(facing), tries;

    RemoveLocal(LOCALID_ROVER_TRAINER + i);
    RemoveLocal(LOCALID_ROVER_MON + i);
    if (Spawn(LOCALID_ROVER_TRAINER + i, Look(a)->gfx, x, y, facing) >= OBJECT_EVENTS_COUNT)
        return FALSE;
    a->x = x;
    a->y = y;
    // The Pokémon stands behind, or anywhere next to it.
    for (tries = 0; tries < 5; tries++)
    {
        mx = x;
        my = y;
        Step(&mx, &my, tries == 0 ? back : tries);
        if (CanStandAt(mx, my, TRUE))
            break;
    }
    if (tries < 5 && CountFreePaletteSlots() >= 1)
        Spawn(LOCALID_ROVER_MON + i, MonGfx(a->w.mons[LeadSlot(&a->w)].species), mx, my, facing);
    return TRUE;
}

// Puts actors i and j in a battle: i on the left facing right, j on the
// right, their Pokémon between them.
static bool32 PlaceDuel(u32 i, u32 j)
{
    s16 x, y;
    u32 tries, k;
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];

    for (tries = 0; tries < 40; tries++)
    {
        if (tries == 0 && sRovers.nearPlayer)
        {
            // Screenshots: right next to the player, to the east.
            x = player->currentCoords.x + 1;
            y = player->currentCoords.y;
        }
        else if (!FindSpot(&x, &y, 2, FALSE))
        {
            continue;
        }
        for (k = 1; k <= 3; k++)
        {
            if (!CanStandAt(x + k, y, TRUE))
                break;
        }
        if (k <= 3)
            continue;
        if (!SpawnActor(i, x, y, DIR_EAST) || !SpawnActor(j, x + 3, y, DIR_WEST))
            return FALSE;
        // Move the Pokémon into the middle.
        if (MonObj(i))
            MoveObjectEventToMapCoords(MonObj(i), x + 1, y);
        if (MonObj(j))
            MoveObjectEventToMapCoords(MonObj(j), x + 2, y);
        ObjectEventTurn(MonObj(i), DIR_EAST);
        ObjectEventTurn(MonObj(j), DIR_WEST);
        return TRUE;
    }
    return FALSE;
}

static u16 TrainerIdOfObject(struct ObjectEvent *obj)
{
    const u8 *script;
    struct ScriptContext ctx;

    if (obj->trainerType != TRAINER_TYPE_NORMAL && obj->trainerType != TRAINER_TYPE_SEE_ALL_DIRECTIONS)
        return TRAINER_NONE;
    script = GetObjectEventScriptPointerByObjectEventId(obj - gObjectEvents);
    if (script == NULL)
        return TRAINER_NONE;
    if (RunScriptImmediatelyUntilEffect(SCREFF_V1 | SCREFF_SAVE | SCREFF_HARDWARE | SCREFF_TRAINERBATTLE, script, &ctx)
     && *ctx.scriptPtr == SCR_OP_TRAINERBATTLE)
    {
        const TrainerBattleParameter *p = (const TrainerBattleParameter *)(ctx.scriptPtr + TRAINERBATTLE_OPCODE_OFFSET);
        return p->params.opponentA;
    }
    return TRAINER_NONE;
}

// A rover squares up to one of the map's own trainers: trainer, its
// Pokémon, the rover's Pokémon, the rover, in a line.
static bool32 PlaceTrainerDuel(u32 i)
{
    u32 o, k;

    for (o = 0; o < OBJECT_EVENTS_COUNT; o++)
    {
        struct ObjectEvent *t = &gObjectEvents[o];
        u16 trainerId;
        const struct Trainer *trainer;
        s16 x, y;
        u32 dir;

        if (!t->active || t->isPlayer || t->localId >= LOCALID_ROVER_FIRST)
            continue;
        trainerId = TrainerIdOfObject(t);
        if (trainerId == TRAINER_NONE || trainerId >= TRAINERS_COUNT)
            continue;
        trainer = GetTrainerStructFromId(trainerId);
        if (trainer->partySize == 0 || (trainer->battleType == TRAINER_BATTLE_TYPE_DOUBLES))
            continue;
        dir = t->facingDirection;
        x = t->currentCoords.x;
        y = t->currentCoords.y;
        for (k = 1; k <= 3; k++)
        {
            Step(&x, &y, dir);
            if (!CanStandAt(x, y, TRUE))
                break;
        }
        if (k <= 3 || !RoomForAMon())
            continue;
        if (!SpawnActor(i, x, y, Opposite(dir)))
            return FALSE;
        x = t->currentCoords.x;
        y = t->currentCoords.y;
        Step(&x, &y, dir);
        Spawn(LOCALID_ROVER_FOE_MON, MonGfx(trainer->party[0].species), x, y, dir);
        Step(&x, &y, dir);
        if (MonObj(i))
        {
            MoveObjectEventToMapCoords(MonObj(i), x, y);
            ObjectEventTurn(MonObj(i), Opposite(dir));
        }
        sRovers.foeTrainer = trainerId;
        sRovers.foeLocalId = t->localId;
        sRovers.actors[i].partner = FOE_TRAINER;
        sRovers.actors[i].foeLeft = trainer->partySize;
        sRovers.actors[i].foeOut = 0;
        return TRUE;
    }
    return FALSE;
}

static void StartDuel(u32 i, u32 j)
{
    struct RoverActor *a = &sRovers.actors[i];
    a->state = RS_DUEL;
    a->timer = 30;
    a->hits = 0;
    a->out = LeadSlot(&a->w);
    a->sub = 0;
    if (j != FOE_TRAINER)
    {
        struct RoverActor *b = &sRovers.actors[j];
        a->partner = j;
        b->partner = i;
        b->state = RS_DUEL;
        b->out = LeadSlot(&b->w);
        b->timer = 0xFF; // i drives the duel
    }
    Do(TrainerObj(i), MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK);
}

static u32 MaxHere(void)
{
    u32 objs = CountFreeObjects(), pals = CountFreePaletteSlots(), n;
    if (objs < 6 || pals < 3)
        return 0;
    n = min((objs - 3) / 2, (pals - 2) / 2);
    return min(n, ROVERS_MAX_HERE);
}

static void SetUpScene(u32 count, u32 scene)
{
    u32 i, placed = 0;

    for (i = 0; i < count; i++)
    {
        sRovers.actors[i].partner = ACTOR_NONE;
        sRovers.actors[i].state = RS_NONE;
    }
    if (count == 0)
        return;
    if (scene == 0)
    {
        u32 r = Random() % 6;
        scene = (count >= 2 && r < 3) ? ROVER_SCENE_DUEL : (r == 3 ? ROVER_SCENE_TRAINER : ROVER_SCENE_WANDER);
    }
    if (scene == ROVER_SCENE_DUEL && count >= 2 && PlaceDuel(0, 1))
    {
        StartDuel(0, 1);
        placed = 2;
    }
    else if (scene == ROVER_SCENE_TRAINER && PlaceTrainerDuel(0))
    {
        StartDuel(0, FOE_TRAINER);
        placed = 1;
    }
    for (i = placed; i < count; i++)
    {
        s16 x, y;
        struct RoverActor *a = &sRovers.actors[i];
        if (!FindSpot(&x, &y, 3, FALSE) || !SpawnActor(i, x, y, Random() % 4 + 1))
        {
            a->state = RS_NONE;
            continue;
        }
        a->state = RS_WANDER;
        a->timer = 30 + Random() % 60;
        if (scene == ROVER_SCENE_WILD || scene == ROVER_SCENE_CATCH)
        {
            a->timer = 20;
            a->sub = scene == ROVER_SCENE_CATCH ? 2 : 1; // forced encounter
            scene = 0;
        }
    }
}

// The server --------------------------------------------------------------------

static void QueueRequest(u8 type, u8 a, u8 b, u8 c, u16 d)
{
    struct RoverRequest *r;
    // A showcase's rovers stay off the server; league results don't.
    if (!Npx_IsOnline() || (gRoverShowcase.offline && type != REQ_LEAGUE_RESULT && type != REQ_LEAGUE_SEEN))
        return;
    if (sRovers.queued >= QUEUE_SIZE)
        return;
    r = &sRovers.queue[sRovers.queued++];
    r->type = type;
    r->a = a;
    r->b = b;
    r->c = c;
    r->d = d;
}

static void Sync(u32 i, u8 event, u16 arg)
{
    QueueRequest(REQ_SYNC, i, event, sRovers.actors[i].w.id, arg);
}

static s32 AnswerNum(u32 row, u32 index)
{
    const u8 *resp = gNetplayExtMailbox.resp;
    u32 pos = 0, len = min(gNetplayExtMailbox.respLen, NETPLAY_EXT_RESP_SIZE);
    s32 n = 0;
    bool32 neg = FALSE;

    while (row != 0 && pos < len && resp[pos] != 0)
    {
        if (resp[pos++] == '\n')
            row--;
    }
    while (index != 0 && pos < len && resp[pos] != 0 && resp[pos] != '\n')
    {
        if (resp[pos++] == '|')
            index--;
    }
    if (pos < len && resp[pos] == '-')
    {
        neg = TRUE;
        pos++;
    }
    while (pos < len && resp[pos] >= '0' && resp[pos] <= '9')
        n = n * 10 + resp[pos++] - '0';
    return neg ? -n : n;
}

static u32 AnswerText(u8 *dest, u32 row, u32 index, u32 maxChars)
{
    const u8 *resp = gNetplayExtMailbox.resp;
    char field[64];
    u32 pos = 0, n = 0, len = min(gNetplayExtMailbox.respLen, NETPLAY_EXT_RESP_SIZE);

    while (row != 0 && pos < len && resp[pos] != 0)
    {
        if (resp[pos++] == '\n')
            row--;
    }
    while (index != 0 && pos < len && resp[pos] != 0 && resp[pos] != '\n')
    {
        if (resp[pos++] == '|')
            index--;
    }
    while (pos < len && resp[pos] != '|' && resp[pos] != 0 && resp[pos] != '\n' && n < ARRAY_COUNT(field) - 1)
        field[n++] = resp[pos++];
    field[n] = 0;
    Npx_AsciiToGame(dest, field, maxChars);
    return n;
}

static u32 AnswerRows(void)
{
    const u8 *resp = gNetplayExtMailbox.resp;
    u32 pos, rows = 1, len = min(gNetplayExtMailbox.respLen, NETPLAY_EXT_RESP_SIZE);
    if (len == 0)
        return 0;
    for (pos = 0; pos < len && resp[pos] != 0; pos++)
    {
        if (resp[pos] == '\n')
            rows++;
    }
    return rows;
}

static const u8 *AnswerBinary(u32 *size)
{
    const u8 *resp = gNetplayExtMailbox.resp;
    u32 pos, len = min(gNetplayExtMailbox.respLen, NETPLAY_EXT_RESP_SIZE);
    for (pos = 0; pos < len; pos++)
    {
        if (resp[pos] == 0)
        {
            *size = len - pos - 1;
            return &resp[pos + 1];
        }
    }
    *size = 0;
    return NULL;
}

static bool32 OnRoverMap(void)
{
    return sRovers.mapGroup == gSaveBlock1Ptr->location.mapGroup && sRovers.mapNum == gSaveBlock1Ptr->location.mapNum;
}

static void AfterHere(void)
{
    u32 size, count, i;
    const u8 *data = AnswerBinary(&size);

    if (!OnRoverMap())
        return;
    count = min(AnswerNum(0, 0), ROVERS_MAX_HERE);
    sRovers.majorId = AnswerNum(0, 1);
    sRovers.majorSpecies = AnswerNum(0, 2);
    sRovers.majorLevel = AnswerNum(0, 3);
    if (sRovers.majorSpecies >= NUM_SPECIES)
        sRovers.majorId = 0;
    if (data == NULL || size < count * sizeof(struct RoverWire))
        count = 0;
    count = min(count, MaxHere());
    for (i = 0; i < count; i++)
    {
        struct RoverActor *a = &sRovers.actors[i];
        memset(a, 0, sizeof(*a));
        memcpy(&a->w, data + i * sizeof(struct RoverWire), sizeof(struct RoverWire));
        if (a->w.id >= ROVER_COUNT || a->w.count == 0)
        {
            count = i;
            break;
        }
        if (NormalizeTeam(a))
            Sync(i, EV_NONE, 0);
    }
    SetUpScene(count, 0);
    sRovers.spawned = TRUE;
}

static bool32 SendRequest(const struct RoverRequest *r)
{
    switch (r->type)
    {
    case REQ_SYNC:
    {
        struct RoverActor *a = &sRovers.actors[r->a];
        if (a->w.id != r->c)
            return TRUE; // that rover is gone; drop it
        if (!NpxRpc_Begin("ROVER"))
            return FALSE;
        NpxRpc_Word("SYNC");
        NpxRpc_Num(a->w.id);
        NpxRpc_Num(r->b);
        NpxRpc_Num(r->d);
        NpxRpc_Data(&a->w, sizeof(a->w));
        break;
    }
    case REQ_FIGHT:
        if (!NpxRpc_Begin("ROVER"))
            return FALSE;
        NpxRpc_Word("FIGHT");
        NpxRpc_Num(r->a);
        NpxRpc_Num(r->b);
        NpxRpc_Num(r->c);
        break;
    case REQ_BEAT:
        if (!NpxRpc_Begin("ROVER"))
            return FALSE;
        NpxRpc_Word("BEAT");
        NpxRpc_Num(r->a);
        NpxRpc_Num(r->b);
        NpxRpc_Num(r->c);
        break;
    case REQ_LEAVE:
        if (!NpxRpc_Begin("ROVER"))
            return FALSE;
        NpxRpc_Word("LEAVE");
        NpxRpc_Num(r->a);
        NpxRpc_Num(r->b);
        NpxRpc_Num(r->c);
        break;
    case REQ_EXPLORED:
        if (!NpxRpc_Begin("ROVER"))
            return FALSE;
        NpxRpc_Word("EXPLORED");
        NpxRpc_Num(r->a);
        NpxRpc_Num(gSaveBlock1Ptr->location.mapGroup);
        NpxRpc_Num(gSaveBlock1Ptr->location.mapNum);
        break;
    case REQ_LEAGUE_RESULT:
    case REQ_LEAGUE_SEEN:
        if (!NpxRpc_Begin("ROVER"))
            return FALSE;
        NpxRpc_Word("LEAGUE");
        NpxRpc_Word(r->type == REQ_LEAGUE_RESULT ? "RESULT" : "SEEN");
        NpxRpc_Num(r->d);
        NpxRpc_Num(r->b);
        break;
    case REQ_MAJOR_CLEAR:
        if (!NpxRpc_Begin("MAJOR"))
            return FALSE;
        NpxRpc_Word("CLEAR");
        NpxRpc_Num(r->d);
        NpxRpc_Num(r->a);
        break;
    default:
        return TRUE;
    }
    NpxRpc_Send();
    sRovers.waiting = r->type;
    return TRUE;
}

static void SendLeagueAsk(void);

static void UpdateRequests(void)
{
    if (sRovers.waiting != REQ_NONE)
    {
        u8 state = NpxRpc_Poll();
        bool32 ok;
        if (state == NPX_RPC_WAITING)
            return;
        ok = (state == NPX_RPC_DONE && gNetplayExtMailbox.status == 0);
        if (sRovers.waiting == REQ_HERE)
        {
            if (ok)
                AfterHere();
            else
                sRovers.spawned = TRUE;
        }
        else if (sRovers.waiting == REQ_GIFT)
        {
            gSpecialVar_0x8004 = ok ? AnswerNum(0, 0) : ITEM_NONE;
            gSpecialVar_0x8005 = ok ? AnswerNum(0, 1) : 0;
            ScriptContext_Enable();
        }
        else if (sRovers.waiting == REQ_BOARD || sRovers.waiting == REQ_MAJORS)
        {
            if (!sRovers.boardWaiting)
                return; // the board script is reading the answer
            sRovers.boardOk = ok;
            sRovers.boardWaiting = FALSE;
            ScriptContext_Enable();
            if (ok)
                return; // the board reads the answer, then releases it
        }
        sRovers.waiting = REQ_NONE;
        NpxRpc_Release();
        return;
    }
    if (!Npx_IsOnline())
    {
        sRovers.queued = 0;
        return;
    }
    if (sRovers.leagueAsk != 0)
    {
        SendLeagueAsk();
        return;
    }
    if (sRovers.wantHere && !ArePlayerFieldControlsLocked())
    {
        if (!NpxRpc_Begin("ROVER"))
            return;
        NpxRpc_Word("HERE");
        NpxRpc_Num(sRovers.mapGroup);
        NpxRpc_Num(sRovers.mapNum);
        NpxRpc_Num(MaxHere());
        NpxRpc_Send();
        sRovers.wantHere = FALSE;
        sRovers.waiting = REQ_HERE;
        return;
    }
    if (sRovers.queued != 0)
    {
        if (SendRequest(&sRovers.queue[0]))
        {
            sRovers.queued--;
            memmove(&sRovers.queue[0], &sRovers.queue[1], sRovers.queued * sizeof(sRovers.queue[0]));
        }
    }
}

// Map changes ---------------------------------------------------------------------

void Rovers_OnMapTransition(void)
{
    // A floor explored together ends here; the rover shares a find.
    if (sRovers.buddy != 0)
    {
        static const u16 sFinds[] = {ITEM_RED_SHARD, ITEM_BLUE_SHARD, ITEM_YELLOW_SHARD, ITEM_GREEN_SHARD,
                                     ITEM_TINY_MUSHROOM, ITEM_BIG_MUSHROOM, ITEM_STARDUST, ITEM_ESCAPE_ROPE};
        struct RoverActor *a = &sRovers.actors[sRovers.buddy - 1];
        sRovers.farewellId = a->w.id + 1;
        sRovers.farewellItem = (Random() % 8 == 0) ? ITEM_STAR_PIECE : sFinds[Random() % ARRAY_COUNT(sFinds)];
        sRovers.farewellDelay = 40;
        QueueRequest(REQ_EXPLORED, a->w.id, 0, 0, 0);
        sRovers.buddy = 0;
    }
    memset(sRovers.actors, 0, sizeof(sRovers.actors));
    sRovers.queued = 0;
    sRovers.spawned = FALSE;
    sRovers.wantHere = FALSE;
    sRovers.mapGroup = gSaveBlock1Ptr->location.mapGroup;
    sRovers.mapNum = gSaveBlock1Ptr->location.mapNum;
    sRovers.chargerState = CHARGER_NONE;
    sRovers.chargeCountdown = 60 * (15 + Random() % 30);
    sRovers.majorId = 0;
    sRovers.majorSpawned = FALSE;
    sRovers.foeTrainer = TRAINER_NONE;
    if (!IsOutdoorsOrCave() || QuietForTests())
    {
        sRovers.spawned = TRUE;
        return;
    }
    if (Npx_IsOnline() && !gRoverShowcase.offline)
        sRovers.wantHere = TRUE;
}

static void SpawnOffline(void)
{
    u32 count = 0, i, room = MaxHere();
    u32 seed = gSaveBlock1Ptr->location.mapGroup * 131 + gSaveBlock1Ptr->location.mapNum * 17 + Random();

    sRovers.spawned = TRUE;
    // Divers are common in the Rift Below.
    if (room == 0 || (Random() % (IsRift() ? 2 : 3)) != 0)
        return;
    count = 1 + (Random() % 3 == 0);
    count = min(count, room);
    for (i = 0; i < count; i++)
        MakeOfflineRover(&sRovers.actors[i], (seed + i * 53) % ROVER_COUNT, OfflineLevel(), 1 + Random() % 4);
    SetUpScene(count, 0);
}

static void RunShowcase(void)
{
    u32 i, count = min(gRoverShowcase.count, ROVERS_MAX_HERE);

    gRoverShowcase.pending = FALSE;
    for (i = 0; i < ROVERS_MAX_HERE; i++)
        Actor_Remove(i);
    RemoveLocal(LOCALID_ROVER_CHARGER);
    RemoveLocal(LOCALID_ROVER_MAJOR);
    RemoveLocal(LOCALID_ROVER_FOE_MON);
    sRovers.chargerState = CHARGER_NONE;
    for (i = 0; i < count; i++)
        MakeOfflineRover(&sRovers.actors[i], gRoverShowcase.ids[i],
                         (i != 0 && gRoverShowcase.levelB) ? gRoverShowcase.levelB : (gRoverShowcase.level ? gRoverShowcase.level : 20),
                         gRoverShowcase.teamSize ? gRoverShowcase.teamSize : 3);
    if (count != 0)
    {
        struct RoverActor *a = &sRovers.actors[0];
        a->w.bond = gRoverShowcase.bond;
        if (gRoverShowcase.leadSpecies != SPECIES_NONE && gRoverShowcase.leadSpecies < NUM_SPECIES)
        {
            a->w.mons[0].species = gRoverShowcase.leadSpecies;
            a->w.mons[0].fusion = gRoverShowcase.leadFusion;
            a->w.mons[0].movesLevel = 0;
            memset(a->w.mons[0].moves, 0, sizeof(a->w.mons[0].moves));
            NormalizeTeam(a);
        }
    }
    sRovers.spawned = TRUE;
    sRovers.nearPlayer = TRUE;
    if (gRoverShowcase.talk != 0)
        sRovers.talkCountdown = 90;
    switch (gRoverShowcase.scene)
    {
    case ROVER_SCENE_CHARGER:
        sRovers.chargeCountdown = 1;
        break;
    case ROVER_SCENE_MAJOR:
        if (sRovers.majorId == 0)
        {
            sRovers.majorId = 0xFFFF;
            PickWild(&sRovers.majorSpecies, &sRovers.majorLevel);
            sRovers.majorLevel += 10;
        }
        sRovers.majorSpawned = FALSE;
        break;
    default:
        SetUpScene(count, gRoverShowcase.scene);
        break;
    }
    sRovers.nearPlayer = FALSE;
}

// Walking around -------------------------------------------------------------------

static void FollowerStep(u32 i, s16 toX, s16 toY)
{
    struct ObjectEvent *mon = MonObj(i);
    s16 mx, my;
    u32 dir;

    if (mon == NULL)
        return;
    mx = mon->currentCoords.x;
    my = mon->currentCoords.y;
    if (abs(mx - toX) + abs(my - toY) != 1)
    {
        MoveObjectEventToMapCoords(mon, toX, toY);
        return;
    }
    dir = GetDirectionToFace(mx, my, toX, toY);
    Do(mon, GetWalkNormalMovementAction(dir));
}

static bool32 TryWalk(u32 i, u32 dir)
{
    struct ObjectEvent *t = TrainerObj(i);
    struct ObjectEvent *mon = MonObj(i);
    s16 x, y;

    if (t == NULL || Busy(t) || Busy(mon))
        return FALSE;
    x = t->currentCoords.x;
    y = t->currentCoords.y;
    Step(&x, &y, dir);
    if (!CanStandAt(x, y, TRUE))
        return FALSE;
    Do(t, GetWalkNormalMovementAction(dir));
    FollowerStep(i, t->currentCoords.x, t->currentCoords.y);
    sRovers.actors[i].x = x;
    sRovers.actors[i].y = y;
    return TRUE;
}

static void Actor_Remove(u32 i)
{
    RemoveLocal(LOCALID_ROVER_TRAINER + i);
    RemoveLocal(LOCALID_ROVER_MON + i);
    sRovers.actors[i].state = RS_NONE;
}

static void StartWild(u32 i, bool32 forceCatch);

// A rival the player beat walks up to them for a rematch.
static bool32 UpdateRivalApproach(u32 i)
{
    struct RoverActor *a = &sRovers.actors[i];
    struct ObjectEvent *t = TrainerObj(i);
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 dx, dy;
    u32 k;

    if (!(a->w.bond & ROVER_BOND_RIVAL) || a->rival == 2 || sRovers.startRival != 0)
        return FALSE;
    dx = player->currentCoords.x - t->currentCoords.x;
    dy = player->currentCoords.y - t->currentCoords.y;
    if (abs(dx) + abs(dy) > 9)
        return FALSE;
    if (abs(dx) + abs(dy) == 1)
    {
        ObjectEventTurn(t, GetDirectionToFace(t->currentCoords.x, t->currentCoords.y,
                                              player->currentCoords.x, player->currentCoords.y));
        a->rival = 2;
        sRovers.startRival = i + 1;
        return TRUE;
    }
    if (a->rival == 0)
    {
        Do(t, MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK);
        PlaySE(SE_PIN);
        a->rival = 1;
        return TRUE;
    }
    for (k = 0; k < 2; k++)
    {
        u32 dir;
        if ((abs(dx) >= abs(dy)) == (k == 0) && dx != 0)
            dir = dx > 0 ? DIR_EAST : DIR_WEST;
        else if (dy != 0)
            dir = dy > 0 ? DIR_SOUTH : DIR_NORTH;
        else
            continue;
        if (TryWalk(i, dir))
            return TRUE;
    }
    return FALSE;
}

static void UpdateWander(u32 i)
{
    struct RoverActor *a = &sRovers.actors[i];
    struct ObjectEvent *t = TrainerObj(i);

    if (t == NULL || Busy(t))
        return;
    if (UpdateRivalApproach(i))
        return;
    if (a->timer != 0)
    {
        a->timer--;
        return;
    }
    a->timer = 18 + Random() % 50;
    if (a->sub != 0)
    {
        StartWild(i, a->sub == 2);
        a->sub = 0;
        return;
    }
    if (InGrass(t->currentCoords.x, t->currentCoords.y) && (Random() % 5) == 0 && sRovers.chargerState == CHARGER_NONE
     && RoomForAMon())
    {
        StartWild(i, FALSE);
        return;
    }
    // Hurt or done here: off it goes.
    if (++a->leaveTimer > 40 || a->w.hurt >= 3)
    {
        a->state = RS_LEAVING;
        a->timer = 0;
        a->sub = Random() % 4 + 1;
        return;
    }
    // Mostly keep going the same way; grass is tempting.
    {
        u32 dir = (Random() % 3 == 0) ? Random() % 4 + 1 : t->facingDirection, k;
        for (k = 0; k < 4; k++)
        {
            if (TryWalk(i, dir))
                return;
            dir = dir % 4 + 1;
        }
    }
}

// Next to the player, where nobody stands, if there's such a tile.
static bool32 SpotNextToPlayer(s16 *outX, s16 *outY)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 k, back = Opposite(player->facingDirection);

    for (k = 0; k < 4; k++)
    {
        u32 dir = (back + k - 1) % 4 + 1;
        s16 x = player->currentCoords.x, y = player->currentCoords.y;
        Step(&x, &y, dir);
        if (CanStandAt(x, y, TRUE))
        {
            *outX = x;
            *outY = y;
            return TRUE;
        }
    }
    return FALSE;
}

// Exploring with the player: keep up, a step behind.
static void UpdateBuddy(u32 i)
{
    struct ObjectEvent *t = TrainerObj(i);
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 dx, dy;
    u32 dir, other;

    if (t == NULL || Busy(t) || Busy(MonObj(i)))
        return;
    dx = player->currentCoords.x - t->currentCoords.x;
    dy = player->currentCoords.y - t->currentCoords.y;
    if (abs(dx) + abs(dy) <= 1)
    {
        // In the way of where the player faces: step aside.
        s16 fx = player->currentCoords.x, fy = player->currentCoords.y;
        Step(&fx, &fy, player->facingDirection);
        if (fx == t->currentCoords.x && fy == t->currentCoords.y)
        {
            u32 face = player->facingDirection;
            u32 side = (face == DIR_NORTH || face == DIR_SOUTH) ? DIR_WEST : DIR_NORTH;
            if (!TryWalk(i, side) && !TryWalk(i, Opposite(side)))
                TryWalk(i, face);
        }
        return;
    }
    if (abs(dx) + abs(dy) > 6)
    {
        s16 x, y;
        if (SpotNextToPlayer(&x, &y))
        {
            MoveObjectEventToMapCoords(t, x, y);
            FollowerStep(i, x, y);
            sRovers.actors[i].x = x;
            sRovers.actors[i].y = y;
        }
        return;
    }
    if (abs(dx) >= abs(dy))
    {
        dir = dx > 0 ? DIR_EAST : DIR_WEST;
        other = dy > 0 ? DIR_SOUTH : DIR_NORTH;
    }
    else
    {
        dir = dy > 0 ? DIR_SOUTH : DIR_NORTH;
        other = dx > 0 ? DIR_EAST : DIR_WEST;
    }
    if (!TryWalk(i, dir) && (dx == 0 || dy == 0 || !TryWalk(i, other)))
        ObjectEventTurn(t, dir);
}

static void UpdateLeaving(u32 i)
{
    struct RoverActor *a = &sRovers.actors[i];
    struct ObjectEvent *t = TrainerObj(i);
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (t == NULL || Busy(t))
        return;
    if (abs(t->currentCoords.x - player->currentCoords.x) > 9 || abs(t->currentCoords.y - player->currentCoords.y) > 7
     || ++a->timer > 30)
    {
        QueueRequest(REQ_LEAVE, a->w.id, sRovers.mapGroup, sRovers.mapNum, 0);
        Actor_Remove(i);
        return;
    }
    if (!TryWalk(i, a->sub))
        a->sub = Random() % 4 + 1;
}

// Wild Pokémon ---------------------------------------------------------------------------

static void StartWild(u32 i, bool32 forceCatch)
{
    struct RoverActor *a = &sRovers.actors[i];
    struct ObjectEvent *t = TrainerObj(i);
    struct ObjectEvent *mon = MonObj(i);
    u32 k, dir;
    s16 x, y;

    if (t == NULL || !PickWild(&a->wildSpecies, &a->wildLevel))
        return;
    for (k = 0; k < 4; k++)
    {
        dir = (t->facingDirection + k - 1) % 4 + 1;
        x = t->currentCoords.x;
        y = t->currentCoords.y;
        Step(&x, &y, dir);
        if (CanStandAt(x, y, TRUE))
            break;
    }
    if (k == 4)
        return;
    if (Spawn(LOCALID_ROVER_WILD, MonGfx(a->wildSpecies), x, y, Opposite(dir)) >= OBJECT_EVENTS_COUNT)
        return;
    a->wildDir = dir;
    a->state = RS_WILD_APPEAR;
    a->timer = 40;
    a->hits = 0;
    a->sub = forceCatch;
    Do(ObjByLocalId(LOCALID_ROVER_WILD), GetJumpInPlaceMovementAction(Opposite(dir)));
    Do(t, MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK);
    ObjectEventTurn(t, dir);
    if (mon != NULL)
        ObjectEventTurn(mon, GetDirectionToFace(mon->currentCoords.x, mon->currentCoords.y, x, y));
}

// One "hit": the attacker lunges, the target flickers.
static void Hit(struct ObjectEvent *attacker, struct ObjectEvent *target)
{
    if (attacker != NULL && target != NULL)
    {
        u32 dir = GetDirectionToFace(attacker->currentCoords.x, attacker->currentCoords.y,
                                     target->currentCoords.x, target->currentCoords.y);
        Do(attacker, GetWalkInPlaceFastMovementAction(dir));
    }
    if (target != NULL)
        target->invisible = TRUE;
}

static void Unflicker(struct ObjectEvent *obj)
{
    if (obj != NULL)
        obj->invisible = FALSE;
}

static void LevelUp(u32 i, u32 slot)
{
    struct RoverActor *a = &sRovers.actors[i];
    struct RoverMon *m = &a->w.mons[slot];
    u16 before = m->species, evolved;

    if (m->level >= MAX_LEVEL)
        return;
    m->level++;
    LearnMoves(m, m->level - 1, m->level, Cast(a)->favType);
    m->movesLevel = m->level;
    evolved = Evolution(m->species, m->level);
    StringCopy(gStringVar1, Cast(a)->name);
    StringCopy(gStringVar2, GetSpeciesName(before));
    ConvertIntToDecimalStringN(gStringVar3, m->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (evolved != SPECIES_NONE)
    {
        struct ObjectEvent *mon = MonObj(i);
        m->species = evolved;
        LearnMoves(m, 0, m->level, Cast(a)->favType);
        StringCopy(gStringVar3, GetSpeciesName(evolved));
        Toast(COMPOUND_STRING("EVOLUTION!"), COMPOUND_STRING("{STR_VAR_1}'s {STR_VAR_2} became {STR_VAR_3}!"));
        if (mon != NULL && slot == LeadSlot(&a->w))
        {
            ObjectEventSetGraphicsId(mon, MonGfx(evolved));
            Do(mon, MOVEMENT_ACTION_EMOTE_HEART);
        }
        PlaySE(SE_EXP_MAX);
        Sync(i, EV_EVOLVED, evolved);
    }
    else
    {
        Toast(COMPOUND_STRING("LEVEL UP"), COMPOUND_STRING("{STR_VAR_1}'s {STR_VAR_2} grew to Lv. {STR_VAR_3}!"));
        Sync(i, EV_LEVEL, m->level);
    }
}

static void UpdateWild(u32 i)
{
    struct RoverActor *a = &sRovers.actors[i];
    struct ObjectEvent *t = TrainerObj(i);
    struct ObjectEvent *mon = MonObj(i);
    struct ObjectEvent *wild = ObjByLocalId(LOCALID_ROVER_WILD);
    struct ObjectEvent *ball = ObjByLocalId(LOCALID_ROVER_BALL);

    if (t == NULL || wild == NULL)
    {
        RemoveLocal(LOCALID_ROVER_WILD);
        RemoveLocal(LOCALID_ROVER_BALL);
        a->state = RS_WANDER;
        return;
    }
    if (a->timer != 0)
    {
        a->timer--;
        if (a->timer == 34 && a->state == RS_WILD_FIGHT)
        {
            Unflicker(wild);
            Unflicker(mon);
        }
        return;
    }
    switch (a->state)
    {
    case RS_WILD_APPEAR:
        PlayCry_NormalNoDucking(a->wildSpecies, 0, CRY_VOLUME_RS / 2, CRY_PRIORITY_NORMAL);
        a->state = RS_WILD_FIGHT;
        a->timer = 20;
        break;
    case RS_WILD_FIGHT:
        Unflicker(wild);
        Unflicker(mon);
        if (a->hits < 4 + Random() % 3)
        {
            if (a->hits % 2 == 0)
                Hit(mon != NULL ? mon : t, wild);
            else
                Hit(wild, mon);
            PlaySE(a->hits % 2 == 0 ? SE_M_COMET_PUNCH : SE_M_DOUBLE_SLAP);
            a->hits++;
            a->timer = 40;
            break;
        }
        // Catch it, or knock it out?
        {
            const struct RoverCast *c = Cast(a);
            u32 want = (c->style == ROVER_STYLE_COLLECTOR) ? 2 : 4;
            if (a->sub || (a->w.count < PARTY_SIZE && Random() % want == 0)
             || (c->style == ROVER_STYLE_COLLECTOR && Random() % 4 == 0))
            {
                u32 dir = a->wildDir;
                RemoveLocal(LOCALID_ROVER_BALL);
                Spawn(LOCALID_ROVER_BALL, OBJ_EVENT_GFX_POKE_BALL, t->currentCoords.x, t->currentCoords.y, dir);
                Do(ObjByLocalId(LOCALID_ROVER_BALL), GetJumpSpecialMovementAction(dir)); // one tile, high arc
                PlaySE(SE_BALL_THROW);
                a->state = RS_THROW;
                a->timer = 18;
            }
            else
            {
                wild->invisible = TRUE;
                PlaySE(SE_FAINT);
                a->state = RS_KO;
                a->timer = 30;
            }
        }
        break;
    case RS_THROW:
        if (Busy(ball))
            break;
        wild->invisible = TRUE;
        PlaySE(SE_BALL_OPEN);
        a->state = RS_SHAKE;
        a->hits = 0;
        a->timer = 24;
        break;
    case RS_SHAKE:
        if (Busy(ball))
            break;
        if (a->hits < 3)
        {
            Do(ball, GetJumpInPlaceMovementAction(a->wildDir));
            PlaySE(SE_BALL);
            a->hits++;
            a->timer = 20;
            if (a->hits == 2 && !a->sub && Random() % 3 == 0)
                a->hits = 10; // breaks out
            break;
        }
        RemoveLocal(LOCALID_ROVER_BALL);
        StringCopy(gStringVar1, Cast(a)->name);
        StringCopy(gStringVar2, GetSpeciesName(a->wildSpecies));
        if (a->hits == 3)
        {
            RemoveLocal(LOCALID_ROVER_WILD);
            PlaySE(SE_RG_BALL_CLICK);
            AddToTeam(a, a->wildSpecies, a->wildLevel);
            Toast(COMPOUND_STRING("GOTCHA!"), COMPOUND_STRING("{STR_VAR_1} caught a wild {STR_VAR_2}!"));
            Do(t, MOVEMENT_ACTION_EMOTE_HEART);
            Sync(i, EV_CAUGHT, a->wildSpecies);
        }
        else
        {
            // It broke free and runs off.
            wild->invisible = FALSE;
            Do(t, MOVEMENT_ACTION_EMOTE_QUESTION_MARK);
            Do(wild, GetJumpInPlaceMovementAction(Opposite(a->wildDir)));
            a->state = RS_KO;
            a->hits = 1; // no experience
            a->timer = 40;
            return;
        }
        a->state = RS_WANDER;
        a->timer = 60;
        break;
    case RS_KO:
        RemoveLocal(LOCALID_ROVER_WILD);
        if (a->hits != 1)
        {
            u32 slot = LeadSlot(&a->w);
            s32 chance = 35 + (a->wildLevel - (s32)a->w.mons[slot].level) * 12;
            if (Cast(a)->style == ROVER_STYLE_TRAINER)
                chance += 20;
            if ((s32)(Random() % 100) < max(10, min(chance, 95)))
                LevelUp(i, slot);
        }
        a->state = RS_WANDER;
        a->timer = 40;
        break;
    }
}

// Duels ----------------------------------------------------------------------------------

static u16 FoeSpecies(u32 slot)
{
    const struct Trainer *trainer = GetTrainerStructFromId(sRovers.foeTrainer);
    if (slot >= trainer->partySize)
        return SPECIES_NONE;
    return trainer->party[slot].species;
}

static u8 FoeLevel(u32 slot)
{
    const struct Trainer *trainer = GetTrainerStructFromId(sRovers.foeTrainer);
    if (slot >= trainer->partySize)
        return 1;
    return trainer->party[slot].lvl;
}

static void EndDuel(u32 i, bool32 leftWon)
{
    struct RoverActor *a = &sRovers.actors[i];
    struct ObjectEvent *t = TrainerObj(i);

    StringCopy(gStringVar1, Cast(a)->name);
    if (a->partner == FOE_TRAINER)
    {
        const struct Trainer *trainer = GetTrainerStructFromId(sRovers.foeTrainer);
        StringCopy(gStringVar2, trainer->trainerName);
        RemoveLocal(LOCALID_ROVER_FOE_MON);
        QueueRequest(REQ_BEAT, a->w.id, leftWon, 0, 0);
        if (leftWon)
            a->w.wins++;
        else
            a->w.losses++, a->w.hurt++;
        Toast(COMPOUND_STRING("TRAINER BATTLE"), leftWon ? COMPOUND_STRING("{STR_VAR_1} beat {STR_VAR_2}!")
                                                         : COMPOUND_STRING("{STR_VAR_2} beat {STR_VAR_1}!"));
        Do(t, leftWon ? MOVEMENT_ACTION_EMOTE_HEART : MOVEMENT_ACTION_EMOTE_QUESTION_MARK);
        sRovers.foeTrainer = TRAINER_NONE;
    }
    else
    {
        struct RoverActor *b = &sRovers.actors[a->partner];
        StringCopy(gStringVar2, Cast(b)->name);
        QueueRequest(REQ_FIGHT, a->w.id, b->w.id, leftWon, 0);
        if (leftWon)
            a->w.wins++, b->w.losses++, b->w.hurt++;
        else
            b->w.wins++, a->w.losses++, a->w.hurt++;
        Toast(COMPOUND_STRING("RIVAL BATTLE"), leftWon ? COMPOUND_STRING("{STR_VAR_1} beat {STR_VAR_2}!")
                                                       : COMPOUND_STRING("{STR_VAR_2} beat {STR_VAR_1}!"));
        Do(leftWon ? TrainerObj(i) : TrainerObj(a->partner), MOVEMENT_ACTION_EMOTE_HEART);
        Do(leftWon ? TrainerObj(a->partner) : TrainerObj(i), MOVEMENT_ACTION_EMOTE_QUESTION_MARK);
        b->state = RS_DUEL_OVER;
        b->timer = 90;
        b->partner = ACTOR_NONE;
        if (MonObj(a->partner))
            ObjectEventSetGraphicsId(MonObj(a->partner), MonGfx(b->w.mons[LeadSlot(&b->w)].species));
    }
    PlaySE(SE_EXP_MAX);
    a->state = RS_DUEL_OVER;
    a->timer = 90;
    a->partner = ACTOR_NONE;
    if (MonObj(i))
        ObjectEventSetGraphicsId(MonObj(i), MonGfx(a->w.mons[LeadSlot(&a->w)].species));
}

// Actor i drives a duel: hits back and forth, and every few hits one
// Pokémon goes down (the weaker side more often) and the next comes out.
static void UpdateDuel(u32 i)
{
    struct RoverActor *a = &sRovers.actors[i];
    struct RoverActor *b = a->partner == FOE_TRAINER ? NULL : &sRovers.actors[a->partner];
    struct ObjectEvent *mine = MonObj(i);
    struct ObjectEvent *theirs = b != NULL ? MonObj(a->partner) : ObjByLocalId(LOCALID_ROVER_FOE_MON);
    u32 myLevel, theirLevel;

    if (a->timer == 0xFF)
        return; // the other side drives
    if (a->timer != 0)
    {
        a->timer--;
        if (a->timer == 34)
        {
            Unflicker(mine);
            Unflicker(theirs);
        }
        return;
    }
    if (mine == NULL || theirs == NULL)
    {
        EndDuel(i, Random() % 2);
        return;
    }
    Unflicker(mine);
    Unflicker(theirs);
    if (a->hits < 3 + Random() % 3)
    {
        if (a->hits % 2 == 0)
            Hit(mine, theirs);
        else
            Hit(theirs, mine);
        PlaySE(a->hits % 2 == 0 ? SE_M_COMET_PUNCH : SE_M_MEGA_KICK);
        a->hits++;
        a->timer = 40;
        return;
    }
    // Someone goes down.
    a->hits = 0;
    myLevel = a->w.mons[a->out].level;
    theirLevel = b != NULL ? b->w.mons[b->out].level : FoeLevel(a->foeOut);
    if (Random() % (myLevel + theirLevel) < theirLevel)
    {
        // Mine faints.
        mine->invisible = TRUE;
        PlaySE(SE_FAINT);
        if (a->out + 1 >= a->w.count)
        {
            EndDuel(i, FALSE);
            return;
        }
        a->out++;
        ObjectEventSetGraphicsId(mine, MonGfx(a->w.mons[a->out].species));
        mine->invisible = FALSE;
    }
    else
    {
        theirs->invisible = TRUE;
        PlaySE(SE_FAINT);
        if (b != NULL)
        {
            if (b->out + 1 >= b->w.count)
            {
                EndDuel(i, TRUE);
                return;
            }
            b->out++;
            ObjectEventSetGraphicsId(theirs, MonGfx(b->w.mons[b->out].species));
        }
        else
        {
            if (a->foeOut + 1 >= a->foeLeft)
            {
                EndDuel(i, TRUE);
                return;
            }
            a->foeOut++;
            ObjectEventSetGraphicsId(theirs, MonGfx(FoeSpecies(a->foeOut)));
        }
        theirs->invisible = FALSE;
    }
    a->timer = 50;
}

static void UpdateDuelOver(u32 i)
{
    struct RoverActor *a = &sRovers.actors[i];
    if (a->timer != 0)
    {
        a->timer--;
        return;
    }
    a->state = a->w.hurt >= 2 ? RS_LEAVING : RS_WANDER;
    a->timer = 20;
    a->sub = Random() % 4 + 1;
    a->out = 0;
}

// Chargers and the rampaging Pokémon ------------------------------------------------------

static struct ObjectEvent *ChargerObj(void)
{
    return ObjByLocalId(sRovers.chargerIsMajor ? LOCALID_ROVER_MAJOR : LOCALID_ROVER_CHARGER);
}

static void SpawnCharger(bool32 major)
{
    s16 x, y;
    u16 species;
    u8 level;
    struct ObjectEvent *obj;

    if (major)
    {
        species = sRovers.majorSpecies;
        level = sRovers.majorLevel;
    }
    else if (!PickWild(&species, &level))
    {
        return;
    }
    if (!RoomForAMon() || !FindSpot(&x, &y, major ? 5 : 4, TRUE))
        return;
    if (Spawn(major ? LOCALID_ROVER_MAJOR : LOCALID_ROVER_CHARGER, MonGfx(species), x, y, DIR_SOUTH) >= OBJECT_EVENTS_COUNT)
        return;
    sRovers.chargerIsMajor = major;
    sRovers.chargerSpecies = species;
    sRovers.chargerLevel = level;
    sRovers.chargerState = (major || Random() % 3 != 0 || gRoverShowcase.scene == ROVER_SCENE_CHARGER) ? CHARGER_CHASE : CHARGER_CALM;
    sRovers.chargerTimer = 40;
    obj = ChargerObj();
    Do(obj, sRovers.chargerState == CHARGER_CHASE ? MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK : MOVEMENT_ACTION_EMOTE_QUESTION_MARK);
    PlayCry_NormalNoDucking(species, 0, major ? CRY_VOLUME_RS : CRY_VOLUME_RS / 2, CRY_PRIORITY_NORMAL);
    if (major)
    {
        StringCopy(gStringVar1, GetSpeciesName(species));
        Toast(COMPOUND_STRING("DANGER!"), COMPOUND_STRING("A rampaging {STR_VAR_1} is near!"));
    }
}

// Whoever is nearest: the player, or a wandering rover.
static bool32 ChargerTarget(struct ObjectEvent *c, s16 *tx, s16 *ty, s32 *rover)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 i, best = abs(player->currentCoords.x - c->currentCoords.x) + abs(player->currentCoords.y - c->currentCoords.y);

    *tx = player->currentCoords.x;
    *ty = player->currentCoords.y;
    *rover = -1;
    for (i = 0; i < ROVERS_MAX_HERE; i++)
    {
        struct ObjectEvent *t = TrainerObj(i);
        u32 d;
        if (t == NULL || sRovers.actors[i].state != RS_WANDER || gRoverShowcase.scene == ROVER_SCENE_CHARGER)
            continue;
        d = abs(t->currentCoords.x - c->currentCoords.x) + abs(t->currentCoords.y - c->currentCoords.y);
        if (d < best)
        {
            best = d;
            *tx = t->currentCoords.x;
            *ty = t->currentCoords.y;
            *rover = i;
        }
    }
    return best <= (sRovers.chargerIsMajor ? 10u : 8u) || gRoverShowcase.scene == ROVER_SCENE_CHARGER;
}

static bool32 UpdateCharger(void)
{
    struct ObjectEvent *c = ChargerObj();
    s16 tx, ty, x, y;
    s32 rover;
    u32 dir, k;
    s32 dx, dy;

    if (c == NULL)
    {
        sRovers.chargerState = CHARGER_NONE;
        return FALSE;
    }
    if (Busy(c))
        return FALSE;
    if (sRovers.chargerTimer != 0)
    {
        sRovers.chargerTimer--;
        return FALSE;
    }
    if (sRovers.chargerState == CHARGER_CALM)
    {
        sRovers.chargerTimer = 30 + Random() % 40;
        dir = Random() % 4 + 1;
        x = c->currentCoords.x;
        y = c->currentCoords.y;
        Step(&x, &y, dir);
        if (CanStandAt(x, y, TRUE))
            Do(c, GetWalkNormalMovementAction(dir));
        else
            ObjectEventTurn(c, dir);
        return FALSE;
    }
    if (!ChargerTarget(c, &tx, &ty, &rover))
    {
        sRovers.chargerTimer = 20;
        return FALSE; // waits for someone to come closer
    }
    dx = tx - c->currentCoords.x;
    dy = ty - c->currentCoords.y;
    if (abs(dx) + abs(dy) == 1)
    {
        ObjectEventTurn(c, GetDirectionToFace(c->currentCoords.x, c->currentCoords.y, tx, ty));
        if (rover < 0)
        {
            sRovers.chargerState = CHARGER_FIGHTING;
            gSpecialVar_0x8004 = sRovers.chargerSpecies;
            ScriptContext_SetupScript(Rovers_EventScript_Charger);
            return TRUE;
        }
        // A rover takes it on: it becomes that rover's wild Pokémon.
        {
            struct RoverActor *a = &sRovers.actors[rover];
            struct ObjectEvent *t = TrainerObj(rover);
            if (ObjByLocalId(LOCALID_ROVER_WILD) == NULL && !sRovers.chargerIsMajor)
            {
                u8 localId = LOCALID_ROVER_WILD;
                c->localId = localId;
                a->wildSpecies = sRovers.chargerSpecies;
                a->wildLevel = sRovers.chargerLevel;
                a->wildDir = GetDirectionToFace(t->currentCoords.x, t->currentCoords.y, c->currentCoords.x, c->currentCoords.y);
                a->state = RS_WILD_FIGHT;
                a->timer = 20;
                a->hits = 0;
                a->sub = 0;
                ObjectEventTurn(t, a->wildDir);
                Do(t, MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK);
                sRovers.chargerState = CHARGER_NONE;
            }
            else
            {
                sRovers.chargerTimer = 30;
            }
        }
        return FALSE;
    }
    // Step toward the target along the longer axis first.
    for (k = 0; k < 2; k++)
    {
        if ((abs(dx) >= abs(dy)) == (k == 0) && dx != 0)
            dir = dx > 0 ? DIR_EAST : DIR_WEST;
        else if (dy != 0)
            dir = dy > 0 ? DIR_SOUTH : DIR_NORTH;
        else
            dir = dx > 0 ? DIR_EAST : DIR_WEST;
        x = c->currentCoords.x;
        y = c->currentCoords.y;
        Step(&x, &y, dir);
        if (CanStandAt(x, y, TRUE))
        {
            Do(c, GetWalkFastMovementAction(dir));
            return FALSE;
        }
    }
    // Blocked: step aside and try again.
    dir = Random() % 4 + 1;
    for (k = 0; k < 4; k++, dir = dir % 4 + 1)
    {
        x = c->currentCoords.x;
        y = c->currentCoords.y;
        Step(&x, &y, dir);
        if (CanStandAt(x, y, TRUE))
        {
            Do(c, GetWalkNormalMovementAction(dir));
            sRovers.chargerTimer = 4;
            return FALSE;
        }
    }
    sRovers.chargerTimer = 16;
    ObjectEventTurn(c, Random() % 4 + 1);
    return FALSE;
}

// Script natives for the charge.
void Rovers_SetupChargerBattle(void)
{
    CreateScriptedWildMon(sRovers.chargerSpecies, sRovers.chargerLevel, ITEM_NONE);
}

void Rovers_BufferCharger(void)
{
    StringCopy(gStringVar1, GetSpeciesName(sRovers.chargerSpecies));
    gSpecialVar_0x8004 = sRovers.chargerSpecies;
    gSpecialVar_Result = sRovers.chargerIsMajor;
}

void Rovers_AfterChargerBattle(void)
{
    bool32 cleared = (gBattleOutcome == B_OUTCOME_WON || gBattleOutcome == B_OUTCOME_CAUGHT);

    RemoveLocal(sRovers.chargerIsMajor ? LOCALID_ROVER_MAJOR : LOCALID_ROVER_CHARGER);
    if (sRovers.chargerIsMajor)
    {
        if (cleared && sRovers.majorId != 0 && sRovers.majorId != 0xFFFF)
            QueueRequest(REQ_MAJOR_CLEAR, gBattleOutcome == B_OUTCOME_CAUGHT, 0, 0, sRovers.majorId);
        if (cleared)
            sRovers.majorId = 0;
        StringCopy(gStringVar1, GetSpeciesName(sRovers.chargerSpecies));
        if (cleared)
            Toast(COMPOUND_STRING("JOB DONE!"), COMPOUND_STRING("Collect the reward at any quest board."));
    }
    gSpecialVar_Result = cleared;
    sRovers.chargerState = CHARGER_NONE;
    sRovers.chargeCountdown = 60 * (30 + Random() % 30);
}

// Talking to them -------------------------------------------------------------------------

static s32 ActorOfObject(struct ObjectEvent *obj)
{
    u32 i;
    for (i = 0; i < ROVERS_MAX_HERE; i++)
    {
        if (sRovers.actors[i].state == RS_NONE)
            continue;
        if (obj->localId == LOCALID_ROVER_TRAINER + i || obj->localId == LOCALID_ROVER_MON + i)
            return i;
        if (obj->localId == LOCALID_ROVER_FOE_MON && sRovers.actors[i].partner == FOE_TRAINER)
            return i;
    }
    return -1;
}

bool32 Rovers_IsRoverObject(struct ObjectEvent *obj)
{
    return obj->localId >= LOCALID_ROVER_FIRST && obj->localId <= LOCALID_ROVER_LAST && obj->active
        && OnRoverMap();
}

const u8 *Rovers_GetTalkScript(struct ObjectEvent *obj)
{
    s32 i;

    if (!Rovers_IsRoverObject(obj))
        return NULL;
    if (obj->localId == LOCALID_ROVER_BOARD)
        return Rovers_EventScript_Board;
    if (obj->localId == LOCALID_ROVER_CHARGER || obj->localId == LOCALID_ROVER_MAJOR)
    {
        sRovers.chargerIsMajor = (obj->localId == LOCALID_ROVER_MAJOR);
        return Rovers_EventScript_Charger;
    }
    i = ActorOfObject(obj);
    if (i < 0)
        return NULL;
    // A duel the map's trainer is in: talking to that trainer works too
    // (GetObjectEventScriptPointer handles it), talking to its Pokémon here.
    sRovers.talk = i;
    return Rovers_EventScript_Talk;
}

// VAR_RESULT: 0 wandering, 1 in a duel (watchable), 2 busy (catching).
void Rovers_BufferTalk(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];

    StringCopy(gStringVar1, Cast(a)->name);
    StringCopy(gStringVar2, gTrainerClasses[Look(a)->trainerClass].name);
    if (a->state == RS_DUEL)
    {
        u32 left = a->timer == 0xFF ? a->partner : sRovers.talk;
        struct RoverActor *l = &sRovers.actors[left];
        StringCopy(gStringVar1, Cast(l)->name);
        if (l->partner == FOE_TRAINER)
            StringCopy(gStringVar2, GetTrainerStructFromId(sRovers.foeTrainer)->trainerName);
        else
            StringCopy(gStringVar2, Cast(&sRovers.actors[l->partner])->name);
        gSpecialVar_Result = 1;
        return;
    }
    if (a->state == RS_BUDDY)
    {
        gSpecialVar_Result = 3;
        return;
    }
    if (a->state != RS_WANDER && a->state != RS_DUEL_OVER && a->state != RS_LEAVING)
    {
        gSpecialVar_Result = 2;
        return;
    }
    ConvertIntToDecimalStringN(gStringVar3, a->w.badges, STR_CONV_MODE_LEFT_ALIGN, 1);
    gSpecialVar_Result = 0;
}

// A team member's name: a fusion's is made from both.
static const u8 *MonName(const struct RoverMon *m)
{
    if (m->fusion != SPECIES_NONE && m->fusion < NUM_SPECIES && BreedFusion_CanFuse(m->species, m->fusion))
    {
        BreedFusion_BufferName(gStringVar3, m->species, m->fusion);
        return gStringVar3;
    }
    return GetSpeciesName(m->species);
}

// After the hello (which the message box expands into gStringVar4): the
// rover's record, money, bag and team.
void Rovers_BufferTalkStats(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];
    u8 *s = gStringVar4;
    u32 i;

    s = StringCopy(s, COMPOUND_STRING("Wins: "));
    s = ConvertIntToDecimalStringN(s, a->w.wins, STR_CONV_MODE_LEFT_ALIGN, 5);
    s = StringCopy(s, COMPOUND_STRING("  Losses: "));
    s = ConvertIntToDecimalStringN(s, a->w.losses, STR_CONV_MODE_LEFT_ALIGN, 5);
    s = StringCopy(s, COMPOUND_STRING("\nMoney: "));
    s = ConvertIntToDecimalStringN(s, a->w.money, STR_CONV_MODE_LEFT_ALIGN, 5);
    s = StringCopy(s, COMPOUND_STRING("  Balls: "));
    s = ConvertIntToDecimalStringN(s, a->w.balls, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (a->w.ranch != 0)
    {
        s = StringCopy(s, COMPOUND_STRING("\pI keep "));
        s = ConvertIntToDecimalStringN(s, a->w.ranch, STR_CONV_MODE_LEFT_ALIGN, 1);
        s = StringCopy(s, COMPOUND_STRING(" more Pokémon\nat my ranch."));
    }
    s = StringCopy(s, COMPOUND_STRING("\pMy team: "));
    for (i = 0; i < a->w.count; i++)
    {
        if (i == 2 || i == 4)
            s = StringCopy(s, COMPOUND_STRING("\p"));
        else if (i != 0)
            s = StringCopy(s, COMPOUND_STRING(", "));
        s = StringCopy(s, MonName(&a->w.mons[i]));
        s = StringCopy(s, COMPOUND_STRING(" Lv"));
        s = ConvertIntToDecimalStringN(s, a->w.mons[i].level, STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    gSpecialVar_Result = 0;
}

// Battles -------------------------------------------------------------------------------------

static void BuildTrainerInto(const struct RoverActor *a, struct Trainer *trainer, struct TrainerMon *party, u32 most)
{
    u32 i;
    const struct RoverLook *look = Look(a);
    u32 count = min(a->w.count, most);

    memset(party, 0, sizeof(struct TrainerMon) * most);
    for (i = 0; i < count; i++)
    {
        const struct RoverMon *m = &a->w.mons[i];
        struct TrainerMon *mon = &party[i];
        u32 j, iv = 12 + (a->w.id + i) % 20;
        mon->species = m->species;
        mon->lvl = m->level;
        for (j = 0; j < MAX_MON_MOVES; j++)
            mon->moves[j] = m->moves[j] < MOVES_COUNT ? m->moves[j] : MOVE_NONE;
        mon->iv = TRAINER_PARTY_IVS(iv, iv, iv, iv, iv, iv);
        mon->nature = (a->w.id * 7 + i) % NUM_NATURES;
        mon->gender = TRAINER_MON_RANDOM_GENDER;
        mon->ball = POKEBALL_COUNT;
        mon->dynamaxLevel = MAX_DYNAMAX_LEVEL;
    }
    memset(trainer, 0, sizeof(*trainer));
    trainer->aiFlags = AI_FLAG_BASIC_TRAINER;
    trainer->party = party;
    trainer->partySize = count;
    trainer->trainerClass = look->trainerClass;
    trainer->encounterMusic = look->music;
    trainer->gender = look->female ? TRAINER_GENDER_FEMALE : TRAINER_GENDER_MALE;
    trainer->trainerPic = look->pic;
    // Its crafted and bought medicine, best first.
    {
        static const u16 sMedicine[] = {ITEM_REVIVE, ITEM_SUPER_POTION, ITEM_FULL_HEAL, ITEM_POTION};
        const u8 counts[] = {a->w.revives, a->w.superPotions, a->w.fullHeals, a->w.potions};
        u32 k, n = 0;
        for (k = 0; k < ARRAY_COUNT(sMedicine) && n < MAX_TRAINER_ITEMS; k++)
        {
            u32 c;
            for (c = 0; c < counts[k] && n < MAX_TRAINER_ITEMS; c++)
                trainer->items[n++] = sMedicine[k];
        }
    }
    StringCopyN(trainer->trainerName, Cast(a)->name, TRAINER_NAME_LENGTH);
    trainer->trainerName[TRAINER_NAME_LENGTH] = EOS;
}

static void BuildTrainer(const struct RoverActor *a)
{
    BuildTrainerInto(a, &sRoverTrainer, sRoverParty, PARTY_SIZE);
}

// A fusion bred at the Day Care: its father's species, so its colors,
// types, stats and name come out (src/breed_fusion.c).
static void SetFusion(struct Pokemon *mon, const struct RoverMon *m)
{
    u32 partner = m->fusion;
    if (partner == SPECIES_NONE || partner >= NUM_SPECIES || !BreedFusion_CanFuse(m->species, partner))
        return;
    SetMonData(mon, MON_DATA_FUSION_PARTNER, &partner);
    BreedFusion_SetHatchedName(mon);
    CalculateMonStats(mon);
}

// Injuries and fusions, which the trainer party data can't carry.
static void ApplyInjuries(struct Pokemon *party, const struct RoverActor *a)
{
    u32 i;
    for (i = 0; i < a->w.count; i++)
    {
        const struct RoverMon *m = &a->w.mons[i];
        if (GetMonData(&party[i], MON_DATA_SPECIES) != m->species)
            continue;
        SetFusion(&party[i], m);
        if (m->injuryPart || m->lostParts || m->injuryFlags)
            Injury_Set(&party[i], m->injuryPart, m->injuryDamage * INJURY_DAMAGE_UNIT, m->lostParts,
                       m->injuryFlags & INJURY_FLAG_SCAR);
    }
}


static void ReadInjuries(struct Pokemon *party, struct RoverActor *a)
{
    u32 i;
    for (i = 0; i < a->w.count; i++)
    {
        struct RoverMon *m = &a->w.mons[i];
        if (GetMonData(&party[i], MON_DATA_SPECIES) != m->species)
            continue;
        m->injuryPart = GetMonData(&party[i], MON_DATA_INJURY_PART);
        m->injuryDamage = GetMonData(&party[i], MON_DATA_INJURY_DAMAGE);
        m->lostParts = GetMonData(&party[i], MON_DATA_LOST_PARTS);
        m->injuryFlags = GetMonData(&party[i], MON_DATA_INJURY_FLAGS) & INJURY_FLAG_SCAR;
    }
}

// Tag battles ---------------------------------------------------------------------------------

// Two other rovers wandering here to team up against. VAR_RESULT TRUE if
// there are; STR_VAR_2 and STR_VAR_3 are their names.
void Rovers_TagOffer(void)
{
    u32 i, found = 0;

    gSpecialVar_Result = FALSE;
    for (i = 0; i < ROVERS_MAX_HERE && found < 2; i++)
    {
        if (i == sRovers.talk || TrainerObj(i) == NULL
         || (sRovers.actors[i].state != RS_WANDER && !IsWildState(sRovers.actors[i].state)))
            continue;
        if (found++ == 0)
            sRovers.tagA = i;
        else
            sRovers.tagB = i;
    }
    if (found < 2)
        return;
    StringCopy(gStringVar2, Cast(&sRovers.actors[sRovers.tagA])->name);
    StringCopy(gStringVar3, Cast(&sRovers.actors[sRovers.tagB])->name);
    gSpecialVar_Result = TRUE;
}

static void CB2_EndRoverTag(void)
{
    bool32 won = (gBattleOutcome == B_OUTCOME_WON);

    gRoverTagTrainers = NULL;
    gPartnerTrainerId = 0;
    QueueRequest(REQ_BEAT, sRovers.actors[sRovers.tagA].w.id, !won, 1, 0);
    QueueRequest(REQ_BEAT, sRovers.actors[sRovers.tagB].w.id, !won, 1, 0);
    gSpecialVar_Result = won;
    if (gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW)
        SetMainCallback2(CB2_WhiteOut);
    else
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

// The player and the rover talked to against the other two (waitstate).
void Rovers_StartTag(void)
{
    struct TrainerMon *parties[3] = {sRoverParty, sRoverParty + MULTI_PARTY_SIZE, sTagParty};
    const struct RoverActor *side[3] = {&sRovers.actors[sRovers.talk], &sRovers.actors[sRovers.tagA],
                                        &sRovers.actors[sRovers.tagB]};
    u32 i;

    for (i = 0; i < 3; i++)
        BuildTrainerInto(side[i], &sTagTrainers[i], parties[i], MULTI_PARTY_SIZE);
    // Whatever wild Pokémon they were busy with runs off.
    for (i = 1; i < 3; i++)
    {
        struct RoverActor *a = (struct RoverActor *)side[i];
        if (IsWildState(a->state))
        {
            RemoveLocal(LOCALID_ROVER_WILD);
            RemoveLocal(LOCALID_ROVER_BALL);
            a->state = RS_WANDER;
            a->timer = 30;
        }
    }
    gRoverTagTrainers = sTagTrainers;
    gPartnerTrainerId = TRAINER_PARTNER(PARTNER_STEVEN);
    memset(&gTrainerBattleParameter, 0, sizeof(gTrainerBattleParameter));
    TRAINER_BATTLE_PARAM.opponentA = ROVER_TAG_FOE_A;
    TRAINER_BATTLE_PARAM.opponentB = ROVER_TAG_FOE_B;
    TRAINER_BATTLE_PARAM.defeatTextA = (u8 *)COMPOUND_STRING("We'll get you next time!");
    TRAINER_BATTLE_PARAM.defeatTextB = (u8 *)COMPOUND_STRING("Good teamwork...");
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS
                     | BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER;
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sTagTrainers[1]);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_B], &sTagTrainers[2]);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_PARTNER], &sTagTrainers[0]);
    ApplyInjuries(gParties[B_TRAINER_OPPONENT_A], side[1]);
    ApplyInjuries(gParties[B_TRAINER_OPPONENT_B], side[2]);
    ApplyInjuries(gParties[B_TRAINER_PARTNER], side[0]);
    BattleSetup_StartRoverWatch(CB2_EndRoverTag);
}

// Caves and the Rift Below: the rover talked to may explore this floor
// with the player. VAR_RESULT TRUE if it would.
void Rovers_BuddyOffer(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];
    gSpecialVar_Result = gMapHeader.mapType == MAP_TYPE_UNDERGROUND && sRovers.buddy == 0
                      && a->state == RS_WANDER && a->w.count != 0 && a->w.hurt < 3;
}

void Rovers_StartBuddy(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];
    a->state = RS_BUDDY;
    a->timer = 0;
    sRovers.buddy = sRovers.talk + 1;
}

// Wild Pokémon met while exploring together: two of them, against the
// player and the rover (src/battle_setup.c, src/wild_encounter.c).
bool32 Rovers_HasBuddy(void)
{
    return sRovers.buddy != 0 && OnRoverMap() && sRovers.actors[sRovers.buddy - 1].state == RS_BUDDY;
}

void Rovers_PrepareBuddyBattle(void)
{
    BuildTrainerInto(&sRovers.actors[sRovers.buddy - 1], &sTagTrainers[0], sRoverParty, MULTI_PARTY_SIZE);
    gRoverTagTrainers = sTagTrainers;
    gPartnerTrainerId = TRAINER_PARTNER(PARTNER_STEVEN);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_PARTNER], &sTagTrainers[0]);
    ApplyInjuries(gParties[B_TRAINER_PARTNER], &sRovers.actors[sRovers.buddy - 1]);
}

void Rovers_AfterBuddyBattle(void)
{
    if (gRoverTagTrainers == NULL)
        return;
    if (sRovers.buddy != 0)
        ReadInjuries(gParties[B_TRAINER_PARTNER], &sRovers.actors[sRovers.buddy - 1]);
    gRoverTagTrainers = NULL;
    gPartnerTrainerId = 0;
}

// Friends' presents (waitstate): VAR_0x8004 the item, VAR_0x8005 how many,
// ITEM_NONE if there's none today.
void Rovers_GiftFetch(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];

    gSpecialVar_0x8004 = ITEM_NONE;
    gSpecialVar_0x8005 = 0;
    if (!(a->w.bond & ROVER_BOND_GIFT) || !Npx_IsOnline() || sRovers.waiting != REQ_NONE || !NpxRpc_Begin("ROVER"))
    {
        ScriptContext_Enable();
        return;
    }
    NpxRpc_Word("GIFT");
    NpxRpc_Num(a->w.id);
    NpxRpc_Send();
    sRovers.waiting = REQ_GIFT;
    a->w.bond &= ~ROVER_BOND_GIFT;
}

// VAR_RESULT: TRUE if the rover talked to is the player's friend.
void Rovers_IsFriend(void)
{
    gSpecialVar_Result = (sRovers.actors[sRovers.talk].w.bond & ROVER_BOND_FRIEND) != 0;
}

// Trading -----------------------------------------------------------------------------------

// What the rover talked to would give away: its weakest Pokémon that isn't
// its lead. VAR_RESULT FALSE if it has nothing to spare. STR_VAR_1 is the
// rover, STR_VAR_2 the Pokémon, STR_VAR_3 the type it wants.
void Rovers_TradeOffer(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];
    u32 i, lead = LeadSlot(&a->w), slot = PARTY_SIZE;

    for (i = 0; i < a->w.count; i++)
    {
        if (i != lead && (slot == PARTY_SIZE || a->w.mons[i].level < a->w.mons[slot].level))
            slot = i;
    }
    StringCopy(gStringVar1, Cast(a)->name);
    StringCopy(gStringVar3, gTypesInfo[Cast(a)->favType].name);
    if (slot == PARTY_SIZE)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    sRovers.tradeSlot = slot;
    StringCopy(gStringVar2, GetSpeciesName(a->w.mons[slot].species));
    gSpecialVar_Result = TRUE;
}

// After ChoosePartyMon (VAR_0x8004): 0 it's a deal, 1 an Egg, 2 not the
// type the rover wants, 3 the player's last Pokémon that can battle.
void Rovers_TradeCheck(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 type = Cast(a)->favType, i, able = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *m = &gParties[B_TRAINER_PLAYER][i];
        if (i != gSpecialVar_0x8004 && GetMonData(m, MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(m, MON_DATA_IS_EGG) && GetMonData(m, MON_DATA_HP) != 0)
            able++;
    }
    StringCopy(gStringVar2, GetSpeciesName(species));
    if (GetMonData(mon, MON_DATA_IS_EGG))
        gSpecialVar_Result = 1;
    else if (GetSpeciesType(species, 0) != type && GetSpeciesType(species, 1) != type)
        gSpecialVar_Result = 2;
    else if (able == 0)
        gSpecialVar_Result = 3;
    else
        gSpecialVar_Result = 0;
}

// Puts the rover's Pokémon where the in-game trade scene looks for it.
// A rover's Pokémon as a real one: the rover is its trainer.
static void MakeRoverMon(struct Pokemon *mon, u32 roverId, const struct RoverMon *m, metloc_u8_t met)
{
    const struct RoverCast *c = &gRoverCast[roverId % ROVER_COUNT];
    u8 gender = sRoverLooks[c->look % ARRAY_COUNT(sRoverLooks)].female ? FEMALE : MALE;
    u32 i;

    CreateMon(mon, m->species, m->level, Random32(), OTID_STRUCT_PRESET(ROVER_OT_ID_BASE + roverId));
    if (m->moves[0] == MOVE_NONE)
        GiveMonInitialMoveset(mon);
    else
    {
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            u16 move = m->moves[i] < MOVES_COUNT ? m->moves[i] : MOVE_NONE;
            u8 pp = move != MOVE_NONE ? GetMovePP(move) : 0;
            SetMonData(mon, MON_DATA_MOVE1 + i, &move);
            SetMonData(mon, MON_DATA_PP1 + i, &pp);
        }
    }
    SetMonData(mon, MON_DATA_OT_NAME, c->name);
    SetMonData(mon, MON_DATA_OT_GENDER, &gender);
    SetMonData(mon, MON_DATA_MET_LOCATION, &met);
    if (m->injuryPart || m->lostParts || m->injuryFlags)
        Injury_Set(mon, m->injuryPart, m->injuryDamage * INJURY_DAMAGE_UNIT, m->lostParts, m->injuryFlags & INJURY_FLAG_SCAR);
    CalculateMonStats(mon);
    SetFusion(mon, m);
}

void Rovers_PrepareTrade(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];

    MakeRoverMon(&gParties[B_TRAINER_OPPONENT_A][0], a->w.id, &a->w.mons[sRovers.tradeSlot], METLOC_IN_GAME_TRADE);
    gSpecialVar_0x8005 = 0;
    sRovers.trading = TRUE;
}

// A Pokémon a rover sold on the market or at the auction house comes as a
// short note instead of a real one (server/rovers_trade.py): "RVR", 1, the
// rover id, then its RoverMon. Turns it into the real Pokémon.
bool32 Rovers_FromListing(struct Pokemon *mon)
{
    const u8 *raw = (const u8 *)mon;
    struct RoverMon m;
    u32 id;

    if (raw[0] != 'R' || raw[1] != 'V' || raw[2] != 'R' || raw[3] != 1)
        return FALSE;
    id = raw[4];
    memcpy(&m, raw + 5, sizeof(m));
    if (id >= ROVER_COUNT || m.species == SPECIES_NONE || m.species >= NUM_SPECIES || m.level == 0 || m.level > MAX_LEVEL)
    {
        ZeroMonData(mon);
        return FALSE;
    }
    MakeRoverMon(mon, id, &m, METLOC_IN_GAME_TRADE);
    return TRUE;
}

// The trade scene uses these instead of its own table's names.
bool32 Rovers_IsTrading(void)
{
    return sRovers.trading;
}

const u8 *Rovers_TradeName(void)
{
    return Cast(&sRovers.actors[sRovers.talk])->name;
}

// Back on the map: the player's Pokémon joins the rover's team.
void Rovers_AfterTrade(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];
    struct RoverMon *m = &a->w.mons[sRovers.tradeSlot];
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u32 i;

    sRovers.trading = FALSE;
    if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
        return;
    memset(m, 0, sizeof(*m));
    m->species = GetMonData(mon, MON_DATA_SPECIES);
    m->level = GetMonData(mon, MON_DATA_LEVEL);
    m->movesLevel = m->level;
    for (i = 0; i < MAX_MON_MOVES; i++)
        m->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
    m->injuryPart = GetMonData(mon, MON_DATA_INJURY_PART);
    m->injuryDamage = GetMonData(mon, MON_DATA_INJURY_DAMAGE);
    m->lostParts = GetMonData(mon, MON_DATA_LOST_PARTS);
    m->injuryFlags = GetMonData(mon, MON_DATA_INJURY_FLAGS) & INJURY_FLAG_SCAR;
    Sync(sRovers.talk, EV_TRADED, m->species);
}

// The player battles a rover (from the talk script, waitstate).
void Rovers_StartChallenge(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.talk];
    BuildTrainer(a);
    LifeBattle_Start(&sRoverTrainer, COMPOUND_STRING("Good battle! I'll train harder."));
    ApplyInjuries(gParties[B_TRAINER_OPPONENT_A], a);
}

void Rovers_OnLifeBattleEnd(const struct Trainer *trainer, bool32 playerWon)
{
    struct RoverActor *a;
    if (trainer != &sRoverTrainer)
        return;
    a = &sRovers.actors[sRovers.talk];
    if (sRovers.leagueFighting)
    {
        // A league match: the server keeps the rover's record.
        sRovers.leagueFighting = FALSE;
        sRovers.leagueWon = playerWon;
        QueueRequest(REQ_LEAGUE_RESULT, 0, playerWon, 0, sRovers.leagueMatch);
        a->state = RS_NONE;
        return;
    }
    ReadInjuries(gParties[B_TRAINER_OPPONENT_A], a);
    if (playerWon)
        a->w.losses++, a->w.hurt++;
    else
        a->w.wins++;
    QueueRequest(REQ_BEAT, a->w.id, !playerWon, 1, 0);
    Sync(sRovers.talk, EV_NONE, 0);
}

bool32 Rovers_IsWatching(void)
{
    return sRovers.watching;
}

static void CB2_EndWatch(void)
{
    struct RoverActor *a = &sRovers.actors[sRovers.watchA];
    bool32 leftWon = (gBattleOutcome == B_OUTCOME_WON);

    ReadInjuries(gParties[B_TRAINER_PLAYER], a);
    if (sRovers.watchB != FOE_TRAINER)
        ReadInjuries(gParties[B_TRAINER_OPPONENT_A], &sRovers.actors[sRovers.watchB]);
    LoadPlayerParty();
    StringCopy(gSaveBlock2Ptr->playerName, sRovers.savedName);
    gSaveBlock2Ptr->playerGender = sRovers.savedGender;
    SetMoney(&gSaveBlock1Ptr->money, sRovers.savedMoney);
    gPartnerTrainerId = sRovers.savedPartner;
    gLifeTrainer = NULL;
    sRovers.watching = FALSE;

    // The battle on the map ends the same way, once the map is back.
    sRovers.duelResult = leftWon ? 2 : 1;
    gSpecialVar_Result = leftWon;
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

// Watch the duel the talked-to rover is in (waitstate). The left rover
// takes the player's place, both sides played by the AI; the player's own
// team, name and money are put back afterwards.
void Rovers_StartWatch(void)
{
    u32 left = sRovers.talk;
    struct RoverActor *a, *b = NULL;
    const struct RoverLook *look;
    u32 i;

    if (sRovers.actors[left].timer == 0xFF && sRovers.actors[left].partner != FOE_TRAINER)
        left = sRovers.actors[left].partner;
    a = &sRovers.actors[left];
    look = Look(a);
    sRovers.watchA = left;
    sRovers.watchB = a->partner;

    sRovers.savedMoney = GetMoney(&gSaveBlock1Ptr->money);
    sRovers.savedPartner = gPartnerTrainerId;
    sRovers.savedGender = gSaveBlock2Ptr->playerGender;
    StringCopy(sRovers.savedName, gSaveBlock2Ptr->playerName);
    SavePlayerParty();

    // The left rover's team.
    ZeroPlayerPartyMons();
    for (i = 0; i < a->w.count; i++)
    {
        const struct RoverMon *m = &a->w.mons[i];
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 j;
        CreateMonWithIVsPersonality(mon, m->species, m->level, 12 + (a->w.id + i) % 20, Random32());
        for (j = 0; j < MAX_MON_MOVES; j++)
            SetMonMoveSlot(mon, m->moves[j] < MOVES_COUNT ? m->moves[j] : MOVE_NONE, j);
    }
    gPartiesCount[B_TRAINER_PLAYER] = a->w.count;
    ApplyInjuries(gParties[B_TRAINER_PLAYER], a);
    StringCopyN(gSaveBlock2Ptr->playerName, Cast(a)->name, PLAYER_NAME_LENGTH);
    gSaveBlock2Ptr->playerName[PLAYER_NAME_LENGTH] = EOS;
    gSaveBlock2Ptr->playerGender = look->female ? FEMALE : MALE;

    memset(&gTrainerBattleParameter, 0, sizeof(gTrainerBattleParameter));
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    if (a->partner == FOE_TRAINER)
    {
        gLifeTrainer = NULL;
        TRAINER_BATTLE_PARAM.opponentA = sRovers.foeTrainer;
        gPartnerTrainerId = sRovers.foeTrainer;
        CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], GetTrainerStructFromId(sRovers.foeTrainer));
    }
    else
    {
        b = &sRovers.actors[a->partner];
        BuildTrainer(b);
        gLifeTrainer = &sRoverTrainer;
        TRAINER_BATTLE_PARAM.opponentA = TRAINER_NONE;
        CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sRoverTrainer);
        ApplyInjuries(gParties[B_TRAINER_OPPONENT_A], b);
    }
    TRAINER_BATTLE_PARAM.defeatTextA = (u8 *)COMPOUND_STRING("What a battle!");
    sRovers.watching = TRUE;
    BattleSetup_StartRoverWatch(CB2_EndWatch);
}

// Quest board -------------------------------------------------------------------------------

static void SpawnBoard(void)
{
    u32 i;

    if (!IsTown() || gMapHeader.events == NULL)
        return;
    for (i = 0; i < gMapHeader.events->warpCount; i++)
    {
        const struct WarpEvent *warp = &gMapHeader.events->warps[i];
        const struct MapHeader *dest = Overworld_GetMapHeaderByGroupAndId(warp->mapGroup, warp->mapNum);
        s16 x = warp->x + MAP_OFFSET, y = warp->y + MAP_OFFSET + 1;
        u32 k;

        if (warp->mapGroup == MAP_GROUP(MAP_DYNAMIC) || dest == NULL || dest->mapType != MAP_TYPE_INDOOR)
            continue;
        if (dest->regionMapSectionId != gMapHeader.regionMapSectionId)
            continue;
        // A Pokémon Center's door: its map's music is the Pokémon Center's.
        if (dest->music != MUS_POKE_CENTER)
            continue;
        for (k = 0; k < 4; k++)
        {
            static const s8 sOffsets[][2] = {{-2, 0}, {2, 0}, {-3, 0}, {3, 0}};
            s16 bx = x + sOffsets[k][0], by = y + sOffsets[k][1];
            if (CanStandAt(bx, by, FALSE))
            {
                Spawn(LOCALID_ROVER_BOARD, OBJ_EVENT_GFX_TRAINER_TIPS, bx, by, DIR_SOUTH);
                return;
            }
        }
    }
}

// Asks the server for the board (waitstate). VAR_RESULT afterwards: TRUE
// if it answered.
void Rovers_BoardFetch(void)
{
    if (!Npx_IsOnline() || sRovers.waiting != REQ_NONE || !NpxRpc_Begin("MAJOR"))
    {
        sRovers.boardOk = FALSE;
        gSpecialVar_Result = FALSE;
        ScriptContext_Enable();
        return;
    }
    NpxRpc_Word("LIST");
    NpxRpc_Num(gSaveBlock1Ptr->location.mapGroup);
    NpxRpc_Num(gSaveBlock1Ptr->location.mapNum);
    NpxRpc_Send();
    sRovers.waiting = REQ_MAJORS;
    sRovers.boardWaiting = TRUE;
}

void Rovers_BoardRankingsFetch(void)
{
    if (!Npx_IsOnline() || sRovers.waiting != REQ_NONE || !NpxRpc_Begin("ROVER"))
    {
        sRovers.boardOk = FALSE;
        ScriptContext_Enable();
        return;
    }
    NpxRpc_Word("BOARD");
    NpxRpc_Send();
    sRovers.waiting = REQ_BOARD;
    sRovers.boardWaiting = TRUE;
}

static void FinishBoardRead(void)
{
    sRovers.waiting = REQ_NONE;
    NpxRpc_Release();
}

// After Rovers_BoardFetch: the jobs in gStringVar4, the reward paid out.
// VAR_RESULT FALSE if the server didn't answer.
void Rovers_BoardShowJobs(void)
{
    u8 *s = gStringVar4;
    u8 name[24];
    u32 rows, r, jobs = 0, reward;

    gSpecialVar_Result = sRovers.boardOk;
    if (!sRovers.boardOk)
        return;
    rows = AnswerRows();
    reward = AnswerNum(0, 0);
    gSpecialVar_0x8005 = 0;
    if (reward != 0)
    {
        AddMoney(&gSaveBlock1Ptr->money, reward);
        ConvertIntToDecimalStringN(gStringVar2, reward, STR_CONV_MODE_LEFT_ALIGN, 7);
        gSpecialVar_0x8005 = 1;
    }
    for (r = 1; r < rows; r++)
    {
        u32 id = AnswerNum(r, 0), species = AnswerNum(r, 1), level = AnswerNum(r, 2);
        if (species == 0 || species >= NUM_SPECIES)
            continue;
        AnswerText(name, r, 3, 20);
        if (jobs != 0)
            s = StringCopy(s, COMPOUND_STRING("\p"));
        if (id != 0)
        {
            s = StringCopy(s, COMPOUND_STRING("WANTED: rampaging "));
            s = StringCopy(s, GetSpeciesName(species));
            s = StringCopy(s, COMPOUND_STRING(" Lv"));
            s = ConvertIntToDecimalStringN(s, level, STR_CONV_MODE_LEFT_ALIGN, 3);
            s = StringCopy(s, COMPOUND_STRING("\nSeen at: "));
            s = StringCopy(s, name);
        }
        else
        {
            u8 who[16];
            AnswerText(who, r, 4, 10);
            s = StringCopy(s, COMPOUND_STRING("CLEARED: "));
            s = StringCopy(s, GetSpeciesName(species));
            s = StringCopy(s, COMPOUND_STRING(" Lv"));
            s = ConvertIntToDecimalStringN(s, level, STR_CONV_MODE_LEFT_ALIGN, 3);
            s = StringCopy(s, COMPOUND_STRING("\nby "));
            s = StringCopy(s, who);
            s = StringCopy(s, COMPOUND_STRING(" at "));
            s = StringCopy(s, name);
        }
        jobs++;
    }
    if (jobs == 0)
        StringCopy(gStringVar4, COMPOUND_STRING("No dangerous Pokémon nearby.\nThe roads are safe for now."));
    FinishBoardRead();
}

void Rovers_BoardShowRankings(void)
{
    u8 *s = gStringVar4;
    u8 field[48];
    u32 rows, r, shown = 0;
    bool32 newsPart = FALSE;

    gSpecialVar_Result = sRovers.boardOk;
    if (!sRovers.boardOk)
        return;
    rows = AnswerRows();
    for (r = 0; r < rows && shown < 10; r++)
    {
        AnswerText(field, r, 0, 46);
        if (field[0] == CHAR_HYPHEN && field[1] == EOS)
        {
            newsPart = TRUE;
            s = StringCopy(s, COMPOUND_STRING("\pLATEST NEWS"));
            continue;
        }
        s = StringCopy(s, shown == 0 ? COMPOUND_STRING("TOP TRAINERS\n") : (shown % 2 ? COMPOUND_STRING("\n") : COMPOUND_STRING("\p")));
        if (newsPart)
        {
            s = StringCopy(s, field);
        }
        else
        {
            s = ConvertIntToDecimalStringN(s, shown + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
            s = StringCopy(s, COMPOUND_STRING(". "));
            s = StringCopy(s, field);
            s = StringCopy(s, COMPOUND_STRING(": "));
            s = ConvertIntToDecimalStringN(s, AnswerNum(r, 1), STR_CONV_MODE_LEFT_ALIGN, 1);
            s = StringCopy(s, AnswerNum(r, 1) == 1 ? COMPOUND_STRING(" badge, won ") : COMPOUND_STRING(" badges, won "));
            s = ConvertIntToDecimalStringN(s, AnswerNum(r, 2), STR_CONV_MODE_LEFT_ALIGN, 4);
            s = StringCopy(s, COMPOUND_STRING(", lost "));
            s = ConvertIntToDecimalStringN(s, AnswerNum(r, 3), STR_CONV_MODE_LEFT_ALIGN, 4);
        }
        shown++;
    }
    FinishBoardRead();
}

// The weekly Rover League ------------------------------------------------------------------------
//
// On the quest boards (server/rovers_league.py): the bracket, entering,
// playing the player's match, and watching a match still to be played.
// The rovers in a league match are loaded into the last two actor slots
// (whoever was there walks off) and never spawned on the map.

#define LEAGUE_A (ROVERS_MAX_HERE - 2)
#define LEAGUE_B (ROVERS_MAX_HERE - 1)

static const char *const sLeagueWords[] = {"INFO", "ENTER", "FIGHT", "WATCH"};
enum { LEAGUE_INFO, LEAGUE_ENTER, LEAGUE_FIGHT, LEAGUE_WATCH };

// The request goes out when the channel is free (other features share it);
// the script waits for the answer.
static void LeagueAsk(u32 word, u16 arg)
{
    if (!Npx_IsOnline())
    {
        sRovers.boardOk = FALSE;
        ScriptContext_Enable();
        return;
    }
    sRovers.leagueAsk = word + 1;
    sRovers.leagueAskArg = arg;
    sRovers.leagueAskWait = 0;
}

static void SendLeagueAsk(void)
{
    u32 word = sRovers.leagueAsk - 1;

    if (sRovers.waiting != REQ_NONE || !NpxRpc_Begin("ROVER"))
    {
        if (++sRovers.leagueAskWait > 60 * 20)
        {
            sRovers.leagueAsk = 0;
            sRovers.boardOk = FALSE;
            ScriptContext_Enable();
        }
        return;
    }
    sRovers.leagueAsk = 0;
    NpxRpc_Word("LEAGUE");
    NpxRpc_Word(sLeagueWords[word]);
    if (word == LEAGUE_WATCH)
        NpxRpc_Num(sRovers.leagueAskArg);
    NpxRpc_Send();
    sRovers.waiting = REQ_BOARD;
    sRovers.boardWaiting = TRUE;
}

void Rovers_LeagueInfoFetch(void)  { LeagueAsk(LEAGUE_INFO, 0); }
void Rovers_LeagueEnterFetch(void) { LeagueAsk(LEAGUE_ENTER, 0); }
void Rovers_LeagueFightFetch(void) { LeagueAsk(LEAGUE_FIGHT, 0); }
void Rovers_LeagueWatchFetch(void) { LeagueAsk(LEAGUE_WATCH, sRovers.leagueWatch); }

static const u8 *const sRoundNames[] = {
    COMPOUND_STRING("QUARTERFINALS"), COMPOUND_STRING("SEMIFINALS"), COMPOUND_STRING("FINAL"),
};

// After Rovers_LeagueInfoFetch: the bracket in gStringVar4. VAR_RESULT FALSE
// if the server didn't answer; VAR_0x8004 the player can enter, VAR_0x8005
// their match is waiting (STR_VAR_3 the opponent), VAR_0x8006 a match can
// be watched (STR_VAR_1 and STR_VAR_2 its rovers).
void Rovers_LeagueShow(void)
{
    u8 *s = gStringVar4;
    u8 a[16], b[16];
    u32 rows, r, round, state, lastRound = 0xFF, n = 0;

    gSpecialVar_Result = sRovers.boardOk;
    gSpecialVar_0x8004 = gSpecialVar_0x8005 = gSpecialVar_0x8006 = 0;
    if (!sRovers.boardOk)
        return;
    round = AnswerNum(0, 1);
    state = AnswerNum(0, 3);
    sRovers.leagueMatch = AnswerNum(0, 4);
    AnswerText(gStringVar3, 0, 5, PLAYER_NAME_LENGTH);
    gSpecialVar_0x8004 = AnswerNum(0, 6);
    gSpecialVar_0x8005 = (state == 1);
    s = StringCopy(s, COMPOUND_STRING("ROVER LEAGUE, this week\n"));
    if (round < 3)
    {
        s = StringCopy(s, sRoundNames[round]);
        s = StringCopy(s, COMPOUND_STRING(": "));
        s = ConvertIntToDecimalStringN(s, AnswerNum(0, 2), STR_CONV_MODE_LEFT_ALIGN, 3);
        s = StringCopy(s, COMPOUND_STRING(" hours left"));
    }
    else
    {
        s = StringCopy(s, COMPOUND_STRING("It's over until next week!"));
    }
    rows = AnswerRows();
    for (r = 1; r < rows; r++)
    {
        u32 rnd = AnswerNum(r, 0), winner = AnswerNum(r, 4);
        AnswerText(a, r, 2, PLAYER_NAME_LENGTH);
        AnswerText(b, r, 3, PLAYER_NAME_LENGTH);
        if (rnd != lastRound)
        {
            s = StringCopy(s, COMPOUND_STRING("\p"));
            s = StringCopy(s, sRoundNames[rnd % 3]);
            s = StringCopy(s, COMPOUND_STRING("\n"));
            lastRound = rnd;
            n = 0;
        }
        else
        {
            s = StringCopy(s, n % 2 ? COMPOUND_STRING("\p") : COMPOUND_STRING("\n"));
        }
        n++;
        s = StringCopy(s, a);
        s = StringCopy(s, COMPOUND_STRING(" vs "));
        s = StringCopy(s, b);
        if (winner != 0)
        {
            s = StringCopy(s, COMPOUND_STRING(": "));
            s = StringCopy(s, winner == 1 ? a : b);
            s = StringCopy(s, COMPOUND_STRING(" won"));
        }
        if (AnswerNum(r, 6) && !gSpecialVar_0x8006)
        {
            gSpecialVar_0x8006 = TRUE;
            sRovers.leagueWatch = AnswerNum(r, 5);
            StringCopy(gStringVar1, a);
            StringCopy(gStringVar2, b);
        }
    }
    if (state == 3)
        s = StringCopy(s, COMPOUND_STRING("\pYou're out this week. Next time!"));
    else if (state == 4)
        s = StringCopy(s, COMPOUND_STRING("\pAnd you're this week's champion!"));
    else if (state == 2)
        s = StringCopy(s, COMPOUND_STRING("\pYour next match comes when this\nround is over."));
    FinishBoardRead();
}

// After Rovers_LeagueEnterFetch: STR_VAR_3 the first opponent.
void Rovers_LeagueEntered(void)
{
    gSpecialVar_Result = sRovers.boardOk;
    if (!sRovers.boardOk)
        return;
    sRovers.leagueMatch = AnswerNum(0, 0);
    AnswerText(gStringVar3, 0, 1, PLAYER_NAME_LENGTH);
    FinishBoardRead();
}

static bool32 LoadLeagueRover(u32 slot, const u8 *wire)
{
    struct RoverActor *a = &sRovers.actors[slot];
    struct RoverActor *other;

    // Whoever stood in this slot walks off; a duel it was in ends.
    if (a->state == RS_DUEL || a->state == RS_DUEL_OVER)
    {
        if (a->partner < ROVERS_MAX_HERE)
        {
            other = &sRovers.actors[a->partner];
            if (other->state == RS_DUEL || other->state == RS_DUEL_OVER)
                other->state = RS_WANDER;
        }
    }
    Actor_Remove(slot);
    memset(a, 0, sizeof(*a));
    memcpy(&a->w, wire, sizeof(a->w));
    if (a->w.id >= ROVER_COUNT || a->w.count == 0)
        return FALSE;
    NormalizeTeam(a);
    return TRUE;
}

// After Rovers_LeagueFightFetch: the opponent, ready for
// Rovers_StartChallenge. VAR_RESULT FALSE if it couldn't be loaded.
void Rovers_LeagueLoadFight(void)
{
    u32 size;
    const u8 *data;

    gSpecialVar_Result = FALSE;
    if (!sRovers.boardOk)
        return;
    data = AnswerBinary(&size);
    sRovers.leagueMatch = AnswerNum(0, 0);
    sRovers.leagueRound = AnswerNum(0, 1);
    if (data != NULL && size >= sizeof(struct RoverWire) && LoadLeagueRover(LEAGUE_A, data))
    {
        sRovers.talk = LEAGUE_A;
        sRovers.leagueFighting = TRUE;
        StringCopy(gStringVar3, Cast(&sRovers.actors[LEAGUE_A])->name);
        gSpecialVar_Result = TRUE;
    }
    FinishBoardRead();
}

// After the match: VAR_RESULT TRUE if the player won (the prize is paid,
// STR_VAR_2), and whether that made them champion (VAR_0x8004).
void Rovers_LeagueAfterFight(void)
{
    u32 prize = 2000 * (sRovers.leagueRound + 1);

    gSpecialVar_Result = sRovers.leagueWon;
    gSpecialVar_0x8004 = sRovers.leagueWon && sRovers.leagueRound == 2;
    if (gSpecialVar_0x8004)
        prize += 10000;
    if (sRovers.leagueWon)
    {
        AddMoney(&gSaveBlock1Ptr->money, prize);
        ConvertIntToDecimalStringN(gStringVar2, prize, STR_CONV_MODE_LEFT_ALIGN, 6);
    }
    sRovers.leagueWon = FALSE;
}

// After Rovers_LeagueWatchFetch: both rovers, ready for Rovers_StartWatch.
void Rovers_LeagueLoadWatch(void)
{
    u32 size;
    const u8 *data;

    gSpecialVar_Result = FALSE;
    if (!sRovers.boardOk)
        return;
    data = AnswerBinary(&size);
    sRovers.leagueWatch = AnswerNum(0, 0);
    if (data != NULL && size >= 2 * sizeof(struct RoverWire)
     && LoadLeagueRover(LEAGUE_A, data) && LoadLeagueRover(LEAGUE_B, data + sizeof(struct RoverWire)))
    {
        sRovers.actors[LEAGUE_A].partner = LEAGUE_B;
        sRovers.actors[LEAGUE_B].partner = LEAGUE_A;
        sRovers.talk = LEAGUE_A;
        gSpecialVar_Result = TRUE;
    }
    FinishBoardRead();
}

// After watching: what the player saw is the result. STR_VAR_1 the winner.
void Rovers_LeagueAfterWatch(void)
{
    bool32 leftWon = gSpecialVar_Result;
    StringCopy(gStringVar1, Cast(&sRovers.actors[leftWon ? LEAGUE_A : LEAGUE_B])->name);
    QueueRequest(REQ_LEAGUE_SEEN, 0, leftWon, 0, sRovers.leagueWatch);
    sRovers.actors[LEAGUE_A].state = RS_NONE;
    sRovers.actors[LEAGUE_B].state = RS_NONE;
}

// Every frame ------------------------------------------------------------------------------------

static void RespawnMissing(void)
{
    u32 i;
    for (i = 0; i < ROVERS_MAX_HERE; i++)
    {
        struct RoverActor *a = &sRovers.actors[i];
        if (a->state == RS_NONE || TrainerObj(i) != NULL)
        {
            a->missing = 0;
            continue;
        }
        if (++a->missing < (a->state == RS_BUDDY ? 2 : 60))
            continue;
        a->missing = 0;
        if (a->state == RS_BUDDY)
        {
            s16 x, y;
            if (SpotNextToPlayer(&x, &y))
                SpawnActor(i, x, y, DIR_SOUTH);
            continue;
        }
        if (a->state == RS_DUEL || a->state == RS_DUEL_OVER)
            a->state = RS_WANDER;
        {
            s16 x, y;
            if (FindSpot(&x, &y, 4, FALSE))
                SpawnActor(i, x, y, Random() % 4 + 1);
            a->state = RS_WANDER;
        }
    }
}

bool32 Rovers_OverworldUpdate(void)
{
    u32 i;

    UpdateRequests();
    if (gRoverShowcase.pending && !ArePlayerFieldControlsLocked())
        RunShowcase();
    if (sRovers.duelResult != 0 && OnRoverMap())
    {
        u32 i = sRovers.watchA;
        if (sRovers.actors[i].state == RS_DUEL)
        {
            EndDuel(i, sRovers.duelResult - 1);
            Sync(i, EV_NONE, 0);
            if (sRovers.watchB != FOE_TRAINER)
                Sync(sRovers.watchB, EV_NONE, 0);
        }
        sRovers.duelResult = 0;
    }
    if (!OnRoverMap() || ArePlayerFieldControlsLocked() || QuietForTests())
        return FALSE;
    if (!sRovers.spawned)
    {
        if (!Npx_IsOnline() || gRoverShowcase.offline)
            SpawnOffline();
        else if (!sRovers.wantHere && sRovers.waiting == REQ_NONE)
            sRovers.wantHere = TRUE;
        return FALSE;
    }
    if (sRovers.talkCountdown != 0 && !IsWildState(sRovers.actors[(gRoverShowcase.talk - 1) % ROVERS_MAX_HERE].state)
     && --sRovers.talkCountdown == 0)
    {
        sRovers.talk = (gRoverShowcase.talk - 1) % ROVERS_MAX_HERE;
        gRoverShowcase.talk = 0;
        ScriptContext_SetupScript(Rovers_EventScript_Talk);
        return TRUE;
    }
    if (sRovers.farewellId != 0 && (sRovers.farewellDelay == 0 || --sRovers.farewellDelay == 0))
    {
        StringCopy(gStringVar1, gRoverCast[sRovers.farewellId - 1].name);
        gSpecialVar_0x8004 = sRovers.farewellItem;
        sRovers.farewellId = 0;
        ScriptContext_SetupScript(Rovers_EventScript_BuddyBye);
        return TRUE;
    }
    if (ObjByLocalId(LOCALID_ROVER_BOARD) == NULL && IsTown())
        SpawnBoard();
    RespawnMissing();
    for (i = 0; i < ROVERS_MAX_HERE; i++)
    {
        switch (sRovers.actors[i].state)
        {
        case RS_WANDER:
            UpdateWander(i);
            break;
        case RS_WILD_APPEAR:
        case RS_WILD_FIGHT:
        case RS_THROW:
        case RS_SHAKE:
        case RS_KO:
            UpdateWild(i);
            break;
        case RS_DUEL:
            UpdateDuel(i);
            break;
        case RS_DUEL_OVER:
            UpdateDuelOver(i);
            break;
        case RS_LEAVING:
            UpdateLeaving(i);
            break;
        case RS_BUDDY:
            UpdateBuddy(i);
            break;
        }
    }
    if (sRovers.startRival != 0)
    {
        sRovers.talk = sRovers.startRival - 1;
        sRovers.startRival = 0;
        StringCopy(gStringVar1, Cast(&sRovers.actors[sRovers.talk])->name);
        ScriptContext_SetupScript(Rovers_EventScript_Rival);
        return TRUE;
    }
    // The rampaging Pokémon, if this map has one.
    if (sRovers.majorId != 0 && !sRovers.majorSpawned && sRovers.chargerState == CHARGER_NONE)
    {
        sRovers.majorSpawned = TRUE;
        SpawnCharger(TRUE);
    }
    if (sRovers.chargerState == CHARGER_CHASE || sRovers.chargerState == CHARGER_CALM)
        return UpdateCharger();
    if (sRovers.chargerState == CHARGER_NONE && gMapHeader.mapType != MAP_TYPE_TOWN && gMapHeader.mapType != MAP_TYPE_CITY)
    {
        if (sRovers.chargeCountdown != 0)
            sRovers.chargeCountdown--;
        else
        {
            sRovers.chargeCountdown = 60 * (20 + Random() % 40);
            if (gRoverShowcase.scene == ROVER_SCENE_CHARGER
             || (gRoverShowcase.scene == 0 && Random() % 2 == 0))
                SpawnCharger(FALSE);
        }
    }
    return FALSE;
}

bool32 Rovers_InjuresBattler(u32 battler)
{
    if (sRovers.watching)
        return TRUE;
    return gLifeTrainer == &sRoverTrainer && !IsOnPlayerSide(battler);
}

// The picture on the player's side of an AI-vs-AI battle: the rover the
// player is watching.
u16 Rovers_AiVsAiTrainerPic(void)
{
    if (sRovers.watching)
        return Look(&sRovers.actors[sRovers.watchA])->pic;
    return GetTrainerPicFromId(gPartnerTrainerId);
}
