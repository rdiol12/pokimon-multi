// Changes to the original Hoenn for many players at once (docs/hoenn_multi.md):
// - other players and their Pokémon don't block anyone (HoennMulti_PassesThrough),
// - other players aren't drawn in private or per-player places, or during the
//   player's own cutscenes (HoennMulti_ShowOtherPlayers),
// - friend travel only goes to maps the player has reached (HoennMulti_HasVisitedMap).
#include "global.h"
#include "hoenn_multi.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "netplay.h"
#include "netplay_ext.h"
#include "online.h"
#include "overworld.h"
#include "pokemon.h"
#include "region_map.h"
#include "save_ext.h"
#include "script.h"
#include "script_movement.h"
#include "string_util.h"
#include "trainer.h"
#include "constants/battle.h"
#include "constants/battle_ai.h"
#include "constants/battle_partner.h"
#include "constants/characters.h"
#include "constants/flags.h"
#include "constants/map_types.h"
#include "constants/trainers.h"

// Local ids of other players (70-93) and their following Pokémon (94-117).
#define LOCALID_OTHERS_FIRST LOCALID_NETPLAY_PLAYER_START
#define LOCALID_OTHERS_LAST  (LOCALID_NETPLAY_PLAYER_END + NETPLAY_MAX_REMOTES)

STATIC_ASSERT(MAP_GROUPS_COUNT <= HOENN_MULTI_MAP_GROUPS, HoennMultiTooManyMapGroups);

static EWRAM_DATA bool8 sInCutscene = FALSE;

// Places where every player has their own copy (their house, their secret base,
// their run through a gym, the League or a Frontier challenge). Seeing another
// player there looks wrong: they'd stand in your bedroom, walk through gym gates
// that are shut for you, or wander into the Champion's room mid-battle.
static const u16 sPrivateMaps[] =
{
    MAP_INSIDE_OF_TRUCK,
    MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F,
    MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F,
    MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F,
    MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F,
    // Gyms
    MAP_RUSTBORO_CITY_GYM,
    MAP_DEWFORD_TOWN_GYM,
    MAP_MAUVILLE_CITY_GYM,
    MAP_LAVARIDGE_TOWN_GYM_1F,
    MAP_LAVARIDGE_TOWN_GYM_B1F,
    MAP_PETALBURG_CITY_GYM,
    MAP_FORTREE_CITY_GYM,
    MAP_MOSSDEEP_CITY_GYM,
    MAP_SOOTOPOLIS_CITY_GYM_1F,
    MAP_SOOTOPOLIS_CITY_GYM_B1F,
    // The League
    MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM,
    MAP_EVER_GRANDE_CITY_PHOEBES_ROOM,
    MAP_EVER_GRANDE_CITY_GLACIAS_ROOM,
    MAP_EVER_GRANDE_CITY_DRAKES_ROOM,
    MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM,
    MAP_EVER_GRANDE_CITY_HALL1,
    MAP_EVER_GRANDE_CITY_HALL2,
    MAP_EVER_GRANDE_CITY_HALL3,
    MAP_EVER_GRANDE_CITY_HALL4,
    MAP_EVER_GRANDE_CITY_HALL5,
    MAP_EVER_GRANDE_CITY_HALL_OF_FAME,
    // Trick House puzzles
    MAP_ROUTE110_TRICK_HOUSE_CORRIDOR,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE1,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE2,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE3,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE4,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE5,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE6,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE7,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE8,
    MAP_ROUTE110_TRICK_HOUSE_END,
    // Frontier challenges (the lobbies stay shared)
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_ELEVATOR,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_MULTI_PARTNER_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_MULTI_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_MULTI_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_DOME_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_DOME_PRE_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_DOME_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_PALACE_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_PALACE_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_ARENA_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_ARENA_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_FACTORY_PRE_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_FACTORY_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_THREE_PATH_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_NORMAL,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_FINAL,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS,
    MAP_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR,
    MAP_BATTLE_FRONTIER_BATTLE_PYRAMID_TOP,
    MAP_TRAINER_HILL_1F,
    MAP_TRAINER_HILL_2F,
    MAP_TRAINER_HILL_3F,
    MAP_TRAINER_HILL_4F,
    MAP_TRAINER_HILL_ROOF,
    MAP_TRAINER_HILL_ELEVATOR,
};

