#include "global.h"
#include "battle_setup.h"
#include "community.h"
#include "new_game.h"
#include "event_data.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "reserve.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "world.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/event_objects.h"

// The Haven Reserve (docs/reserve.md), west of Verdanturf: trainers leave
// Pokémon they don't want with the warden at the gate, and those Pokémon
// roam the meadow inside. Online, the reserve is one place shared by every
// player: the server (server/community_reserve.py) keeps the Pokémon left
// there, by players and by the roaming NPC trainers, and hands the game
// the ones roaming now. Anyone can walk up to one and battle it: catch it
// and it's yours, knock it out and it leaves the reserve; either way it's
// gone for everyone. Offline, Pokémon left at the reserve stay in the save
// (SaveBlock3 reserve) and roam there on their own.
//
// Strays (a few wild Pokémon that wander in each day, stronger as the
// player earns badges) fill the empty spots, so the reserve is never empty.

#define ID_LOCAL 0x40000000u // + index in the save's mons[]
#define ID_STRAY 0x80000000u // + stray slot

#define LIST_RECORD_SIZE 12
#define BOX_MON_SIZE     sizeof(struct BoxPokemon)
#define NICK_SIZE        12

// List record flags (server and game).
#define LISTED_MINE   (1 << 0)
#define LISTED_SHINY  (1 << 1)
#define LISTED_FEMALE (1 << 2)

struct ReserveShown
{
    u32 id;
    u32 personality;
    u16 species;
    u8 level;
    u8 flags;
    u8 owner[PLAYER_NAME_LENGTH + 1];
};

static EWRAM_DATA struct ReserveShown sShown[RESERVE_SHOWN] = {0};
static EWRAM_DATA u8 sShownCount = 0;
static EWRAM_DATA u8 sShownLoaded = FALSE;
static EWRAM_DATA u8 sBattling = 0; // index into sShown of the one being battled
static EWRAM_DATA u32 sBattlingId = 0; // and its id

#define sSave (gSaveBlock3Ptr->reserve)

// Strays by badges: two badges per tier.
static const u16 sStraySpecies[4][8] =
{
    {SPECIES_ZIGZAGOON, SPECIES_POOCHYENA, SPECIES_WURMPLE, SPECIES_LOTAD,
     SPECIES_SEEDOT, SPECIES_TAILLOW, SPECIES_WINGULL, SPECIES_RALTS},
    {SPECIES_MAKUHITA, SPECIES_ARON, SPECIES_ELECTRIKE, SPECIES_SHROOMISH,
     SPECIES_MEDITITE, SPECIES_PLUSLE, SPECIES_MINUN, SPECIES_ODDISH},
    {SPECIES_SWABLU, SPECIES_SPINDA, SPECIES_NUMEL, SPECIES_SLUGMA,
     SPECIES_TRAPINCH, SPECIES_CACNEA, SPECIES_ZANGOOSE, SPECIES_SEVIPER},
    {SPECIES_KECLEON, SPECIES_ABSOL, SPECIES_TROPIUS, SPECIES_CHIMECHO,
     SPECIES_BAGON, SPECIES_SPHEAL, SPECIES_SNORUNT, SPECIES_SHUPPET},
};
static const u8 sStrayLevel[4] = {5, 15, 25, 33};

static u32 CountBadges(void)
{
    u32 i, n = 0;

    for (i = 0; i < NUM_BADGES; i++)
        n += FlagGet(FLAG_BADGE01_GET + i) != 0;
    return n;
}

static u32 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 Mix(u32 x)
{
    x ^= x >> 16;
    x *= 0x7FEB352Du;
    x ^= x >> 15;
    x *= 0x846CA68Bu;
    x ^= x >> 16;
    return x;
}

static void ResetStraysIfNewDay(void)
{
    u16 today = Today();

    if (sSave.strayDay != today)
    {
        sSave.strayDay = today;
        sSave.straysGone = 0;
    }
}

