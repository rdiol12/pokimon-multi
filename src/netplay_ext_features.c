#include "global.h"
#include "malloc.h"
#include "netplay.h"
#include "netplay_ext.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "item.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "money.h"
#include "naming_screen.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "recorded_battle.h"
#include "save.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "secret_base.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/secret_bases.h"
#include "constants/species.h"
#include "constants/trainers.h"

// The ten features of the second online batch: what each request sends and
// what the game does with the answer. The event scripts that tie them
// together are in data/scripts/netplay_ext.inc; the request channel, boards
// and pick lists in src/netplay_ext.c. See docs/netplay_ext.md.

#define REPLAY_UP_PART   512
#define REPLAY_DOWN_PART 1024
#define REPLAY_RETRY_FRAMES (60 * 10)
#define DRAFT_TEAM_SIZE  3
#define DRAFT_POOL_SIZE  6
#define DRAFT_LEVEL      50
#define DRAFT_WAIT_FRAMES (60 * 120)

void Npx_ChooseAnswerRow(u32 row);
void Npx_SetChosen(u32 col, const char *value);
void Npx_SetMyBadge(u8 badge);

static EWRAM_DATA u8 sGuildTag[8] = {0};
static EWRAM_DATA u8 sGuildName[16] = {0};
static EWRAM_DATA struct SecretBase sStoredBase = {0}; // a base or tower team from the server
static EWRAM_DATA bool8 sBattleWithStoredBase = FALSE;
// The replay itself is recorded_battle.c's (RecordedBattle_BuildLinkReplay);
// sReplayStamp tells if it has changed since.
static EWRAM_DATA const struct RecordedBattleSave *sReplay = NULL;
static EWRAM_DATA u8 sReplayStamp = 0;
static EWRAM_DATA u16 sReplaySize = 0;
// A downloaded replay, on the heap from the download until it plays.
static EWRAM_DATA struct RecordedBattleSave *sDownload = NULL;
static EWRAM_DATA bool8 sReplayPending = FALSE;
static EWRAM_DATA u8 sReplayPart = 0;
static EWRAM_DATA u8 sReplayTries = 0;
static EWRAM_DATA bool8 sReplayUploading = FALSE;
static EWRAM_DATA u16 sReplayTimer = 0;
static EWRAM_DATA u8 sReplayOpponent[PLAYER_NAME_LENGTH + 1] = {0};
static EWRAM_DATA u8 sReplayResult = 0;
static EWRAM_DATA u16 sDraftPool[DRAFT_POOL_SIZE] = {0};
static EWRAM_DATA u16 sDraftPicks[DRAFT_TEAM_SIZE] = {0};
static EWRAM_DATA u8 sDraftPickCount = 0;
static EWRAM_DATA struct Pokemon sDraftBackup[PARTY_SIZE] = {0};
static EWRAM_DATA bool8 sDraftActive = FALSE;
static EWRAM_DATA bool8 sDraftSawSession = FALSE;
static EWRAM_DATA u16 sListValues[NPX_MAX_ROWS] = {0}; // what the rows of a list the game made stand for

// Fully evolved Pokémon the daily draft pool is drawn from. The server only
// sends indexes into this list, so changing it changes everyone's pool.
static const u16 sDraftSpecies[] =
{
    SPECIES_VENUSAUR, SPECIES_CHARIZARD, SPECIES_BLASTOISE, SPECIES_RAICHU,
    SPECIES_ALAKAZAM, SPECIES_MACHAMP, SPECIES_GENGAR, SPECIES_GYARADOS,
    SPECIES_LAPRAS, SPECIES_SNORLAX, SPECIES_DRAGONITE, SPECIES_AMPHAROS,
    SPECIES_SCIZOR, SPECIES_HERACROSS, SPECIES_SKARMORY, SPECIES_TYRANITAR,
    SPECIES_SCEPTILE, SPECIES_BLAZIKEN, SPECIES_SWAMPERT, SPECIES_GARDEVOIR,
    SPECIES_AGGRON, SPECIES_MANECTRIC, SPECIES_FLYGON, SPECIES_ALTARIA,
    SPECIES_MILOTIC, SPECIES_ABSOL, SPECIES_SALAMENCE, SPECIES_METAGROSS,
    SPECIES_LUCARIO, SPECIES_GARCHOMP, SPECIES_TOGEKISS, SPECIES_WEAVILE,
    SPECIES_MAMOSWINE, SPECIES_EXCADRILL, SPECIES_HYDREIGON, SPECIES_VOLCARONA,
    SPECIES_SYLVEON, SPECIES_DRAGAPULT, SPECIES_CORVIKNIGHT, SPECIES_KINGAMBIT,
};

static const u8 *const sMailPhrases[] =
{
    COMPOUND_STRING("Thanks for the battle!"),
    COMPOUND_STRING("Here's a little gift!"),
    COMPOUND_STRING("Good luck out there!"),
    COMPOUND_STRING("Let's trade sometime!"),
    COMPOUND_STRING("Want to team up?"),
    COMPOUND_STRING("Happy birthday!"),
    COMPOUND_STRING("See you at the Tower!"),
    COMPOUND_STRING("You're my rival now!"),
};

// Helpers ----------------------------------------------------------------

static bool32 AsciiEquals(const char *a, const char *b)
{
    while (*a != 0 && *a == *b)
        a++, b++;
    return *a == *b;
}

static void SaveAfterOnlineChange(void)
{
    TrySavingData(SAVE_NORMAL);
}

static void BufferNum(u8 *dest, s32 n)
{
    ConvertIntToDecimalStringN(dest, n, STR_CONV_MODE_LEFT_ALIGN, 7);
}

static void RemovePartyMon(u32 slot)
{
    ZeroMonData(&gParties[B_TRAINER_PLAYER][slot]);
    CompactPartySlots();
    CalculatePlayerPartyCount();
}

