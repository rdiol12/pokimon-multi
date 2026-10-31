#include "global.h"
#include "achievements.h"
#include "battle.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "main.h"
#include "pokemon.h"
#include "script.h"
#include "string_util.h"
#include "task.h"
#include "world.h"
#include "frontier_online.h"
#include "world_mailbox.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/pastime.h"

// The online side of the world features, through gWorldMailbox (see
// include/world_mailbox.h and tools/netplay_bridge/netplay_world.lua):
//
// - Other players' following Pokémon and mounts (shiny ones look shiny).
// - Leaderboards: biggest fish, berries, hardcore runs, Safari hunts.
// - Safari hunts: the server picks a Pokémon every half hour; the biggest
//   one caught before time runs out wins.
// - Photos: the bridge saves a screenshot and uploads it to the gallery.
// - House visits: a player who opens their house to visitors has their
//   furniture shown to other players who come in while they are inside.

EWRAM_DATA struct WorldMailbox gWorldMailbox = {0};

#define LOCALID_REMOTE_FOLLOWER_START 200 // + netplay remote slot
#define REQUEST_QUEUE 4

struct WorldRequest
{
    u8 type;
    u8 board;
    u16 key;
    u32 value;
    u32 extra;
};

struct RemoteFollow
{
    u16 playerId;
    s16 lastX, lastY; // the remote player's tile last frame
    u8 mountSprite;
    u8 mountObject;
    u16 mountGfx;
};

static EWRAM_DATA struct WorldRequest sQueue[REQUEST_QUEUE] = {0};
static EWRAM_DATA u8 sQueued = 0;
static EWRAM_DATA struct RemoteFollow sFollow[NETPLAY_MAX_REMOTES] = {0};
static EWRAM_DATA u16 sEnteredHunt = 0;
static EWRAM_DATA u16 sWildSpecies = 0;
static EWRAM_DATA u32 sWildPersonality = 0;
static EWRAM_DATA u32 sLastHardcoreValue = 0;

// Safari hunt targets: common Hoenn Pokémon, so every hunt can be played
// right away. The server's hunt id picks one.
static const u16 sHuntSpecies[] =
{
    SPECIES_ZIGZAGOON, SPECIES_WURMPLE, SPECIES_POOCHYENA, SPECIES_TAILLOW,
    SPECIES_LOTAD, SPECIES_SEEDOT, SPECIES_MAGIKARP, SPECIES_TENTACOOL,
    SPECIES_MARILL, SPECIES_ODDISH, SPECIES_ZUBAT, SPECIES_GEODUDE,
    SPECIES_WINGULL, SPECIES_ELECTRIKE, SPECIES_SPINDA, SPECIES_SWABLU,
    SPECIES_NUMEL, SPECIES_SANDSHREW, SPECIES_MAKUHITA, SPECIES_SHROOMISH,
};

static bool32 IsOnline(void)
{
    return gNetplayMailbox.bridgeState == NETPLAY_ONLINE;
}

static void InitMailbox(void)
{
    static const char magic[8] = {'P', 'K', 'M', 'N', 'W', 'L', 'D', '1'};
    u32 i;

    if (memcmp(gWorldMailbox.magic, magic, sizeof(magic)) == 0)
        return;
    memset(&gWorldMailbox, 0, sizeof(gWorldMailbox));
    gWorldMailbox.version = WORLD_MAILBOX_VERSION;
    memcpy(gWorldMailbox.magic, magic, sizeof(magic));
    for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
        sFollow[i].mountSprite = MAX_SPRITES;
}

static void Request(u8 type, u8 board, u16 key, u32 value, u32 extra)
{
    struct WorldRequest *req;

    if (sQueued == REQUEST_QUEUE)
        memmove(&sQueue[0], &sQueue[1], sizeof(sQueue[0]) * --sQueued); // drop the oldest
    req = &sQueue[sQueued++];
    req->type = type;
    req->board = board;
    req->key = key;
    req->value = value;
    req->extra = extra;
}