static bool32 IsSecretBaseMap(u32 mapGroup, u32 mapNum)
{
    return mapGroup == MAP_GROUP(MAP_SECRET_BASE_RED_CAVE1)
        && mapNum >= MAP_NUM(MAP_SECRET_BASE_RED_CAVE1)
        && mapNum <= MAP_NUM(MAP_SECRET_BASE_SHRUB4);
}

static bool32 IsPrivateMap(u32 mapGroup, u32 mapNum)
{
    u32 i;
    u16 map = (mapGroup << 8) | mapNum;

    if (IsSecretBaseMap(mapGroup, mapNum))
        return TRUE;
    for (i = 0; i < ARRAY_COUNT(sPrivateMaps); i++)
    {
        if (sPrivateMaps[i] == map)
            return TRUE;
    }
    return FALSE;
}

// A cutscene is a script that moves people around. Once one starts, other
// players stay hidden until the script ends, so nobody stands in the middle
// of the scene or pops in between two of its steps. Their objects also free
// up slots for the scene's own people. A script that only moves the player
// or the person they talk to (the nurse turning to her machine) isn't one.
static void UpdateCutscene(void)
{
    u8 talked;

    if (!ScriptContext_IsEnabled())
    {
        sInCutscene = FALSE;
        return;
    }
    if (sInCutscene)
        return;
    talked = GetObjectEventIdByLocalId(gSpecialVar_LastTalked);
    if (ScriptMovement_IsMovingOtherThan(gPlayerAvatar.objectEventId, talked))
        sInCutscene = TRUE;
}

void HoennMulti_SetVisited(u8 mapGroup, u8 mapNum)
{
    if (mapGroup < HOENN_MULTI_MAP_GROUPS && mapNum < HOENN_MULTI_MAPS_PER_GROUP)
        gSaveBlockExt.visitedMaps[mapGroup][mapNum / 8] |= 1 << (mapNum % 8);
}

