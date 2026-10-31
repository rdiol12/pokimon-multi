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
// Each has its own local id, LOCALID_MECH_FIRST + its index.

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

struct MechNpc
{
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

static bool32 InFallarborTent(void)
{
    return IsMap(MAP_FALLARBOR_TOWN_BATTLE_TENT_LOBBY);
}

// Indexed by local id - LOCALID_MECH_FIRST.
static const struct MechNpc sMechNpcs[] =
{
    {InPokemonCenter, 13, 5, OBJ_EVENT_GFX_EXPERT_M, MOVEMENT_TYPE_FACE_LEFT, Mechanics_EventScript_MoveTutor},
    {InSlateportTent, 9, 6, OBJ_EVENT_GFX_GENTLEMAN, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_Referee},
    {InVerdanturfTent, 4, 6, OBJ_EVENT_GFX_REPORTER_M, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_CupHost},
    {InFallarborTent, 4, 6, OBJ_EVENT_GFX_EXPERT_F, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_StyleMaster},
    {InPokemonSchool, 11, 3, OBJ_EVENT_GFX_WOMAN_2, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_SkillTeacher},
    {InBirchsLab, 9, 10, OBJ_EVENT_GFX_SCIENTIST_2, MOVEMENT_TYPE_FACE_LEFT, Mechanics_EventScript_LabAide},
    {StatueInPetalburgWoods, 14, 34, OBJ_EVENT_GFX_TRICK_HOUSE_STATUE, MOVEMENT_TYPE_NONE, Mechanics_EventScript_StatueTreecko},
    {StatueInFieryPath, 28, 9, OBJ_EVENT_GFX_TRICK_HOUSE_STATUE, MOVEMENT_TYPE_NONE, Mechanics_EventScript_StatueTorchic},
    {StatueOnRoute117, 18, 10, OBJ_EVENT_GFX_TRICK_HOUSE_STATUE, MOVEMENT_TYPE_NONE, Mechanics_EventScript_StatueMudkip},
    {InSagesHouse, 7, 4, OBJ_EVENT_GFX_OLD_WOMAN, MOVEMENT_TYPE_FACE_DOWN, Mechanics_EventScript_EvoSage},
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

static bool32 IsShown(u32 i)
{
    return GetObjectEventIdByLocalIdAndMap(LOCALID_MECH_FIRST + i, gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup) != OBJECT_EVENTS_COUNT;
}

static void UpdateNpcs(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMechNpcs); i++)
    {
        const struct MechNpc *npc = &sMechNpcs[i];
        bool32 wanted = npc->isHere();

        if (wanted == IsShown(i))
            continue;
        if (wanted)
        {
            s16 x = npc->x + MAP_OFFSET, y = npc->y + MAP_OFFSET;
            SpawnSpecialObjectEventParameterized(npc->gfx, npc->movement, LOCALID_MECH_FIRST + i,
                                                 x, y, MapGridGetElevationAt(x, y));
        }
        else
        {
            RemoveObjectEventByLocalIdAndMap(LOCALID_MECH_FIRST + i, gSaveBlock1Ptr->location.mapNum,
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
}

bool32 Mechanics_TryInteract(s16 x, s16 y)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMechNpcs); i++)
    {
        const struct MechNpc *npc = &sMechNpcs[i];
        u8 objectId;

        if (npc->x + MAP_OFFSET != x || npc->y + MAP_OFFSET != y || !IsShown(i))
            continue;
        objectId = GetObjectEventIdByLocalIdAndMap(LOCALID_MECH_FIRST + i, gSaveBlock1Ptr->location.mapNum,
                                                   gSaveBlock1Ptr->location.mapGroup);
        gSelectedObjectEvent = objectId;
        gSpecialVar_LastTalked = LOCALID_MECH_FIRST + i;
        gSpecialVar_Facing = GetPlayerFacingDirection();
        ScriptContext_SetupScript(npc->script);
        return TRUE;
    }
    return FALSE;
}
