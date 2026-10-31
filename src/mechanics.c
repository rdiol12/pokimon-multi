#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "mechanics.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/layouts.h"
#include "constants/map_groups.h"

// People the mechanics features stand on existing maps, spawned from C so
// no map file changes: the move tutor in every Pokémon Center, and so on.
// Each has a local id from include/constants/mechanics.h.

extern const u8 Mechanics_EventScript_MoveTutor[];
extern const u8 Mechanics_EventScript_Referee[];
extern const u8 Mechanics_EventScript_CupHost[];
extern const u8 Mechanics_EventScript_StyleMaster[];
extern const u8 Mechanics_EventScript_SkillTeacher[];
extern const u8 Mechanics_EventScript_LabAide[];
extern const u8 Mechanics_EventScript_StatueTreecko[];
extern const u8 Mechanics_EventScript_StatueTorchic[];
extern const u8 Mechanics_EventScript_StatueMudkip[];
extern const u8 Mechanics_EventScript_EvoSage[];
extern const u8 Mechanics_EventScript_BossGranite[];
extern const u8 Mechanics_EventScript_BossMeteor[];
extern const u8 Mechanics_EventScript_BossShoal[];
extern const u8 Mechanics_EventScript_BossSeafloor[];
extern const u8 Mechanics_EventScript_BossVictory[];
extern const u8 Mechanics_EventScript_StormResearcher[];
extern const u8 Mechanics_EventScript_EchoGroudon[];
extern const u8 Mechanics_EventScript_EchoKyogre[];

struct MechNpc
{
    u8 localId;
    bool32 (*isHere)(void);
    s16 x, y;
    u16 gfx;
    u8 movement;
    const u8 *script;
};

static bool32 InPokemonCenter(void)
{
    return gMapHeader.mapLayoutId == LAYOUT_POKEMON_CENTER_1F
        || gMapHeader.mapLayoutId == LAYOUT_LAVARIDGE_TOWN_POKEMON_CENTER_1F;
}

static bool32 IsMap(u16 map)
{
    return gSaveBlock1Ptr->location.mapGroup == (map >> 8) && gSaveBlock1Ptr->location.mapNum == (map & 0xFF);
}

static bool32 InSlateportTent(void)
{
    return IsMap(MAP_SLATEPORT_CITY_BATTLE_TENT_LOBBY);
}

static bool32 InVerdanturfTent(void)
{
    return IsMap(MAP_VERDANTURF_TOWN_BATTLE_TENT_LOBBY);
}

static bool32 InPokemonSchool(void)
{
    return IsMap(MAP_RUSTBORO_CITY_POKEMON_SCHOOL);
}

static bool32 InBirchsLab(void)
{
    return IsMap(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB);
}

static bool32 StatueInPetalburgWoods(void)
{
    return IsMap(MAP_PETALBURG_WOODS) && Mechanics_IsStatueHere(STARTER_LINE_TREECKO);
}

static bool32 StatueInFieryPath(void)
{
    return IsMap(MAP_FIERY_PATH) && Mechanics_IsStatueHere(STARTER_LINE_TORCHIC);
}

static bool32 StatueOnRoute117(void)
{
    return IsMap(MAP_ROUTE117) && Mechanics_IsStatueHere(STARTER_LINE_MUDKIP);
}

static bool32 InSagesHouse(void)
{
    return IsMap(MAP_MAUVILLE_CITY_HOUSE1);
}

static bool32 BossInGraniteCave(void)
{
    return IsMap(MAP_GRANITE_CAVE_1F) && Mechanics_IsBossHere(BOSS_GRANITE);
}

static bool32 BossInMeteorFalls(void)
{
    return IsMap(MAP_METEOR_FALLS_1F_2R) && Mechanics_IsBossHere(BOSS_METEOR);
}

static bool32 BossInShoalCave(void)
{
    return IsMap(MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM) && Mechanics_IsBossHere(BOSS_SHOAL);
}

static bool32 BossInSeafloorCavern(void)
{
    return IsMap(MAP_SEAFLOOR_CAVERN_ROOM4) && Mechanics_IsBossHere(BOSS_SEAFLOOR);
}

static bool32 BossOnVictoryRoad(void)
{
    return IsMap(MAP_VICTORY_ROAD_1F) && Mechanics_IsBossHere(BOSS_VICTORY);
}

static bool32 InFallarborTent(void)
{
    return IsMap(MAP_FALLARBOR_TOWN_BATTLE_TENT_LOBBY);
}

static bool32 InWeatherInstitute(void)
{
    return IsMap(MAP_ROUTE119_WEATHER_INSTITUTE_2F) && FlagGet(FLAG_SYS_GAME_CLEAR);
}

static bool32 EchoOnRoute114(void)
{
    return IsMap(MAP_ROUTE114) && Mechanics_IsStormActive(STORM_GROUDON);
}

static bool32 EchoOnRoute118(void)
{
    return IsMap(MAP_ROUTE118) && Mechanics_IsStormActive(STORM_KYOGRE);
}