// One request at a time: the next goes out once the bridge has taken the last.
static void FlushRequests(void)
{
    if (sQueued == 0 || gWorldMailbox.reqAck != gWorldMailbox.reqSeq || !IsOnline())
        return;
    gWorldMailbox.reqType = sQueue[0].type;
    gWorldMailbox.reqBoard = sQueue[0].board;
    gWorldMailbox.reqKey = sQueue[0].key;
    gWorldMailbox.reqValue = sQueue[0].value;
    gWorldMailbox.reqExtra = sQueue[0].extra;
    gWorldMailbox.reqSeq++;
    memmove(&sQueue[0], &sQueue[1], sizeof(sQueue[0]) * --sQueued);
}

static u16 HuntSpecies(void)
{
    if (gWorldMailbox.huntId == 0 || !IsOnline())
        return SPECIES_NONE;
    return sHuntSpecies[gWorldMailbox.huntId % ARRAY_COUNT(sHuntSpecies)];
}

void World_SubmitScore(u8 board, u32 value, u32 extra)
{
    Request(WORLD_REQ_SCORE, board, board == WORLD_BOARD_HUNT ? gWorldMailbox.huntId : 0, value, extra);
}

static u32 CountBadges(void)
{
    u32 i, n = 0;

    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        n += FlagGet(i);
    return n;
}

// Hardcore ladder: more badges first (the Champion counts as a ninth), then
// fewer losses, then runs still going. extra: losses | state << 8.
void World_OnHardcoreProgress(void)
{
    const struct WorldSaveData *world = &gSaveBlock3Ptr->world;
    u32 badges = FlagGet(FLAG_SYS_GAME_CLEAR) ? 9 : CountBadges();
    u32 value;

    if (world->hardcore == HARDCORE_OFF)
        return;
    value = (badges << 16) | ((255 - world->hardcoreDeaths) << 8) | (world->hardcore == HARDCORE_ON);
    if (value == sLastHardcoreValue)
        return;
    sLastHardcoreValue = value;
    World_SubmitScore(WORLD_BOARD_HARDCORE, value, world->hardcoreDeaths | (world->hardcore << 8));
}

// Our look, for other players: the follower (unless riding) and the mount.
static void PublishLook(void)
{
    struct ObjectEvent *follower = GetFollowerObject();

    gWorldMailbox.mountGfx = Mount_GetGfx();
    gWorldMailbox.followerGfx = (follower != NULL && !follower->invisible && gWorldMailbox.mountGfx == 0)
                              ? follower->graphicsId : 0;
    memcpy(gWorldMailbox.decor, gSaveBlock3Ptr->world.houseDecor, sizeof(gWorldMailbox.decor));
    gWorldMailbox.houseOpen = FlagGet(FLAG_WORLD_HOUSE_OPEN);
}

static const struct WorldRemoteLook *FindLook(u16 id)
{
    u32 i;

    for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
    {
        if (id != 0 && gWorldMailbox.remotes[i].id == id)
            return &gWorldMailbox.remotes[i];
    }
    return NULL;
}