static void MarkVisited(void)
{
    // Hide-and-seek and races take players to their arena and back; that
    // doesn't count as having found the place.
    if (Online_HideIsOn() || Online_RaceIsOn())
        return;
    HoennMulti_SetVisited(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

// Shared milestones -------------------------------------------------------
//
// The server hears the player's badges and whether they're Champion
// (OEV MILESTONE mask champion fresh): once after joining online (fresh 0,
// just for the records), then whenever it changes (fresh 1: everyone online
// hears about it).

static EWRAM_DATA u16 sReportedMilestones = 0; // 0: not reported since joining online (else mask + 1)

static u32 GetMilestones(void)
{
    u32 i, mask = 0;

    for (i = 0; i < NUM_BADGES; i++)
    {
        if (FlagGet(FLAG_BADGE01_GET + i))
            mask |= 1 << i;
    }
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        mask |= 1 << NUM_BADGES;
    return mask;
}

static void UpdateMilestones(void)
{
    u32 now;
    s32 args[3];

    if (!Netplay_IsOnline() || Online_RaceIsOn() || Online_HideIsOn())
    {
        if (!Netplay_IsOnline())
            sReportedMilestones = 0;
        return;
    }
    if ((gMain.vblankCounter1 & 31) != 0)
        return;
    now = GetMilestones();
    if (now + 1 == sReportedMilestones)
        return;
    args[0] = now & ((1 << NUM_BADGES) - 1);
    args[1] = (now >> NUM_BADGES) & 1;
    args[2] = sReportedMilestones != 0;
    Online_Report("MILESTONE", ARRAY_COUNT(args), args);
    sReportedMilestones = now + 1;
}

// Team up ------------------------------------------------------------------
//
// Another player's team (HUB > TEAM UP, as a secret base record) fights
// beside the player against TEAM AQUA and TEAM MAGMA, the rivals and
// legendary Pokémon: the battle becomes a double with their team as the
// in-game partner, played by the AI.

static EWRAM_EXT_DATA struct SecretBase sTeam;
static EWRAM_EXT_DATA struct Trainer sPartner[3]; // [0] the partner, as gRoverTagTrainers wants
static EWRAM_EXT_DATA struct TrainerMon sPartnerParty[PARTY_SIZE];
static EWRAM_DATA bool8 sHasTeam = FALSE;
static EWRAM_DATA bool8 sPartnerInBattle = FALSE;

void HoennMulti_StoreTeam(void)
{
    u32 size;
    const u8 *data = Npx_Binary(&size);

    if (data == NULL || size != sizeof(sTeam))
        return;
    memcpy(&sTeam, data, sizeof(sTeam));
    sHasTeam = sTeam.party.species[0] != SPECIES_NONE;
}

void HoennMulti_DropTeam(void)
{
    sHasTeam = FALSE;
}

void HoennMulti_HasTeam(void)
{
    gSpecialVar_Result = sHasTeam;
}

static bool32 IsBossClass(u32 trainerClass)
{
    switch (trainerClass)
    {
    case TRAINER_CLASS_AQUA_LEADER:
    case TRAINER_CLASS_AQUA_ADMIN:
    case TRAINER_CLASS_MAGMA_LEADER:
    case TRAINER_CLASS_MAGMA_ADMIN:
    case TRAINER_CLASS_RIVAL:
        return TRUE;
    }
    return FALSE;
}

static bool32 IsLegendSpecies(u32 species)
{
    return species != SPECIES_NONE && species < NUM_SPECIES
        && (gSpeciesInfo[species].isRestrictedLegendary || gSpeciesInfo[species].isSubLegendary
         || gSpeciesInfo[species].isMythical);
}

static void BuildPartner(void)
{
    const struct SecretBaseParty *party = &sTeam.party;
    u32 i, j, count = 0;

    memset(sPartnerParty, 0, sizeof(sPartnerParty));
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct TrainerMon *mon = &sPartnerParty[count];

        if (party->species[i] == SPECIES_NONE || party->species[i] >= NUM_SPECIES)
            continue;
        mon->species = party->species[i];
        mon->lvl = party->levels[i] ? party->levels[i] : 5;
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            enum Move move = party->moves[i * MAX_MON_MOVES + j];
            mon->moves[j] = move < MOVES_COUNT ? move : MOVE_NONE;
        }
        mon->heldItem = party->heldItems[i];
        mon->iv = TRAINER_PARTY_IVS(20, 20, 20, 20, 20, 20);
        mon->nature = party->personality[i] % NUM_NATURES;
        mon->gender = TRAINER_MON_RANDOM_GENDER;
        mon->ball = POKEBALL_COUNT;
        mon->dynamaxLevel = MAX_DYNAMAX_LEVEL;
        count++;
    }
    memset(sPartner, 0, sizeof(sPartner));
    sPartner[0].aiFlags = AI_FLAG_BASIC_TRAINER;
    sPartner[0].party = sPartnerParty;
    sPartner[0].partySize = count;
    sPartner[0].trainerClass = TRAINER_CLASS_PKMN_TRAINER_1;
    sPartner[0].gender = sTeam.gender ? TRAINER_GENDER_FEMALE : TRAINER_GENDER_MALE;
    // An in-game partner's picture is a back picture, like the player's.
    sPartner[0].trainerPic = GetPlayerTrainerPic(sTeam.gender ? FEMALE : MALE, GAME_VERSION);
    StringCopyN(sPartner[0].trainerName, sTeam.trainerName, PLAYER_NAME_LENGTH);
    sPartner[0].trainerName[PLAYER_NAME_LENGTH] = EOS;
}

