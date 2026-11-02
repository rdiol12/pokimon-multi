#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "item.h"
#include "netplay.h"
#include "netplay_ext.h"
#include "online.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "save_ext.h"
#include "script.h"
#include "sound.h"
#include "stormwild.h"
#include "string_util.h"
#include "task.h"
#include "constants/characters.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/songs.h"

// The Stormwild (docs/stormwild.md): an extraction run. The Storm Warden by
// Mauville's Pokémon Center packs the player's pouch (up to ten kinds of
// items; the rest of the bag waits at the gate, in the extended save), and
// after the lobby's countdown drops them at one of eight points on the edge
// of the Stormwild. There the server's clock runs: supply caches go to
// whoever opens them first, one beacon at a time lights up, and standing by a
// lit beacon for ten seconds gets the player out with everything they carry.
// When the clock runs out, the team all faints or the player leaves any
// other way, the run fails: the bag (pouch and finds) and every Pokémon
// caught in the run are lost, and the bag left at the gate comes back.
// A runner who loses a battle against another runner gives up their most
// valuable stack of items and up to two Pokémon caught in the run
// (server/online_xrun.py holds them for the winner until they get out).

#define sSave (gSaveBlockExt.stormwild)

#define NUM_BEACONS 3
#define NUM_CACHES  24
#define NUM_DROPS   8
#define LOCALID_BEACON(i) (1 + (i))
#define LOCALID_CACHE(i)  (1 + NUM_BEACONS + (i))
#define HOLD_FRAMES (10 * 60)
#define HOLD_REACH  1
#define OVER_WAIT   (20 * 60) // a run whose clock ran out waits this long for the server
#define MESSAGE_LENGTH 200

// Where the Warden stands, and where runners come back to.
#define GATE_MAP MAP_MAUVILLE_CITY
#define GATE_X 20
#define GATE_Y 7

// As tools/areas/stormwild.py and server/online_xrun.py.
static const u8 sBeacons[NUM_BEACONS][2] = {{11, 10}, {54, 28}, {30, 53}};
static const u8 *const sBeaconNames[NUM_BEACONS] = {
    COMPOUND_STRING("TOWER"), COMPOUND_STRING("LAKE"), COMPOUND_STRING("HOLLOW"),
};
static const u8 sCaches[NUM_CACHES][2] = {
    {17, 9}, {6, 14}, {44, 7}, {53, 12}, {58, 20}, {49, 24}, {57, 38}, {51, 46},
    {44, 52}, {36, 58}, {24, 56}, {12, 51}, {17, 44}, {4, 40}, {6, 24}, {18, 24},
    {24, 29}, {38, 25}, {41, 37}, {28, 42}, {32, 31}, {36, 16}, {26, 13}, {60, 52},
};
static const u8 sDrops[NUM_DROPS][2] = {
    {8, 8}, {32, 5}, {56, 8}, {59, 32}, {56, 56}, {32, 59}, {8, 56}, {5, 32},
};

// The bag pockets that go in the pouch, wait at the gate, or are lost.
static const u8 sPockets[] = {POCKET_ITEMS, POCKET_POKE_BALLS, POCKET_BERRIES};

enum
{
    PHASE_IDLE,
    PHASE_LOBBY,      // at the gate, counting down
    PHASE_ENTERING,   // warping in
    PHASE_LIVE,
    PHASE_SCRIPT,     // the extraction, raid or end script runs
    PHASE_AFTER,      // back at the gate: the result is shown once the field is free
};

struct Stormwild
{
    u8 phase;
    bool8 session;     // this boot started the run
    bool8 failed;      // PHASE_AFTER: TRUE if the run failed
    s8 holdBeacon;
    u16 countdown;     // frames: the lobby
    u16 timer;         // frames left in the run
    u16 holdFrames;
    u16 overWait;
    s16 serverSeconds;
    u16 hudFrame;
    u32 hidden;        // caches hidden on this map
    u32 enemyPersonality;
    u16 robItem, robCount;
    u32 robMons[2];
    u8 message[MESSAGE_LENGTH];
};

static EWRAM_DATA struct Stormwild sStorm = {0};

