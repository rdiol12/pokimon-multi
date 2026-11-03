#include "global.h"
#include "alive.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "main.h"
#include "menu.h"
#include "metatile_behavior.h"
#include "netplay.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "script.h"
#include "shared_map.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "wild_encounter_ow.h"
#include "window.h"
#include "world.h"
#include "world_mailbox.h"
#include "constants/battle.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Shared wild spawns and map channels (docs/shared_map.md).

STATIC_ASSERT(sizeof(struct SharedOwnSpawn) == 0xC, SharedOwnSpawnSize);
STATIC_ASSERT(sizeof(struct SharedRemoteSpawn) == 0x18, SharedRemoteSpawnSize);
STATIC_ASSERT(sizeof(struct SharedChannelRow) == 0xC, SharedChannelRowSize);
STATIC_ASSERT(sizeof(struct SharedMapMailbox) == 0x198, SharedMapMailboxSize);
STATIC_ASSERT(SHARED_OWN_SPAWNS == OWE_SPAWNS_MAX, SharedOwnSpawnsMatchOWE);

EWRAM_DATA struct SharedMapMailbox gSharedMapMailbox = {0};
EWRAM_DATA struct SharedMapShowcase gSharedMapShowcase = {0};

#define REQUEST_QUEUE    4
#define CLAIM_TIMEOUT    (2 * 60)  // frames to wait for the server's answer to a claim
#define LIST_TIMEOUT     (3 * 60)
#define HOLD_NEARBY      3         // remote spawns this close stop this game making more
#define HOLD_RANGE_X     8
#define HOLD_RANGE_Y     6
#define RARE_PERCENT     4         // a species at most this common on the map is rare

struct SharedRequest
{
    u8 type;
    u8 arg;
    u16 owner;
    u16 uid;
};

// The wild battle this player is having with a shared spawn.
struct SharedBattle
{
    bool8 active;
    bool8 remote;   // someone else's spawn
    u16 owner;
    u16 uid;
    u16 species;
};

static EWRAM_DATA struct SharedRequest sRequests[REQUEST_QUEUE] = {0};
static EWRAM_DATA u8 sRequestCount = 0;
static EWRAM_DATA u16 sNextUid = 0;
static EWRAM_DATA u16 sOwnGfx[SHARED_OWN_SPAWNS] = {0};       // what the uid in own[] is of
static EWRAM_DATA u8 sOwnHeld[SHARED_OWN_SPAWNS] = {0};       // 1: held for someone, 2: and it had no-despawn before
static EWRAM_DATA u32 sRemoteKeys[SHARED_REMOTE_SPAWNS] = {0}; // owner << 16 | uid of the object in each slot
static EWRAM_DATA struct SharedBattle sBattle = {0};
static EWRAM_DATA u8 sSeenChannelSeq = 0;
static EWRAM_DATA u8 sSeenRareSeq = 0;
static EWRAM_DATA bool8 sChannelToast = FALSE;
static EWRAM_DATA bool8 sRareToast = FALSE;
static EWRAM_DATA u16 sRareOwner = 0;
static EWRAM_DATA u16 sRareUid = 0;
static EWRAM_DATA s8 sTalkedSlot = 0; // the remote slot talked to last

static void ShowIcon(struct ObjectEvent *obj, bool32 star);
static void HideIcons(u8 localId);

static u16 LocalId(void)
{
    return gNetplayMailbox.localId;
}

void SharedMap_Init(void)
{
    static const char magic[8] = {'P', 'K', 'M', 'N', 'S', 'H', 'R', '1'};

    gWorldMailbox.shared = &gSharedMapMailbox;
    if (memcmp(gSharedMapMailbox.magic, magic, sizeof(magic)) == 0)
        return;
    memset(&gSharedMapMailbox, 0, sizeof(gSharedMapMailbox));
    gSharedMapMailbox.version = SHARED_MAP_VERSION;
    memcpy(gSharedMapMailbox.magic, magic, sizeof(magic));
}

// Requests -----------------------------------------------------------------

static void Request(u8 type, u16 owner, u16 uid, u8 arg)
{
    if (sRequestCount == REQUEST_QUEUE)
        return;
    sRequests[sRequestCount].type = type;
    sRequests[sRequestCount].owner = owner;
    sRequests[sRequestCount].uid = uid;
    sRequests[sRequestCount].arg = arg;
    sRequestCount++;
}

static void FlushRequests(void)
{
    struct SharedMapMailbox *box = &gSharedMapMailbox;

    if (!Netplay_IsOnline())
    {
        sRequestCount = 0;
        return;
    }
    if (sRequestCount == 0 || box->reqAck != box->reqSeq)
        return;
    box->reqType = sRequests[0].type;
    box->reqOwner = sRequests[0].owner;
    box->reqUid = sRequests[0].uid;
    box->reqArg = sRequests[0].arg;
    box->reqSeq++;
    sRequestCount--;
    memmove(&sRequests[0], &sRequests[1], sizeof(sRequests[0]) * sRequestCount);
}

// This game's spawns ---------------------------------------------------------