// Adds a Pokémon that came from another player: to the party, or the PC if
// the party is full. gStringVar2: its species name.
static void ReceiveMon(const u8 *data)
{
    struct Pokemon mon;
    u16 species;

    memcpy(&mon, data, sizeof(mon));
    mon.mail = MAIL_NONE; // their mail slots aren't ours
    species = GetMonData(&mon, MON_DATA_SPECIES);
    if (!GetMonData(&mon, MON_DATA_IS_EGG))
    {
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_SEEN);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_CAUGHT);
    }
    CalculatePlayerPartyCount();
    if (gPartiesCount[B_TRAINER_PLAYER] < PARTY_SIZE)
    {
        gParties[B_TRAINER_PLAYER][gPartiesCount[B_TRAINER_PLAYER]] = mon;
        CalculatePlayerPartyCount();
    }
    else
    {
        CopyMonToPC(&mon);
    }
    StringCopy(gStringVar2, GetSpeciesName(species));
}

// Answers that hand over a Pokémon: "from|species", a NUL, then the mon.
// VAR_0x8005: TRUE if one came. gStringVar1: who it came from.
static bool32 TryReceiveMonAnswer(void)
{
    u32 size;
    const u8 *mon = Npx_Binary(&size);

    gSpecialVar_0x8005 = FALSE;
    if (mon == NULL || size < sizeof(struct Pokemon))
        return FALSE;
    Npx_CopyField(gStringVar1, 0, 0, PLAYER_NAME_LENGTH);
    ReceiveMon(mon);
    gSpecialVar_0x8005 = TRUE;
    return TRUE;
}

static void FillTeamParty(struct SecretBaseParty *party)
{
    u32 i, j, n = 0;

    memset(party, 0, sizeof(*party));
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 ev = 0;

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        for (j = 0; j < MAX_MON_MOVES; j++)
            party->moves[n * MAX_MON_MOVES + j] = GetMonData(mon, MON_DATA_MOVE1 + j);
        for (j = 0; j < NUM_STATS; j++)
            ev += GetMonData(mon, MON_DATA_HP_EV + j);
        party->species[n] = GetMonData(mon, MON_DATA_SPECIES);
        party->heldItems[n] = GetMonData(mon, MON_DATA_HELD_ITEM);
        party->levels[n] = GetMonData(mon, MON_DATA_LEVEL);
        party->personality[n] = GetMonData(mon, MON_DATA_PERSONALITY);
        party->EVs[n] = ev / NUM_STATS;
        n++;
    }
}

// The player's current team as a secret base record: what others meet in
// the Battle Tower.
static void BuildTeamRecord(struct SecretBase *team)
{
    memset(team, 0, sizeof(*team));
    StringCopyN(team->trainerName, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    memcpy(team->trainerId, gSaveBlock2Ptr->playerTrainerId, TRAINER_ID_LENGTH);
    team->gender = gSaveBlock2Ptr->playerGender;
    team->language = GAME_LANGUAGE;
    FillTeamParty(&team->party);
}

static bool32 StoreBaseAnswer(void)
{
    u32 size;
    const u8 *data = Npx_Binary(&size);

    if (data == NULL || size != sizeof(struct SecretBase))
        return FALSE;
    memcpy(&sStoredBase, data, sizeof(sStoredBase));
    return TRUE;
}

// For other features' requests (guild base raids, src/online_guilds.c): the
// player's team goes with the request, and a team in the answer is stored
// for NetplayExt_PrepStoredBattle.
void NetplayExt_AddTeamRecord(void)
{
    struct SecretBase team;

    BuildTeamRecord(&team);
    NpxRpc_Data(&team, sizeof(team));
}

bool32 NetplayExt_StoreBattleTeam(void)
{
    return StoreBaseAnswer();
}

// Guilds -----------------------------------------------------------------

static void BuildGuildInfo(void)   { NpxRpc_Word("INFO"); }
static void BuildGuildList(void)   { NpxRpc_Word("LIST"); }
static void BuildGuildLeave(void)  { NpxRpc_Word("LEAVE"); }

static void BuildGuildCreate(void)
{
    NpxRpc_Word("CREATE");
    NpxRpc_Name(sGuildTag);
    NpxRpc_Name(sGuildName);
}

static void BuildGuildJoin(void)
{
    NpxRpc_Word("JOIN");
    NpxRpc_Word(Npx_Chosen(1));
}

// gStringVar1: tag, gStringVar2: name.
static void AfterGuildNamed(void)
{
    Npx_CopyField(gStringVar1, 0, 0, 4);
    Npx_CopyField(gStringVar2, 0, 1, 12);
}

static void CB2_ReturnFromGuildNaming(void)
{
    CB2_ReturnToFieldContinueScript();
}

// VAR_0x8004: 0 the guild's tag, 1 its name. VAR_RESULT: FALSE if left empty.
void NetplayExt_NameGuild(void)
{
    u8 *buffer = gSpecialVar_0x8004 == 0 ? sGuildTag : sGuildName;

    buffer[0] = EOS;
    DoNamingScreen(gSpecialVar_0x8004 == 0 ? NAMING_SCREEN_GUILD_TAG : NAMING_SCREEN_GUILD_NAME,
                   buffer, 0, 0, 0, CB2_ReturnFromGuildNaming);
}

void NetplayExt_CheckGuildName(void)
{
    gSpecialVar_Result = sGuildTag[0] != EOS && sGuildTag[1] != EOS && sGuildName[0] != EOS;
}

// Titles -----------------------------------------------------------------

static void BuildTitleList(void) { NpxRpc_Word("LIST"); }

static void BuildTitleSet(void)
{
    NpxRpc_Word("SET");
    NpxRpc_Word(Npx_Chosen(0));
}

static void AfterTitleList(void)
{
    u32 i;

    for (i = 0; i < Npx_RowCount(); i++)
        if (Npx_Num(i, 2))
            Npx_SetMyBadge(Npx_Num(i, 0));
}

static void AfterTitleSet(void)
{
    Npx_SetMyBadge(Npx_ChosenNum(0));
    Npx_AsciiToGame(gStringVar1, Npx_Chosen(1), 12);
}

// Pokédex race -----------------------------------------------------------

static void BuildDexSubmit(void)
{
    NpxRpc_Word("SUBMIT");
    NpxRpc_Num(GetNationalPokedexCount(FLAG_GET_SEEN));
    NpxRpc_Num(GetNationalPokedexCount(FLAG_GET_CAUGHT));
    NpxRpc_Num(NATIONAL_DEX_COUNT);
}

static void BuildDexBoard(void) { NpxRpc_Word("BOARD"); }

// Wonder Trade -----------------------------------------------------------

static void BuildWonderCheck(void) { NpxRpc_Word("CHECK"); }

static void BuildWonderSend(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005];

    NpxRpc_Word("SEND");
    NpxRpc_Num(GetMonData(mon, MON_DATA_SPECIES));
    NpxRpc_Data(mon, sizeof(*mon));
}