static u32 RoamerLocalId(u32 i)
{
    return i < RESERVE_SHOWN_FIRST ? RESERVE_FIRST_LOCALID + i : RESERVE_MORE_LOCALID + i - RESERVE_SHOWN_FIRST;
}

static u32 RoamerHideFlag(u32 i)
{
    return i < RESERVE_SHOWN_FIRST ? FLAG_RESERVE_HIDE_1 + i : FLAG_RESERVE_HIDE_9 + i - RESERVE_SHOWN_FIRST;
}

// Gives spot i's object the look of its Pokémon, in the map's object
// templates (the GBA only had 16 VAR_OBJ_GFX_ID vars for this).
static void SetRoamerGfx(u32 i, u32 gfx)
{
    u32 j, localId = RoamerLocalId(i);

    for (j = 0; j < OBJECT_EVENT_TEMPLATES_COUNT; j++)
    {
        if (gSaveBlock1Ptr->objectEventTemplates[j].localId == localId)
        {
            gSaveBlock1Ptr->objectEventTemplates[j].graphicsId = gfx;
            return;
        }
    }
}

// Puts a Pokémon in the first free spot (a new one, or one whose Pokémon
// left). Returns the spot, or RESERVE_SHOWN if there's no room.
static u32 AddShown(u32 id, u16 species, u8 level, u32 personality, u8 flags, const u8 *owner)
{
    struct ReserveShown *s;
    u32 i;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return RESERVE_SHOWN;
    for (i = 0; i < sShownCount; i++)
    {
        if (sShown[i].id == 0xFFFFFFFF)
            break;
    }
    if (i == sShownCount)
    {
        if (sShownCount >= RESERVE_SHOWN)
            return RESERVE_SHOWN;
        sShownCount++;
    }
    s = &sShown[i];
    s->id = id;
    s->species = species;
    s->level = level;
    s->personality = personality;
    s->flags = flags;
    if (owner != NULL)
        StringCopyN(s->owner, owner, PLAYER_NAME_LENGTH);
    else
        s->owner[0] = EOS;
    s->owner[PLAYER_NAME_LENGTH] = EOS;
    return i;
}

static void StrayFor(u32 slot, u16 *species, u8 *level, u32 *personality)
{
    u32 tier = min(CountBadges() / 2, ARRAY_COUNT(sStrayLevel) - 1);
    u32 day = Mix(sSave.strayDay * 0x9E3779B1u + GetTrainerId(gSaveBlock2Ptr->playerTrainerId));
    u32 seed = Mix(day + slot * 0x85EBCA6Bu);

    // Three apart in a list of eight: never the same species twice a day.
    *species = sStraySpecies[tier][(day + slot * 3) % ARRAY_COUNT(sStraySpecies[0])];
    *level = sStrayLevel[tier] + (seed >> 8) % 5;
    *personality = Mix(seed);
}

// Fills the empty spots with the player's own offline Pokémon, then strays.
static void AddLocalAndStrays(void)
{
    u32 i;

    for (i = 0; i < sSave.count && i < RESERVE_LOCAL_MAX; i++)
    {
        struct ReserveLocalMon *mon = &sSave.mons[i];
        AddShown(ID_LOCAL + i, mon->species, mon->level, mon->personality, LISTED_MINE, gSaveBlock2Ptr->playerName);
    }
    ResetStraysIfNewDay();
    for (i = 0; i < RESERVE_STRAYS; i++)
    {
        u16 species;
        u8 level;
        u32 personality;
        if (sSave.straysGone & (1 << i))
            continue;
        StrayFor(i, &species, &level, &personality);
        AddShown(ID_STRAY + i, species, level, personality, 0, NULL);
    }
}

// Script, waitstate: asks the server who roams the reserve now (the guard
// at the Verdanturf gate calls this before taking the player in).
void Reserve_FetchList(void)
{
    Community_RequestAndWait(COMMUNITY_REQ_RESERVE_LIST, 0, 0, 0, NULL, 0);
}