static u8 SpawnFlags(struct ObjectEvent *owe)
{
    u16 species = OW_SPECIES(owe);
    bool32 water = MetatileBehavior_IsWaterWildEncounter(MapGridGetMetatileBehaviorAt(owe->currentCoords.x, owe->currentCoords.y));
    u8 flags = water ? SHARED_SPAWN_WATER : 0;

    if (Alive_WildIsAsleep(owe))
        flags |= SHARED_SPAWN_ASLEEP;
    if (OWE_IsSpecialCategory(owe) || OWE_GetSpeciesRate(species, water) <= RARE_PERCENT)
        flags |= SHARED_SPAWN_RARE;
    return flags;
}

static void HoldOwn(u32 slot, struct ObjectEvent *owe, bool32 held)
{
    if (held == (sOwnHeld[slot] != 0))
        return;
    if (held)
    {
        // Feebas and the like never despawn: remember to keep that.
        sOwnHeld[slot] = (owe->trainerRange_berryTreeId & 0x80) ? 2 : 1;
        OWE_SetHeld(owe, TRUE, FALSE);
        ShowIcon(owe, FALSE);
    }
    else
    {
        OWE_SetHeld(owe, FALSE, sOwnHeld[slot] == 2);
        sOwnHeld[slot] = 0;
        HideIcons(owe->localId);
        if (Alive_WildIsAsleep(owe))
            SetTrainerMovementType(owe, MOVEMENT_TYPE_FACE_DOWN);
    }
}

static void PublishOwn(void)
{
    u32 i;

    for (i = 0; i < SHARED_OWN_SPAWNS; i++)
    {
        struct SharedOwnSpawn *own = &gSharedMapMailbox.own[i];
        struct ObjectEvent *owe = Netplay_IsOnline() ? OWE_GetGeneratedInSlot(i) : NULL;

        if (owe == NULL)
        {
            own->uid = 0;
            own->claim = SHARED_CLAIM_FREE;
            sOwnHeld[i] = 0;
            continue;
        }
        if (own->uid == 0 || sOwnGfx[i] != owe->graphicsId)
        {
            // A new one in this slot.
            if (++sNextUid == 0)
                sNextUid = 1;
            own->claim = SHARED_CLAIM_FREE;
            own->level = OWE_GetLevel(owe);
            own->gfx = owe->graphicsId;
            own->flags = SpawnFlags(owe);
            own->uid = sNextUid;
            sOwnGfx[i] = owe->graphicsId;
            sOwnHeld[i] = 0;
        }
        own->x = owe->currentCoords.x - MAP_OFFSET;
        own->y = owe->currentCoords.y - MAP_OFFSET;
        own->facing = owe->facingDirection;
        if (Alive_WildIsAsleep(owe))
            own->flags |= SHARED_SPAWN_ASLEEP;
        else
            own->flags &= ~SHARED_SPAWN_ASLEEP;

        if (own->claim == SHARED_CLAIM_TAKEN && !(sBattle.active && !sBattle.remote && sBattle.uid == own->uid))
        {
            // Someone else caught or beat it.
            HideIcons(owe->localId);
            RemoveObjectEvent(owe);
            own->uid = 0;
            own->claim = SHARED_CLAIM_FREE;
            sOwnHeld[i] = 0;
            continue;
        }
        HoldOwn(i, owe, own->claim == SHARED_CLAIM_OTHER);
        if (own->claim == SHARED_CLAIM_OTHER && (gMain.vblankCounter1 & 31) == (i * 8))
            ShowIcon(owe, FALSE);
    }
}

static s32 OwnSlotOf(const struct ObjectEvent *owe)
{
    u32 i;

    for (i = 0; i < SHARED_OWN_SPAWNS; i++)
    {
        if (OWE_GetGeneratedInSlot(i) == owe)
            return i;
    }
    return -1;
}

bool32 SharedMap_IsOwnHeld(const struct ObjectEvent *owe)
{
    s32 slot = OwnSlotOf(owe);
    return slot >= 0 && gSharedMapMailbox.own[slot].claim != SHARED_CLAIM_FREE;
}

// Everyone else's ------------------------------------------------------------

static bool32 IsRemoteHere(const struct SharedRemoteSpawn *remote)
{
    return remote->owner != 0 && Netplay_IsOnline()
        && remote->mapGroup == gSaveBlock1Ptr->location.mapGroup
        && remote->mapNum == gSaveBlock1Ptr->location.mapNum
        && (remote->gfx & OBJ_EVENT_MON);
}

static bool32 IsRareRemote(const struct SharedRemoteSpawn *remote)
{
    return (remote->flags & SHARED_SPAWN_RARE) || (remote->gfx & OBJ_EVENT_MON_SHINY);
}

static u32 RemoteKey(const struct SharedRemoteSpawn *remote)
{
    return (remote->owner << 16) | remote->uid;
}

static void RemoveRemote(u32 slot, u8 objId)
{
    if (objId != OBJECT_EVENTS_COUNT)
    {
        HideIcons(gObjectEvents[objId].localId);
        RemoveObjectEvent(&gObjectEvents[objId]);
    }
    sRemoteKeys[slot] = 0;
}