// VAR_0x8005: TRUE if a Pokémon came, else VAR_0x8006: TRUE if ours is still waiting.
static void AfterWonderCheck(void)
{
    gSpecialVar_0x8006 = (Npx_RowCount() != 0 && AsciiEquals(Npx_Field(0, 0), "WAITING"));
    if (TryReceiveMonAnswer())
        SaveAfterOnlineChange();
}

static void AfterWonderSend(void)
{
    StringCopy(gStringVar3, GetSpeciesName(GetMonData(&gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005], MON_DATA_SPECIES)));
    RemovePartyMon(gSpecialVar_0x8005);
    TryReceiveMonAnswer();
    SaveAfterOnlineChange();
}

// VAR_0x8005: party slot. VAR_RESULT: 0 ok, 1 nothing picked, 2 egg, 3 last
// Pokémon. gStringVar1: its species name.
void NetplayExt_CheckOfferedMon(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005];

    if (gSpecialVar_0x8005 >= PARTY_SIZE)
        gSpecialVar_Result = 1;
    else if (GetMonData(mon, MON_DATA_IS_EGG))
        gSpecialVar_Result = 2;
    else if (CalculatePlayerPartyCount() < 2)
        gSpecialVar_Result = 3;
    else
        gSpecialVar_Result = 0;
    if (gSpecialVar_0x8005 < PARTY_SIZE)
        StringCopy(gStringVar1, GetSpeciesName(GetMonData(mon, MON_DATA_SPECIES)));
}

// Bounties ---------------------------------------------------------------

static void BuildBountyList(void)    { NpxRpc_Word("LIST"); }
static void BuildBountyCollect(void) { NpxRpc_Word("COLLECT"); }

static void BuildBountyPost(void)
{
    NpxRpc_Word("POST");
    NpxRpc_Num(gSpecialVar_0x8005);
    NpxRpc_Num(gSpecialVar_0x8006 * 100);
}

static void BuildBountyFill(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005];

    NpxRpc_Word("FILL");
    NpxRpc_Word(Npx_Chosen(0));
    NpxRpc_Num(GetMonData(mon, MON_DATA_SPECIES));
    NpxRpc_Data(mon, sizeof(*mon));
}

static void BuildBountyCancel(void)
{
    NpxRpc_Word("CANCEL");
    NpxRpc_Word(Npx_Chosen(0));
}

static void AfterBountyPost(void)
{
    RemoveMoney(&gSaveBlock1Ptr->money, gSpecialVar_0x8006 * 100);
    StringCopy(gStringVar1, GetSpeciesName(gSpecialVar_0x8005));
    BufferNum(gStringVar2, gSpecialVar_0x8006 * 100);
    SaveAfterOnlineChange();
}

static void AfterBountyFill(void)
{
    s32 reward = Npx_Num(0, 0);

    RemovePartyMon(gSpecialVar_0x8005);
    AddMoney(&gSaveBlock1Ptr->money, reward);
    BufferNum(gStringVar2, reward);
    SaveAfterOnlineChange();
}

static void AfterBountyCollect(void)
{
    if (TryReceiveMonAnswer())
        SaveAfterOnlineChange();
}

static void AfterBountyCancel(void)
{
    AddMoney(&gSaveBlock1Ptr->money, Npx_Num(0, 0));
    BufferNum(gStringVar2, Npx_Num(0, 0));
    SaveAfterOnlineChange();
}

// VAR_0x8005: party slot. VAR_RESULT: TRUE if it's the chosen bounty's species.
void NetplayExt_CheckBountyMon(void)
{
    gSpecialVar_Result = GetMonData(&gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005], MON_DATA_SPECIES) == (u32)Npx_ChosenNum(2);
}

// VAR_0x8006: amount / 100. VAR_RESULT: TRUE if the player has that much.
void NetplayExt_CheckMoney(void)
{
    gSpecialVar_Result = IsEnoughMoney(&gSaveBlock1Ptr->money, gSpecialVar_0x8006 * 100);
    BufferNum(gStringVar2, gSpecialVar_0x8006 * 100);
}

// Mail and gifts ---------------------------------------------------------

static void BuildMailWho(void)   { NpxRpc_Word("WHO"); }
static void BuildMailInbox(void) { NpxRpc_Word("INBOX"); }

static void BuildMailSend(void)
{
    NpxRpc_Word("SEND");
    NpxRpc_Word(Npx_Chosen(0));
    NpxRpc_Word(Npx_Chosen(1));
    NpxRpc_Num(gSpecialVar_0x8005);
    NpxRpc_Num(gSpecialVar_0x8006 * 100);
    NpxRpc_Num(gSpecialVar_0x8007);
    NpxRpc_Num(gSpecialVar_0x8007 != ITEM_NONE ? 1 : 0);
}

static void BuildMailOpen(void)
{
    NpxRpc_Word("OPEN");
    NpxRpc_Word(Npx_Chosen(0));
}

static void AfterMailSend(void)
{
    if (gSpecialVar_0x8006 != 0)
        RemoveMoney(&gSaveBlock1Ptr->money, gSpecialVar_0x8006 * 100);
    if (gSpecialVar_0x8007 != ITEM_NONE)
        RemoveBagItem(gSpecialVar_0x8007, 1);
    SaveAfterOnlineChange();
}