static const struct MechNpc sMechNpcs[] =
{
    {LOCALID_MECH_TUTOR, InPokemonCenter, 13, 5, OBJ_EVENT_GFX_EXPERT_M, MOVEMENT_TYPE_FACE_LEFT, Mechanics_EventScript_MoveTutor},
    {LOCALID_MECH_REFEREE, InSlateportTent, 9, 6, OBJ_EVENT_GFX_GENTLEMAN, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_Referee},
    {LOCALID_MECH_CUP_HOST, InVerdanturfTent, 4, 6, OBJ_EVENT_GFX_REPORTER_M, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_CupHost},
    {LOCALID_MECH_STYLE_MASTER, InFallarborTent, 4, 6, OBJ_EVENT_GFX_EXPERT_F, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_StyleMaster},
    {LOCALID_MECH_SKILL_TEACHER, InPokemonSchool, 11, 3, OBJ_EVENT_GFX_WOMAN_2, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_SkillTeacher},
    {LOCALID_MECH_LAB_AIDE, InBirchsLab, 9, 10, OBJ_EVENT_GFX_SCIENTIST_2, MOVEMENT_TYPE_FACE_LEFT, Mechanics_EventScript_LabAide},
    {LOCALID_MECH_STATUE, StatueInPetalburgWoods, 14, 34, OBJ_EVENT_GFX_TRICK_HOUSE_STATUE, MOVEMENT_TYPE_NONE, Mechanics_EventScript_StatueTreecko},
    {LOCALID_MECH_STATUE, StatueInFieryPath, 28, 9, OBJ_EVENT_GFX_TRICK_HOUSE_STATUE, MOVEMENT_TYPE_NONE, Mechanics_EventScript_StatueTorchic},
    {LOCALID_MECH_STATUE, StatueOnRoute117, 18, 10, OBJ_EVENT_GFX_TRICK_HOUSE_STATUE, MOVEMENT_TYPE_NONE, Mechanics_EventScript_StatueMudkip},
    {LOCALID_MECH_EVO_SAGE, InSagesHouse, 7, 4, OBJ_EVENT_GFX_OLD_WOMAN, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_EvoSage},
    {LOCALID_MECH_BOSS, BossInGraniteCave, 31, 5, OBJ_EVENT_GFX_SPECIES(SABLEYE), MOVEMENT_TYPE_FACE_RIGHT, Mechanics_EventScript_BossGranite},
    {LOCALID_MECH_BOSS, BossInMeteorFalls, 13, 28, OBJ_EVENT_GFX_SPECIES(LUNATONE), MOVEMENT_TYPE_FACE_LEFT, Mechanics_EventScript_BossMeteor},
    {LOCALID_MECH_BOSS, BossInShoalCave, 21, 13, OBJ_EVENT_GFX_SPECIES(GLALIE), MOVEMENT_TYPE_FACE_LEFT, Mechanics_EventScript_BossShoal},
    {LOCALID_MECH_BOSS, BossInSeafloorCavern, 12, 11, OBJ_EVENT_GFX_SPECIES(SHARPEDO), MOVEMENT_TYPE_FACE_LEFT, Mechanics_EventScript_BossSeafloor},
    {LOCALID_MECH_BOSS, BossOnVictoryRoad, 13, 38, OBJ_EVENT_GFX_SPECIES(FLYGON), MOVEMENT_TYPE_FACE_RIGHT, Mechanics_EventScript_BossVictory},
    {LOCALID_MECH_STORM_RESEARCHER, InWeatherInstitute, 17, 3, OBJ_EVENT_GFX_SCIENTIST_1, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_StormResearcher},
    {LOCALID_MECH_STORM_ECHO, EchoOnRoute114, 9, 33, OBJ_EVENT_GFX_SPECIES(GROUDON), MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_EchoGroudon},
    {LOCALID_MECH_STORM_ECHO, EchoOnRoute118, 30, 11, OBJ_EVENT_GFX_SPECIES(KYOGRE), MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_EchoKyogre},
};

static EWRAM_DATA u8 sTimer = 0;

// The screenshot driver (tools/mechanics_shots/) writes a request here; the
// next overworld frame runs it.
struct MechanicsShowcase
{
    u8 pending;
    u8 fixMoves; // give party Pokémon without moves their level-up moves
    u16 moves[MAX_MON_MOVES]; // if set, the lead's moves
    u16 species;  // if set, a Pokémon of this species...
    u8 level;
    u8 variant;   // ...with this COLOR_* variant...
    u8 wild;      // ...given as the lead (0) or met in a wild battle (1)
    u8 persona;   // PERSONA_* + 1, or 0 for any
    u8 badges;    // sets the first this many badge flags
    u8 hurt;      // brings the lead down to a third of its HP
    u16 varId;    // if set, VarSet(varId, varValue): story state for a scene
                  // (below VARS_START: a flag, set if varValue isn't 0)
    u16 varValue;
    u8 friendship; // if set, the lead's friendship
};

EWRAM_DATA struct MechanicsShowcase gMechanicsShowcase = {0};