static void SyncRemote(u32 slot)
{
    const struct SharedRemoteSpawn *remote = &gSharedMapMailbox.remote[slot];
    u8 localId = LOCALID_SHARED_WILD_FIRST + slot;
    u8 objId;
    struct ObjectEvent *obj;
    s16 x, y, dx, dy;

    if (!IsRemoteHere(remote) && sRemoteKeys[slot] == 0 && (gMain.vblankCounter1 & 15) != (slot & 15))
        return;
    objId = GetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    // In battle with it: it's on the battle screen, not the map.
    if (!IsRemoteHere(remote) || (sBattle.active && sBattle.remote && RemoteKey(remote) == ((u32)sBattle.owner << 16 | sBattle.uid)))
    {
        RemoveRemote(slot, objId);
        return;
    }
    x = remote->x + MAP_OFFSET;
    y = remote->y + MAP_OFFSET;
    if (objId != OBJECT_EVENTS_COUNT && (sRemoteKeys[slot] != RemoteKey(remote) || gObjectEvents[objId].graphicsId != remote->gfx))
    {
        RemoveRemote(slot, objId);
        objId = OBJECT_EVENTS_COUNT;
    }
    if (objId == OBJECT_EVENTS_COUNT)
    {
        objId = SpawnSpecialObjectEventParameterized(remote->gfx, MOVEMENT_TYPE_NONE, localId, x, y, MapGridGetElevationAt(x, y));
        if (objId == OBJECT_EVENTS_COUNT)
            return; // no free object event or palette
        obj = &gObjectEvents[objId];
        obj->disableCoveringGroundEffects = TRUE;
        sRemoteKeys[slot] = RemoteKey(remote);
        ObjectEventTurn(obj, remote->facing ? remote->facing : DIR_SOUTH);
        if (IsRareRemote(remote))
            ShowIcon(obj, TRUE);
        return;
    }

    obj = &gObjectEvents[objId];
    if ((gMain.vblankCounter1 & 31) == (slot * 4))
    {
        if (remote->claimed)
            ShowIcon(obj, FALSE);
        else if (remote->flags & SHARED_SPAWN_ASLEEP)
            Alive_ShowZzz(obj);
        else if (IsRareRemote(remote))
            ShowIcon(obj, TRUE);
    }
    if (ObjectEventIsHeldMovementActive(obj) && !ObjectEventClearHeldMovementIfFinished(obj))
        return; // still walking to the previous tile
    dx = x - obj->currentCoords.x;
    dy = y - obj->currentCoords.y;
    if (dx == 0 && dy == 0)
    {
        if (remote->facing && obj->facingDirection != remote->facing)
            ObjectEventTurn(obj, remote->facing);
    }
    else if (abs(dx) + abs(dy) == 1)
    {
        enum Direction dir = dx > 0 ? DIR_EAST : dx < 0 ? DIR_WEST : dy > 0 ? DIR_SOUTH : DIR_NORTH;
        ObjectEventSetHeldMovement(obj, GetWalkNormalMovementAction(dir));
    }
    else
    {
        MoveObjectEventToMapCoords(obj, x, y);
        ObjectEventTurn(obj, remote->facing ? remote->facing : DIR_SOUTH);
    }
}

bool32 SharedMap_IsRemoteSpawn(const struct ObjectEvent *obj)
{
    return obj->localId >= LOCALID_SHARED_WILD_FIRST && obj->localId <= LOCALID_SHARED_WILD_LAST;
}

