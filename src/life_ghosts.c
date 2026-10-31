#include "global.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "life.h"
#include "metatile_behavior.h"
#include "netplay_ext.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "world.h"
#include "constants/battle_ai.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/map_types.h"
#include "constants/songs.h"
#include "constants/trainer_types.h"
#include "constants/trainers.h"

// Ghost trainers. While online, the game keeps the player's team on the
// server (server/life_ghosts.py) and now and then, on entering a route,
// asks for another player's. That team shows up as a see-through trainer
// nearby; talk to it to battle it. How it went is sent back, and the
// team's owner hears how their ghost did the next time their team uploads.
//
// The requests run in the background over the Online Plaza's channel
// (netplay_ext.c), so they never hold up the player.

#define UPLOAD_FRAMES   (60 * 60 * 10) // send the team again after 10 minutes
#define CHECK_FRAMES    (60 * 5)       // or sooner, if the team has changed
#define COOLDOWN_FRAMES (60 * 60 * 3)  // no new ghost for 3 minutes after one
#define GHOST_CHANCE    4              // 1 in this many routes has a ghost
#define LEVEL_ALLOWANCE 5              // ghosts are at most this far above your best

enum
{
    REQ_NONE,
    REQ_PUT,
    REQ_GET,
    REQ_RESULT,
};

struct GhostState
{
    u32 uploadTimer;
    u32 cooldown;
    u32 ghostId;      // the server's id of the ghost on this map, 0 if none
    u32 resultId;     // a battle result waiting to be sent
    u32 teamHash;     // of the team last sent
    u8 resultWon;
    u8 waiting;       // REQ_* sent and not answered yet
    bool8 wantGet;
    bool8 uploaded;
    u8 mapGroup;      // where the GET was asked from
    u8 mapNum;
    u8 gender;
    u8 count;
    u16 wins;
    u16 losses;
    u8 name[PLAYER_NAME_LENGTH + 1];
    struct LifeGhostMon mons[PARTY_SIZE];
};

static EWRAM_DATA struct GhostState sGhost = {0};
static EWRAM_DATA struct TrainerMon sGhostParty[PARTY_SIZE] = {0};
static EWRAM_DATA u8 sGhostEvs[PARTY_SIZE][NUM_STATS] = {0};
static EWRAM_DATA u8 sGhostNicknames[PARTY_SIZE][POKEMON_NAME_LENGTH + 1] = {0};
static EWRAM_DATA struct Trainer sGhostTrainer = {0};

// Uploading the team -------------------------------------------------------

static void PackMon(struct LifeGhostMon *out, struct Pokemon *mon)
{
    static const u8 sEvOrder[NUM_STATS] = {STAT_HP, STAT_ATK, STAT_DEF, STAT_SPATK, STAT_SPDEF, STAT_SPEED};
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u32 i, gender = GetMonGender(mon);

    memset(out, 0, sizeof(*out));
    out->species = GetMonData(mon, MON_DATA_SPECIES);
    out->heldItem = GetMonData(mon, MON_DATA_HELD_ITEM);
    for (i = 0; i < MAX_MON_MOVES; i++)
        out->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
    out->ability = GetMonAbility(mon);
    out->level = GetMonData(mon, MON_DATA_LEVEL);
    out->nature = GetNature(mon);
    for (i = 0; i < NUM_STATS; i++)
    {
        out->ivs[i] = GetMonData(mon, MON_DATA_HP_IV + i);
        out->evs[i] = GetMonData(mon, MON_DATA_HP_EV + sEvOrder[i]);
    }
    out->shiny = GetMonData(mon, MON_DATA_IS_SHINY);
    out->gender = gender == MON_MALE ? TRAINER_MON_MALE : gender == MON_FEMALE ? TRAINER_MON_FEMALE : TRAINER_MON_RANDOM_GENDER;
    out->friendship = GetMonData(mon, MON_DATA_FRIENDSHIP);
    GetMonData(mon, MON_DATA_NICKNAME, nickname);
    memcpy(out->nickname, nickname, POKEMON_NAME_LENGTH);
}

// Changes when a Pokémon joins, leaves or levels up.
static u32 TeamHash(void)
{
    u32 i, hash = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        hash = hash * 31 + GetMonData(mon, MON_DATA_SPECIES);
        hash = hash * 31 + GetMonData(mon, MON_DATA_LEVEL);
    }
    return hash;
}

static bool32 SendPut(void)
{
    struct LifeGhostMon mon;
    u32 i, count = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *p = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(p, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(p, MON_DATA_IS_EGG))
            count++;
    }
    if (count == 0 || !NpxRpc_Begin("GHOST"))
        return FALSE;
    NpxRpc_Word("PUT");
    NpxRpc_Num(gSaveBlock2Ptr->playerGender);
    NpxRpc_Num(count);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *p = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(p, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(p, MON_DATA_IS_EGG))
            continue;
        PackMon(&mon, p);
        NpxRpc_Data(&mon, sizeof(mon));
    }
    NpxRpc_Send();
    sGhost.teamHash = TeamHash();
    return TRUE;
}