static void RunShowcase(void)
{
    u32 i;

    gMechanicsShowcase.pending = FALSE;
    if (gMechanicsShowcase.varId)
    {
        if (gMechanicsShowcase.varId < VARS_START && gMechanicsShowcase.varValue)
            FlagSet(gMechanicsShowcase.varId);
        else if (gMechanicsShowcase.varId < VARS_START)
            FlagClear(gMechanicsShowcase.varId);
        else
            VarSet(gMechanicsShowcase.varId, gMechanicsShowcase.varValue);
        gMechanicsShowcase.varId = 0;
    }
    if (gMechanicsShowcase.friendship)
    {
        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_FRIENDSHIP, &gMechanicsShowcase.friendship);
        gMechanicsShowcase.friendship = 0;
    }
    for (i = 0; i < gMechanicsShowcase.badges; i++)
        FlagSet(FLAG_BADGE01_GET + i);
    if (gMechanicsShowcase.hurt)
    {
        u32 hp = max(1, GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP) / 3);

        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
        gMechanicsShowcase.hurt = FALSE;
    }
    for (i = 0; gMechanicsShowcase.fixMoves && i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_MOVE1) == MOVE_NONE)
            GiveMonInitialMoveset(&gParties[B_TRAINER_PLAYER][i]);
    }
    for (i = 0; gMechanicsShowcase.moves[0] && i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], gMechanicsShowcase.moves[i], i);
    gMechanicsShowcase.moves[0] = MOVE_NONE;
    if (gMechanicsShowcase.species != SPECIES_NONE)
    {
        struct Pokemon *mon = gMechanicsShowcase.wild ? &gParties[B_TRAINER_OPPONENT_A][0] : &gParties[B_TRAINER_PLAYER][0];
        u32 personality;

        do
            personality = Random32();
        while (Mechanics_GetColorVariant(personality) != gMechanicsShowcase.variant
            || (gMechanicsShowcase.persona != 0 && Mechanics_GetPersona(personality) != gMechanicsShowcase.persona - 1));
        if (gMechanicsShowcase.wild)
        {
            ZeroEnemyPartyMons();
        }
        else if (gPartiesCount[B_TRAINER_PLAYER] < PARTY_SIZE)
        {
            memmove(&gParties[B_TRAINER_PLAYER][1], &gParties[B_TRAINER_PLAYER][0], sizeof(struct Pokemon) * (PARTY_SIZE - 1));
        }
        CreateMonWithIVsPersonality(mon, gMechanicsShowcase.species, gMechanicsShowcase.level, Random32() & 0x3FFFFFFF, personality);
        CalculatePlayerPartyCount();
        gMechanicsShowcase.species = SPECIES_NONE;
        if (gMechanicsShowcase.wild)
            BattleSetup_StartWildBattle();
        else
            UpdateFollowingPokemon();
    }
}

static bool32 IsShown(u8 localId)
{
    return GetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup) != OBJECT_EVENTS_COUNT;
}

// The entry that wants to be on this map under `localId`, or NULL.
static const struct MechNpc *GetWantedNpc(u8 localId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMechNpcs); i++)
    {
        if (sMechNpcs[i].localId == localId && sMechNpcs[i].isHere())
            return &sMechNpcs[i];
    }
    return NULL;
}

static void UpdateNpcs(void)
{
    u32 localId;

    for (localId = LOCALID_MECH_FIRST; localId <= LOCALID_MECH_LAST; localId++)
    {
        const struct MechNpc *npc = GetWantedNpc(localId);

        if ((npc != NULL) == IsShown(localId))
            continue;
        if (npc != NULL)
        {
            s16 x = npc->x + MAP_OFFSET, y = npc->y + MAP_OFFSET;
            SpawnSpecialObjectEventParameterized(npc->gfx, npc->movement, localId,
                                                 x, y, MapGridGetElevationAt(x, y));
        }
        else
        {
            RemoveObjectEventByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum,
                                             gSaveBlock1Ptr->location.mapGroup);
        }
    }
}

void Mechanics_Update(void)
{
    if (gMechanicsShowcase.pending)
        RunShowcase();
    Mechanics_UpdatePersonaFollower();
    Mechanics_UpdateSkills();
    if (++sTimer < 8 || ArePlayerFieldControlsLocked())
        return;
    sTimer = 0;
    UpdateNpcs();
    Mechanics_UpdateStorms();
}

bool32 Mechanics_TryInteract(s16 x, s16 y)
{
    u32 localId;

    for (localId = LOCALID_MECH_FIRST; localId <= LOCALID_MECH_LAST; localId++)
    {
        const struct MechNpc *npc = GetWantedNpc(localId);

        if (npc == NULL || npc->x + MAP_OFFSET != x || npc->y + MAP_OFFSET != y || !IsShown(localId))
            continue;
        gSelectedObjectEvent = GetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum,
                                                               gSaveBlock1Ptr->location.mapGroup);
        gSpecialVar_LastTalked = localId;
        gSpecialVar_Facing = GetPlayerFacingDirection();
        ScriptContext_SetupScript(npc->script);
        return TRUE;
    }
    return FALSE;
}