// Too many of other players' Pokémon around already: this game waits.
bool32 SharedMap_HoldLocalSpawns(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 i, near = 0;

    if (gSharedMapShowcase.noSpawns)
        return TRUE;
    if (!Netplay_IsOnline())
        return FALSE;
    for (i = 0; i < SHARED_REMOTE_SPAWNS; i++)
    {
        u8 objId;
        if (sRemoteKeys[i] == 0)
            continue;
        objId = GetObjectEventIdByLocalIdAndMap(LOCALID_SHARED_WILD_FIRST + i, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
        if (objId == OBJECT_EVENTS_COUNT)
            continue;
        if (abs(gObjectEvents[objId].currentCoords.x - player->currentCoords.x) <= HOLD_RANGE_X
         && abs(gObjectEvents[objId].currentCoords.y - player->currentCoords.y) <= HOLD_RANGE_Y)
            near++;
    }
    return near >= HOLD_NEARBY;
}

// Icons: the VS bubble and the rare star --------------------------------------

static const u32 sVsGfx[] =
{
    0x00000000, 0x11111100, 0x22222210, 0x22222210, 0x33233210, 0x33233210, 0x33233210, 0x23332210,
    0x00000000, 0x00111111, 0x01222222, 0x01222222, 0x01223332, 0x01222332, 0x01223332, 0x01223222,
    0x22322210, 0x22222210, 0x21111100, 0x21000000, 0x10000000, 0x00000000, 0x00000000, 0x00000000,
    0x01223332, 0x01222222, 0x00111112, 0x00000122, 0x00000122, 0x00000011, 0x00000000, 0x00000000,
};

static const u32 sStarGfx[] =
{
    0x00000000, 0x10000000, 0x41000000, 0x41000000, 0x44111110, 0x44466410, 0x44444100, 0x44441000,
    0x00000000, 0x00000001, 0x00000016, 0x00000016, 0x01111144, 0x01444444, 0x00144444, 0x00014444,
    0x44410000, 0x54410000, 0x15441000, 0x01541000, 0x00155100, 0x00011100, 0x00000000, 0x00000000,
    0x00001444, 0x00001445, 0x00014451, 0x00014510, 0x00155100, 0x00111000, 0x00000000, 0x00000000,
};

// tools/shared_map/icons.py: 1 outline, 2 white, 3 red, 4 yellow, 5 orange, 6 light yellow.
static const u16 sIconPal[16] =
{
    RGB(31, 0, 31), RGB(6, 7, 12), RGB(31, 31, 31), RGB(29, 6, 5), RGB(31, 26, 6), RGB(27, 15, 3), RGB(31, 31, 22),
};

#define TAG_SHARED_VS   0x5A21
#define TAG_SHARED_STAR 0x5A22
#define TAG_SHARED_PAL  0x5A21

static const struct OamData sIconOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static const struct SpriteSheet sVsSheet = {sVsGfx, sizeof(sVsGfx), TAG_SHARED_VS};
static const struct SpriteSheet sStarSheet = {sStarGfx, sizeof(sStarGfx), TAG_SHARED_STAR};
static const struct SpritePalette sIconPalette = {sIconPal, TAG_SHARED_PAL};

#define sLocalId data[0]
#define sMapNum  data[1]
#define sMapGrp  data[2]
#define sTimer   data[3]
#define sStar    data[4]

static void SpriteCB_Icon(struct Sprite *sprite)
{
    u8 id = GetObjectEventIdByLocalIdAndMap(sprite->sLocalId, sprite->sMapNum, sprite->sMapGrp);
    struct Sprite *owner;

    if (id >= OBJECT_EVENTS_COUNT || gObjectEvents[id].invisible || gMain.callback2 != CB2_Overworld)
    {
        DestroySprite(sprite);
        return;
    }
    owner = &gSprites[gObjectEvents[id].spriteId];
    sprite->sTimer++;
    sprite->x = owner->x + owner->x2;
    sprite->y = owner->y + owner->y2 - 22 + ((sprite->sTimer >> 4) & 1);
    if (sprite->sStar)
    {
        // The star twinkles for a moment every few seconds.
        if (sprite->sTimer >= 90)
        {
            DestroySprite(sprite);
            return;
        }
        sprite->invisible = (sprite->sTimer & 8) != 0 && sprite->sTimer > 60;
    }
}

static void SpriteCB_IconDummy(struct Sprite *sprite)
{
}

static const struct SpriteTemplate sVsTemplate =
{
    .tileTag = TAG_SHARED_VS,
    .paletteTag = TAG_SHARED_PAL,
    .oam = &sIconOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_IconDummy,
};

static const struct SpriteTemplate sStarTemplate =
{
    .tileTag = TAG_SHARED_STAR,
    .paletteTag = TAG_SHARED_PAL,
    .oam = &sIconOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_IconDummy,
};

static void ShowIcon(struct ObjectEvent *obj, bool32 star)
{
    u8 spriteId;
    u32 i;

    if (obj == NULL || !obj->active || gMain.callback2 != CB2_Overworld)
        return;
    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].callback == SpriteCB_Icon && gSprites[i].sLocalId == obj->localId)
        {
            if (gSprites[i].sStar == star)
                return; // already showing
            DestroySprite(&gSprites[i]);
        }
    }
    if (GetSpriteTileStartByTag(star ? TAG_SHARED_STAR : TAG_SHARED_VS) == 0xFFFF)
        LoadSpriteSheet(star ? &sStarSheet : &sVsSheet);
    if (IndexOfSpritePaletteTag(TAG_SHARED_PAL) == 0xFF)
        LoadSpritePalette(&sIconPalette);
    spriteId = CreateSprite(star ? &sStarTemplate : &sVsTemplate, 0, 0, 0);
    if (spriteId == MAX_SPRITES)
        return;
    gSprites[spriteId].sLocalId = obj->localId;
    gSprites[spriteId].sMapNum = obj->mapNum;
    gSprites[spriteId].sMapGrp = obj->mapGroup;
    gSprites[spriteId].sTimer = 0;
    gSprites[spriteId].sStar = star;
    gSprites[spriteId].coordOffsetEnabled = TRUE; // scrolls with the map, like the Pokémon
    gSprites[spriteId].callback = SpriteCB_Icon;
    SpriteCB_Icon(&gSprites[spriteId]);
}

static void HideIcons(u8 localId)
{
    u32 i;

    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].callback == SpriteCB_Icon && gSprites[i].sLocalId == localId)
            DestroySprite(&gSprites[i]);
    }
}