// Script: after Reserve_FetchList (VAR_RESULT = its status), or offline.
// Builds the list of roaming Pokémon.
void Reserve_BuildList(void)
{
    u32 i, count = 0;

    sShownCount = 0;
    if (gSpecialVar_Result == COMMUNITY_OK)
        count = min(gCommunityMailbox.respArgs[0], gCommunityMailbox.respLen / LIST_RECORD_SIZE);
    for (i = 0; i < count && i < RESERVE_SERVER_SHOWN; i++)
    {
        const u8 *r = gCommunityMailbox.respData + i * LIST_RECORD_SIZE;
        u32 id = r[0] | (r[1] << 8) | (r[2] << 16) | ((u32)r[3] << 24);
        u32 personality = r[4] | (r[5] << 8) | (r[6] << 16) | ((u32)r[7] << 24);
        const u8 *owner = (i + 1 < gCommunityMailbox.rowCount) ? gCommunityMailbox.rows[i + 1] : NULL;
        AddShown(id, r[8] | (r[9] << 8), r[10], personality, r[11], owner);
    }
    AddLocalAndStrays();
    sShownLoaded = TRUE;
}

static u32 GfxFor(const struct ReserveShown *s)
{
    u32 gfx = OBJ_EVENT_MON + s->species;

    if (s->flags & LISTED_SHINY)
        gfx |= OBJ_EVENT_MON_SHINY;
    if (s->flags & LISTED_FEMALE)
        gfx |= OBJ_EVENT_MON_FEMALE;
    return gfx;
}

// The reserve's map loads: one object per roaming Pokémon, the rest hidden.
void Reserve_SetupMap(void)
{
    u32 i;

    if (!sShownLoaded)
    {
        // Came in another way (continued a save here): what's known offline.
        gSpecialVar_Result = COMMUNITY_OFFLINE;
        Reserve_BuildList();
    }
    for (i = 0; i < RESERVE_SHOWN; i++)
    {
        if (i < sShownCount && sShown[i].id != 0xFFFFFFFF)
        {
            SetRoamerGfx(i, GfxFor(&sShown[i]));
            FlagClear(RoamerHideFlag(i));
        }
        else
        {
            FlagSet(RoamerHideFlag(i));
        }
    }
}

// A Pokémon was just added to spot `i` while the player is in the reserve:
// VAR_0x8006 = the object to bring out (addobject), or 0.
static void ShowNew(u32 i)
{
    gSpecialVar_0x8006 = 0;
    if (i >= RESERVE_SHOWN)
        return;
    SetRoamerGfx(i, GfxFor(&sShown[i]));
    FlagClear(RoamerHideFlag(i));
    gSpecialVar_0x8006 = RoamerLocalId(i);
}

static enum Species ShownSpecies(u32 i)
{
    return sShown[i].species;
}

