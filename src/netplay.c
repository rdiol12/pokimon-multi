#include "global.h"
#include "netplay.h"
#include "battle.h"
#include "battle_setup.h"
#include "config/netplay.h"
#include "cable_club.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "menu.h"
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
#include "strings.h"
#include "task.h"
#include "text.h"
#include "trade.h"
#include "window.h"
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
    memcpy(gNetplayMailbox.serverHost, NETPLAY_SERVER_HOST, min(sizeof(NETPLAY_SERVER_HOST), NETPLAY_HOST_LENGTH - 1));
    gNetplayMailbox.serverPort = NETPLAY_SERVER_PORT;
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

// JOIN ONLINE on the title menu: lets the bridge connect to the server.
void Netplay_JoinOnline(void)
{
    gNetplayMailbox.joinOnline = TRUE;
}

bool32 Netplay_HasJoined(void)
{
    return gNetplayMailbox.joinOnline;
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
    u8 n = gNetplayMailbox.testBoot & 0xF;
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
    if ((gNetplayMailbox.testBoot & 0xF0) != NETPLAY_TEST_BOOT_MAGIC)
        return FALSE;
    Netplay_JoinOnline();
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
// A deal swaps any number of Pokémon and an amount of money between the two
// players, in either direction. Both players edit their offer on the deal
// screen and see the other's offer live; any change clears both Accepts. The
// deal happens once both have accepted the same pair of offers: each game
// then sends a commit and applies the deal when it has the other's commit for
// the same offers. No link is opened; offers and answers go over the packet
// rings (see docs/netplay.md).

#define DEAL_MAX_MONS PARTY_SIZE
#define DEAL_TIMEOUT  (60 * 60 * 10)

struct NetplayDealOffer
{
    u32 money;
    u8 count;
    u8 filler[3];
    struct Pokemon mons[DEAL_MAX_MONS];
};

#define DEAL_CHUNK_SIZE (CMD_LENGTH * 2)
#define DEAL_CHUNKS     ((sizeof(struct NetplayDealOffer) + DEAL_CHUNK_SIZE - 1) / DEAL_CHUNK_SIZE)

union NetplayDealBuffer
{
    struct NetplayDealOffer offer;
    u8 bytes[DEAL_CHUNKS * DEAL_CHUNK_SIZE];
};

enum
{
    DEAL_ACTION_ADD,
    DEAL_ACTION_REMOVE,
    DEAL_ACTION_MONEY,
    DEAL_ACTION_ACCEPT,
    DEAL_ACTION_QUIT,
    DEAL_ACTION_COUNT,
};

enum
{
    DEAL_STATE_MAIN,
    DEAL_STATE_REMOVE,
    DEAL_STATE_MONEY,
    DEAL_STATE_COMMITTED,
};

enum
{
    DEAL_WIN_MINE,
    DEAL_WIN_THEIRS,
    DEAL_WIN_ACTIONS,
    DEAL_WIN_STATUS,
    DEAL_WIN_COUNT,
};

struct NetplayDeal
{
    union NetplayDealBuffer mine;
    union NetplayDealBuffer theirs;
    union NetplayDealBuffer incoming;    // their offer while its chunks arrive
    u8 incomingChunks[DEAL_CHUNKS];
    u8 mySlots[DEAL_MAX_MONS];           // party slot of each Pokémon in our offer
    u16 myRev;
    u16 theirRev;                        // revision of `theirs`; 0 until we have one
    u16 incomingRev;
    u8 chunksSent;                       // of our offer at myRev
    bool8 iAccepted;
    bool8 acceptSent;                    // our current accept state reached the ring
    bool8 peerAccepted;
    u16 peerAcceptRev;                   // their revision when they accepted
    u16 peerAcceptSeen;                  // our revision they accepted
    bool8 peerCommitted;
    u16 peerCommitRev;
    u16 peerCommitSeen;
    bool8 commitSent;
    u8 state;
    u8 cursor;
    u8 removeCursor;
    u8 moneyDigit;
    u32 moneyEdit;
    u8 windowIds[DEAL_WIN_COUNT];
    const u8 *status;
};

static EWRAM_DATA struct NetplayDeal sDeal = {0};

static const struct WindowTemplate sDealWindowTemplates[DEAL_WIN_COUNT] =
{
    [DEAL_WIN_MINE]    = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 1,  .width = 13, .height = 10, .paletteNum = 15, .baseBlock = 0x008},
    [DEAL_WIN_THEIRS]  = {.bg = 0, .tilemapLeft = 16, .tilemapTop = 1,  .width = 13, .height = 10, .paletteNum = 15, .baseBlock = 0x08A},
    [DEAL_WIN_ACTIONS] = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 13, .width = 28, .height = 2,  .paletteNum = 15, .baseBlock = 0x10C},
    [DEAL_WIN_STATUS]  = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 17, .width = 28, .height = 2,  .paletteNum = 15, .baseBlock = 0x144},
};