static u8 FindObject(u8 localId)
{
    return GetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

static void RemoveFollower(u32 slot)
{
    u8 objId = FindObject(LOCALID_REMOTE_FOLLOWER_START + slot);

    if (objId != OBJECT_EVENTS_COUNT)
        RemoveObjectEvent(&gObjectEvents[objId]);
}

static void DropMount(u32 slot)
{
    if (sFollow[slot].mountGfx != 0 && Mount_IsAttached(sFollow[slot].mountSprite, sFollow[slot].mountObject))
        Mount_Detach(sFollow[slot].mountSprite);
    sFollow[slot].mountSprite = MAX_SPRITES;
    sFollow[slot].mountGfx = 0;
}

// Another player's follower walks to the tile they just left, like ours.
static void SyncFollower(u32 slot, struct ObjectEvent *avatar, u16 gfx)
{
    struct RemoteFollow *follow = &sFollow[slot];
    u8 objId = FindObject(LOCALID_REMOTE_FOLLOWER_START + slot);
    struct ObjectEvent *obj;
    s16 x = avatar->currentCoords.x, y = avatar->currentCoords.y, tx, ty, dx, dy;

    if (objId != OBJECT_EVENTS_COUNT && gObjectEvents[objId].graphicsId != gfx)
    {
        RemoveObjectEvent(&gObjectEvents[objId]);
        objId = OBJECT_EVENTS_COUNT;
    }
    if (objId == OBJECT_EVENTS_COUNT)
    {
        // Start one tile behind them.
        tx = x;
        ty = y;
        MoveCoords(GetOppositeDirection(avatar->facingDirection), &tx, &ty);
        objId = SpawnSpecialObjectEventParameterized(gfx, MOVEMENT_TYPE_NONE, LOCALID_REMOTE_FOLLOWER_START + slot,
                                                     tx, ty, avatar->currentElevation);
        follow->lastX = x;
        follow->lastY = y;
        if (objId != OBJECT_EVENTS_COUNT)
            ObjectEventTurn(&gObjectEvents[objId], avatar->facingDirection);
        return;
    }
    if (x == follow->lastX && y == follow->lastY)
        return;
    tx = follow->lastX;
    ty = follow->lastY;
    follow->lastX = x;
    follow->lastY = y;
    obj = &gObjectEvents[objId];
    if (ObjectEventIsHeldMovementActive(obj))
        ObjectEventClearHeldMovement(obj);
    dx = tx - obj->currentCoords.x;
    dy = ty - obj->currentCoords.y;
    obj->currentElevation = avatar->currentElevation;
    if (abs(dx) + abs(dy) == 1)
        ObjectEventSetHeldMovement(obj, GetWalkNormalMovementAction(dx > 0 ? DIR_EAST : dx < 0 ? DIR_WEST : dy > 0 ? DIR_SOUTH : DIR_NORTH));
    else if (dx != 0 || dy != 0)
        MoveObjectEventToMapCoords(obj, tx, ty);
}

static void SyncRemote(u32 slot)
{
    const struct NetplayRemotePlayer *remote = &gNetplayMailbox.remotes[slot];
    const struct WorldRemoteLook *look = FindLook(remote->id);
    u8 avatarId = FindObject(LOCALID_NETPLAY_PLAYER_START + slot);
    struct ObjectEvent *avatar;

    if (avatarId == OBJECT_EVENTS_COUNT || look == NULL || sFollow[slot].playerId != remote->id)
    {
        RemoveFollower(slot);
        DropMount(slot);
        sFollow[slot].playerId = (avatarId != OBJECT_EVENTS_COUNT) ? remote->id : 0;
        return;
    }
    avatar = &gObjectEvents[avatarId];

    if (look->mountGfx != 0)
    {
        if (sFollow[slot].mountGfx != look->mountGfx || !Mount_IsAttached(sFollow[slot].mountSprite, avatarId))
        {
            DropMount(slot);
            sFollow[slot].mountSprite = Mount_Attach(avatarId, look->mountGfx);
            sFollow[slot].mountObject = avatarId;
            sFollow[slot].mountGfx = look->mountGfx;
        }
    }
    else
    {
        DropMount(slot);
    }

    if (look->followerGfx != 0 && look->mountGfx == 0)
        SyncFollower(slot, avatar, look->followerGfx);
    else
        RemoveFollower(slot);
}

// Called every overworld frame.
void WorldOnline_Update(void)
{
    u32 i;

    InitMailbox();
    PublishLook();
    if ((gMain.vblankCounter1 & 0xFF) == 0)
        World_OnHardcoreProgress();
    FlushRequests();
    for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
        SyncRemote(i);
}

// The house of a player who opened it and is on this map, or NULL.
const u8 *WorldOnline_GetOpenHouse(u16 *playerId, u8 *name)
{
    u32 i;

    for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
    {
        const struct NetplayRemotePlayer *remote = &gNetplayMailbox.remotes[i];
        const struct WorldRemoteLook *look = FindLook(remote->id);
        if (look == NULL || !look->houseOpen || !IsOnline()
         || remote->mapGroup != gSaveBlock1Ptr->location.mapGroup || remote->mapNum != gSaveBlock1Ptr->location.mapNum)
            continue;
        if (*playerId != 0 && remote->id != *playerId)
            continue;
        *playerId = remote->id;
        if (name != NULL)
        {
            memcpy(name, remote->name, NETPLAY_NAME_LENGTH);
            name[NETPLAY_NAME_LENGTH - 1] = EOS;
        }
        return look->decor;
    }
    return NULL;
}

// Wild battles -------------------------------------------------------------

void World_OnWildBattleStart(void)
{
    sWildSpecies = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES);
    sWildPersonality = GetMonData(&gEnemyParty[0], MON_DATA_PERSONALITY);
}