extern const u8 Stormwild_EventScript_TimeUp[];
extern const u8 Stormwild_EventScript_LeftZone[];
extern const u8 Stormwild_EventScript_Lost[];
extern const u8 Stormwild_EventScript_Extract[];
extern const u8 Stormwild_EventScript_Raided[];
extern const u8 Stormwild_EventScript_After[];

static bool32 InZone(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_STORMWILD)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_STORMWILD);
}

bool32 Stormwild_IsOn(void)
{
    return sStorm.phase != PHASE_IDLE && sStorm.phase != PHASE_AFTER;
}

// The server's status of this run: 0 run id, 1 seconds left, 2 the lit
// beacon (beacon * 100 + seconds it stays lit, or -1), 3 the caches taken
// (bits), 4 raids lost and not yet settled, 5 runners still in.
static bool32 StatusIsOurs(void)
{
    return *Online_Status("XRUN", 0) != '\0' && Online_StatusNum("XRUN", 0) == sSave.runId;
}

static s32 RunStatus(u32 field)
{
    return StatusIsOurs() ? Online_StatusNum("XRUN", field) : 0;
}

static s32 LitBeacon(void)
{
    s32 lit;

    if (!StatusIsOurs())
        return -1;
    lit = Online_StatusNum("XRUN", 2);
    return lit >= 0 ? lit / 100 : -1;
}

// --- the bag ---------------------------------------------------------------

static u32 BagValue(void)
{
    u32 p, i, value = 0;

    for (p = 0; p < ARRAY_COUNT(sPockets); p++)
    {
        struct BagPocket *pocket = &gBagPockets[sPockets[p]];
        for (i = 0; i < pocket->capacity; i++)
        {
            struct ItemSlot slot = BagPocket_GetSlotData(pocket, i);
            if (slot.itemId != ITEM_NONE)
                value += GetItemPrice(slot.itemId) * slot.quantity;
        }
    }
    return value;
}

static void ClearPockets(void)
{
    u32 p, i;

    for (p = 0; p < ARRAY_COUNT(sPockets); p++)
    {
        struct BagPocket *pocket = &gBagPockets[sPockets[p]];
        for (i = 0; i < pocket->capacity; i++)
            BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_NONE, 0);
    }
}

// Puts the bag's pockets in the stash at the gate.
static void StashBag(void)
{
    u32 p, i, n = 0;

    memset(sSave.stash, 0, sizeof(sSave.stash));
    for (p = 0; p < ARRAY_COUNT(sPockets); p++)
    {
        struct BagPocket *pocket = &gBagPockets[sPockets[p]];
        for (i = 0; i < pocket->capacity && n < STORMWILD_STASH_SLOTS; i++)
        {
            struct ItemSlot slot = BagPocket_GetSlotData(pocket, i);
            if (slot.itemId != ITEM_NONE && slot.quantity != 0)
                sSave.stash[n++] = slot;
        }
    }
    ClearPockets();
}

// Gives back what waited at the gate (to the PC if the bag is full).
static void UnstashBag(void)
{
    u32 i;

    for (i = 0; i < STORMWILD_STASH_SLOTS; i++)
    {
        struct ItemSlot slot = sSave.stash[i];
        if (slot.itemId == ITEM_NONE || slot.quantity == 0)
            continue;
        if (!AddBagItem(slot.itemId, slot.quantity))
            AddPCItem(slot.itemId, slot.quantity);
    }
    memset(sSave.stash, 0, sizeof(sSave.stash));
}

// --- the Pokémon caught in the run -----------------------------------------