static const u8 *const sDealActionNames[DEAL_ACTION_COUNT] =
{
    [DEAL_ACTION_ADD]    = COMPOUND_STRING("ADD"),
    [DEAL_ACTION_REMOVE] = COMPOUND_STRING("REMOVE"),
    [DEAL_ACTION_MONEY]  = COMPOUND_STRING("MONEY"),
    [DEAL_ACTION_ACCEPT] = COMPOUND_STRING("ACCEPT"),
    [DEAL_ACTION_QUIT]   = COMPOUND_STRING("QUIT"),
};

static const u8 sDealActionX[DEAL_ACTION_COUNT] = {8, 50, 104, 152, 200};

#define DEAL_LINE_HEIGHT 10
#define DEAL_MONEY_DIGITS 6

static void Task_DealScreen(u8 taskId);

static void BufferDealPeerName(void)
{
    BufferRemoteName(gStringVar1, sSession.peerId);
    if (gStringVar1[0] == EOS)
        StringCopy(gStringVar1, COMPOUND_STRING("The other player"));
}

// Packets --------------------------------------------------------------

static void SendDealChunks(void)
{
    struct NetplayRing *ring = &gNetplayMailbox.tx;

    while (sDeal.chunksSent < DEAL_CHUNKS)
    {
        struct NetplayPacket *packet;

        if (!SendPacket(NETPLAY_PACKET_DEAL_OFFER, (const u16 *)&sDeal.mine.bytes[sDeal.chunksSent * DEAL_CHUNK_SIZE]))
            break;
        // SendPacket clears the header; it carries the chunk index and revision.
        packet = &ring->packets[(ring->head + NETPLAY_RING_SIZE - 1) % NETPLAY_RING_SIZE];
        packet->filler1 = sDeal.chunksSent;
        packet->filler2 = sDeal.myRev;
        sDeal.chunksSent++;
    }
    if (sDeal.chunksSent == DEAL_CHUNKS && !sDeal.acceptSent)
    {
        // Sent after the offer, so the peer never sees an accept for an
        // offer it doesn't have yet.
        u16 cmd[CMD_LENGTH] = {sDeal.iAccepted, sDeal.myRev, sDeal.theirRev};

        sDeal.acceptSent = SendPacket(NETPLAY_PACKET_DEAL_ANSWER, cmd);
    }
}

// Our offer changed: send it again and take back both accepts.
static void OnMyOfferChanged(void)
{
    sDeal.myRev++;
    sDeal.chunksSent = 0;
    sDeal.iAccepted = FALSE;
    sDeal.acceptSent = FALSE;
    sDeal.commitSent = FALSE;
}

static void SetAccepted(bool8 accepted)
{
    sDeal.iAccepted = accepted;
    sDeal.acceptSent = FALSE;
}