// Script: VAR_0x8004 = which roaming Pokémon (0-19). Its cry; STR_VAR_1 =
// species, STR_VAR_2 = level, STR_VAR_3 = who left it. VAR_RESULT =
// RESERVE_KIND_*, VAR_0x8006 = its object's local id.
void Reserve_DescribeRoamer(void)
{
    u32 i = gSpecialVar_0x8004;
    struct ReserveShown *s;

    gSpecialVar_0x8006 = RoamerLocalId(i);
    if (i >= sShownCount || sShown[i].id == 0xFFFFFFFF)
    {
        gSpecialVar_Result = RESERVE_KIND_NONE;
        return;
    }
    s = &sShown[i];
    sBattling = i;
    sBattlingId = s->id;
    StringCopy(gStringVar1, GetSpeciesName(ShownSpecies(i)));
    ConvertIntToDecimalStringN(gStringVar2, s->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(gStringVar3, s->owner);
    PlayCry_Normal(ShownSpecies(i), 0);
    if (s->id & ID_STRAY)
        gSpecialVar_Result = RESERVE_KIND_STRAY;
    else if (s->id & ID_LOCAL)
        gSpecialVar_Result = RESERVE_KIND_LOCAL;
    else if (s->flags & LISTED_MINE)
        gSpecialVar_Result = RESERVE_KIND_MINE;
    else
        gSpecialVar_Result = RESERVE_KIND_SHARED;
}

// Script, waitstate: the one being battled is on the server; claim it so
// nobody else battles it at the same time. VAR_RESULT: COMMUNITY_OK, or
// RESERVE_GONE if someone got to it first, or COMMUNITY_OFFLINE.
void Reserve_Claim(void)
{
    Community_RequestAndWait(COMMUNITY_REQ_RESERVE_CLAIM, sBattlingId, 0, 0, NULL, 0);
}

static u32 RandomIvs(u32 seed)
{
    u32 i, ivs = 0;

    for (i = 0; i < NUM_STATS; i++)
        ivs |= ((Mix(seed + i) % (MAX_PER_STAT_IVS + 1)) << (i * 5));
    return ivs;
}

static void SetIvs(struct Pokemon *mon, u32 ivs)
{
    u32 i;

    for (i = 0; i < NUM_STATS; i++)
    {
        u32 iv = (ivs >> (i * 5)) & 0x1F;
        SetMonData(mon, MON_DATA_HP_IV + i, &iv);
    }
    CalculateMonStats(mon);
}

// Script, waitstate: starts the battle with the roaming Pokémon VAR_0x8004
// (after Reserve_Claim answered COMMUNITY_OK for a shared one).
void Reserve_StartBattle(void)
{
    struct ReserveShown *s = &sShown[sBattling];
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];

    ZeroEnemyPartyMons();
    if (!(sBattlingId & (ID_LOCAL | ID_STRAY)) && gCommunityMailbox.respLen >= BOX_MON_SIZE)
    {
        // A trainer's own Pokémon, exactly as they left it.
        struct BoxPokemon box;
        memcpy(&box, gCommunityMailbox.respData, BOX_MON_SIZE);
        BoxMonToMon(&box, mon);
        HealPokemon(mon);
    }
    else
    {
        CreateMon(mon, s->species, s->level, s->personality, OTID_STRUCT_RANDOM_NO_SHINY);
        if (s->id & ID_LOCAL)
            SetIvs(mon, sSave.mons[s->id - ID_LOCAL].ivs);
        else
            SetIvs(mon, RandomIvs(s->personality));
        HealPokemon(mon);
    }
    gPartiesCount[B_TRAINER_OPPONENT_A] = 1;
    BattleSetup_StartScriptedWildBattle();
}

static void RemoveLocal(u32 index)
{
    u32 i;

    if (index >= sSave.count)
        return;
    for (i = index; i + 1 < sSave.count; i++)
        sSave.mons[i] = sSave.mons[i + 1];
    sSave.count--;
    memset(&sSave.mons[sSave.count], 0, sizeof(sSave.mons[0]));
    // The others moved down one.
    for (i = 0; i < sShownCount; i++)
    {
        if (sShown[i].id != 0xFFFFFFFF && !(sShown[i].id & ID_STRAY) && (sShown[i].id & ID_LOCAL)
         && sShown[i].id - ID_LOCAL > index)
            sShown[i].id--;
    }
}

// Script: VAR_0x8005 = the battle's outcome (GetBattleOutcome). VAR_RESULT
// = RESERVE_END_*; a Pokémon that's gone leaves the map (the script removes
// object VAR_0x8006). Strays and offline Pokémon are settled here; for a
// shared one, the script then tells the server (Reserve_ReportResult).
void Reserve_AfterBattle(void)
{
    struct ReserveShown *s = &sShown[sBattling];
    u32 end;

    switch (gSpecialVar_0x8005)
    {
    case B_OUTCOME_CAUGHT:
        end = RESERVE_END_CAUGHT;
        sSave.caught++;
        break;
    case B_OUTCOME_WON:
        end = RESERVE_END_KNOCKED_OUT;
        break;
    default:
        end = RESERVE_END_STILL_HERE;
        break;
    }
    gSpecialVar_Result = end;
    gSpecialVar_0x8005 = end;
    gSpecialVar_0x8006 = RoamerLocalId(sBattling);
    StringCopy(gStringVar1, GetSpeciesName(s->species));
    if (end == RESERVE_END_STILL_HERE || s->id != sBattlingId)
        return;
    if (s->id & ID_STRAY)
    {
        ResetStraysIfNewDay();
        sSave.straysGone |= 1 << (s->id - ID_STRAY);
    }
    else if (s->id & ID_LOCAL)
    {
        RemoveLocal(s->id - ID_LOCAL);
    }
    s->id = 0xFFFFFFFF;
    FlagSet(RoamerHideFlag(sBattling));
}