// Describes a gift in gStringVar3: "¥500 and a POTION", "a POTION", "¥500" or "".
static void BufferGift(u32 money, u32 item)
{
    u8 *str = gStringVar3;

    *str = EOS;
    if (money != 0)
    {
        *str++ = CHAR_CURRENCY;
        str = ConvertIntToDecimalStringN(str, money, STR_CONV_MODE_LEFT_ALIGN, 6);
    }
    if (item != ITEM_NONE)
    {
        if (money != 0)
            str = StringCopy(str, COMPOUND_STRING(" and "));
        str = StringCopy(str, COMPOUND_STRING("a "));
        CopyItemName(item, str);
    }
}

// The mail's from|phrase|money|item|quantity. gStringVar1: from,
// gStringVar2: their message, gStringVar3: the gift. VAR_0x8005: TRUE if it had a gift.
static void AfterMailOpen(void)
{
    u32 phrase = Npx_Num(0, 1), money = Npx_Num(0, 2), item = Npx_Num(0, 3), qty = Npx_Num(0, 4);

    Npx_CopyField(gStringVar1, 0, 0, PLAYER_NAME_LENGTH);
    StringCopy(gStringVar2, sMailPhrases[phrase < ARRAY_COUNT(sMailPhrases) ? phrase : 0]);
    if (money != 0)
        AddMoney(&gSaveBlock1Ptr->money, money);
    if (item != ITEM_NONE && qty != 0)
        AddBagItem(item, qty);
    BufferGift(money, item);
    gSpecialVar_0x8005 = (money != 0 || item != ITEM_NONE);
    SaveAfterOnlineChange();
}

// For the chosen mail: VAR_RESULT TRUE if the bag has room for its item.
void NetplayExt_CheckGiftRoom(void)
{
    u32 item = Npx_ChosenNum(4), qty = Npx_ChosenNum(5);

    gSpecialVar_Result = item == ITEM_NONE || qty == 0 || CheckBagHasSpace(item, qty);
}

// Before sending: VAR_RESULT TRUE if the gift is still in the bag and wallet.
// gStringVar1: recipient, gStringVar3: the gift.
void NetplayExt_CheckGift(void)
{
    gSpecialVar_Result = IsEnoughMoney(&gSaveBlock1Ptr->money, gSpecialVar_0x8006 * 100)
                      && (gSpecialVar_0x8007 == ITEM_NONE || CheckBagHasItem(gSpecialVar_0x8007, 1));
    Npx_AsciiToGame(gStringVar1, Npx_Chosen(0), PLAYER_NAME_LENGTH);
    BufferGift(gSpecialVar_0x8006 * 100, gSpecialVar_0x8007);
}

// Secret bases -----------------------------------------------------------

static void BuildBasePut(void)
{
    SetPlayerSecretBaseParty();
    NpxRpc_Word("PUT");
    NpxRpc_Num(gSaveBlock1Ptr->secretBases[0].secretBaseId);
    NpxRpc_Data(&gSaveBlock1Ptr->secretBases[0], sizeof(struct SecretBase));
}

static void BuildBaseList(void) { NpxRpc_Word("LIST"); }

static void BuildBaseGet(void)
{
    NpxRpc_Word("GET");
    NpxRpc_Word(Npx_Chosen(0));
}

static void AfterBaseGet(void)
{
    Npx_CopyField(gStringVar1, 0, 0, PLAYER_NAME_LENGTH);
    gSpecialVar_0x8005 = StoreBaseAnswer();
}

// VAR_RESULT: TRUE if the player has a secret base to share.
void NetplayExt_HasSecretBase(void)
{
    gSpecialVar_Result = gSaveBlock1Ptr->secretBases[0].secretBaseId != 0;
}

// Copies the downloaded base into the save, so it can be entered like a base
// met through record mixing, and warps the player into it. VAR_RESULT: FALSE
// if it's where the player's own base is.
void NetplayExt_VisitBase(void)
{
    struct SecretBase *bases = gSaveBlock1Ptr->secretBases;
    u32 i, slot = SECRET_BASES_COUNT - 1;

    gSpecialVar_Result = FALSE;
    if (sStoredBase.secretBaseId == 0 || sStoredBase.secretBaseId == bases[0].secretBaseId)
        return;
    // Reuse the slot of a base at the same spot, else an empty one.
    for (i = 1; i < SECRET_BASES_COUNT; i++)
    {
        if (bases[i].secretBaseId == sStoredBase.secretBaseId)
        {
            slot = i;
            break;
        }
        if (bases[i].secretBaseId == 0 && slot == SECRET_BASES_COUNT - 1)
            slot = i;
    }
    bases[slot] = sStoredBase;
    bases[slot].registryStatus = 1; // registered: record mixing won't replace it
    bases[slot].battledOwnerToday = FALSE;
    VarSet(VAR_CURRENT_SECRET_BASE, slot);
    SetCurSecretBaseIdTo(sStoredBase.secretBaseId);
    gSpecialVar_Result = TRUE;
}

// Battles against a downloaded team (a secret base's owner, or a Battle
// Tower floor) use the secret base battle, with the team from here instead
// of from the save (see AllocateBattleResources).
const struct SecretBase *NetplayExt_GetBattleBase(void)
{
    return sBattleWithStoredBase ? &sStoredBase : NULL;
}

// Sets up the battle; the script then runs DoSpecialTrainerBattle.
void NetplayExt_PrepStoredBattle(void)
{
    sBattleWithStoredBase = TRUE;
    HealPlayerParty();
    PrepSecretBaseBattleFlags();
}

// After the battle. VAR_RESULT is still the battle outcome.
void NetplayExt_EndStoredBattle(void)
{
    sBattleWithStoredBase = FALSE;
    HealPlayerParty();
}

// Battle Tower -----------------------------------------------------------

static void BuildTowerState(void) { NpxRpc_Word("STATE"); }
static void BuildTowerNext(void)  { NpxRpc_Word("NEXT"); }
static void BuildTowerWon(void)   { NpxRpc_Word("WON"); }
static void BuildTowerLost(void)  { NpxRpc_Word("LOST"); }
static void BuildTowerBoard(void) { NpxRpc_Word("BOARD"); }

static void BuildTowerStart(void)
{
    struct SecretBase team;

    BuildTeamRecord(&team);
    NpxRpc_Word("START");
    NpxRpc_Data(&team, sizeof(team));
}