// Returns TRUE if their offer or answers changed.
static bool32 ReceiveDealPackets(void)
{
    struct NetplayPacket *packet;
    bool32 changed = FALSE;
    u32 i;

    while ((packet = PeekPacket()) != NULL)
    {
        switch (packet->kind)
        {
        case NETPLAY_PACKET_DEAL_OFFER:
            if (packet->filler1 >= DEAL_CHUNKS)
                break;
            if (packet->filler2 != sDeal.incomingRev)
            {
                sDeal.incomingRev = packet->filler2;
                memset(sDeal.incomingChunks, 0, sizeof(sDeal.incomingChunks));
            }
            memcpy(&sDeal.incoming.bytes[packet->filler1 * DEAL_CHUNK_SIZE], packet->cmd, DEAL_CHUNK_SIZE);
            sDeal.incomingChunks[packet->filler1] = TRUE;
            for (i = 0; i < DEAL_CHUNKS && sDeal.incomingChunks[i]; i++)
                ;
            if (i == DEAL_CHUNKS && sDeal.incomingRev != sDeal.theirRev)
            {
                sDeal.theirs = sDeal.incoming;
                if (sDeal.theirs.offer.count > DEAL_MAX_MONS)
                    sDeal.theirs.offer.count = DEAL_MAX_MONS;
                sDeal.theirRev = sDeal.incomingRev;
                // Their offer changed: our accept was for the old one.
                if (sDeal.iAccepted || sDeal.commitSent)
                {
                    SetAccepted(FALSE);
                    sDeal.commitSent = FALSE;
                }
                changed = TRUE;
            }
            break;
        case NETPLAY_PACKET_DEAL_ANSWER:
            sDeal.peerAccepted = packet->cmd[0];
            sDeal.peerAcceptRev = packet->cmd[1];
            sDeal.peerAcceptSeen = packet->cmd[2];
            changed = TRUE;
            break;
        case NETPLAY_PACKET_DEAL_COMMIT:
            sDeal.peerCommitted = TRUE;
            sDeal.peerCommitRev = packet->cmd[0];
            sDeal.peerCommitSeen = packet->cmd[1];
            break;
        }
        PopPacket();
    }
    return changed;
}

static bool32 PeerAcceptsCurrentOffers(void)
{
    return sDeal.peerAccepted && sDeal.theirRev != 0
        && sDeal.peerAcceptRev == sDeal.theirRev && sDeal.peerAcceptSeen == sDeal.myRev;
}

static bool32 PeerCommittedCurrentOffers(void)
{
    return sDeal.peerCommitted && sDeal.peerCommitRev == sDeal.theirRev && sDeal.peerCommitSeen == sDeal.myRev;
}

// Party size after the deal must stay between 1 and 6.
static bool32 DealFitsParty(void)
{
    s32 after = gPartiesCount[B_TRAINER_PLAYER] - sDeal.mine.offer.count + sDeal.theirs.offer.count;

    return after >= 1 && after <= PARTY_SIZE;
}

// Drawing --------------------------------------------------------------