// Script: VAR_RESULT = TRUE if the one just battled is on the server.
void Reserve_IsShared(void)
{
    gSpecialVar_Result = !(sBattlingId & (ID_LOCAL | ID_STRAY));
}

// Script, waitstate: tells the server how the battle ended (VAR_0x8005 =
// RESERVE_END_*, from Reserve_AfterBattle); a caught or knocked-out Pokémon leaves for everyone.
void Reserve_ReportResult(void)
{
    Community_RequestAndWait(COMMUNITY_REQ_RESERVE_RESULT, sBattlingId, gSpecialVar_0x8005, 0, NULL, 0);
}

// Script: the one VAR_0x8004 went (someone else caught it first).
void Reserve_ForgetRoamer(void)
{
    if (sBattling < sShownCount)
        sShown[sBattling].id = 0xFFFFFFFF;
    FlagSet(RoamerHideFlag(sBattling));
}

// --- Leaving a Pokémon --------------------------------------------------------

// Script: VAR_0x8004 = party slot chosen. VAR_RESULT = RESERVE_LEAVE_*.
// STR_VAR_1 = its nickname.
void Reserve_CanLeave(void)
{
    struct Pokemon *party = gParties[B_TRAINER_PLAYER];
    u32 slot = gSpecialVar_0x8004, i, others = 0;

    GetMonData(&party[slot], MON_DATA_NICKNAME, gStringVar1);
    if (GetMonData(&party[slot], MON_DATA_IS_EGG))
    {
        gSpecialVar_Result = RESERVE_LEAVE_EGG;
        return;
    }
    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        if (i != slot && !GetMonData(&party[i], MON_DATA_IS_EGG) && GetMonData(&party[i], MON_DATA_HP) != 0)
            others++;
    }
    gSpecialVar_Result = others == 0 ? RESERVE_LEAVE_LAST : RESERVE_LEAVE_OK;
}

static u8 ListFlags(struct Pokemon *mon)
{
    u8 flags = 0;

    if (GetMonData(mon, MON_DATA_IS_SHINY))
        flags |= LISTED_SHINY;
    if (GetMonGender(mon) == MON_FEMALE)
        flags |= LISTED_FEMALE;
    return flags;
}

// Script, waitstate: sends party slot VAR_0x8004 to the server's reserve.
// It leaves the party only once the server has it (Reserve_FinishLeave).
void Reserve_LeaveOnline(void)
{
    u8 data[BOX_MON_SIZE + NICK_SIZE];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];

    memcpy(data, &mon->box, BOX_MON_SIZE);
    memset(data + BOX_MON_SIZE, EOS, NICK_SIZE);
    GetMonData(mon, MON_DATA_NICKNAME, data + BOX_MON_SIZE);
    data[BOX_MON_SIZE + NICK_SIZE - 1] = EOS;
    Community_RequestAndWait(COMMUNITY_REQ_RESERVE_LEAVE, GetMonData(mon, MON_DATA_SPECIES),
                             GetMonData(mon, MON_DATA_LEVEL), ListFlags(mon), data, sizeof(data));
}