// Reading answers ----------------------------------------------------------

// The i-th '|'-separated number of the answer's first line.
static s32 AnswerNum(u32 index)
{
    const u8 *resp = gNetplayExtMailbox.resp;
    u32 pos = 0, len = min(gNetplayExtMailbox.respLen, NETPLAY_EXT_RESP_SIZE);
    s32 n = 0;

    while (index != 0 && pos < len && resp[pos] != 0 && resp[pos] != '\n')
    {
        if (resp[pos++] == '|')
            index--;
    }
    while (pos < len && resp[pos] >= '0' && resp[pos] <= '9')
        n = n * 10 + resp[pos++] - '0';
    return n;
}

static void AnswerText(u8 *dest, u32 index, u32 maxChars)
{
    const u8 *resp = gNetplayExtMailbox.resp;
    char field[PLAYER_NAME_LENGTH + 1];
    u32 pos = 0, n = 0, len = min(gNetplayExtMailbox.respLen, NETPLAY_EXT_RESP_SIZE);

    while (index != 0 && pos < len && resp[pos] != 0)
    {
        if (resp[pos++] == '|')
            index--;
    }
    while (pos < len && resp[pos] != '|' && resp[pos] != 0 && resp[pos] != '\n' && n < ARRAY_COUNT(field) - 1)
        field[n++] = resp[pos++];
    field[n] = 0;
    Npx_AsciiToGame(dest, field, maxChars);
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

static void AfterPut(void)
{
    u32 wins = AnswerNum(0), losses = AnswerNum(1);
    u8 text[48];

    sGhost.uploaded = TRUE;
    if (wins + losses == 0)
        return;
    ConvertIntToDecimalStringN(gStringVar1, wins, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar2, losses, STR_CONV_MODE_LEFT_ALIGN, 4);
    StringExpandPlaceholders(text, COMPOUND_STRING("Won {STR_VAR_1}, lost {STR_VAR_2} while you were away"));
    World_ShowToast(COMPOUND_STRING("YOUR GHOST TEAM"), text);
}

// Meeting a ghost ------------------------------------------------------------

static bool32 OnGhostMap(void)
{
    return sGhost.mapGroup == gSaveBlock1Ptr->location.mapGroup && sGhost.mapNum == gSaveBlock1Ptr->location.mapNum;
}

static u8 GhostObjectId(void)
{
    return GetObjectEventIdByLocalIdAndMap(LOCALID_LIFE_GHOST, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

bool32 LifeGhost_IsGhost(struct ObjectEvent *objectEvent)
{
    return objectEvent->localId == LOCALID_LIFE_GHOST && sGhost.ghostId != 0 && OnGhostMap();
}

static bool32 CanStandAt(s16 x, s16 y)
{
    u32 behavior;

    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    return !MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) && !MetatileBehavior_IsWarpDoor(behavior)
        && !MetatileBehavior_IsLongGrass(behavior)
        && MapGridGetElevationAt(x, y) == gObjectEvents[gPlayerAvatar.objectEventId].currentElevation;
}

// Puts the ghost a few steps from the player, facing them.
static bool32 SpawnGhost(void)
{
    static const u8 sFace[] =
    {
        [DIR_SOUTH] = MOVEMENT_TYPE_FACE_UP, [DIR_NORTH] = MOVEMENT_TYPE_FACE_DOWN,
        [DIR_WEST] = MOVEMENT_TYPE_FACE_RIGHT, [DIR_EAST] = MOVEMENT_TYPE_FACE_LEFT,
    };
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEventTemplate template = {0};
    u32 dir = player->facingDirection, tries;
    s16 x = 0, y = 0;

    // First choice: three steps ahead; otherwise anywhere close by.
    for (tries = 0; tries < 24; tries++)
    {
        if (tries == 0)
        {
            x = player->currentCoords.x + (dir == DIR_WEST ? -3 : dir == DIR_EAST ? 3 : 0);
            y = player->currentCoords.y + (dir == DIR_NORTH ? -3 : dir == DIR_SOUTH ? 3 : 0);
        }
        else
        {
            x = player->currentCoords.x - 5 + Random() % 11;
            y = player->currentCoords.y - 4 + Random() % 9;
        }
        if (abs(x - player->currentCoords.x) + abs(y - player->currentCoords.y) >= 3 && CanStandAt(x, y))
            break;
    }
    if (tries == 24)
        return FALSE;
    template.localId = LOCALID_LIFE_GHOST;
    template.graphicsId = sGhost.gender == FEMALE ? OBJ_EVENT_GFX_MAY_NORMAL : OBJ_EVENT_GFX_BRENDAN_NORMAL;
    template.kind = OBJ_KIND_NORMAL;
    template.x = x - MAP_OFFSET;
    template.y = y - MAP_OFFSET;
    template.elevation = MapGridGetElevationAt(x, y);
    template.movementType = tries == 0 ? sFace[dir] : MOVEMENT_TYPE_LOOK_AROUND;
    template.trainerType = TRAINER_TYPE_NONE;
    SpawnSpecialObjectEvent(&template);
    PlaySE(SE_M_CONFUSE_RAY);
    return TRUE;
}

static void AfterGet(void)
{
    u32 size, i;
    const u8 *party = AnswerBinary(&size);

    sGhost.ghostId = 0;
    if (party == NULL || !OnGhostMap())
        return;
    sGhost.count = min(AnswerNum(5), PARTY_SIZE);
    if (sGhost.count == 0 || size < sGhost.count * sizeof(struct LifeGhostMon))
        return;
    AnswerText(sGhost.name, 1, PLAYER_NAME_LENGTH);
    sGhost.gender = AnswerNum(2) ? FEMALE : MALE;
    sGhost.wins = AnswerNum(3);
    sGhost.losses = AnswerNum(4);
    memcpy(sGhost.mons, party, sGhost.count * sizeof(struct LifeGhostMon));
    for (i = 0; i < sGhost.count; i++)
    {
        if (sGhost.mons[i].species == SPECIES_NONE || sGhost.mons[i].species >= NUM_SPECIES)
            return;
    }
    sGhost.ghostId = AnswerNum(0);
    if (!SpawnGhost())
        sGhost.ghostId = 0;
    else
        sGhost.cooldown = COOLDOWN_FRAMES;
}

// The battle -----------------------------------------------------------------

static u32 BestPartyLevel(void)
{
    u32 i, best = 1;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG))
            best = max(best, GetMonData(mon, MON_DATA_LEVEL));
    }
    return best;
}