static u8 *BufferMonLine(u8 *dest, struct Pokemon *mon)
{
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return StringCopy(dest, COMPOUND_STRING("EGG"));
    dest = StringCopy(dest, GetSpeciesName(GetMonData(mon, MON_DATA_SPECIES)));
    dest = StringCopy(dest, COMPOUND_STRING(" Lv"));
    return ConvertIntToDecimalStringN(dest, GetMonData(mon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
}

static void DrawOffer(u8 win, const u8 *title, struct NetplayDealOffer *offer, bool8 accepted, bool8 known, s8 cursor)
{
    u8 windowId = sDeal.windowIds[win];
    u8 *str;
    s32 i;

    // Lines are closer together than the font's box, so they are drawn from
    // the bottom up: each line's empty top would otherwise cut off the
    // line above.
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (!known)
    {
        AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("…"), 8, DEAL_LINE_HEIGHT, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        str = StringCopy(gStringVar4, COMPOUND_STRING("¥"));
        ConvertIntToDecimalStringN(str, offer->money, STR_CONV_MODE_LEFT_ALIGN, 7);
        AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 8, DEAL_LINE_HEIGHT * (DEAL_MAX_MONS + 1), TEXT_SKIP_DRAW, NULL);
        for (i = offer->count - 1; i >= 0; i--)
        {
            BufferMonLine(gStringVar4, &offer->mons[i]);
            AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 8, DEAL_LINE_HEIGHT * (i + 1), TEXT_SKIP_DRAW, NULL);
        }
        if (offer->count == 0)
            AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("No POKéMON"), 8, DEAL_LINE_HEIGHT, TEXT_SKIP_DRAW, NULL);
    }
    if (cursor >= 0)
        AddTextPrinterParameterized(windowId, FONT_SMALL, gText_SelectorArrow3, 0, DEAL_LINE_HEIGHT * (cursor + 1), TEXT_SKIP_DRAW, NULL);
    str = StringCopy(gStringVar4, title);
    if (accepted)
        StringAppend(gStringVar4, COMPOUND_STRING(" - OK!"));
    AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 2, 0, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void DrawDealOffers(void)
{
    u8 *title = gStringVar2;

    DrawOffer(DEAL_WIN_MINE, COMPOUND_STRING("YOUR OFFER"), &sDeal.mine.offer, sDeal.iAccepted, TRUE,
              sDeal.state == DEAL_STATE_REMOVE ? sDeal.removeCursor : -1);
    BufferDealPeerName();
    StringCopy(title, gStringVar1);
    StringAppend(title, COMPOUND_STRING("'S OFFER"));
    DrawOffer(DEAL_WIN_THEIRS, title, &sDeal.theirs.offer, PeerAcceptsCurrentOffers(), sDeal.theirRev != 0, -1);
}