// floor|best. VAR_0x8005: floor (0: no run going), VAR_0x8006: best.
// gStringVar1: floor, gStringVar2: best.
static void AfterTowerState(void)
{
    gSpecialVar_0x8005 = Npx_Num(0, 0);
    gSpecialVar_0x8006 = Npx_Num(0, 1);
    BufferNum(gStringVar1, gSpecialVar_0x8005);
    BufferNum(gStringVar2, gSpecialVar_0x8006);
}

// floor|name, then their team. gStringVar1: floor, gStringVar2: their name.
static void AfterTowerNext(void)
{
    Npx_CopyField(gStringVar1, 0, 0, 4);
    Npx_CopyField(gStringVar2, 0, 1, PLAYER_NAME_LENGTH);
    gSpecialVar_0x8005 = StoreBaseAnswer();
}

// Draft battles ----------------------------------------------------------

static void BuildDraftPool(void)
{
    NpxRpc_Word("POOL");
    NpxRpc_Num(ARRAY_COUNT(sDraftSpecies));
}

static void BuildDraftQueue(void) { NpxRpc_Word("QUEUE"); }
static void BuildDraftLeave(void) { NpxRpc_Word("LEAVE"); }

static void AfterDraftPool(void)
{
    u32 i;

    for (i = 0; i < DRAFT_POOL_SIZE; i++)
    {
        u32 index = Npx_Num(0, i + 1);
        sDraftPool[i] = index < ARRAY_COUNT(sDraftSpecies) ? sDraftSpecies[index] : SPECIES_NONE;
    }
    sDraftPickCount = 0;
}

// VAR_0x8005: TRUE if a match was found at once.
static void AfterDraftQueue(void)
{
    gSpecialVar_0x8005 = AsciiEquals(Npx_Field(0, 0), "MATCHED");
}

void NpxFeatures_DrawDraftPool(u8 windowId, u32 y, u32 lineHeight)
{
    u32 i;

    for (i = 0; i < DRAFT_POOL_SIZE; i++)
    {
        u8 *str = StringCopy(gStringVar4, GetSpeciesName(sDraftPool[i]));
        str = StringCopy(str, COMPOUND_STRING(" Lv"));
        BufferNum(str, DRAFT_LEVEL);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 6 + (i % 2) * 96, y + (i / 2) * lineHeight, TEXT_SKIP_DRAW, NULL);
    }
    AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("Everyone drafts from the same six today.\nPick three, then get matched with a\nplayer who drafted too. Your own party\nis kept safe and comes back after."),
                                6, y + 3 * lineHeight + 4, TEXT_SKIP_DRAW, NULL);
}

// VAR_RESULT: picks made so far.
void NetplayExt_DraftPickCount(void)
{
    gSpecialVar_Result = sDraftPickCount;
}

// Swaps the party for the drafted team. Our party comes back once the
// battle is over (or the draft is canceled).
void NetplayExt_DraftBegin(void)
{
    u32 i;

    memcpy(sDraftBackup, gParties[B_TRAINER_PLAYER], sizeof(sDraftBackup));
    ZeroPlayerPartyMons();
    for (i = 0; i < sDraftPickCount; i++)
    {
        // Same nature and stats for everyone who picks this Pokémon.
        CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][i], sDraftPicks[i], DRAFT_LEVEL, sDraftPicks[i] * 0x9E37 + 7, OTID_STRUCT_PLAYER_ID, MAX_PER_STAT_IVS);
    }
    CalculatePlayerPartyCount();
    sDraftActive = TRUE;
    sDraftSawSession = FALSE;
}

static void RestoreDraftParty(void)
{
    memcpy(gParties[B_TRAINER_PLAYER], sDraftBackup, sizeof(sDraftBackup));
    CalculatePlayerPartyCount();
    sDraftActive = FALSE;
    sDraftSawSession = FALSE;
    SaveAfterOnlineChange();
}

void NetplayExt_DraftCancel(void)
{
    if (sDraftActive)
        RestoreDraftParty();
}

#define tTimer data[0]

