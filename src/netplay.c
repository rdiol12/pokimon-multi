#include "global.h"
#include "netplay.h"
#include "battle.h"
#include "battle_setup.h"
#include "cable_club.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "money.h"
#include "new_game.h"
#include "play_time.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "trade.h"
#include "constants/battle.h"
#include "constants/cable_club.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/species.h"
#include "constants/songs.h"
#include "constants/trainers.h"

// Online multiplayer: shares the player's position with the netplay bridge,
// shows other players on the overworld, handles battle/trade invitations, and
// replaces the serial port as the link transport while a session is running.
// See docs/netplay.md for the mailbox layout and the bridge/server protocol.

#define REQUEST_QUEUE_SIZE 4
#define PENDING_CMDS_SIZE  64

#define RESPONSE_TIMEOUT  (60 * 60)
#define START_TIMEOUT     (60 * 15)
#define LINKUP_TIMEOUT    (60 * 20)
#define PEER_LEFT_GRACE_FRAMES (60 * 3)

enum
{
    RESPONSE_NONE,
    RESPONSE_STARTED,
    RESPONSE_DECLINED,
};

struct NetplaySession
{
    u16 id;
    u16 peerId;
    u8 kind;
    u8 role;
    bool8 active;
    bool8 inActivity; // handed off to the battle or trade screen
    bool8 peerLeft;
    u8 peerLeftTimer;
};

struct NetplayInvite
{
    u16 sessionId;
    u16 peerId;
    u8 kind;
    bool8 pending;
    bool8 showing;
};

struct NetplayLinkState
{
    u16 pending[PENDING_CMDS_SIZE][CMD_LENGTH]; // our sent cmds still waiting for the peer's matching cmd
    u8 pendingHead;
    u8 pendingCount;
    bool8 handshakeSent;
};

EWRAM_DATA struct NetplayMailbox gNetplayMailbox = {0};

static EWRAM_DATA struct NetplaySession sSession = {0};
static EWRAM_DATA struct NetplayInvite sInvite = {0};
static EWRAM_DATA struct NetplayLinkState sLink = {0};
static EWRAM_DATA struct NetplayRequest sRequestQueue[REQUEST_QUEUE_SIZE] = {0};
static EWRAM_DATA u8 sRequestCount = 0;
static EWRAM_DATA u16 sOutgoingSessionId = 0; // our own challenge waiting for an answer
static EWRAM_DATA u8 sResponse = 0;
static EWRAM_DATA u16 sAvatarIds[NETPLAY_MAX_REMOTES] = {0};
static EWRAM_DATA bool8 sInGame = FALSE;

extern const u8 Netplay_EventScript_TalkToPlayer[];
extern const u8 Netplay_EventScript_Invite[];

static void Task_WaitForResponse(u8 taskId);
static void Task_WaitForStart(u8 taskId);
static void Task_Linkup(u8 taskId);

void Netplay_Init(void)
{
    static const char magic[8] = {'P', 'K', 'M', 'N', 'N', 'E', 'T', '1'};

    CpuFill32(0, &gNetplayMailbox, sizeof(gNetplayMailbox));
    memcpy(gNetplayMailbox.magic, magic, sizeof(magic));
    gNetplayMailbox.version = NETPLAY_MAILBOX_VERSION;
    gNetplayMailbox.name[0] = EOS; // the bridge waits for a name before connecting
    sSession = (struct NetplaySession){0};
    sInvite = (struct NetplayInvite){0};
    sLink = (struct NetplayLinkState){0};
    sRequestCount = 0;
    sOutgoingSessionId = 0;
    sResponse = RESPONSE_NONE;
    sInGame = FALSE;
}

static bool32 IsOnline(void)
{
    return gNetplayMailbox.bridgeState == NETPLAY_ONLINE;
}

// Requests ---------------------------------------------------------------

static void QueueRequest(u8 type, u16 targetId, u16 sessionId, u16 arg)
{
    struct NetplayRequest *req;

    if (sRequestCount >= REQUEST_QUEUE_SIZE)
        return;
    req = &sRequestQueue[sRequestCount++];
    req->type = type;
    req->targetId = targetId;
    req->sessionId = sessionId;
    req->arg = arg;
}

// The bridge looks at the request once per frame, so publish at most one per frame.
static void FlushRequest(void)
{
    struct NetplayRequest *out = &gNetplayMailbox.request;
    u32 i;

    if (sRequestCount == 0)
        return;
    out->type = sRequestQueue[0].type;
    out->targetId = sRequestQueue[0].targetId;
    out->sessionId = sRequestQueue[0].sessionId;
    out->arg = sRequestQueue[0].arg;
    out->seq++;
    for (i = 1; i < sRequestCount; i++)
        sRequestQueue[i - 1] = sRequestQueue[i];
    sRequestCount--;
}

// Events -----------------------------------------------------------------