// Called as a battle starts (Task_BattleStart), with the battle's flags set.
void HoennMulti_PrepareBattle(void)
{
    bool32 boss;

    if (!sHasTeam || gRoverTagTrainers != NULL || gPartnerTrainerId != 0)
        return;
    if (gBattleTypeFlags == BATTLE_TYPE_TRAINER)
        boss = IsBossClass(GetTrainerClassFromId(TRAINER_BATTLE_PARAM.opponentA));
    else if ((gBattleTypeFlags & ~(BATTLE_TYPE_LEGENDARY | BATTLE_TYPE_ROAMER)) == 0)
        boss = IsLegendSpecies(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES));
    else
        boss = FALSE;
    if (!boss)
        return;
    BuildPartner();
    if (sPartner[0].partySize == 0)
        return;
    gRoverTagTrainers = sPartner;
    gPartnerTrainerId = TRAINER_PARTNER(PARTNER_STEVEN);
    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        TRAINER_BATTLE_PARAM.opponentB = 0xFFFF;
    gBattleTypeFlags |= BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER | BATTLE_TYPE_DOUBLE;
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_PARTNER], &sPartner[0]);
    sPartnerInBattle = TRUE;
}

bool32 HoennMulti_PartnerInBattle(void)
{
    return sPartnerInBattle;
}

// Back on the field after a battle with the partner.
static void EndPartnerBattle(void)
{
    if (!sPartnerInBattle)
        return;
    sPartnerInBattle = FALSE;
    gRoverTagTrainers = NULL;
    gPartnerTrainerId = 0;
    if (gBattleOutcome == B_OUTCOME_WON || gBattleOutcome == B_OUTCOME_CAUGHT)
        Online_Report("TEAMWIN", 0, NULL);
}

// Live leader battles ------------------------------------------------------

bool32 HoennMulti_IsWatchableBattle(void)
{
    u32 trainerClass;

    if ((gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_FRONTIER))
        != BATTLE_TYPE_TRAINER)
        return FALSE;
    trainerClass = GetTrainerClassFromId(TRAINER_BATTLE_PARAM.opponentA);
    return trainerClass == TRAINER_CLASS_LEADER || trainerClass == TRAINER_CLASS_ELITE_FOUR
        || trainerClass == TRAINER_CLASS_CHAMPION;
}

void HoennMulti_GetWatchableOpponent(u8 *dest)
{
    StringCopyN(dest, GetTrainerNameFromId(TRAINER_BATTLE_PARAM.opponentA), NETPLAY_SOLO_NAME_LENGTH - 1);
    dest[NETPLAY_SOLO_NAME_LENGTH - 1] = EOS;
}

// Scripts ------------------------------------------------------------------

void HoennMulti_IsOnline(void)
{
    gSpecialVar_Result = Netplay_IsOnline();
}

// After QM JOIN: the trainer found, for Netplay_SendInvite.
void HoennMulti_AfterQuickMatch(void)
{
    gSpecialVar_0x8007 = Npx_Num(0, 3);
}

// The gym statues' WATCH LIVE (waitstate).
void HoennMulti_WatchLive(void)
{
    Netplay_WatchFromScript();
}

void HoennMulti_OverworldUpdate(void)
{
    UpdateCutscene();
    MarkVisited();
    UpdateMilestones();
    EndPartnerBattle();
}

bool32 HoennMulti_ShowOtherPlayers(void)
{
    if (sInCutscene)
        return FALSE;
    return !IsPrivateMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

bool32 HoennMulti_PassesThrough(const struct ObjectEvent *mover, const struct ObjectEvent *other)
{
    return (other->localId >= LOCALID_OTHERS_FIRST && other->localId <= LOCALID_OTHERS_LAST)
        || (mover->localId >= LOCALID_OTHERS_FIRST && mover->localId <= LOCALID_OTHERS_LAST);
}

bool32 HoennMulti_HasVisitedMap(u8 mapGroup, u8 mapNum)
{
    const struct MapHeader *header;

    if (mapGroup < HOENN_MULTI_MAP_GROUPS && mapNum < HOENN_MULTI_MAPS_PER_GROUP
     && (gSaveBlockExt.visitedMaps[mapGroup][mapNum / 8] & (1 << (mapNum % 8))))
        return TRUE;
    // Saves from before this existed: towns and cities the player can fly to.
    header = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    return RegionMap_CanFlyToMapsec(header->regionMapSectionId);
}