static void Task_DraftWait(u8 taskId)
{
    if (Netplay_InSession())
        gSpecialVar_Result = TRUE;
    else if (JOY_NEW(B_BUTTON) || ++gTasks[taskId].tTimer > DRAFT_WAIT_FRAMES || !Npx_IsOnline())
        gSpecialVar_Result = FALSE;
    else
        return;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Waits for the server to pair us. VAR_RESULT: TRUE once a battle session started.
void NetplayExt_DraftWait(void)
{
    u8 taskId = CreateTask(Task_DraftWait, 80);
    gTasks[taskId].tTimer = 0;
}

#undef tTimer

// Replays ----------------------------------------------------------------
//
// After every online battle the game builds the battle's recording and
// uploads it in the background; REPLAYS lists them and plays one back.

static u32 ReplayParts(void)
{
    return (sReplaySize + REPLAY_UP_PART - 1) / REPLAY_UP_PART;
}

void NetplayExt_OnLinkBattleEnd(u8 battleOutcome)
{
    if (!Netplay_InSession())
        return;
    sReplay = RecordedBattle_BuildLinkReplay();
    sReplayStamp = RecordedBattle_LinkReplayStamp();
    sReplaySize = sizeof(*sReplay);
    StringCopyN(sReplayOpponent, gLinkPlayers[GetMultiplayerId() ^ 1].name, PLAYER_NAME_LENGTH);
    sReplayOpponent[PLAYER_NAME_LENGTH] = EOS;
    switch (battleOutcome & ~B_OUTCOME_LINK_BATTLE_RAN)
    {
    case B_OUTCOME_WON:  sReplayResult = 0; break;
    case B_OUTCOME_LOST: sReplayResult = 1; break;
    default:             sReplayResult = 2; break;
    }
    sReplayPart = 0;
    sReplayTries = 0;
    sReplayTimer = 0;
    sReplayUploading = FALSE;
    sReplayPending = TRUE;
}

static void PumpReplayUpload(void)
{
    static const char *const results[] = {"win", "loss", "draw"};

    if (!sReplayPending)
        return;
    if (sReplayStamp != RecordedBattle_LinkReplayStamp())
    {
        // Another link battle began, or a recording played back, before the
        // whole replay went up: what's left of it is gone.
        if (sReplayUploading)
            NpxRpc_Release();
        sReplayUploading = FALSE;
        sReplayPending = FALSE;
        return;
    }
    if (sReplayUploading)
    {
        switch (NpxRpc_Poll())
        {
        case NPX_RPC_WAITING:
            return;
        case NPX_RPC_DONE:
            sReplayUploading = FALSE;
            NpxRpc_Release();
            if (gNetplayExtMailbox.status == 0)
            {
                if (++sReplayPart >= ReplayParts())
                    sReplayPending = FALSE;
                return;
            }
            break;
        }
        // Failed: start over in a while, and give up after a few tries.
        NpxRpc_Release();
        sReplayUploading = FALSE;
        sReplayPart = 0;
        sReplayTimer = REPLAY_RETRY_FRAMES;
        if (++sReplayTries >= 3)
            sReplayPending = FALSE;
        return;
    }
    if (sReplayTimer != 0)
    {
        sReplayTimer--;
        return;
    }
    if (!Npx_IsOnline() || !NpxRpc_Begin("REPLAY"))
        return;
    NpxRpc_Word("PUT");
    NpxRpc_Num(sReplayPart);
    NpxRpc_Num(ReplayParts());
    NpxRpc_Name(sReplayOpponent);
    NpxRpc_Word(results[sReplayResult]);
    NpxRpc_Data((const u8 *)sReplay + sReplayPart * REPLAY_UP_PART,
                min(REPLAY_UP_PART, sReplaySize - sReplayPart * REPLAY_UP_PART));
    NpxRpc_Send();
    sReplayUploading = TRUE;
}

static void BuildReplayList(void) { NpxRpc_Word("LIST"); }

#define tState data[0]
#define tPart  data[1]
#define tParts data[2]

static void FinishDownload(u8 taskId, bool32 ok)
{
    if (!ok && sDownload != NULL)
        FREE_AND_SET_NULL(sDownload);
    gSpecialVar_Result = ok;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_DownloadReplay(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:
        // Our own last battle may still be going up. Playing a replay back
        // keeps the player's parties where that one's are, so let it finish.
        if (sReplayPending && Npx_IsOnline())
            return;
        if (tParts == 0)
        {
            // Recorded by a different version of the game.
            FinishDownload(taskId, FALSE);
            return;
        }
        // The heap is free in the overworld, and nothing resets it before
        // NetplayExt_PlayReplay starts the battle.
        sDownload = AllocZeroedUnchecked(sizeof(*sDownload));
        if (sDownload == NULL)
        {
            FinishDownload(taskId, FALSE);
            return;
        }
        tState++;
        // fallthrough
    case 1:
        if (!NpxRpc_Begin("REPLAY"))
            return;
        NpxRpc_Word("GET");
        NpxRpc_Word(Npx_Chosen(0));
        NpxRpc_Num(tPart);
        NpxRpc_Send();
        tState++;
        break;
    case 2:
        switch (NpxRpc_Poll())
        {
        case NPX_RPC_WAITING:
            return;
        case NPX_RPC_DONE:
            if (gNetplayExtMailbox.status == 0)
            {
                u32 offset = tPart * REPLAY_DOWN_PART;
                u32 len = min(gNetplayExtMailbox.respLen, sizeof(*sDownload) - offset);
                memcpy((u8 *)sDownload + offset, gNetplayExtMailbox.resp, len);
                NpxRpc_Release();
                if (++tPart < tParts)
                {
                    tState = 1;
                    return;
                }
                FinishDownload(taskId, TRUE);
                return;
            }
            // fallthrough
        default:
            NpxRpc_Release();
            FinishDownload(taskId, FALSE);
            return;
        }
        break;
    }
}

// Downloads the chosen replay. VAR_RESULT: TRUE if it came whole.
void NetplayExt_DownloadReplay(void)
{
    u8 taskId = CreateTask(Task_DownloadReplay, 80);
    u32 size = Npx_ChosenNum(4);

    // A pointer left from before is stale: the heap has been reset since.
    sDownload = NULL;
    gTasks[taskId].tState = 0;
    gTasks[taskId].tPart = 0;
    gTasks[taskId].tParts = 0;
    if (size == sizeof(*sDownload))
        gTasks[taskId].tParts = (size + REPLAY_DOWN_PART - 1) / REPLAY_DOWN_PART;
}

#undef tState
#undef tPart
#undef tParts

#define tState data[0]

static void Task_PlayReplay(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        if (sDownload == NULL)
            return;
        // It copies what it needs before the battle resets the heap.
        PlayRecordedBattleFromSave(sDownload, CB2_ReturnToFieldContinueScriptPlayMapMusic);
        FREE_AND_SET_NULL(sDownload);
    }
}

void NetplayExt_PlayReplay(void)
{
    FadeScreen(FADE_TO_BLACK, 0);
    CreateTask(Task_PlayReplay, 80);
}

#undef tState

// Request table ----------------------------------------------------------