static void HandleEvent(const struct NetplayEvent *ev)
{
    switch (ev->type)
    {
    case NETPLAY_EV_INVITE:
        if (sSession.active || sInvite.pending || sOutgoingSessionId != 0 || !sInGame)
        {
            QueueRequest(NETPLAY_REQ_DECLINE, 0, ev->sessionId, 0);
            break;
        }
        sInvite.sessionId = ev->sessionId;
        sInvite.peerId = ev->peerId;
        sInvite.kind = ev->kind;
        sInvite.pending = TRUE;
        sInvite.showing = FALSE;
        break;
    case NETPLAY_EV_PENDING:
        sOutgoingSessionId = ev->sessionId;
        break;
    case NETPLAY_EV_START:
        sSession.id = ev->sessionId;
        sSession.peerId = ev->peerId;
        sSession.kind = ev->kind;
        sSession.role = ev->role;
        sSession.active = TRUE;
        sSession.inActivity = FALSE;
        sInvite.pending = FALSE;
        sOutgoingSessionId = 0;
        sResponse = RESPONSE_STARTED;
        break;
    case NETPLAY_EV_DECLINED:
        if (ev->sessionId == sOutgoingSessionId)
        {
            sOutgoingSessionId = 0;
            sResponse = RESPONSE_DECLINED;
        }
        break;
    case NETPLAY_EV_ENDED:
        if (sInvite.pending && ev->sessionId == sInvite.sessionId)
        {
            sInvite.pending = FALSE;
            sResponse = RESPONSE_DECLINED;
        }
        if (ev->sessionId == sOutgoingSessionId)
        {
            sOutgoingSessionId = 0;
            sResponse = RESPONSE_DECLINED;
        }
        if (sSession.active && ev->sessionId == sSession.id)
        {
            if (sSession.inActivity)
            {
                // The peer may have just finished the battle or trade and
                // closed its end first; its last packets are still on the
                // way. Give our link time to finish before calling it lost.
                sSession.peerLeft = TRUE;
                sSession.peerLeftTimer = PEER_LEFT_GRACE_FRAMES;
            }
            else
            {
                // The peer left. A running link stalls, so report it as a link error.
                sSession.active = FALSE;
                if (gLink.state == LINK_STATE_CONN_ESTABLISHED || gLink.state == LINK_STATE_HANDSHAKE)
                    gLink.hardwareError = TRUE;
            }
        }
        break;
    }
}

static void ReceiveEvent(void)
{
    if (gNetplayMailbox.event.seq == gNetplayMailbox.eventAck)
        return;
    HandleEvent(&gNetplayMailbox.event);
    gNetplayMailbox.eventAck = gNetplayMailbox.event.seq;
}

// Called once per frame from the main loop.
void Netplay_Update(void)
{
    if (!IsOnline())
    {
        // Lost the server: nothing we were waiting for will arrive.
        if (sOutgoingSessionId != 0 || sInvite.pending)
            sResponse = RESPONSE_DECLINED;
        sOutgoingSessionId = 0;
        sInvite.pending = FALSE;
        sRequestCount = 0;
        if (sSession.active && !sSession.inActivity)
            sSession.active = FALSE;
    }
    ReceiveEvent();
    FlushRequest();

    if (sSession.peerLeft && gReceivedRemoteLinkPlayers && sSession.peerLeftTimer != 0
     && --sSession.peerLeftTimer == 0)
        gLink.hardwareError = TRUE;
}

// Overworld --------------------------------------------------------------

static void PublishLocalPlayer(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (!sInGame)
    {
        StringCopyN(gNetplayMailbox.name, gSaveBlock2Ptr->playerName, NETPLAY_NAME_LENGTH);
        gNetplayMailbox.name[NETPLAY_NAME_LENGTH - 1] = EOS;
        gNetplayMailbox.trainerId = gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8);
        sInGame = TRUE;
    }
    gNetplayMailbox.mapGroup = gSaveBlock1Ptr->location.mapGroup;
    gNetplayMailbox.mapNum = gSaveBlock1Ptr->location.mapNum;
    gNetplayMailbox.x = player->currentCoords.x - MAP_OFFSET;
    gNetplayMailbox.y = player->currentCoords.y - MAP_OFFSET;
    gNetplayMailbox.facing = player->facingDirection;
    gNetplayMailbox.elevation = player->currentElevation;
    gNetplayMailbox.graphicsId = player->graphicsId;
}

static bool32 IsRemoteHere(const struct NetplayRemotePlayer *remote)
{
    return remote->id != 0
        && IsOnline()
        && remote->mapGroup == gSaveBlock1Ptr->location.mapGroup
        && remote->mapNum == gSaveBlock1Ptr->location.mapNum;
}