// Script: offline, keeps party slot VAR_0x8004 in the save instead.
// VAR_RESULT = RESERVE_LEAVE_OK or RESERVE_LEAVE_LOCAL_FULL.
void Reserve_LeaveOffline(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    struct ReserveLocalMon *local;
    u32 i, ivs = 0;

    if (sSave.count >= RESERVE_LOCAL_MAX)
    {
        gSpecialVar_Result = RESERVE_LEAVE_LOCAL_FULL;
        return;
    }
    local = &sSave.mons[sSave.count];
    local->species = GetMonData(mon, MON_DATA_SPECIES);
    local->level = GetMonData(mon, MON_DATA_LEVEL);
    local->personality = GetMonData(mon, MON_DATA_PERSONALITY);
    for (i = 0; i < NUM_STATS; i++)
        ivs |= (GetMonData(mon, MON_DATA_HP_IV + i) & 0x1F) << (i * 5);
    local->ivs = ivs;
    // It roams in the first free spot right away.
    ShowNew(AddShown(ID_LOCAL + sSave.count, local->species, local->level, local->personality,
                     LISTED_MINE | ListFlags(mon), gSaveBlock2Ptr->playerName));
    sSave.count++;
    gSpecialVar_Result = RESERVE_LEAVE_OK;
}

// Script: after Reserve_LeaveOnline answered COMMUNITY_OK (its first
// number is the new resident's id): it roams in a free spot right away
// (VAR_0x8006 = the object to bring out, or 0).
void Reserve_ShowLeftOnline(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];

    ShowNew(AddShown(gCommunityMailbox.respArgs[0], GetMonData(mon, MON_DATA_SPECIES), GetMonData(mon, MON_DATA_LEVEL),
                     GetMonData(mon, MON_DATA_PERSONALITY), LISTED_MINE | ListFlags(mon), gSaveBlock2Ptr->playerName));
}

// Script: VAR_RESULT = TRUE if the game is online.
void Reserve_CheckOnline(void)
{
    gSpecialVar_Result = Community_IsOnline();
}

// Script: the Pokémon in party slot VAR_0x8004 is at the reserve now.
void Reserve_FinishLeave(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];

    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    sSave.left++;
}

// --- The notice board ---------------------------------------------------------

// Script, waitstate: the latest news from the server (who left what, who
// caught what).
void Reserve_FetchNews(void)
{
    Community_RequestAndWait(COMMUNITY_REQ_RESERVE_LOG, 0, 0, 0, NULL, 0);
}

static void GetNewsLine(u16 index, u8 *dest)
{
    StringCopyN(dest, gCommunityMailbox.rows[index + 1], COMMUNITY_ROW_LENGTH);
    dest[COMMUNITY_ROW_LENGTH] = EOS;
}

static void GetResidentLine(u16 index, u8 *dest)
{
    struct ReserveShown *s = &sShown[index];

    StringCopy(dest, GetSpeciesName(s->species));
    StringAppend(dest, COMPOUND_STRING("{CLEAR_TO 70}Lv. "));
    ConvertIntToDecimalStringN(dest + StringLength(dest), s->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(dest, COMPOUND_STRING("{CLEAR_TO 110}"));
    if (s->id == 0xFFFFFFFF)
        StringAppend(dest, COMPOUND_STRING("gone"));
    else if (s->id & ID_STRAY)
        StringAppend(dest, COMPOUND_STRING("a stray"));
    else
    {
        StringAppend(dest, COMPOUND_STRING("left by "));
        StringAppend(dest, s->owner);
    }
}

// Script, waitstate: after Reserve_FetchNews. Online: the news; offline:
// who roams the reserve.
void Reserve_ShowBoard(void)
{
    if (gSpecialVar_Result == COMMUNITY_OK && gCommunityMailbox.rowCount > 1)
        WorldBoard_Show(COMPOUND_STRING("HAVEN RESERVE NEWS"), gCommunityMailbox.rowCount - 1, GetNewsLine, NULL);
    else
        WorldBoard_Show(COMPOUND_STRING("ROAMING THE RESERVE"), sShownCount, GetResidentLine, NULL);
}

// For the screenshot runs (tools/reserve_shots): forget the list so the
// next fetch or map load builds it again.
void Reserve_ForgetList(void)
{
    sShownLoaded = FALSE;
    sShownCount = 0;
}