// A Pokémon was caught: count it for the Safari hunt if it's the target.
void WorldOnline_OnCatch(void)
{
    u16 target = HuntSpecies();
    u32 size;

    if (target == SPECIES_NONE || sWildSpecies != target || gBattleResults.caughtMonSpecies != target)
        return;
    size = Records_SizeOf(target, sWildPersonality);
    World_SubmitScore(WORLD_BOARD_HUNT, size, target);
    if (sEnteredHunt != gWorldMailbox.huntId)
    {
        sEnteredHunt = gWorldMailbox.huntId;
        if (gSaveBlock3Ptr->world.huntsEntered < 0xFFFF)
            gSaveBlock3Ptr->world.huntsEntered++;
        Achievements_Unlock(ACH_HUNTER);
    }
    StringCopy(gStringVar1, GetSpeciesName(target));
    Records_FormatSize(gStringVar2, size);
    StringExpandPlaceholders(gStringVar3, COMPOUND_STRING("HUNT: {STR_VAR_1}, {STR_VAR_2}"));
    World_ShowToast(gStringVar3, COMPOUND_STRING("Entered! R > SAFARI HUNT for standings."));
}

// Script: R > SAFARI HUNT. VAR_RESULT: 0 offline, 1 no hunt yet, 2 a hunt
// is on (STR_VAR_1 target, STR_VAR_2 minutes left).
void WorldOnline_GetHunt(void)
{
    u16 target = HuntSpecies();

    if (!IsOnline())
        gSpecialVar_Result = 0;
    else if (target == SPECIES_NONE)
        gSpecialVar_Result = 1;
    else
    {
        gSpecialVar_Result = 2;
        StringCopy(gStringVar1, GetSpeciesName(target));
        ConvertIntToDecimalStringN(gStringVar2, gWorldMailbox.huntMinutesLeft, STR_CONV_MODE_LEFT_ALIGN, 2);
    }
}

// Photos -------------------------------------------------------------------

// Script: R > PHOTO, once the menu is closed. VAR_RESULT = TRUE if online.
void WorldOnline_TakePhoto(void)
{
    InitMailbox();
    gWorldMailbox.photoSeq++;
    if (gSaveBlock3Ptr->world.photosTaken < 0xFFFF)
        gSaveBlock3Ptr->world.photosTaken++;
    Achievements_Unlock(ACH_PHOTOGRAPHER);
    gSpecialVar_Result = IsOnline();
}

// Leaderboards ---------------------------------------------------------------

static EWRAM_DATA u8 sBoardSeq = 0;
static EWRAM_DATA u8 sBoardShown = 0;

static void FormatBoardValue(u8 *dest, u8 board, u32 value, u32 extra)
{
    switch (board)
    {
    case WORLD_BOARD_FISH:
    case WORLD_BOARD_HUNT:
        StringCopy(gStringVar1, GetSpeciesName(extra < NUM_SPECIES ? extra : SPECIES_NONE));
        Records_FormatSize(gStringVar2, value);
        StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} {STR_VAR_2}"));
        break;
    case WORLD_BOARD_BERRIES:
        ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} berries"));
        break;
    case WORLD_BOARD_QUIZ: // extra: answers right
        ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, extra, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} pts ({STR_VAR_2}/10)"));
        break;
    case WORLD_BOARD_BUG_CONTEST: // extra: species
        ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringCopy(gStringVar2, GetSpeciesName(extra));
        StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} {STR_VAR_2}"));
        break;
    case WORLD_BOARD_COOKING: // extra: contests won
        ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, extra, STR_CONV_MODE_LEFT_ALIGN, 4);
        StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} pts, won {STR_VAR_2}"));
        break;
    case WORLD_BOARD_HARDCORE:
        ConvertIntToDecimalStringN(gStringVar1, min(value >> 16, 8), STR_CONV_MODE_LEFT_ALIGN, 1);
        ConvertIntToDecimalStringN(gStringVar2, extra & 0xFF, STR_CONV_MODE_LEFT_ALIGN, 3);
        if ((value >> 16) == 9)
            StringExpandPlaceholders(dest, COMPOUND_STRING("CHAMPION, {STR_VAR_2} lost"));
        else if ((extra >> 8) == HARDCORE_FAILED)
            StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} badges, over"));
        else
            StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} badges, {STR_VAR_2} lost"));
        break;
    case WORLD_BOARD_FACTORY:
    case WORLD_BOARD_ARENA:
    case WORLD_BOARD_PYRAMID:
        FrontierOnline_FormatScore(dest, board, value, extra);
        break;
    case WORLD_BOARD_TREASURE:
        ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringExpandPlaceholders(dest, COMPOUND_STRING("{STR_VAR_1} found"));
        break;
    case WORLD_BOARD_DUNGEON:
        ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(dest, COMPOUND_STRING("B{STR_VAR_1}F"));
        break;
    }
}