static void SyncAvatar(u32 slot)
{
    const struct NetplayRemotePlayer *remote = &gNetplayMailbox.remotes[slot];
    u8 localId = LOCALID_NETPLAY_PLAYER_START + slot;
    u8 objId = GetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    struct ObjectEvent *obj;
    s16 x, y, dx, dy;

    if (!IsRemoteHere(remote))
    {
        if (objId != OBJECT_EVENTS_COUNT)
            RemoveObjectEvent(&gObjectEvents[objId]);
        sAvatarIds[slot] = 0;
        return;
    }

    x = remote->x + MAP_OFFSET;
    y = remote->y + MAP_OFFSET;
    if (objId != OBJECT_EVENTS_COUNT
     && (sAvatarIds[slot] != remote->id || gObjectEvents[objId].graphicsId != remote->graphicsId))
    {
        RemoveObjectEvent(&gObjectEvents[objId]);
        objId = OBJECT_EVENTS_COUNT;
    }
    if (objId == OBJECT_EVENTS_COUNT)
    {
        objId = SpawnSpecialObjectEventParameterized(remote->graphicsId, MOVEMENT_TYPE_NONE, localId, x, y, remote->elevation);
        if (objId == OBJECT_EVENTS_COUNT)
            return; // no free object event slot
        sAvatarIds[slot] = remote->id;
        ObjectEventTurn(&gObjectEvents[objId], remote->facing);
        return;
    }

    obj = &gObjectEvents[objId];
    if (ObjectEventIsHeldMovementActive(obj) && !ObjectEventClearHeldMovementIfFinished(obj))
        return; // still walking to the previous tile

    dx = x - obj->currentCoords.x;
    dy = y - obj->currentCoords.y;
    if (dx == 0 && dy == 0)
    {
        if (obj->facingDirection != remote->facing)
            ObjectEventTurn(obj, remote->facing);
    }
    else if (abs(dx) + abs(dy) == 1)
    {
        enum Direction dir = dx > 0 ? DIR_EAST : dx < 0 ? DIR_WEST : dy > 0 ? DIR_SOUTH : DIR_NORTH;
        obj->currentElevation = remote->elevation;
        ObjectEventSetHeldMovement(obj, GetWalkNormalMovementAction(dir));
    }
    else
    {
        MoveObjectEventToMapCoords(obj, x, y);
        obj->currentElevation = remote->elevation;
        ObjectEventTurn(obj, remote->facing);
    }
}

// Starts the invitation prompt when the player is free. Returns TRUE if a script was started.
static bool32 TryStartInviteScript(void)
{
    if (!sInvite.pending || sInvite.showing || ArePlayerFieldControlsLocked() || !IsPlayerStandingStill())
        return FALSE;
    sInvite.showing = TRUE;
    ScriptContext_SetupScript(Netplay_EventScript_Invite);
    return TRUE;
}

// Called every overworld frame from DoCB1_Overworld. Returns TRUE if it started a script.
bool32 Netplay_OverworldUpdate(void)
{
    u32 i;

    PublishLocalPlayer();
    for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
        SyncAvatar(i);

    // Back from the battle or trade once the link has finished: the session is
    // over. The save after a link battle can leave the link open but idle.
    if (sSession.active && sSession.inActivity && !gReceivedRemoteLinkPlayers)
    {
        CloseLink();
        Netplay_EndSession();
    }

    return TryStartInviteScript();
}

bool32 Netplay_IsNetplayObjectEvent(const struct ObjectEvent *objectEvent)
{
    return objectEvent->localId >= LOCALID_NETPLAY_PLAYER_START
        && objectEvent->localId <= LOCALID_NETPLAY_PLAYER_END;
}

const u8 *Netplay_GetTalkScript(u8 objectEventId)
{
    if (!Netplay_IsNetplayObjectEvent(&gObjectEvents[objectEventId]))
        return NULL;
    return Netplay_EventScript_TalkToPlayer;
}

static const struct NetplayRemotePlayer *FindRemote(u16 id)
{
    u32 i;

    for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
    {
        if (gNetplayMailbox.remotes[i].id == id && id != 0)
            return &gNetplayMailbox.remotes[i];
    }
    return NULL;
}

static void BufferRemoteName(u8 *dest, u16 id)
{
    const struct NetplayRemotePlayer *remote = FindRemote(id);

    if (remote == NULL)
    {
        dest[0] = EOS;
        return;
    }
    StringCopyN(dest, remote->name, NETPLAY_NAME_LENGTH);
    dest[NETPLAY_NAME_LENGTH - 1] = EOS;
}

// Sessions ---------------------------------------------------------------

bool32 Netplay_InSession(void)
{
    return sSession.active;
}

void Netplay_EndSession(void)
{
    if (sSession.id != 0 && !sSession.peerLeft)
        QueueRequest(NETPLAY_REQ_LEAVE, 0, sSession.id, 0);
    sSession = (struct NetplaySession){0};
    sLink = (struct NetplayLinkState){0};
}

void Netplay_OnBattleEnd(u8 battleOutcome)
{
    u16 result;

    if (!sSession.active || sSession.kind != NETPLAY_KIND_BATTLE)
        return;
    // Running from a link battle sets B_OUTCOME_LINK_BATTLE_RAN on top of won/lost.
    switch (battleOutcome & ~B_OUTCOME_LINK_BATTLE_RAN)
    {
    case B_OUTCOME_WON:
        result = NETPLAY_RESULT_WON;
        break;
    case B_OUTCOME_LOST:
        result = NETPLAY_RESULT_LOST;
        break;
    case B_OUTCOME_DREW:
        result = NETPLAY_RESULT_DRAW;
        break;
    default:
        return;
    }
    QueueRequest(NETPLAY_REQ_RESULT, 0, sSession.id, result);
}

// Specials ---------------------------------------------------------------