static void BuildTrainer(void)
{
    u32 i, j, cap = min(BestPartyLevel() + LEVEL_ALLOWANCE, MAX_LEVEL);

    memset(sGhostParty, 0, sizeof(sGhostParty));
    for (i = 0; i < sGhost.count; i++)
    {
        const struct LifeGhostMon *g = &sGhost.mons[i];
        struct TrainerMon *mon = &sGhostParty[i];
        const u8 *iv = g->ivs;

        mon->species = g->species;
        mon->lvl = min(max(g->level, 1), cap);
        mon->heldItem = g->heldItem < ITEMS_COUNT ? g->heldItem : ITEM_NONE;
        for (j = 0; j < MAX_MON_MOVES; j++)
            mon->moves[j] = g->moves[j] < MOVES_COUNT ? g->moves[j] : MOVE_NONE;
        mon->ability = g->ability;
        mon->nature = g->nature % NUM_NATURES;
        mon->iv = TRAINER_PARTY_IVS(min(iv[0], 31), min(iv[1], 31), min(iv[2], 31), min(iv[3], 31), min(iv[4], 31), min(iv[5], 31));
        memcpy(sGhostEvs[i], g->evs, NUM_STATS);
        mon->ev = sGhostEvs[i];
        memcpy(sGhostNicknames[i], g->nickname, POKEMON_NAME_LENGTH);
        sGhostNicknames[i][POKEMON_NAME_LENGTH] = EOS;
        mon->nickname = sGhostNicknames[i];
        mon->isShiny = g->shiny != 0;
        mon->gender = (g->gender >= TRAINER_MON_MALE && g->gender <= TRAINER_MON_RANDOM_GENDER) ? g->gender : TRAINER_MON_RANDOM_GENDER;
        mon->friendship = g->friendship;
        mon->ball = POKEBALL_COUNT;
        mon->dynamaxLevel = MAX_DYNAMAX_LEVEL;
    }
    memset(&sGhostTrainer, 0, sizeof(sGhostTrainer));
    sGhostTrainer.aiFlags = AI_FLAG_BASIC_TRAINER;
    sGhostTrainer.party = sGhostParty;
    sGhostTrainer.partySize = sGhost.count;
    sGhostTrainer.trainerClass = TRAINER_CLASS_LIFE_GHOST;
    sGhostTrainer.encounterMusic = sGhost.gender == FEMALE ? TRAINER_ENCOUNTER_MUSIC_FEMALE : TRAINER_ENCOUNTER_MUSIC_MALE;
    sGhostTrainer.gender = sGhost.gender == FEMALE ? TRAINER_GENDER_FEMALE : TRAINER_GENDER_MALE;
    sGhostTrainer.trainerPic = sGhost.gender == FEMALE ? TRAINER_PIC_MAY : TRAINER_PIC_BRENDAN;
    StringCopyN(sGhostTrainer.trainerName, sGhost.name, TRAINER_NAME_LENGTH);
    sGhostTrainer.trainerName[TRAINER_NAME_LENGTH] = EOS;
}