#undef sLocalId
#undef sMapNum
#undef sMapGrp
#undef sTimer
#undef sStar

// Notices ------------------------------------------------------------------------

static const struct SharedRemoteSpawn *FindRemote(u16 owner, u16 uid)
{
    u32 i;

    for (i = 0; i < SHARED_REMOTE_SPAWNS; i++)
    {
        const struct SharedRemoteSpawn *remote = &gSharedMapMailbox.remote[i];
        if (remote->owner == owner && remote->uid == uid && IsRemoteHere(remote))
            return remote;
    }
    return NULL;
}

static const u8 *RemoteOwnerName(u16 owner, u8 *dest)
{
    u32 i;

    for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
    {
        const struct NetplayRemotePlayer *player = Netplay_GetRemoteSlot(i);
        if (player->id == owner)
        {
            memcpy(dest, player->name, NETPLAY_NAME_LENGTH);
            dest[NETPLAY_NAME_LENGTH - 1] = EOS;
            return dest;
        }
    }
    return NULL;
}

static void UpdateNotices(void)
{
    struct SharedMapMailbox *box = &gSharedMapMailbox;

    if (box->channelSeq != sSeenChannelSeq)
    {
        sSeenChannelSeq = box->channelSeq;
        sChannelToast = box->channelCount > 1;
    }
    if (box->rareSeq != sSeenRareSeq)
    {
        const struct SharedRemoteSpawn *waiting = sRareToast ? FindRemote(sRareOwner, sRareUid) : NULL;
        const struct SharedRemoteSpawn *next = FindRemote(box->rareOwner, box->rareUid);

        sSeenRareSeq = box->rareSeq;
        // A shiny still waiting to be shown isn't bumped by a merely rare one.
        if (next != NULL && !(waiting != NULL && (waiting->gfx & OBJ_EVENT_MON_SHINY) && !(next->gfx & OBJ_EVENT_MON_SHINY)))
        {
            sRareOwner = box->rareOwner;
            sRareUid = box->rareUid;
            sRareToast = TRUE;
        }
    }
    if (World_IsToastBusy() || ArePlayerFieldControlsLocked() || gMain.callback2 != CB2_Overworld)
        return;
    if (sChannelToast)
    {
        u8 title[24], *end;

        sChannelToast = FALSE;
        end = StringCopy(title, COMPOUND_STRING("CHANNEL "));
        end = ConvertIntToDecimalStringN(end, box->channel ? box->channel : 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        end = StringCopy(end, COMPOUND_STRING(" OF "));
        ConvertIntToDecimalStringN(end, box->channelCount, STR_CONV_MODE_LEFT_ALIGN, 2);
        World_ShowToast(title, COMPOUND_STRING("Busy map! START > ONLINE > CHANNEL"));
        return;
    }
    if (sRareToast)
    {
        const struct SharedRemoteSpawn *remote = FindRemote(sRareOwner, sRareUid);
        u8 name[NETPLAY_NAME_LENGTH + 1], text[48], *end;

        sRareToast = FALSE;
        if (remote == NULL)
            return;
        end = StringCopy(text, (remote->gfx & OBJ_EVENT_MON_SHINY) ? COMPOUND_STRING("A shiny ") : COMPOUND_STRING("A rare "));
        end = StringCopy(end, GetSpeciesName(remote->gfx & OBJ_EVENT_MON_SPECIES_MASK));
        if (RemoteOwnerName(remote->owner, name) != NULL)
        {
            end = StringCopy(end, COMPOUND_STRING(" near "));
            StringCopy(end, name);
        }
        else
        {
            StringCopy(end, COMPOUND_STRING(" is near!"));
        }
        PlaySE(SE_SHINY);
        World_ShowToast(COMPOUND_STRING("RARE POKéMON!"), text);
    }
}

// Per frame ------------------------------------------------------------------------

void SharedMap_OverworldUpdate(void)
{
    u32 i;

    SharedMap_Init();
    FlushRequests();
    PublishOwn();
    for (i = 0; i < SHARED_REMOTE_SPAWNS; i++)
        SyncRemote(i);
    UpdateNotices();
}

void SharedMap_OnMapTransition(void)
{
    u32 i;

    for (i = 0; i < SHARED_OWN_SPAWNS; i++)
    {
        gSharedMapMailbox.own[i].uid = 0;
        sOwnHeld[i] = 0;
    }
    for (i = 0; i < SHARED_REMOTE_SPAWNS; i++)
    {
        u8 objId = GetObjectEventIdByLocalId(LOCALID_SHARED_WILD_FIRST + i);
        if (objId != OBJECT_EVENTS_COUNT)
            RemoveObjectEvent(&gObjectEvents[objId]);
        sRemoteKeys[i] = 0;
    }
    sChannelToast = FALSE;
}

// Claims (scripts) -----------------------------------------------------------------

#define tSeq   data[0]
#define tTimer data[1]
#define tOwn   data[2]

static void FinishClaim(u8 taskId, u8 result)
{
    struct Task *task = &gTasks[taskId];

    gSpecialVar_Result = result == SHARED_CLAIMED_OK;
    if (result == SHARED_CLAIMED_TAKEN)
    {
        memcpy(gStringVar1, gSharedMapMailbox.claimer, NETPLAY_NAME_LENGTH);
        gStringVar1[NETPLAY_NAME_LENGTH - 1] = EOS;
    }
    else if (result != SHARED_CLAIMED_OK)
    {
        StringCopy(gStringVar1, COMPOUND_STRING("Someone"));
    }
    if (result == SHARED_CLAIMED_OK)
        sBattle.active = TRUE;
    else
        sBattle.active = FALSE;
    gSpecialVar_0x8006 = result;
    (void)task;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_WaitClaim(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (gSharedMapMailbox.claimSeq != (u8)task->tSeq)
        FinishClaim(taskId, gSharedMapMailbox.claimResult);
    else if (++task->tTimer > CLAIM_TIMEOUT || !Netplay_IsOnline())
        // No answer: your own one is yours, someone else's got away.
        FinishClaim(taskId, task->tOwn ? SHARED_CLAIMED_OK : SHARED_CLAIMED_GONE);
}

static void StartClaim(u16 owner, u16 uid, u16 species, bool32 remote)
{
    u8 taskId = CreateTask(Task_WaitClaim, 80);

    sBattle.active = FALSE;
    sBattle.remote = remote;
    sBattle.owner = owner;
    sBattle.uid = uid;
    sBattle.species = species;
    gTasks[taskId].tSeq = gSharedMapMailbox.claimSeq;
    gTasks[taskId].tOwn = !remote;
    Request(SHARED_REQ_CLAIM, owner, uid, 0);
}

// InteractWithOverworldWildEncounter, before anything else (waitstate):
// VAR_RESULT FALSE if someone else is battling it (their name in STR_VAR_1).
void SharedMap_ClaimOwn(void)
{
    u8 objId = GetObjectEventIdByLocalId(gSpecialVar_LastTalked);
    struct ObjectEvent *owe = objId < OBJECT_EVENTS_COUNT ? &gObjectEvents[objId] : NULL;
    s32 slot = owe != NULL ? OwnSlotOf(owe) : -1;
    struct SharedOwnSpawn *own = slot >= 0 ? &gSharedMapMailbox.own[slot] : NULL;

    sBattle.active = FALSE;
    gSpecialVar_Result = TRUE;
    if (own == NULL || own->uid == 0 || !Netplay_IsOnline() || LocalId() == 0)
    {
        ScriptContext_Enable();
        return;
    }
    if (own->claim != SHARED_CLAIM_FREE)
    {
        memcpy(gStringVar1, gSharedMapMailbox.ownClaimer[slot], NETPLAY_NAME_LENGTH);
        gStringVar1[NETPLAY_NAME_LENGTH - 1] = EOS;
        if (gStringVar1[0] == EOS || gStringVar1[0] == 0)
            StringCopy(gStringVar1, COMPOUND_STRING("Someone"));
        StringCopy(gStringVar2, GetSpeciesName(OW_SPECIES(owe)));
        gSpecialVar_Result = FALSE;
        ScriptContext_Enable();
        return;
    }
    StringCopy(gStringVar2, GetSpeciesName(OW_SPECIES(owe)));
    StartClaim(LocalId(), own->uid, OW_SPECIES(owe), FALSE);
}

static const struct SharedRemoteSpawn *TalkedRemote(void)
{
    if (gSpecialVar_LastTalked < LOCALID_SHARED_WILD_FIRST || gSpecialVar_LastTalked > LOCALID_SHARED_WILD_LAST)
        return NULL;
    sTalkedSlot = gSpecialVar_LastTalked - LOCALID_SHARED_WILD_FIRST;
    if (sRemoteKeys[sTalkedSlot] != RemoteKey(&gSharedMapMailbox.remote[sTalkedSlot])
     || !IsRemoteHere(&gSharedMapMailbox.remote[sTalkedSlot]))
        return NULL;
    return &gSharedMapMailbox.remote[sTalkedSlot];
}

// Someone else's Pokémon was talked to. VAR_RESULT: 0 free (species in
// VAR_0x8005), 1 being battled (the battler in STR_VAR_1), 2 gone.
void SharedMap_BufferRemote(void)
{
    const struct SharedRemoteSpawn *remote = TalkedRemote();

    if (remote == NULL)
    {
        gSpecialVar_Result = 2;
        return;
    }
    gSpecialVar_0x8005 = remote->gfx & OBJ_EVENT_MON_SPECIES_MASK;
    StringCopy(gStringVar2, GetSpeciesName(gSpecialVar_0x8005));
    if (remote->claimed)
    {
        memcpy(gStringVar1, remote->claimer, NETPLAY_NAME_LENGTH);
        gStringVar1[NETPLAY_NAME_LENGTH - 1] = EOS;
        gSpecialVar_Result = 1;
        return;
    }
    gSpecialVar_Result = 0;
}

// waitstate: VAR_RESULT TRUE if it's ours to battle.
void SharedMap_ClaimRemote(void)
{
    const struct SharedRemoteSpawn *remote = TalkedRemote();

    if (remote == NULL || !Netplay_IsOnline())
    {
        gSpecialVar_Result = FALSE;
        StringCopy(gStringVar1, COMPOUND_STRING("Someone"));
        ScriptContext_Enable();
        return;
    }
    StartClaim(remote->owner, remote->uid, remote->gfx & OBJ_EVENT_MON_SPECIES_MASK, TRUE);
}

void SharedMap_StartRemoteBattle(void)
{
    const struct SharedRemoteSpawn *remote = sTalkedSlot >= 0 ? &gSharedMapMailbox.remote[sTalkedSlot] : NULL;
    u16 species;
    bool32 shiny;
    u32 personality, level;

    if (remote == NULL || remote->owner != sBattle.owner || remote->uid != sBattle.uid)
    {
        // It went while we waited for the answer.
        sBattle.active = FALSE;
        ScriptContext_Enable();
        return;
    }
    species = remote->gfx & OBJ_EVENT_MON_SPECIES_MASK;
    shiny = (remote->gfx & OBJ_EVENT_MON_SHINY) != 0;
    level = remote->level;
    if (level < MIN_LEVEL || level > MAX_LEVEL)
        level = MIN_LEVEL;
    ZeroEnemyPartyMons();
    personality = GetMonPersonality(species, (remote->gfx & OBJ_EVENT_MON_FEMALE) ? MON_FEMALE : MON_MALE,
                                    NATURE_RANDOM, RANDOM_UNOWN_LETTER);
    CreateMonWithIVs(&gParties[B_TRAINER_OPPONENT_A][0], species, level, personality, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    GiveMonInitialMoveset(&gParties[B_TRAINER_OPPONENT_A][0]);
    SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_IS_SHINY, &shiny);
    if (remote->flags & SHARED_SPAWN_ASLEEP)
    {
        u32 status = STATUS1_SLEEP_TURN(2 + Random() % 3);
        SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_STATUS, &status);
    }
    gSpecialVar_LastTalked = LOCALID_NONE;
    BattleSetup_StartWildBattle();
}

// Back from a wild battle (World_OnWildBattleEnd).
void SharedMap_OnWildBattleEnd(void)
{
    u8 result;

    if (!sBattle.active)
        return;
    sBattle.active = FALSE;
    result = gBattleOutcome == B_OUTCOME_CAUGHT ? SHARED_RESULT_CAUGHT
           : gBattleOutcome == B_OUTCOME_WON ? SHARED_RESULT_BEATEN : SHARED_RESULT_RAN;
    Request(SHARED_REQ_END, sBattle.owner, sBattle.uid, result);
}

bool32 SharedMap_IsWatchableBattle(void)
{
    return sBattle.active && gMain.inBattle && !(gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK));
}