// Talking to another player. VAR_RESULT: TRUE if that player is still here.
// Their name goes in gStringVar1 and their id in VAR_0x8007.
void Netplay_GetTalkedPlayer(void)
{
    u32 slot = gSpecialVar_LastTalked - LOCALID_NETPLAY_PLAYER_START;
    const struct NetplayRemotePlayer *remote;

    gSpecialVar_Result = FALSE;
    if (slot >= NETPLAY_MAX_REMOTES || sAvatarIds[slot] == 0)
        return;
    remote = FindRemote(sAvatarIds[slot]);
    if (remote == NULL || !IsRemoteHere(remote))
        return;
    gSpecialVar_0x8007 = remote->id;
    BufferRemoteName(gStringVar1, remote->id);
    gSpecialVar_Result = TRUE;
}

// VAR_0x8004: NETPLAY_KIND_*, VAR_0x8007: target player id.
void Netplay_SendInvite(void)
{
    u8 type;

    switch (gSpecialVar_0x8004)
    {
    case NETPLAY_KIND_BATTLE:
        type = NETPLAY_REQ_CHALLENGE;
        break;
    case NETPLAY_KIND_DEAL:
        type = NETPLAY_REQ_DEAL;
        break;
    default:
        type = NETPLAY_REQ_TRADE;
        break;
    }
    sResponse = RESPONSE_NONE;
    sOutgoingSessionId = 0;
    QueueRequest(type, gSpecialVar_0x8007, 0, 0);
}

#define tTimer data[0]

// Waits for the other player to answer our invite. VAR_RESULT: NETPLAY_RESPONSE_*.
void Netplay_WaitForResponse(void)
{
    u8 taskId = CreateTask(Task_WaitForResponse, 80);
    gTasks[taskId].tTimer = 0;
}