// For the talk script: gStringVar1 the owner, gStringVar2 the ghost's
// wins, gStringVar3 its losses, gStringVar4... left alone.
void LifeGhost_BufferIntro(void)
{
    StringCopy(gStringVar1, sGhost.name);
    ConvertIntToDecimalStringN(gStringVar2, sGhost.wins, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar3, sGhost.losses, STR_CONV_MODE_LEFT_ALIGN, 4);
}

// Use with waitstate. VAR_RESULT: TRUE if the player won.
void LifeGhost_StartBattle(void)
{
    BuildTrainer();
    LifeBattle_Start(&sGhostTrainer, COMPOUND_STRING("The ghost team fades with a smile…"));
}

void LifeGhost_OnBattleEnd(const struct Trainer *trainer, bool32 playerWon)
{
    if (trainer != &sGhostTrainer || sGhost.ghostId == 0)
        return;
    sGhost.resultId = sGhost.ghostId;
    sGhost.resultWon = playerWon;
    sGhost.ghostId = 0; // it's gone from the map after the battle
}

// Walking away from a ghost without battling it.
void LifeGhost_Dismiss(void)
{
    RemoveObjectEventByLocalIdAndMap(LOCALID_LIFE_GHOST, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    sGhost.ghostId = 0;
}

// Overworld --------------------------------------------------------------

void LifeGhost_OnMapTransition(void)
{
    sGhost.ghostId = 0;
    if (!Npx_IsOnline() || sGhost.cooldown != 0 || gMapHeader.mapType != MAP_TYPE_ROUTE)
        return;
    if (LifePhoto_IsActive() || (Random() % GHOST_CHANCE) != 0)
        return;
    sGhost.wantGet = TRUE;
}

static bool32 SendNext(void)
{
    if (!sGhost.uploaded || sGhost.uploadTimer >= UPLOAD_FRAMES
     || (sGhost.uploadTimer % CHECK_FRAMES == 0 && TeamHash() != sGhost.teamHash))
    {
        if (SendPut())
        {
            sGhost.uploadTimer = 0;
            sGhost.waiting = REQ_PUT;
            return TRUE;
        }
        return FALSE;
    }
    if (sGhost.resultId != 0)
    {
        if (!NpxRpc_Begin("GHOST"))
            return FALSE;
        NpxRpc_Word("RESULT");
        NpxRpc_Num(sGhost.resultId);
        NpxRpc_Num(sGhost.resultWon);
        NpxRpc_Send();
        sGhost.resultId = 0;
        sGhost.waiting = REQ_RESULT;
        return TRUE;
    }
    if (sGhost.wantGet)
    {
        if (!NpxRpc_Begin("GHOST"))
            return FALSE;
        NpxRpc_Word("GET");
        NpxRpc_Send();
        sGhost.wantGet = FALSE;
        sGhost.mapGroup = gSaveBlock1Ptr->location.mapGroup;
        sGhost.mapNum = gSaveBlock1Ptr->location.mapNum;
        sGhost.waiting = REQ_GET;
        return TRUE;
    }
    return FALSE;
}

// Called every overworld frame.
void LifeGhost_Update(void)
{
    u8 objectId;

    if (sGhost.uploadTimer < UPLOAD_FRAMES)
        sGhost.uploadTimer++;
    if (sGhost.cooldown != 0)
        sGhost.cooldown--;
    if (gLifeShowcase.ghostNow)
    {
        gLifeShowcase.ghostNow = FALSE;
        sGhost.cooldown = 0;
        sGhost.wantGet = TRUE;
    }

    if (sGhost.waiting != REQ_NONE)
    {
        u8 state = NpxRpc_Poll();
        if (state == NPX_RPC_WAITING)
            return;
        if (state == NPX_RPC_DONE && gNetplayExtMailbox.status == 0)
        {
            if (sGhost.waiting == REQ_PUT)
                AfterPut();
            else if (sGhost.waiting == REQ_GET)
                AfterGet();
        }
        else if (sGhost.waiting == REQ_PUT)
        {
            sGhost.uploaded = TRUE; // try again on the next timer
        }
        sGhost.waiting = REQ_NONE;
        NpxRpc_Release();
    }
    else if (Npx_IsOnline() && !ArePlayerFieldControlsLocked())
    {
        SendNext();
    }

    // The ghost is see-through.
    objectId = GhostObjectId();
    if (objectId != OBJECT_EVENTS_COUNT && sGhost.ghostId != 0)
        gSprites[gObjectEvents[objectId].spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
}