const struct NpxRequest gNpxRequests[NPX_REQ_COUNT] =
{
    [NPX_REQ_GUILD_INFO]     = {"GUILD", BuildGuildInfo, AfterGuildNamed},
    [NPX_REQ_GUILD_LIST]     = {"GUILD", BuildGuildList, NULL},
    [NPX_REQ_GUILD_CREATE]   = {"GUILD", BuildGuildCreate, AfterGuildNamed},
    [NPX_REQ_GUILD_JOIN]     = {"GUILD", BuildGuildJoin, AfterGuildNamed},
    [NPX_REQ_GUILD_LEAVE]    = {"GUILD", BuildGuildLeave, NULL},
    [NPX_REQ_TITLE_LIST]     = {"TITLE", BuildTitleList, AfterTitleList},
    [NPX_REQ_TITLE_SET]      = {"TITLE", BuildTitleSet, AfterTitleSet},
    [NPX_REQ_DEX_SUBMIT]     = {"DEX", BuildDexSubmit, NULL},
    [NPX_REQ_DEX_BOARD]      = {"DEX", BuildDexBoard, NULL},
    [NPX_REQ_WONDER_CHECK]   = {"WONDER", BuildWonderCheck, AfterWonderCheck},
    [NPX_REQ_WONDER_SEND]    = {"WONDER", BuildWonderSend, AfterWonderSend},
    [NPX_REQ_BOUNTY_LIST]    = {"BOUNTY", BuildBountyList, NULL},
    [NPX_REQ_BOUNTY_POST]    = {"BOUNTY", BuildBountyPost, AfterBountyPost},
    [NPX_REQ_BOUNTY_FILL]    = {"BOUNTY", BuildBountyFill, AfterBountyFill},
    [NPX_REQ_BOUNTY_COLLECT] = {"BOUNTY", BuildBountyCollect, AfterBountyCollect},
    [NPX_REQ_BOUNTY_CANCEL]  = {"BOUNTY", BuildBountyCancel, AfterBountyCancel},
    [NPX_REQ_MAIL_WHO]       = {"MAIL", BuildMailWho, NULL},
    [NPX_REQ_MAIL_SEND]      = {"MAIL", BuildMailSend, AfterMailSend},
    [NPX_REQ_MAIL_INBOX]     = {"MAIL", BuildMailInbox, NULL},
    [NPX_REQ_MAIL_OPEN]      = {"MAIL", BuildMailOpen, AfterMailOpen},
    [NPX_REQ_BASE_PUT]       = {"BASE", BuildBasePut, NULL},
    [NPX_REQ_BASE_LIST]      = {"BASE", BuildBaseList, NULL},
    [NPX_REQ_BASE_GET]       = {"BASE", BuildBaseGet, AfterBaseGet},
    [NPX_REQ_TOWER_STATE]    = {"TOWER", BuildTowerState, AfterTowerState},
    [NPX_REQ_TOWER_START]    = {"TOWER", BuildTowerStart, AfterTowerState},
    [NPX_REQ_TOWER_NEXT]     = {"TOWER", BuildTowerNext, AfterTowerNext},
    [NPX_REQ_TOWER_WON]      = {"TOWER", BuildTowerWon, AfterTowerState},
    [NPX_REQ_TOWER_LOST]     = {"TOWER", BuildTowerLost, AfterTowerState},
    [NPX_REQ_TOWER_BOARD]    = {"TOWER", BuildTowerBoard, NULL},
    [NPX_REQ_DRAFT_POOL]     = {"DRAFT", BuildDraftPool, AfterDraftPool},
    [NPX_REQ_DRAFT_QUEUE]    = {"DRAFT", BuildDraftQueue, AfterDraftQueue},
    [NPX_REQ_DRAFT_LEAVE]    = {"DRAFT", BuildDraftLeave, NULL},
    [NPX_REQ_REPLAY_LIST]    = {"REPLAY", BuildReplayList, NULL},
};

// Pick lists -------------------------------------------------------------

static u8 *AppendField(u8 *str, u32 row, u32 col, u32 maxChars)
{
    return Npx_CopyField(str, row, col, maxChars);
}

static u32 BuildSpeciesList(u8 (*text)[32], u32 max)
{
    u32 n = 0, dexNum;

    // Wanted posters: Pokémon the player has seen but not caught.
    for (dexNum = 1; dexNum <= NATIONAL_DEX_COUNT && n < max; dexNum++)
    {
        if (GetSetPokedexFlag(dexNum, FLAG_GET_SEEN) && !GetSetPokedexFlag(dexNum, FLAG_GET_CAUGHT))
        {
            sListValues[n] = NationalPokedexNumToSpecies(dexNum);
            StringCopy(text[n], GetSpeciesName(sListValues[n]));
            n++;
        }
    }
    return n;
}

static u32 BuildGiftItemList(u8 (*text)[32], u32 max)
{
    static const u8 pockets[] = {POCKET_ITEMS, POCKET_POKE_BALLS, POCKET_BERRIES};
    u32 n = 0, p, i;

    for (p = 0; p < ARRAY_COUNT(pockets); p++)
    {
        for (i = 0; i < gBagPockets[pockets[p]].capacity && n < max; i++)
        {
            struct ItemSlot slot = GetBagItemIdAndQuantity(pockets[p], i);
            u8 *str;
            if (slot.itemId == ITEM_NONE || slot.quantity == 0)
                continue;
            sListValues[n] = slot.itemId;
            str = CopyItemName(slot.itemId, text[n]);
            str = StringCopy(str, COMPOUND_STRING(" x"));
            BufferNum(str, slot.quantity);
            n++;
        }
    }
    return n;
}