static void Task_WaitForResponse(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (sResponse == RESPONSE_STARTED)
        gSpecialVar_Result = NETPLAY_RESPONSE_ACCEPTED;
    else if (sResponse == RESPONSE_DECLINED || ++tTimer > RESPONSE_TIMEOUT)
        gSpecialVar_Result = NETPLAY_RESPONSE_DECLINED;
    else if (JOY_NEW(B_BUTTON))
        gSpecialVar_Result = NETPLAY_RESPONSE_CANCELED;
    else
        return;

    if (gSpecialVar_Result != NETPLAY_RESPONSE_ACCEPTED)
    {
        if (sOutgoingSessionId != 0)
            QueueRequest(NETPLAY_REQ_LEAVE, 0, sOutgoingSessionId, 0);
        sOutgoingSessionId = 0;
    }
    sResponse = RESPONSE_NONE;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Shows who invited us. gStringVar1: their name. VAR_RESULT: NETPLAY_KIND_*.
void Netplay_BufferInvite(void)
{
    BufferRemoteName(gStringVar1, sInvite.peerId);
    if (gStringVar1[0] == EOS)
        StringCopy(gStringVar1, COMPOUND_STRING("Someone"));
    gSpecialVar_Result = sInvite.pending ? sInvite.kind : NETPLAY_KIND_NONE;
    gSpecialVar_0x8004 = gSpecialVar_Result;
}

// VAR_RESULT from the yes/no box: accept or decline the pending invite.
void Netplay_AnswerInvite(void)
{
    if (sInvite.pending)
    {
        sResponse = RESPONSE_NONE;
        if (gSpecialVar_Result == TRUE) // YES
        {
            QueueRequest(NETPLAY_REQ_ACCEPT, 0, sInvite.sessionId, 0);
        }
        else
        {
            QueueRequest(NETPLAY_REQ_DECLINE, 0, sInvite.sessionId, 0);
            sInvite.pending = FALSE;
        }
    }
    sInvite.showing = FALSE;
}

// After accepting: waits for the session to start. VAR_RESULT: 1 started, 0 not.
void Netplay_WaitForStart(void)
{
    u8 taskId = CreateTask(Task_WaitForStart, 80);
    gTasks[taskId].tTimer = 0;
}

static void Task_WaitForStart(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (sResponse == RESPONSE_STARTED)
        gSpecialVar_Result = 1;
    else if (sResponse == RESPONSE_DECLINED || ++tTimer > START_TIMEOUT)
        gSpecialVar_Result = 0;
    else
        return;

    sInvite.pending = FALSE;
    sResponse = RESPONSE_NONE;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

#undef tTimer

// Linkup -----------------------------------------------------------------

#define tState data[0]
#define tTimer data[1]

// Opens the link with the session's peer and starts the battle or trade.
// Only returns to the script on failure, with VAR_RESULT = FALSE.
void Netplay_StartActivity(void)
{
    u8 taskId = CreateTask(Task_Linkup, 80);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tTimer = 0;
}

static void LinkupFailed(u8 taskId)
{
    CloseLink();
    Netplay_EndSession();
    gSpecialVar_Result = FALSE;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Linkup(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tState < 3 && (!sSession.active || HasLinkErrorOccurred() || ++tTimer > LINKUP_TIMEOUT))
    {
        LinkupFailed(taskId);
        return;
    }

    switch (tState)
    {
    case 0:
        gLinkType = sSession.kind == NETPLAY_KIND_BATTLE ? LINKTYPE_SINGLE_BATTLE : LINKTYPE_TRADE_SETUP;
        gBattleTypeFlags = 0;
        OpenLinkTimed();
        ResetLinkPlayerCount();
        ResetLinkPlayers();
        tState++;
        break;
    case 1:
        switch (GetLinkPlayerDataExchangeStatusTimed(2, 2))
        {
        case EXCHANGE_COMPLETE:
            gFieldLinkPlayerCount = GetLinkPlayerCount_2();
            gLocalLinkPlayerId = GetMultiplayerId();
            SaveLinkPlayers(gFieldLinkPlayerCount);
            tState++;
            break;
        case EXCHANGE_DIFF_SELECTIONS:
        case EXCHANGE_WRONG_NUM_PLAYERS:
        case EXCHANGE_PLAYER_NOT_READY:
        case EXCHANGE_PARTNER_NOT_READY:
            LinkupFailed(taskId);
            break;
        }
        break;
    case 2:
        // Both games are linked; leave the script behind and start the activity.
        ScriptContext_Init();
        UnlockPlayerFieldControls();
        FadeScreen(FADE_TO_BLACK, 0);
        if (sSession.kind == NETPLAY_KIND_BATTLE)
        {
            gSpecialVar_0x8004 = USING_SINGLE_BATTLE;
            gLinkType = LINKTYPE_BATTLE;
            HealPlayerParty();
            SavePlayerParty();
            LoadPlayerBag();
            ClearLinkCallback_2();
        }
        tTimer = 0;
        tState++;
        break;
    case 3:
        if (!gPaletteFade.active)
        {
            if (sSession.kind == NETPLAY_KIND_TRADE)
            {
                CleanupOverworldWindowsAndTilemaps();
                DestroyTask(taskId);
                sSession.inActivity = TRUE;
                SetMainCallback2(CB2_StartCreateTradeMenu);
            }
            else if (++tTimer > 20)
            {
                // Same as a cable club battle: close the link, the battle reopens it.
                SetCloseLinkCallback();
                tState++;
            }
        }
        break;
    case 4:
        if (!gReceivedRemoteLinkPlayers)
        {
            PlayMapChosenOrBattleBGM(MUS_VS_TRAINER);
            gBattleTypeFlags = BATTLE_TYPE_LINK | BATTLE_TYPE_TRAINER;
            CleanupOverworldWindowsAndTilemaps();
            TRAINER_BATTLE_PARAM.opponentA = TRAINER_LINK_OPPONENT;
            sSession.inActivity = TRUE;
            SetMainCallback2(CB2_InitBattle);
            gMain.savedCallback = CB2_ReturnFromCableClubBattle;
            DestroyTask(taskId);
        }
        break;
    }
}

#undef tState
#undef tTimer

// Test boot --------------------------------------------------------------
//
// For automated tests (tools/netplay_bridge/e2e): a test driver writes
// NETPLAY_TEST_BOOT_MAGIC | n into the mailbox's first padding byte on the
// title screen, and "New Game" then skips the intro and drops player TESTn
// with a small party in front of the Oldale Town Pokémon Center.

#define NETPLAY_TEST_BOOT_MAGIC 0xB0

static void CB2_TestNewGame(void)
{
    u8 n = gNetplayMailbox.filler[0] & 0xF;
    u8 name[] = _("TEST0");

    NewGameInitData();
    ResetInitialPlayerAvatarState();
    PlayTimeCounter_Start();
    ScriptContext_Init();
    UnlockPlayerFieldControls();

    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_FAST;
    name[4] += n;
    StringCopy(gSaveBlock2Ptr->playerName, name);
    gSaveBlock2Ptr->playerTrainerId[0] = n;
    gSaveBlock2Ptr->playerTrainerId[1] = 0x10;
    FlagSet(FLAG_SYS_POKEMON_GET);
    ScriptGiveMon(n & 1 ? SPECIES_MUDKIP : SPECIES_TREECKO, 30, ITEM_NONE);
    ScriptGiveMon(n & 1 ? SPECIES_POOCHYENA : SPECIES_ZIGZAGOON, 30, ITEM_NONE);

    SetWarpDestination(MAP_GROUP(MAP_OLDALE_TOWN), MAP_NUM(MAP_OLDALE_TOWN), WARP_ID_NONE, 4 + 2 * n, 17);
    WarpIntoMap();
    gFieldCallback = FieldCB_WarpExitFadeFromBlack;
    SetMainCallback2(CB2_LoadMap);
}

bool32 Netplay_TryTestBoot(u8 taskId)
{
    if ((gNetplayMailbox.filler[0] & 0xF0) != NETPLAY_TEST_BOOT_MAGIC)
        return FALSE;
    DestroyTask(taskId);
    SetMainCallback2(CB2_TestNewGame);
    return TRUE;
}

// Link transport ---------------------------------------------------------
//
// One frame of the network link stands in for one serial transfer: each game
// sends exactly one packet per frame (its next queued cmd, or zeros), and the
// k-th packet of each side is delivered together as one recvQueue entry, just
// like the serial port delivers every player's cmd for a transfer at once.

bool32 Netplay_IsLinkActive(void)
{
    // Deals talk over the rings directly; only battles and trades open a link.
    return sSession.active && sSession.kind != NETPLAY_KIND_DEAL;
}

u8 Netplay_GetLinkRole(void)
{
    return sSession.role;
}

void Netplay_LinkReset(void)
{
    sLink = (struct NetplayLinkState){0};
}

static bool32 SendPacket(u8 kind, const u16 *cmd)
{
    struct NetplayRing *ring = &gNetplayMailbox.tx;
    u8 next = (ring->head + 1) % NETPLAY_RING_SIZE;
    struct NetplayPacket *packet;
    u32 i;

    if (next == ring->tail)
        return FALSE;
    packet = &ring->packets[ring->head];
    packet->kind = kind;
    packet->filler1 = 0;
    packet->filler2 = 0;
    for (i = 0; i < CMD_LENGTH; i++)
        packet->cmd[i] = cmd != NULL ? cmd[i] : 0;
    ring->head = next;
    return TRUE;
}

static struct NetplayPacket *PeekPacket(void)
{
    struct NetplayRing *ring = &gNetplayMailbox.rx;

    if (ring->head == ring->tail)
        return NULL;
    return &ring->packets[ring->tail];
}

static void PopPacket(void)
{
    struct NetplayRing *ring = &gNetplayMailbox.rx;
    ring->tail = (ring->tail + 1) % NETPLAY_RING_SIZE;
}

// Called from LinkMain1 while the link is in LINK_STATE_HANDSHAKE.
// Packets from before the handshake belong to an earlier link and are dropped.
bool32 Netplay_LinkHandshake(void)
{
    struct NetplayPacket *packet;
    bool32 isMaster = (sSession.role == 0);

    if (isMaster && !sLink.handshakeSent)
        sLink.handshakeSent = SendPacket(NETPLAY_PACKET_HANDSHAKE, NULL);

    while ((packet = PeekPacket()) != NULL)
    {
        u8 kind = packet->kind;

        if (isMaster && kind == NETPLAY_PACKET_HANDSHAKE_ACK)
        {
            PopPacket();
            return TRUE;
        }
        if (!isMaster && kind == NETPLAY_PACKET_HANDSHAKE)
        {
            if (!SendPacket(NETPLAY_PACKET_HANDSHAKE_ACK, NULL))
                return FALSE; // try again next frame
            PopPacket();
            return TRUE;
        }
        PopPacket();
    }
    return FALSE;
}

static void EnqueueRecvEntry(const u16 *localCmd, const u16 *remoteCmd)
{
    struct RecvQueue *queue = &gLink.recvQueue;
    u8 localId = sSession.role;
    u8 remoteId = sSession.role ^ 1;
    u16 nonzero = 0;
    u32 i, index;

    for (i = 0; i < CMD_LENGTH; i++)
        nonzero |= localCmd[i] | remoteCmd[i];
    if (nonzero == 0)
        return;

    index = queue->pos + queue->count;
    if (index >= QUEUE_CAPACITY)
        index -= QUEUE_CAPACITY;
    for (i = 0; i < CMD_LENGTH; i++)
    {
        queue->data[localId][i][index] = localCmd[i];
        queue->data[remoteId][i][index] = remoteCmd[i];
    }
    queue->count++;
}

// Called from LinkMain1 once per frame while the link is established.
void Netplay_LinkTransfer(void)
{
    struct SendQueue *sendQueue = &gLink.sendQueue;
    struct NetplayPacket *packet;
    u16 cmd[CMD_LENGTH];
    u32 i;

    // Send: our next cmd (or nothing) for this frame.
    if (sLink.pendingCount < PENDING_CMDS_SIZE)
    {
        for (i = 0; i < CMD_LENGTH; i++)
            cmd[i] = sendQueue->count != 0 ? sendQueue->data[i][sendQueue->pos] : 0;
        if (SendPacket(NETPLAY_PACKET_DATA, cmd))
        {
            u32 slot = (sLink.pendingHead + sLink.pendingCount) % PENDING_CMDS_SIZE;
            for (i = 0; i < CMD_LENGTH; i++)
                sLink.pending[slot][i] = cmd[i];
            sLink.pendingCount++;
            if (sendQueue->count != 0)
            {
                sendQueue->count--;
                if (++sendQueue->pos >= QUEUE_CAPACITY)
                    sendQueue->pos = 0;
            }
        }
    }

    // Receive: pair the peer's packets with ours in order.
    while ((packet = PeekPacket()) != NULL)
    {
        if (packet->kind != NETPLAY_PACKET_DATA)
        {
            PopPacket(); // a late handshake packet
            continue;
        }
        if (sLink.pendingCount == 0 || gLink.recvQueue.count >= QUEUE_CAPACITY)
            break;
        EnqueueRecvEntry(sLink.pending[sLink.pendingHead], packet->cmd);
        PopPacket();
        sLink.pendingHead = (sLink.pendingHead + 1) % PENDING_CMDS_SIZE;
        sLink.pendingCount--;
    }
}

// Deals ------------------------------------------------------------------
//
// A deal swaps an offer of a Pokémon and/or money for the other player's
// offer. It doesn't open a link: each game sends its offer over the packet
// rings in chunks, both players see both offers, and each game applies the
// deal once both have said yes. See Netplay_EventScript_Deal.

#define DEAL_ANSWER_NONE 0
#define DEAL_ANSWER_YES  1
#define DEAL_ANSWER_NO   2

#define DEAL_TIMEOUT (60 * 60 * 3)

struct NetplayDealOffer
{
    u32 money;
    u8 hasMon;
    u8 filler[3];
    struct Pokemon mon;
};

#define DEAL_CHUNK_SIZE  (CMD_LENGTH * 2)
#define DEAL_CHUNKS      ((sizeof(struct NetplayDealOffer) + DEAL_CHUNK_SIZE - 1) / DEAL_CHUNK_SIZE)
#define DEAL_BUFFER_SIZE (DEAL_CHUNKS * DEAL_CHUNK_SIZE)

STATIC_ASSERT(DEAL_CHUNKS <= 32, NetplayDealChunksFitInMask);

struct NetplayDeal
{
    union {
        struct NetplayDealOffer offer;
        u8 bytes[DEAL_BUFFER_SIZE];
    } mine, theirs;
    u32 chunksReceived; // bit mask
    u8 chunksSent;
    u8 mySlot;
    u8 theirAnswer;
    bool8 myAnswerSent;
};

static EWRAM_DATA struct NetplayDeal sDeal = {0};

static const u32 sDealMoneyChoices[NETPLAY_DEAL_MONEY_CHOICES] =
{
    0, 100, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000,
};

static void Task_WaitForDealOffer(u8 taskId);
static void Task_WaitForDealAnswer(u8 taskId);

// Reads the deal packets that have arrived. Other packets are dropped.
static void ReceiveDealPackets(void)
{
    struct NetplayPacket *packet;

    while ((packet = PeekPacket()) != NULL)
    {
        if (packet->kind == NETPLAY_PACKET_DEAL_OFFER && packet->filler1 < DEAL_CHUNKS)
        {
            memcpy(&sDeal.theirs.bytes[packet->filler1 * DEAL_CHUNK_SIZE], packet->cmd, DEAL_CHUNK_SIZE);
            sDeal.chunksReceived |= 1u << packet->filler1;
        }
        else if (packet->kind == NETPLAY_PACKET_DEAL_ANSWER)
        {
            sDeal.theirAnswer = packet->cmd[0] ? DEAL_ANSWER_YES : DEAL_ANSWER_NO;
        }
        PopPacket();
    }
}

static bool32 HaveTheirOffer(void)
{
    return sDeal.chunksReceived == (1u << DEAL_CHUNKS) - 1;
}

static void SendDealChunks(void)
{
    while (sDeal.chunksSent < DEAL_CHUNKS)
    {
        struct NetplayRing *ring = &gNetplayMailbox.tx;

        if (!SendPacket(NETPLAY_PACKET_DEAL_OFFER, (const u16 *)&sDeal.mine.bytes[sDeal.chunksSent * DEAL_CHUNK_SIZE]))
            break;
        // SendPacket clears filler1; it carries the chunk index here.
        ring->packets[(ring->head + NETPLAY_RING_SIZE - 1) % NETPLAY_RING_SIZE].filler1 = sDeal.chunksSent;
        sDeal.chunksSent++;
    }
}

static void BufferOfferText(u8 *dest, struct NetplayDealOffer *offer)
{
    u8 *end = dest;

    *end = EOS;
    if (offer->hasMon)
    {
        u16 species = GetMonData(&offer->mon, MON_DATA_SPECIES);
        u8 level = GetMonData(&offer->mon, MON_DATA_LEVEL);

        if (GetMonData(&offer->mon, MON_DATA_IS_EGG))
        {
            end = StringCopy(end, COMPOUND_STRING("an EGG"));
        }
        else
        {
            end = StringCopy(end, GetSpeciesName(species));
            end = StringCopy(end, COMPOUND_STRING(" Lv"));
            end = ConvertIntToDecimalStringN(end, level, STR_CONV_MODE_LEFT_ALIGN, 3);
        }
        if (offer->money != 0)
            end = StringCopy(end, COMPOUND_STRING(" and "));
    }
    if (offer->money != 0)
    {
        end = StringCopy(end, COMPOUND_STRING("¥"));
        end = ConvertIntToDecimalStringN(end, offer->money, STR_CONV_MODE_LEFT_ALIGN, 7);
    }
    if (!offer->hasMon && offer->money == 0)
        StringCopy(end, COMPOUND_STRING("nothing"));
}

// gStringVar1: the other player's name. Menus in between can reuse it, so
// every deal special that shows text sets it again.
static void BufferDealPeerName(void)
{
    BufferRemoteName(gStringVar1, sSession.peerId);
    if (gStringVar1[0] == EOS)
        StringCopy(gStringVar1, COMPOUND_STRING("The other player"));
}

// Both players start here once the session has started.
void Netplay_DealBegin(void)
{
    sDeal = (struct NetplayDeal){0};
    BufferDealPeerName();
}

// VAR_0x8004: party slot to offer (PARTY_SIZE or more for none).
// VAR_0x8005: index into sDealMoneyChoices.
// VAR_RESULT: FALSE if the player doesn't have that much money.
void Netplay_DealSendOffer(void)
{
    u32 money = gSpecialVar_0x8005 < NETPLAY_DEAL_MONEY_CHOICES ? sDealMoneyChoices[gSpecialVar_0x8005] : 0;
    struct NetplayDealOffer *offer = &sDeal.mine.offer;

    BufferDealPeerName();
    gSpecialVar_Result = FALSE;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, money))
        return;
    memset(&sDeal.mine, 0, sizeof(sDeal.mine));
    offer->money = money;
    sDeal.mySlot = gSpecialVar_0x8004;
    if (sDeal.mySlot < gPartiesCount[B_TRAINER_PLAYER])
    {
        offer->hasMon = TRUE;
        offer->mon = gParties[B_TRAINER_PLAYER][sDeal.mySlot];
    }
    sDeal.chunksSent = 0;
    SendDealChunks();
    gSpecialVar_Result = TRUE;
}