static void DrawDealActions(void)
{
    u8 windowId = sDeal.windowIds[DEAL_WIN_ACTIONS];
    u32 i;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (sDeal.state == DEAL_STATE_MONEY)
    {
        u8 *str = StringCopy(gStringVar4, COMPOUND_STRING("Money: ¥"));
        u32 x;

        ConvertIntToDecimalStringN(str, sDeal.moneyEdit, STR_CONV_MODE_LEADING_ZEROS, DEAL_MONEY_DIGITS);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 8, 1, TEXT_SKIP_DRAW, NULL);
        // Underline the digit being changed.
        x = 8 + GetStringWidth(FONT_NORMAL, COMPOUND_STRING("Money: ¥"), 0) + 6 * (DEAL_MONEY_DIGITS - 1 - sDeal.moneyDigit);
        FillWindowPixelRect(windowId, PIXEL_FILL(2), x, 14, 6, 1);
        AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("{UP_ARROW}{DOWN_ARROW} change  A: OK"), 120, 3, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        for (i = 0; i < DEAL_ACTION_COUNT; i++)
            AddTextPrinterParameterized(windowId, FONT_NORMAL, sDealActionNames[i], sDealActionX[i], 1, TEXT_SKIP_DRAW, NULL);
        if (sDeal.state == DEAL_STATE_MAIN)
            AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_SelectorArrow3, sDealActionX[sDeal.cursor] - 8, 1, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void DrawDealStatus(void)
{
    u8 windowId = sDeal.windowIds[DEAL_WIN_STATUS];
    const u8 *status = sDeal.status;

    if (status == NULL)
    {
        if (sDeal.state == DEAL_STATE_COMMITTED)
            status = COMPOUND_STRING("Both accepted! Making the deal…");
        else if (sDeal.state == DEAL_STATE_REMOVE)
            status = COMPOUND_STRING("Take back which POKéMON? B: back");
        else if (sDeal.theirRev == 0)
            status = COMPOUND_STRING("Waiting for the other offer…");
        else if (sDeal.iAccepted)
            status = COMPOUND_STRING("You accepted. Waiting for them…");
        else if (PeerAcceptsCurrentOffers())
            status = COMPOUND_STRING("They accepted this deal.");
        else
            status = COMPOUND_STRING("Make your offer, then ACCEPT.");
    }
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(windowId, FONT_NORMAL, status, 4, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void DrawDealScreen(void)
{
    DrawDealOffers();
    DrawDealActions();
    DrawDealStatus();
}

static void OpenDealWindows(void)
{
    u32 i;

    for (i = 0; i < DEAL_WIN_COUNT; i++)
    {
        sDeal.windowIds[i] = AddWindow(&sDealWindowTemplates[i]);
        DrawStdWindowFrame(sDeal.windowIds[i], FALSE);
        PutWindowTilemap(sDeal.windowIds[i]);
    }
    DrawDealScreen();
    ScheduleBgCopyTilemapToVram(0);
}

static void CloseDealWindows(void)
{
    u32 i;

    for (i = 0; i < DEAL_WIN_COUNT; i++)
    {
        ClearStdWindowAndFrameToTransparent(sDeal.windowIds[i], FALSE);
        ClearWindowTilemap(sDeal.windowIds[i]);
        RemoveWindow(sDeal.windowIds[i]);
    }
    ScheduleBgCopyTilemapToVram(0);
}

// Specials ---------------------------------------------------------------

// Both players start here once the session has started.
void Netplay_DealBegin(void)
{
    memset(&sDeal, 0, sizeof(sDeal));
    sDeal.myRev = 1;
    BufferDealPeerName();
}

// Shows the deal screen until the player picks ADD, the deal is made, the
// player quits, or the other player leaves. VAR_RESULT: NETPLAY_DEAL_*.
void Netplay_DealScreen(void)
{
    u8 taskId = CreateTask(Task_DealScreen, 80);

    gTasks[taskId].data[0] = 0;
    if (sDeal.state != DEAL_STATE_COMMITTED)
        sDeal.state = DEAL_STATE_MAIN;
    BufferDealPeerName();
    OpenDealWindows();
}

static void EndDealScreen(u8 taskId, u16 result)
{
    CloseDealWindows();
    gSpecialVar_Result = result;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void HandleDealMainInput(u8 taskId)
{
    if (JOY_NEW(DPAD_LEFT) && sDeal.cursor > 0)
    {
        sDeal.cursor--;
        PlaySE(SE_SELECT);
        DrawDealActions();
    }
    else if (JOY_NEW(DPAD_RIGHT) && sDeal.cursor < DEAL_ACTION_COUNT - 1)
    {
        sDeal.cursor++;
        PlaySE(SE_SELECT);
        DrawDealActions();
    }
    else if (JOY_NEW(B_BUTTON) && sDeal.cursor != DEAL_ACTION_QUIT)
    {
        sDeal.cursor = DEAL_ACTION_QUIT;
        PlaySE(SE_SELECT);
        DrawDealActions();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sDeal.status = NULL;
        switch (sDeal.cursor)
        {
        case DEAL_ACTION_ADD:
            if (sDeal.mine.offer.count >= DEAL_MAX_MONS)
            {
                sDeal.status = COMPOUND_STRING("Your offer is full.");
                break;
            }
            // The party menu takes over the screen; we come back after.
            if (sDeal.iAccepted)
                SetAccepted(FALSE);
            EndDealScreen(taskId, NETPLAY_DEAL_ADD_MON);
            return;
        case DEAL_ACTION_REMOVE:
            if (sDeal.mine.offer.count == 0)
            {
                sDeal.status = COMPOUND_STRING("You aren't offering any POKéMON.");
                break;
            }
            sDeal.state = DEAL_STATE_REMOVE;
            sDeal.removeCursor = 0;
            break;
        case DEAL_ACTION_MONEY:
            sDeal.state = DEAL_STATE_MONEY;
            sDeal.moneyEdit = sDeal.mine.offer.money;
            sDeal.moneyDigit = 3;
            break;
        case DEAL_ACTION_ACCEPT:
            if (sDeal.theirRev == 0)
                sDeal.status = COMPOUND_STRING("Wait for the other offer first.");
            else if (sDeal.mine.offer.count == 0 && sDeal.mine.offer.money == 0
                  && sDeal.theirs.offer.count == 0 && sDeal.theirs.offer.money == 0)
                sDeal.status = COMPOUND_STRING("Nobody is offering anything.");
            else if (!DealFitsParty())
                sDeal.status = COMPOUND_STRING("Your party would be empty or over 6.");
            else
                SetAccepted(!sDeal.iAccepted);
            break;
        case DEAL_ACTION_QUIT:
            EndDealScreen(taskId, NETPLAY_DEAL_QUIT);
            return;
        }
        DrawDealScreen();
    }
}

static void RemoveOfferedMon(u8 index)
{
    struct NetplayDealOffer *offer = &sDeal.mine.offer;
    u32 i;

    for (i = index; i + 1 < offer->count; i++)
    {
        offer->mons[i] = offer->mons[i + 1];
        sDeal.mySlots[i] = sDeal.mySlots[i + 1];
    }
    offer->count--;
    memset(&offer->mons[offer->count], 0, sizeof(offer->mons[0]));
    OnMyOfferChanged();
}

static void HandleDealRemoveInput(void)
{
    if (JOY_NEW(DPAD_UP) && sDeal.removeCursor > 0)
        sDeal.removeCursor--;
    else if (JOY_NEW(DPAD_DOWN) && sDeal.removeCursor + 1 < sDeal.mine.offer.count)
        sDeal.removeCursor++;
    else if (JOY_NEW(A_BUTTON))
    {
        RemoveOfferedMon(sDeal.removeCursor);
        sDeal.state = DEAL_STATE_MAIN;
    }
    else if (JOY_NEW(B_BUTTON))
        sDeal.state = DEAL_STATE_MAIN;
    else
        return;
    PlaySE(SE_SELECT);
    DrawDealScreen();
}

static void HandleDealMoneyInput(void)
{
    static const u32 sPowers[DEAL_MONEY_DIGITS] = {1, 10, 100, 1000, 10000, 100000};
    u32 most = min(GetMoney(&gSaveBlock1Ptr->money), 999999);
    u32 step = sPowers[sDeal.moneyDigit];

    if (JOY_NEW(DPAD_UP))
        sDeal.moneyEdit = min(sDeal.moneyEdit + step, most);
    else if (JOY_NEW(DPAD_DOWN))
        sDeal.moneyEdit = sDeal.moneyEdit >= step ? sDeal.moneyEdit - step : 0;
    else if (JOY_NEW(DPAD_LEFT) && sDeal.moneyDigit < DEAL_MONEY_DIGITS - 1)
        sDeal.moneyDigit++;
    else if (JOY_NEW(DPAD_RIGHT) && sDeal.moneyDigit > 0)
        sDeal.moneyDigit--;
    else if (JOY_NEW(A_BUTTON))
    {
        if (sDeal.moneyEdit != sDeal.mine.offer.money)
        {
            sDeal.mine.offer.money = sDeal.moneyEdit;
            OnMyOfferChanged();
        }
        sDeal.state = DEAL_STATE_MAIN;
    }
    else if (JOY_NEW(B_BUTTON))
        sDeal.state = DEAL_STATE_MAIN;
    else
        return;
    PlaySE(SE_SELECT);
    DrawDealScreen();
}

static void Task_DealScreen(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    bool32 changed;

    SendDealChunks();
    changed = ReceiveDealPackets();

    if (!sSession.active || ++data[0] > DEAL_TIMEOUT)
    {
        EndDealScreen(taskId, NETPLAY_DEAL_PEER_LEFT);
        return;
    }

    // Both accepted the same offers: commit, then wait for their commit.
    if (sDeal.state != DEAL_STATE_COMMITTED && sDeal.iAccepted && sDeal.acceptSent && PeerAcceptsCurrentOffers())
    {
        sDeal.state = DEAL_STATE_COMMITTED;
        sDeal.commitSent = FALSE;
        changed = TRUE;
    }
    if (sDeal.state == DEAL_STATE_COMMITTED)
    {
        if (!sDeal.iAccepted)
        {
            // Their offer changed before they committed.
            sDeal.state = DEAL_STATE_MAIN;
            changed = TRUE;
        }
        else
        {
            if (!sDeal.commitSent)
            {
                u16 cmd[CMD_LENGTH] = {sDeal.myRev, sDeal.theirRev};
                sDeal.commitSent = SendPacket(NETPLAY_PACKET_DEAL_COMMIT, cmd);
            }
            if (sDeal.commitSent && PeerCommittedCurrentOffers())
            {
                EndDealScreen(taskId, NETPLAY_DEAL_DONE);
                return;
            }
        }
    }

    switch (sDeal.state)
    {
    case DEAL_STATE_MAIN:
        HandleDealMainInput(taskId);
        if (!gTasks[taskId].isActive)
            return;
        break;
    case DEAL_STATE_REMOVE:
        HandleDealRemoveInput();
        break;
    case DEAL_STATE_MONEY:
        HandleDealMoneyInput();
        break;
    }
    if (changed)
    {
        sDeal.status = NULL;
        DrawDealScreen();
    }
}

// After the party menu: VAR_0x8004 is the chosen slot (PARTY_SIZE or more
// if none).
void Netplay_DealAddMon(void)
{
    struct NetplayDealOffer *offer = &sDeal.mine.offer;
    u8 slot = gSpecialVar_0x8004;
    u32 i;

    sDeal.status = NULL;
    if (slot >= gPartiesCount[B_TRAINER_PLAYER] || offer->count >= DEAL_MAX_MONS)
        return;
    for (i = 0; i < offer->count; i++)
    {
        if (sDeal.mySlots[i] == slot)
        {
            sDeal.status = COMPOUND_STRING("That POKéMON is already offered.");
            return;
        }
    }
    sDeal.mySlots[offer->count] = slot;
    offer->mons[offer->count++] = gParties[B_TRAINER_PLAYER][slot];
    OnMyOfferChanged();
}

// Both committed: hand over our side and take theirs, then save.
void Netplay_DealApply(void)
{
    struct NetplayDealOffer *mine = &sDeal.mine.offer;
    struct NetplayDealOffer *theirs = &sDeal.theirs.offer;
    u32 i;

    for (i = 0; i < mine->count; i++)
        ZeroMonData(&gParties[B_TRAINER_PLAYER][sDeal.mySlots[i]]);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    for (i = 0; i < theirs->count && gPartiesCount[B_TRAINER_PLAYER] < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gPartiesCount[B_TRAINER_PLAYER]];
        u16 species;

        *mon = theirs->mons[i];
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

// Ladder -------------------------------------------------------------------
//
// START > LADDER while online: the server's PvP ladder (top 10 by rating)
// and the player's own rank.

#define LADDER_LINE_HEIGHT 11
#define LADDER_TIMEOUT     (5 * 60)

#define tState     data[0]
#define tWindowId  data[1]
#define tSeq       data[2]
#define tTimer     data[3]

enum
{
    LADDER_STATE_WAIT,
    LADDER_STATE_SHOWN,
};

static const struct WindowTemplate sLadderWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 17,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static void Task_Ladder(u8 taskId);

static u8 *BufferLadderName(u8 *dest, const u8 *name)
{
    u32 i;

    for (i = 0; i < NETPLAY_NAME_LENGTH && name[i] != EOS; i++)
        *dest++ = name[i];
    *dest = EOS;
    return dest;
}

static u8 *BufferLadderRecord(u8 *dest, const struct NetplayLadderEntry *entry)
{
    dest = ConvertIntToDecimalStringN(dest, entry->wins, STR_CONV_MODE_LEFT_ALIGN, 4);
    *dest++ = CHAR_HYPHEN;
    dest = ConvertIntToDecimalStringN(dest, entry->losses, STR_CONV_MODE_LEFT_ALIGN, 4);
    *dest++ = CHAR_HYPHEN;
    return ConvertIntToDecimalStringN(dest, entry->draws, STR_CONV_MODE_LEFT_ALIGN, 4);
}

// Columns: rank, name, rating, wins-losses-draws.
static void PrintLadderRow(u8 windowId, u32 y, u32 rank, const struct NetplayLadderEntry *entry)
{
    u8 *str;

    if (rank != 0)
    {
        str = ConvertIntToDecimalStringN(gStringVar4, rank, STR_CONV_MODE_LEFT_ALIGN, 4);
        *str++ = CHAR_PERIOD;
        *str = EOS;
    }
    else
    {
        StringCopy(gStringVar4, COMPOUND_STRING("-"));
    }
    AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 6, y, TEXT_SKIP_DRAW, NULL);
    BufferLadderName(gStringVar4, entry->name);
    AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 30, y, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(gStringVar4, entry->rating, STR_CONV_MODE_LEFT_ALIGN, 5);
    AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 110, y, TEXT_SKIP_DRAW, NULL);
    BufferLadderRecord(gStringVar4, entry);
    AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 160, y, TEXT_SKIP_DRAW, NULL);
}

static void DrawLadder(u8 windowId, const u8 *message)
{
    const struct NetplayLadder *ladder = &gNetplayMailbox.ladder;
    s32 i;

    // Drawn bottom-up: lines are closer together than the font's box.
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (message != NULL)
    {
        AddTextPrinterParameterized(windowId, FONT_NORMAL, message, 6, LADDER_LINE_HEIGHT * 2, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        u32 bottom = LADDER_LINE_HEIGHT * (NETPLAY_LADDER_SIZE + 1) + 2;

        if (ladder->myRank != 0)
            PrintLadderRow(windowId, bottom, ladder->myRank, &ladder->me);
        else
            AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("You: no rated battles yet"), 30, bottom, TEXT_SKIP_DRAW, NULL);
        for (i = ladder->count - 1; i >= 0; i--)
            PrintLadderRow(windowId, LADDER_LINE_HEIGHT * (i + 1), i + 1, &ladder->top[i]);
        if (ladder->count == 0)
            AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("No rated battles yet."), 30, LADDER_LINE_HEIGHT + 2, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("PvP LADDER"), 30, 0, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("RATING"), 110, 0, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("W-L-D"), 160, 0, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

void ShowNetplayLadder(void)
{
    u8 taskId = CreateTask(Task_Ladder, 80);
    struct Task *task = &gTasks[taskId];

    task->tWindowId = AddWindow(&sLadderWindowTemplate);
    task->tSeq = gNetplayMailbox.ladder.seq;
    task->tTimer = 0;
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PutWindowTilemap(task->tWindowId);
    if (IsOnline())
    {
        QueueRequest(NETPLAY_REQ_LADDER, 0, 0, 0);
        task->tState = LADDER_STATE_WAIT;
        DrawLadder(task->tWindowId, COMPOUND_STRING("Loading the ladder…"));
    }
    else
    {
        task->tState = LADDER_STATE_SHOWN;
        DrawLadder(task->tWindowId, COMPOUND_STRING("Not online. Pick JOIN ONLINE\non the title menu to play online."));
    }
    ScheduleBgCopyTilemapToVram(0);
}

static void Task_Ladder(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case LADDER_STATE_WAIT:
        if (gNetplayMailbox.ladder.seq != (u8)task->tSeq)
        {
            DrawLadder(task->tWindowId, NULL);
            task->tState = LADDER_STATE_SHOWN;
        }
        else if (++task->tTimer > LADDER_TIMEOUT || !IsOnline())
        {
            DrawLadder(task->tWindowId, COMPOUND_STRING("The server didn't answer."));
            task->tState = LADDER_STATE_SHOWN;
        }
        // fallthrough
    case LADDER_STATE_SHOWN:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
            ClearWindowTilemap(task->tWindowId);
            RemoveWindow(task->tWindowId);
            ScheduleBgCopyTilemapToVram(0);
            DestroyTask(taskId);
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
        }
        break;
    }
}

#undef tState
#undef tWindowId
#undef tSeq
#undef tTimer