// Fills the labels for NetplayExt_ChooseRow. Returns how many rows.
u32 NpxFeatures_BuildList(u32 list, u8 (*text)[32], u32 max)
{
    u32 i, rows = min(Npx_RowCount(), max);
    u8 *str;

    switch (list)
    {
    case NPX_LIST_PHRASES:
        for (i = 0; i < ARRAY_COUNT(sMailPhrases); i++)
            StringCopy(text[i], sMailPhrases[i]);
        return ARRAY_COUNT(sMailPhrases);
    case NPX_LIST_GIFT_ITEM:
        return BuildGiftItemList(text, max);
    case NPX_LIST_SPECIES:
        return BuildSpeciesList(text, max);
    case NPX_LIST_DRAFT:
        rows = 0;
        for (i = 0; i < DRAFT_POOL_SIZE; i++)
        {
            u32 j;
            bool32 picked = FALSE;
            for (j = 0; j < sDraftPickCount; j++)
                if (sDraftPicks[j] == sDraftPool[i])
                    picked = TRUE;
            if (picked || sDraftPool[i] == SPECIES_NONE)
                continue;
            sListValues[rows] = sDraftPool[i];
            StringCopy(text[rows++], GetSpeciesName(sDraftPool[i]));
        }
        return rows;
    }

    if (Npx_RowCount() == 0)
        return 0;
    for (i = 0; i < rows; i++)
    {
        str = text[i];
        switch (list)
        {
        case NPX_LIST_GUILDS: // rank|tag|name|members|score
            *str++ = CHAR_SPACE;
            str = StringCopy(str, COMPOUND_STRING("("));
            str = AppendField(str, i, 1, 4);
            str = StringCopy(str, COMPOUND_STRING(") "));
            AppendField(str, i, 2, 12);
            break;
        case NPX_LIST_TITLES: // id|name|shown
            str = AppendField(str, i, 1, 12);
            if (Npx_Num(i, 2))
                StringCopy(str, COMPOUND_STRING(" (shown)"));
            break;
        case NPX_LIST_BOUNTIES: // id|poster|species|reward|mine
            str = StringCopy(str, GetSpeciesName(Npx_Num(i, 2)));
            *str++ = CHAR_SPACE;
            *str++ = CHAR_CURRENCY;
            str = AppendField(str, i, 3, 6);
            if (Npx_Num(i, 4))
                StringCopy(str, COMPOUND_STRING(" (yours)"));
            break;
        case NPX_LIST_PLAYERS: // name|tid|online
            str = AppendField(str, i, 0, PLAYER_NAME_LENGTH);
            if (Npx_Num(i, 2))
                StringCopy(str, COMPOUND_STRING(" (online)"));
            break;
        case NPX_LIST_INBOX: // id|from|phrase|money|item|quantity
            str = StringCopy(str, COMPOUND_STRING("From "));
            str = AppendField(str, i, 1, PLAYER_NAME_LENGTH);
            if (Npx_Num(i, 3) != 0 || Npx_Num(i, 4) != 0)
                StringCopy(str, COMPOUND_STRING(" + gift"));
            break;
        case NPX_LIST_BASES: // rowid|owner|location|visits
            str = AppendField(str, i, 1, PLAYER_NAME_LENGTH);
            StringCopy(str, COMPOUND_STRING("'s base"));
            break;
        case NPX_LIST_REPLAYS: // id|opponent|result|date|size
            str = StringCopy(str, COMPOUND_STRING("vs "));
            str = AppendField(str, i, 1, PLAYER_NAME_LENGTH);
            *str++ = CHAR_SPACE;
            str = AppendField(str, i, 2, 4);
            *str++ = CHAR_SPACE;
            AppendField(str, i, 3, 10);
            break;
        default:
            AppendField(str, i, 0, 20);
            break;
        }
    }
    return rows;
}

void NpxFeatures_OnChosen(u32 list, u32 row)
{
    switch (list)
    {
    case NPX_LIST_PHRASES:
        gSpecialVar_0x8005 = row;
        return;
    case NPX_LIST_GIFT_ITEM:
        gSpecialVar_0x8007 = sListValues[row];
        CopyItemName(sListValues[row], gStringVar2);
        return;
    case NPX_LIST_SPECIES:
        gSpecialVar_0x8005 = sListValues[row];
        StringCopy(gStringVar1, GetSpeciesName(sListValues[row]));
        return;
    case NPX_LIST_DRAFT:
        if (sDraftPickCount < DRAFT_TEAM_SIZE)
            sDraftPicks[sDraftPickCount++] = sListValues[row];
        StringCopy(gStringVar1, GetSpeciesName(sListValues[row]));
        return;
    }

    Npx_ChooseAnswerRow(row);
    switch (list)
    {
    case NPX_LIST_GUILDS:
        Npx_CopyField(gStringVar1, row, 1, 4);
        Npx_CopyField(gStringVar2, row, 2, 12);
        break;
    case NPX_LIST_TITLES:
        Npx_CopyField(gStringVar1, row, 1, 12);
        break;
    case NPX_LIST_BOUNTIES:
        StringCopy(gStringVar1, GetSpeciesName(Npx_Num(row, 2)));
        Npx_CopyField(gStringVar2, row, 3, 7);
        Npx_CopyField(gStringVar3, row, 1, PLAYER_NAME_LENGTH);
        gSpecialVar_0x8007 = Npx_Num(row, 4); // mine
        break;
    case NPX_LIST_PLAYERS:
    case NPX_LIST_BASES:
        Npx_CopyField(gStringVar1, row, list == NPX_LIST_BASES ? 1 : 0, PLAYER_NAME_LENGTH);
        break;
    case NPX_LIST_INBOX:
        Npx_CopyField(gStringVar1, row, 1, PLAYER_NAME_LENGTH);
        break;
    case NPX_LIST_REPLAYS:
        Npx_CopyField(gStringVar1, row, 1, PLAYER_NAME_LENGTH);
        break;
    }
}

// Overworld --------------------------------------------------------------

bool32 NpxFeatures_IsBusy(void)
{
    return sReplayUploading;
}

void NpxFeatures_OverworldUpdate(void)
{
    // A draft battle is over (or never got going): give the party back.
    if (sDraftActive)
    {
        if (Netplay_InSession())
            sDraftSawSession = TRUE;
        else if (sDraftSawSession && !gMain.inBattle)
            RestoreDraftParty();
    }
    PumpReplayUpload();
}

// For automated tests (tools/netplay_bridge/e2e/scenario_ext.lua): gives the
// test player a secret base and a few Pokémon seen in the Pokédex.
void NpxFeatures_SeedTestData(void)
{
    static const u16 seen[] = {NATIONAL_DEX_GYARADOS, NATIONAL_DEX_LUCARIO, NATIONAL_DEX_GARCHOMP, NATIONAL_DEX_ABSOL};
    struct SecretBase *base = &gSaveBlock1Ptr->secretBases[0];
    u32 i;

    for (i = 0; i < ARRAY_COUNT(seen); i++)
        GetSetPokedexFlag(seen[i], FLAG_SET_SEEN);
    if (base->secretBaseId == 0)
    {
        // Two test players get bases at different spots, so they can visit.
        base->secretBaseId = (gSaveBlock2Ptr->playerTrainerId[0] & 1) ? SECRET_BASE_RED_CAVE1_1 : SECRET_BASE_RED_CAVE1_2;
        StringCopyN(base->trainerName, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
        memcpy(base->trainerId, gSaveBlock2Ptr->playerTrainerId, TRAINER_ID_LENGTH);
        base->gender = gSaveBlock2Ptr->playerGender;
        base->language = GAME_LANGUAGE;
    }
}