// Finds a Pokémon by personality: in the party (box 0xFF) or a box.
static bool32 FindMon(u32 personality, u8 *box, u8 *pos)
{
    u32 b, i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) == personality)
        {
            *box = 0xFF;
            *pos = i;
            return TRUE;
        }
    }
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
    {
        for (i = 0; i < IN_BOX_COUNT; i++)
        {
            if (GetBoxMonDataAt(b, i, MON_DATA_SPECIES) != SPECIES_NONE
             && GetBoxMonDataAt(b, i, MON_DATA_PERSONALITY) == personality)
            {
                *box = b;
                *pos = i;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void RemoveMon(u32 personality)
{
    u8 box, pos;

    if (!FindMon(personality, &box, &pos))
        return;
    if (box == 0xFF)
    {
        // Never the last one in the party.
        if (CalculatePlayerPartyCount() <= 1)
            return;
        ZeroMonData(&gPlayerParty[pos]);
        CompactPartySlots();
        CalculatePlayerPartyCount();
    }
    else
    {
        ZeroBoxMonAt(box, pos);
    }
}

static void ForgetCatch(u32 personality)
{
    u32 i;

    for (i = 0; i < sSave.numCatches; i++)
    {
        if (sSave.catches[i] == personality)
        {
            sSave.catches[i] = sSave.catches[--sSave.numCatches];
            return;
        }
    }
}

void Stormwild_OnWildBattleStart(void)
{
    sStorm.enemyPersonality = GetMonData(&gEnemyParty[0], MON_DATA_PERSONALITY);
}

void Stormwild_OnCatch(void)
{
    if (sSave.state != STORMWILD_IN_RUN || !InZone() || sSave.numCatches >= STORMWILD_MAX_CATCHES)
        return;
    sSave.catches[sSave.numCatches++] = sStorm.enemyPersonality;
}

// Wild Pokémon in the Stormwild are shiny about one time in 130.
u32 Stormwild_ShinyRolls(void)
{
    return InZone() ? 31 : 0;
}

// --- the objects -------------------------------------------------------------

static void HideCache(u32 cache)
{
    sStorm.hidden |= 1u << cache;
    FlagSet(FLAG_TEMP_1 + cache);
    RemoveObjectEventByLocalIdAndMap(LOCALID_CACHE(cache), gSaveBlock1Ptr->location.mapNum,
                                     gSaveBlock1Ptr->location.mapGroup);
}

// Map script, ON_TRANSITION: only this run's caches that nobody opened yet.
void Stormwild_SetupObjects(void)
{
    u32 i, gone = sSave.state == STORMWILD_IN_RUN ? ~sSave.activeCaches : ~0u;

    gone |= (u32)RunStatus(3);
    sStorm.hidden = 0;
    for (i = 0; i < NUM_CACHES; i++)
    {
        if (gone & (1u << i))
        {
            FlagSet(FLAG_TEMP_1 + i);
            sStorm.hidden |= 1u << i;
        }
    }
}

// Map script, ON_RESUME (after a battle): the same.
void Stormwild_HideTakenCaches(void)
{
    Stormwild_SetupObjects();
}

static void UpdateObjects(void)
{
    u32 i, taken = (u32)RunStatus(3) & sSave.activeCaches;
    s32 lit = LitBeacon();

    for (i = 0; i < NUM_CACHES; i++)
        if ((taken & (1u << i)) && !(sStorm.hidden & (1u << i)))
            HideCache(i);
    for (i = 0; i < NUM_BEACONS; i++)
    {
        u8 id = GetObjectEventIdByLocalIdAndMap(LOCALID_BEACON(i), gSaveBlock1Ptr->location.mapNum,
                                                gSaveBlock1Ptr->location.mapGroup);
        u16 gfx = (s32)i == lit ? OBJ_EVENT_GFX_STORM_BEACON_ON : OBJ_EVENT_GFX_STORM_BEACON;
        if (id < OBJECT_EVENTS_COUNT && gObjectEvents[id].graphicsId != gfx)
            ObjectEventSetGraphicsId(&gObjectEvents[id], gfx);
    }
}

// Script, a cache: VAR_0x8005 is which one.
void Stormwild_PickCache(void)
{
    gSpecialVar_0x8005 = gSpecialVar_LastTalked - LOCALID_CACHE(0);
}

// Script: the cache was opened (by us or someone else).
void Stormwild_CacheGone(void)
{
    if (gSpecialVar_0x8005 < NUM_CACHES)
        HideCache(gSpecialVar_0x8005);
}

// Script, a beacon: what it says, in gStringVar4.
void Stormwild_BeaconText(void)
{
    u32 beacon = gSpecialVar_LastTalked - LOCALID_BEACON(0);

    if (sSave.state == STORMWILD_IN_RUN && (s32)beacon == LitBeacon())
        StringCopy(gStringVar4, COMPOUND_STRING("The beacon is lit! Stay right by it\nto call for a pickup."));
    else
        StringCopy(gStringVar4, COMPOUND_STRING("The beacon is dark. When one lights\nup, the storm clock will say where."));
}

// --- the HUD -----------------------------------------------------------------

static void HudText(u8 *text)
{
    u8 *end;
    s32 lit = LitBeacon();

    if (sStorm.phase == PHASE_LOBBY)
    {
        end = StringCopy(text, COMPOUND_STRING("GATE "));
        Online_FormatTime(end, sStorm.countdown, FALSE);
        return;
    }
    if (sStorm.holdFrames != 0)
    {
        end = StringCopy(text, COMPOUND_STRING("OUT IN "));
        ConvertIntToDecimalStringN(end, (HOLD_FRAMES - sStorm.holdFrames + 59) / 60, STR_CONV_MODE_LEFT_ALIGN, 2);
        return;
    }
    // A lit beacon shows every other two seconds.
    if (lit >= 0 && lit < NUM_BEACONS && (sStorm.hudFrame / 120) % 2 == 1)
    {
        end = StringCopy(text, sBeaconNames[lit]);
        *end++ = CHAR_SPACE;
        Online_FormatTime(end, (Online_StatusNum("XRUN", 2) % 100) * 60, FALSE);
        return;
    }
    end = StringCopy(text, COMPOUND_STRING("STORM "));
    Online_FormatTime(end, sStorm.timer, FALSE);
}

// Keeps the game's clock in step with the server's.
static void SyncTimer(void)
{
    s32 seconds;

    if (!StatusIsOurs())
        return;
    seconds = Online_StatusNum("XRUN", 1);
    if (seconds == sStorm.serverSeconds)
        return;
    sStorm.serverSeconds = seconds;
    sStorm.timer = min(seconds, 0xFFFF / 60) * 60;
}

// --- every frame ---------------------------------------------------------------

static bool32 StartScript(const u8 *script)
{
    sStorm.phase = PHASE_SCRIPT;
    sStorm.holdFrames = 0;
    Online_HideHud();
    ScriptContext_SetupScript(script);
    return TRUE;
}

static bool32 NearBeacon(s32 beacon)
{
    s16 x, y;

    if (beacon < 0 || beacon >= NUM_BEACONS)
        return FALSE;
    PlayerGetDestCoords(&x, &y);
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;
    return abs(x - sBeacons[beacon][0]) <= HOLD_REACH && abs(y - sBeacons[beacon][1]) <= HOLD_REACH;
}

// Called every overworld frame. TRUE if it started a script.
bool32 Stormwild_Update(void)
{
    bool32 free = !ArePlayerFieldControlsLocked();
    s32 lit;

    if (!sStorm.session)
    {
        if (!free)
            return FALSE;
        // A run from before the game was reset: it's lost.
        if (sSave.state == STORMWILD_IN_RUN)
        {
            sStorm.session = TRUE;
            return StartScript(Stormwild_EventScript_Lost);
        }
        if (sSave.state == STORMWILD_PACKED)
        {
            u32 i;
            for (i = 0; i < sSave.numPouch; i++)
                AddBagItem(sSave.pouch[i].itemId, sSave.pouch[i].quantity);
            sSave.numPouch = 0;
            sSave.state = STORMWILD_NONE;
        }
        return FALSE;
    }

    switch (sStorm.phase)
    {
    case PHASE_ENTERING:
        if (InZone())
            sStorm.phase = PHASE_LIVE;
        return FALSE;
    case PHASE_LIVE:
        if (!InZone())
        {
            if (!free)
                return FALSE;
            // Whited out, flew off, used an Escape Rope...
            return StartScript(Stormwild_EventScript_LeftZone);
        }
        sStorm.hudFrame++;
        SyncTimer();
        if (sStorm.timer != 0)
            sStorm.timer--;
        else
            sStorm.overWait++;
        if (sStorm.timer % 180 == 0)
            Online_PollStatusSoon();
        UpdateObjects();
        if (!free)
            return FALSE;
        if ((StatusIsOurs() && Online_StatusNum("XRUN", 1) == 0) || sStorm.overWait > OVER_WAIT)
            return StartScript(Stormwild_EventScript_TimeUp);
        if (RunStatus(4) > 0)
            return StartScript(Stormwild_EventScript_Raided);
        lit = LitBeacon();
        if (NearBeacon(lit))
        {
            if (sStorm.holdFrames == 0)
                PlaySE(SE_PIN);
            if (++sStorm.holdFrames >= HOLD_FRAMES)
            {
                sStorm.holdBeacon = lit;
                gSpecialVar_0x8005 = lit;
                return StartScript(Stormwild_EventScript_Extract);
            }
            if (sStorm.holdFrames % 60 == 0)
                PlaySE(SE_BALL_BOUNCE_1);
        }
        else
        {
            sStorm.holdFrames = 0;
        }
        Online_ShowHud(HudText, FALSE);
        return FALSE;
    case PHASE_AFTER:
        if (!free)
            return FALSE;
        sStorm.phase = PHASE_IDLE;
        ScriptContext_SetupScript(Stormwild_EventScript_After);
        return TRUE;
    }
    return FALSE;
}

// --- the Warden's scripts --------------------------------------------------------

// VAR_RESULT: 0 the team may go in, 1 too many Pokémon, 2 none that can fight.
void Stormwild_CheckTeam(void)
{
    u32 i, count = CalculatePlayerPartyCount(), able = 0;

    for (i = 0; i < count; i++)
        if (!GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
            able++;
    gSpecialVar_Result = count > STORMWILD_MAX_TEAM ? 1 : able == 0 ? 2 : 0;
    gSpecialVar_0x8005 = count;
    ConvertIntToDecimalStringN(gStringVar1, STORMWILD_MAX_TEAM, STR_CONV_MODE_LEFT_ALIGN, 1);
}

// VAR_RESULT TRUE if the pouch is full; gStringVar1 how many kinds are in it.
void Stormwild_PouchInfo(void)
{
    if (sSave.state == STORMWILD_NONE)
    {
        sSave.state = STORMWILD_PACKED;
        sSave.numPouch = 0;
    }
    ConvertIntToDecimalStringN(gStringVar1, sSave.numPouch, STR_CONV_MODE_LEFT_ALIGN, 2);
    gSpecialVar_Result = sSave.numPouch >= STORMWILD_POUCH_KINDS;
}

// After Online_ChooseBagItem: VAR_0x8005 the item, VAR_0x8006 how many the
// bag has. Packs up to STORMWILD_POUCH_EACH of it.
void Stormwild_PackChosen(void)
{
    u16 item = gSpecialVar_0x8005, count = min(gSpecialVar_0x8006, STORMWILD_POUCH_EACH);
    u32 i;

    for (i = 0; i < sSave.numPouch; i++)
    {
        if (sSave.pouch[i].itemId == item)
        {
            count = min(count, STORMWILD_POUCH_EACH - sSave.pouch[i].quantity);
            break;
        }
    }
    if (count == 0 || (i == sSave.numPouch && sSave.numPouch >= STORMWILD_POUCH_KINDS) || !RemoveBagItem(item, count))
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    if (i == sSave.numPouch)
    {
        sSave.pouch[i].itemId = item;
        sSave.pouch[i].quantity = 0;
        sSave.numPouch++;
    }
    sSave.pouch[i].quantity += count;
    CopyItemNameHandlePlural(item, gStringVar2, count);
    ConvertIntToDecimalStringN(gStringVar3, count, STR_CONV_MODE_LEFT_ALIGN, 2);
    gSpecialVar_Result = TRUE;
}

// Back in the bag (the run didn't start).
void Stormwild_Unpack(void)
{
    u32 i;

    for (i = 0; i < sSave.numPouch; i++)
        if (!AddBagItem(sSave.pouch[i].itemId, sSave.pouch[i].quantity))
            AddPCItem(sSave.pouch[i].itemId, sSave.pouch[i].quantity);
    sSave.numPouch = 0;
    sSave.state = STORMWILD_NONE;
    sStorm.phase = PHASE_IDLE;
    Online_HideHud();
}

// After XRUN JOIN: data row 0 is run id, drop point, run seconds, the caches
// in this run (bits); VAR_0x800B the seconds till the gate opens.
void Stormwild_Joined(void)
{
    sSave.runId = Online_DataNum(0, 0);
    sSave.drop = Online_DataNum(0, 1) % NUM_DROPS;
    sSave.activeCaches = Online_DataNum(0, 3);
    sStorm.timer = min(Online_DataNum(0, 2), 0xFFFF / 60) * 60;
    sStorm.countdown = max(gSpecialVar_0x800B, 3) * 60;
    sStorm.serverSeconds = -1;
    sStorm.overWait = 0;
    sStorm.holdFrames = 0;
    sStorm.session = TRUE;
    sStorm.phase = PHASE_LOBBY;
}

static void Task_Countdown(u8 taskId)
{
    if (sStorm.countdown % 60 == 0 && sStorm.countdown <= 3 * 60 && sStorm.countdown != 0)
        PlaySE(SE_BALL_BOUNCE_1);
    if (sStorm.countdown == 0)
    {
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    sStorm.countdown--;
    Online_ShowHud(HudText, TRUE);
}

// The lobby (waitstate): counts down to the gate opening.
void Stormwild_LobbyCountdown(void)
{
    Online_ShowHud(HudText, TRUE);
    CreateTask(Task_Countdown, 80);
}

// After XRUN GO: the rest of the bag waits at the gate, the pouch goes in
// the bag, the game is saved (a reset can't undo a run) and the player
// drops into the Stormwild.
void Stormwild_Enter(void)
{
    u32 i;

    StashBag();
    for (i = 0; i < sSave.numPouch; i++)
        AddBagItem(sSave.pouch[i].itemId, sSave.pouch[i].quantity);
    sSave.numPouch = 0;
    sSave.numCatches = 0;
    sSave.runs++;
    sSave.state = STORMWILD_IN_RUN;
    sStorm.phase = PHASE_ENTERING;
    sStorm.hudFrame = 0;
    Online_HideHud();
    TrySavingData(SAVE_NORMAL);
    SetWarpDestination(MAP_GROUP(MAP_STORMWILD), MAP_NUM(MAP_STORMWILD), WARP_ID_NONE,
                       sDrops[sSave.drop][0], sDrops[sSave.drop][1]);
    DoWarp();
}

static void BackToGate(bool32 failed)
{
    UnstashBag();
    sSave.state = STORMWILD_NONE;
    sSave.numCatches = 0;
    sStorm.failed = failed;
    sStorm.phase = PHASE_AFTER;
    Online_HideHud();
    SetWarpDestination(MAP_GROUP(GATE_MAP), MAP_NUM(GATE_MAP), WARP_ID_NONE, GATE_X, GATE_Y);
    DoWarp();
}

// XRUN OUT's build: what the haul is worth (the bag's items at mart prices,
// and 1000 for each Pokémon caught).
void Stormwild_AddHaul(void)
{
    NpxRpc_Num(BagValue() + sSave.numCatches * 1000);
}

// After XRUN OUT (the server's message in gStringVar4): out with everything.
void Stormwild_Extracted(void)
{
    u32 haul = BagValue() + sSave.numCatches * 1000;

    StringCopyN(sStorm.message, gStringVar4, MESSAGE_LENGTH - 1);
    sStorm.message[MESSAGE_LENGTH - 1] = EOS;
    sSave.escapes++;
    if (haul > sSave.bestHaul)
        sSave.bestHaul = haul;
    BackToGate(FALSE);
}

// XRUN OUT was refused (the beacon went dark...): keep going.
void Stormwild_ExtractRefused(void)
{
    sStorm.phase = PHASE_LIVE;
    sStorm.holdFrames = 0;
}

// The run failed: the bag and every Pokémon caught in it are lost.
void Stormwild_Fail(void)
{
    u32 i;

    ClearPockets();
    for (i = 0; i < sSave.numCatches; i++)
        RemoveMon(sSave.catches[i]);
    StringCopy(sStorm.message, COMPOUND_STRING("The storm took everything you carried,\nand every POKéMON you caught in it."));
    BackToGate(TRUE);
}

// XRUN ROB's build: the most valuable stack of items, and the last two
// Pokémon caught in the run that are still here.
void Stormwild_AddRobbed(void)
{
    u32 p, i, best = 0, n = 0;

    sStorm.robItem = ITEM_NONE;
    sStorm.robCount = 0;
    for (p = 0; p < ARRAY_COUNT(sPockets); p++)
    {
        struct BagPocket *pocket = &gBagPockets[sPockets[p]];
        for (i = 0; i < pocket->capacity; i++)
        {
            struct ItemSlot slot = BagPocket_GetSlotData(pocket, i);
            u32 value = slot.itemId != ITEM_NONE ? GetItemPrice(slot.itemId) * slot.quantity : 0;
            if (value > best || (slot.itemId != ITEM_NONE && sStorm.robItem == ITEM_NONE))
            {
                best = value;
                sStorm.robItem = slot.itemId;
                sStorm.robCount = slot.quantity;
            }
        }
    }
    NpxRpc_Num(sStorm.robItem);
    NpxRpc_Num(sStorm.robCount);
    sStorm.robMons[0] = sStorm.robMons[1] = 0;
    for (i = sSave.numCatches; i > 0 && n < 2; i--)
    {
        u8 box, pos;
        u32 personality = sSave.catches[i - 1];
        if (!FindMon(personality, &box, &pos))
            continue;
        if (box == 0xFF)
        {
            struct Pokemon *mon = &gPlayerParty[pos];
            NpxRpc_Num(GetMonData(mon, MON_DATA_SPECIES));
            NpxRpc_Num(GetMonData(mon, MON_DATA_LEVEL));
            NpxRpc_Num(GetMonData(mon, MON_DATA_IS_SHINY));
        }
        else
        {
            NpxRpc_Num(GetBoxMonDataAt(box, pos, MON_DATA_SPECIES));
            NpxRpc_Num(GetBoxMonDataAt(box, pos, MON_DATA_LEVEL));
            NpxRpc_Num(GetBoxMonDataAt(box, pos, MON_DATA_IS_SHINY));
        }
        sStorm.robMons[n++] = personality;
    }
}

// After XRUN ROB: they're gone.
void Stormwild_AfterRob(void)
{
    u32 i;

    if (sStorm.robItem != ITEM_NONE && sStorm.robCount != 0)
        RemoveBagItem(sStorm.robItem, sStorm.robCount);
    for (i = 0; i < 2; i++)
    {
        if (sStorm.robMons[i] == 0)
            continue;
        RemoveMon(sStorm.robMons[i]);
        ForgetCatch(sStorm.robMons[i]);
    }
    sStorm.robItem = ITEM_NONE;
}

// The end of a raid or a cache script: back to the run.
void Stormwild_Resume(void)
{
    if (sStorm.phase == PHASE_SCRIPT)
        sStorm.phase = PHASE_LIVE;
}

// The script after coming back: the message (gStringVar4), VAR_RESULT TRUE
// if the run failed; then the game is saved.
void Stormwild_AfterText(void)
{
    StringCopy(gStringVar4, sStorm.message);
    gSpecialVar_Result = sStorm.failed;
}

void Stormwild_Save(void)
{
    TrySavingData(SAVE_NORMAL);
}

// VAR_RESULT: TRUE if a run is on.
void Stormwild_IsOnScript(void)
{
    gSpecialVar_Result = Stormwild_IsOn();
}

// VAR_RESULT: the best haul and gStringVar1-3: best, runs, escapes.
void Stormwild_GetRecord(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sSave.bestHaul, STR_CONV_MODE_LEFT_ALIGN, 7);
    ConvertIntToDecimalStringN(gStringVar2, sSave.runs, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar3, sSave.escapes, STR_CONV_MODE_LEFT_ALIGN, 4);
    gSpecialVar_Result = sSave.runs;
}
// Where the run's save is in gSaveBlockExt, for the screenshot runner
// (tools/stormwild/shots_seed.py).
const u32 gStormwildSaveOffset = offsetof(struct SaveBlockExt, stormwild);