void SharedMap_GetWatchableOpponent(u8 *dest)
{
    StringCopyN(dest, GetSpeciesName(sBattle.species), NETPLAY_SOLO_NAME_LENGTH - 1);
    dest[NETPLAY_SOLO_NAME_LENGTH - 1] = EOS;
}

#undef tSeq
#undef tTimer
#undef tOwn

// START > ONLINE > CHANNEL -----------------------------------------------------------

enum
{
    CHANNELS_WAIT,
    CHANNELS_SHOWN,
    CHANNELS_JOINING,
    CHANNELS_MESSAGE,
};

#define tState    data[0]
#define tWindowId data[1]
#define tSeq      data[2]
#define tTimer    data[3]
#define tCursor   data[4]

static const struct WindowTemplate sChannelsWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 17,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static void DrawChannels(struct Task *task, const u8 *message)
{
    const struct SharedMapMailbox *box = &gSharedMapMailbox;
    u8 windowId = task->tWindowId;
    u8 *end;
    u32 i, mine = box->channel ? box->channel : 1;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    end = StringCopy(gStringVar4, COMPOUND_STRING("CHANNELS: "));
    GetMapName(end, gMapHeader.regionMapSectionId, 0);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 4, 0, TEXT_SKIP_DRAW, NULL);
    if (task->tState == CHANNELS_WAIT)
    {
        AddTextPrinterParameterized(windowId, FONT_NORMAL, COMPOUND_STRING("Loading…"), 12, 20, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        for (i = 0; i < box->listCount && i < SHARED_CHANNEL_ROWS; i++)
        {
            const struct SharedChannelRow *row = &box->rows[i];
            u32 y = 18 + i * 14;

            end = StringCopy(gStringVar4, COMPOUND_STRING("CH "));
            ConvertIntToDecimalStringN(end, row->channel, STR_CONV_MODE_LEFT_ALIGN, 2);
            if (i == task->tCursor)
                AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_SelectorArrow3, 2, y, TEXT_SKIP_DRAW, NULL);
            AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 12, y, TEXT_SKIP_DRAW, NULL);
            end = ConvertIntToDecimalStringN(gStringVar4, row->players, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringCopy(end, row->players == 1 ? COMPOUND_STRING(" player") : COMPOUND_STRING(" players"));
            AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 60, y + 2, TEXT_SKIP_DRAW, NULL);
            if (row->channel == mine)
            {
                AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("YOU ARE HERE"), 150, y + 2, TEXT_SKIP_DRAW, NULL);
            }
            else if (row->friends != 0)
            {
                memcpy(gStringVar1, row->friendName, NETPLAY_NAME_LENGTH);
                gStringVar1[NETPLAY_NAME_LENGTH - 1] = EOS;
                end = StringCopy(gStringVar4, COMPOUND_STRING("FRIEND "));
                end = StringCopy(end, gStringVar1);
                if (row->friends > 1)
                {
                    end = StringCopy(end, COMPOUND_STRING(" +"));
                    ConvertIntToDecimalStringN(end, row->friends - 1, STR_CONV_MODE_LEFT_ALIGN, 2);
                }
                AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 150, y + 2, TEXT_SKIP_DRAW, NULL);
            }
        }
    }
    if (message != NULL)
        AddTextPrinterParameterized(windowId, FONT_NORMAL, message, 8, 104, TEXT_SKIP_DRAW, NULL);
    else if (task->tState == CHANNELS_SHOWN)
        AddTextPrinterParameterized(windowId, FONT_NORMAL, COMPOUND_STRING("Players only meet in their channel."), 8, 104, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("A: move there  B: back"), 4, 122, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void RequestChannels(struct Task *task)
{
    task->tState = CHANNELS_WAIT;
    task->tSeq = gSharedMapMailbox.listSeq;
    task->tTimer = 0;
    Request(SHARED_REQ_CHANNELS, 0, 0, 0);
}

static void Task_Channels(u8 taskId);

void ShowSharedMapChannels(void)
{
    u8 taskId = CreateTask(Task_Channels, 80);
    struct Task *task = &gTasks[taskId];

    task->tWindowId = AddWindow(&sChannelsWindowTemplate);
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PutWindowTilemap(task->tWindowId);
    task->tCursor = 0;
    RequestChannels(task);
    DrawChannels(task, NULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void Task_Channels(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    const struct SharedMapMailbox *box = &gSharedMapMailbox;

    switch (task->tState)
    {
    case CHANNELS_WAIT:
        if (box->listSeq != (u8)task->tSeq)
        {
            u32 i;
            task->tState = CHANNELS_SHOWN;
            task->tCursor = 0;
            for (i = 0; i < box->listCount; i++)
            {
                if (box->rows[i].channel == (box->channel ? box->channel : 1))
                    task->tCursor = i;
            }
            DrawChannels(task, NULL);
        }
        else if (++task->tTimer > LIST_TIMEOUT || !Netplay_IsOnline())
        {
            task->tState = CHANNELS_MESSAGE;
            DrawChannels(task, COMPOUND_STRING("The server didn't answer."));
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            break;
        }
        return;
    case CHANNELS_JOINING:
        if (box->listSeq != (u8)task->tSeq)
        {
            if (box->joinResult == 2)
            {
                task->tState = CHANNELS_MESSAGE;
                DrawChannels(task, COMPOUND_STRING("That channel is full."));
            }
            else
            {
                PlaySE(SE_EXP_MAX);
                RequestChannels(task);
                DrawChannels(task, NULL);
            }
        }
        else if (++task->tTimer > LIST_TIMEOUT || !Netplay_IsOnline())
        {
            task->tState = CHANNELS_MESSAGE;
            DrawChannels(task, COMPOUND_STRING("The server didn't answer."));
        }
        return;
    case CHANNELS_MESSAGE:
        if (JOY_NEW(A_BUTTON) && Netplay_IsOnline())
        {
            PlaySE(SE_SELECT);
            RequestChannels(task);
            DrawChannels(task, NULL);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            break;
        }
        return;
    case CHANNELS_SHOWN:
        if (JOY_NEW(DPAD_UP) && task->tCursor > 0)
        {
            PlaySE(SE_SELECT);
            task->tCursor--;
            DrawChannels(task, NULL);
        }
        else if (JOY_NEW(DPAD_DOWN) && task->tCursor + 1 < box->listCount)
        {
            PlaySE(SE_SELECT);
            task->tCursor++;
            DrawChannels(task, NULL);
        }
        else if (JOY_NEW(A_BUTTON) && task->tCursor < box->listCount)
        {
            PlaySE(SE_SELECT);
            if (box->rows[task->tCursor].channel == (box->channel ? box->channel : 1))
                return;
            task->tState = CHANNELS_JOINING;
            task->tSeq = box->listSeq;
            task->tTimer = 0;
            Request(SHARED_REQ_JOIN, 0, 0, box->rows[task->tCursor].channel);
            DrawChannels(task, COMPOUND_STRING("Moving…"));
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            break;
        }
        return;
    }
    ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
    ClearWindowTilemap(task->tWindowId);
    RemoveWindow(task->tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

#undef tState
#undef tWindowId
#undef tSeq
#undef tTimer
#undef tCursor