#define tTimer data[0]

// Waits for the other player's offer. VAR_RESULT: TRUE once it's here,
// FALSE if they left, it timed out, or we pressed B.
void Netplay_DealWaitForOffer(void)
{
    u8 taskId = CreateTask(Task_WaitForDealOffer, 80);
    gTasks[taskId].tTimer = 0;
}

static void Task_WaitForDealOffer(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    SendDealChunks();
    ReceiveDealPackets();
    if (HaveTheirOffer())
        gSpecialVar_Result = TRUE;
    else if (!sSession.active || ++tTimer > DEAL_TIMEOUT || JOY_NEW(B_BUTTON))
        gSpecialVar_Result = FALSE;
    else
        return;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// gStringVar2: their offer, gStringVar3: ours.
// VAR_RESULT: FALSE if the deal can't happen (nothing offered, or it would
// leave a party empty or overfull).
void Netplay_DealBufferOffers(void)
{
    struct NetplayDealOffer *mine = &sDeal.mine.offer;
    struct NetplayDealOffer *theirs = &sDeal.theirs.offer;
    s32 partyAfter = gPartiesCount[B_TRAINER_PLAYER] - mine->hasMon + theirs->hasMon;

    BufferDealPeerName();
    BufferOfferText(gStringVar2, theirs);
    BufferOfferText(gStringVar3, mine);
    gSpecialVar_Result = (mine->hasMon || mine->money || theirs->hasMon || theirs->money)
                      && partyAfter >= 1 && partyAfter <= PARTY_SIZE;
}

// VAR_RESULT: our answer from the yes/no box.
void Netplay_DealAnswer(void)
{
    u16 cmd[CMD_LENGTH] = {gSpecialVar_Result == TRUE};

    sDeal.myAnswerSent = SendPacket(NETPLAY_PACKET_DEAL_ANSWER, cmd);
}

// Waits for the other player's answer. VAR_RESULT: TRUE if both said yes.
void Netplay_DealWaitForAnswer(void)
{
    u8 taskId = CreateTask(Task_WaitForDealAnswer, 80);
    gTasks[taskId].tTimer = 0;
}

static void Task_WaitForDealAnswer(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    ReceiveDealPackets();
    if (sDeal.theirAnswer != DEAL_ANSWER_NONE)
        gSpecialVar_Result = sDeal.myAnswerSent && sDeal.theirAnswer == DEAL_ANSWER_YES;
    else if (!sSession.active || ++tTimer > DEAL_TIMEOUT)
        gSpecialVar_Result = FALSE;
    else
        return;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

#undef tTimer

// Both said yes: hand over our side and take theirs, then save.
void Netplay_DealApply(void)
{
    const struct NetplayDealOffer *mine = &sDeal.mine.offer;
    const struct NetplayDealOffer *theirs = &sDeal.theirs.offer;

    if (mine->hasMon)
    {
        ZeroMonData(&gParties[B_TRAINER_PLAYER][sDeal.mySlot]);
        CompactPartySlots();
        CalculatePlayerPartyCount();
    }
    if (theirs->hasMon && gPartiesCount[B_TRAINER_PLAYER] < PARTY_SIZE)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gPartiesCount[B_TRAINER_PLAYER]];
        u16 species;

        *mon = theirs->mon;
        mon->mail = MAIL_NONE; // their mail slots aren't ours
        species = GetMonData(mon, MON_DATA_SPECIES);
        if (!GetMonData(mon, MON_DATA_IS_EGG))
        {
            GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_SEEN);
            GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_CAUGHT);
        }
        CalculatePlayerPartyCount();
    }
    RemoveMoney(&gSaveBlock1Ptr->money, mine->money);
    AddMoney(&gSaveBlock1Ptr->money, theirs->money);
    TrySavingData(SAVE_NORMAL);
}

void Netplay_DealEnd(void)
{
    Netplay_EndSession();
}