static void GetBoardLine(u16 i, u8 *dest)
{
    const struct WorldBoardRow *row = &gWorldMailbox.rows[i];
    u8 name[NETPLAY_NAME_LENGTH];
    u8 *end;

    if (!IsOnline())
    {
        StringCopy(dest, COMPOUND_STRING("JOIN ONLINE on the title screen"));
        return;
    }
    memcpy(name, row->name, NETPLAY_NAME_LENGTH);
    name[NETPLAY_NAME_LENGTH - 1] = EOS;
    ConvertIntToDecimalStringN(dest, i + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringAppend(dest, COMPOUND_STRING(". "));
    end = StringAppend(dest, name);
    end = StringAppend(dest, COMPOUND_STRING("{CLEAR_TO 72}"));
    FormatBoardValue(end, sBoardShown, row->value, row->extra);
}

static const u8 *const sBoardTitles[] =
{
    [WORLD_BOARD_FISH]     = COMPOUND_STRING("BIGGEST FISH"),
    [WORLD_BOARD_BERRIES]  = COMPOUND_STRING("BERRY PICKERS"),
    [WORLD_BOARD_HARDCORE] = COMPOUND_STRING("HARDCORE LADDER"),
    [WORLD_BOARD_HUNT]     = COMPOUND_STRING("SAFARI HUNT"),
    [WORLD_BOARD_DUNGEON]  = COMPOUND_STRING("DEEPEST DIVERS"),
    [WORLD_BOARD_TREASURE] = COMPOUND_STRING("TREASURE HUNTERS"),
    [WORLD_BOARD_FACTORY]  = COMPOUND_STRING("BATTLE FACTORY"),
    [WORLD_BOARD_ARENA]    = COMPOUND_STRING("BATTLE ARENA"),
    [WORLD_BOARD_PYRAMID]  = COMPOUND_STRING("BATTLE PYRAMID"),
    [WORLD_BOARD_QUIZ]     = COMPOUND_STRING("QUIZ CHAMPS"),
    [WORLD_BOARD_COOKING]  = COMPOUND_STRING("COOK-OFF CHAMPS"),
    [WORLD_BOARD_BUG_CONTEST] = COMPOUND_STRING("BUG CONTEST"),
};

static void Task_WaitBoard(u8 taskId)
{
    if (!WorldBoard_IsOpen())
    {
        DestroyTask(taskId);
        return;
    }
    if (gWorldMailbox.boardSeq != sBoardSeq && gWorldMailbox.boardId == sBoardShown)
    {
        sBoardSeq = gWorldMailbox.boardSeq;
        WorldBoard_Refresh(gWorldMailbox.boardCount);
    }
    FlushRequests();
}

// Script: VAR_0x8004 = WORLD_BOARD_*. Opens the board and asks the server
// for it; the rows appear when they arrive. waitstate.
void WorldOnline_ShowBoard(void)
{
    u8 board = gSpecialVar_0x8004;

    InitMailbox();
    sBoardShown = board;
    sBoardSeq = gWorldMailbox.boardSeq;
    WorldBoard_Show(sBoardTitles[board], IsOnline() ? 0 : 1, GetBoardLine, NULL);
    if (IsOnline())
    {
        Request(WORLD_REQ_BOARD, board, board == WORLD_BOARD_HUNT ? gWorldMailbox.huntId : 0, 0, 0);
        CreateTask(Task_WaitBoard, 81);
    }
}
