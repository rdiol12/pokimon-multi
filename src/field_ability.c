#include "global.h"
#include "battle.h"
#include "party_menu.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_ability.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "constants/abilities.h"
#include "constants/event_bg.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/songs.h"

// Pokémon abilities and types that do something outside battle
// (docs/field_abilities.md):
//  - Living lantern: a Fire-type lead, or Illuminate / Flame Body / Magma
//    Armor / Flash Fire, lights dark caves like Flash.
//  - Sixth sense: a Psychic-type lead, or Forewarn / Anticipation /
//    Telepathy, makes hidden items nearby sparkle.
//  - Frisk / Pickpocket / Magician: a wild Pokémon's held item ends up in
//    the bag when the player knocks it out.
//  - Swift Swim / Water Veil: surfing at running speed.
//  - Harvest (+2) / Grass-type lead (+1): extra berries from berry trees.
//  - Healer / Natural Cure: walking now and then cures a party member.
//  - World spots, spawned from C (no map file changes): ice blocks a Fire
//    type melts, dead generators an Electric type restarts (they light the
//    cave for good), sparkling sand a Ground type digs up (refills daily),
//    and boulders with something underneath that a Fighting type (or Guts,
//    Huge Power, Pure Power) can shove without Strength.

extern const u8 FieldAbility_EventScript_Ice[];
extern const u8 FieldAbility_EventScript_Generator[];
extern const u8 FieldAbility_EventScript_GeneratorOn[];
extern const u8 FieldAbility_EventScript_Dig[];
extern const u8 FieldAbility_EventScript_BoulderPrize[];
extern const u8 FieldAbility_EventScript_Lantern[];
extern const u8 FieldAbility_EventScript_Healer[];
extern const u8 FieldAbility_EventScript_Frisk[];
extern const u8 EventScript_StrengthBoulder[];

enum
{
    SPOT_ICE,
    SPOT_GENERATOR,
    SPOT_DIG,
    SPOT_BOULDER,
};

struct FieldSpot
{
    u16 map;
    u8 x, y;
    u8 kind;
    u16 item; // ITEM_NONE for a dig spot: it digs up a random treasure
    u16 litMaps[2]; // generators: the dark maps it lights
};

// Indexed by flag / local id - their FIRST. Append only: the index is saved.
static const struct FieldSpot sFieldSpots[] =
{
    {MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, 11, 14, SPOT_ICE, ITEM_NEVER_MELT_ICE},
    {MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, 17, 28, SPOT_ICE, ITEM_ICE_STONE},
    {MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM, 19, 5, SPOT_ICE, ITEM_PP_UP},
    {MAP_GRANITE_CAVE_B1F, 6, 8, SPOT_GENERATOR, ITEM_CELL_BATTERY, {MAP_GRANITE_CAVE_B1F, MAP_GRANITE_CAVE_B2F}},
    {MAP_VICTORY_ROAD_B1F, 10, 6, SPOT_GENERATOR, ITEM_MAGNET, {MAP_VICTORY_ROAD_B1F, MAP_VICTORY_ROAD_B2F}},
    {MAP_ROUTE111, 16, 52, SPOT_DIG},
    {MAP_ROUTE111, 29, 66, SPOT_DIG},
    {MAP_ROUTE111, 33, 79, SPOT_DIG},
    {MAP_ROUTE109, 22, 3, SPOT_DIG},
    {MAP_ROUTE109, 29, 13, SPOT_DIG},
    {MAP_METEOR_FALLS_1F_1R, 11, 10, SPOT_BOULDER, ITEM_RARE_CANDY},
    {MAP_RUSTURF_TUNNEL, 28, 15, SPOT_BOULDER, ITEM_PROTEIN},
    {MAP_GRANITE_CAVE_1F, 7, 8, SPOT_BOULDER, ITEM_EVERSTONE},
};

static const u16 sDigTreasures[] =
{
    ITEM_STARDUST, ITEM_STARDUST, ITEM_STARDUST, ITEM_HEART_SCALE, ITEM_HEART_SCALE,
    ITEM_PEARL, ITEM_PEARL, ITEM_SOFT_SAND, ITEM_HARD_STONE, ITEM_BIG_PEARL,
    ITEM_STAR_PIECE, ITEM_NUGGET,
};

static const u8 sText_Flames[] = _("flames");
static const u8 sText_Charge[] = _("electric charge");
static const u8 sText_Claws[] = _("digging claws");
static const u8 sText_Muscles[] = _("muscles");

struct FieldAbilityState
{
    u16 lastMap;
    u16 friskItem;
    u16 friskSpecies;
    u8 friskPending:1;
    u8 timer:7;
    u8 steps;
    u8 sparkles;
};

static EWRAM_DATA struct FieldAbilityState sState = {0};

// Showcase hooks for the screenshot driver (tools/field_ability_shots/).
EWRAM_DATA u8 gFieldAbilityShowcase = 0;

static bool32 IsMap(u16 map)
{
    return gSaveBlock1Ptr->location.mapGroup == (map >> 8) && gSaveBlock1Ptr->location.mapNum == (map & 0xFF);
}

static bool32 MonHasType(struct Pokemon *mon, enum Type type)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);

    return GetSpeciesType(species, 0) == type || GetSpeciesType(species, 1) == type;
}

static bool32 IsUsable(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG);
}

static bool32 HasAbility(struct Pokemon *mon, const u16 *list)
{
    enum Ability ability = GetMonAbility(mon);

    for (; *list != ABILITY_NONE; list++)
    {
        if (*list == ability)
            return TRUE;
    }
    return FALSE;
}

static const u16 sLanternAbilities[] = {ABILITY_ILLUMINATE, ABILITY_FLAME_BODY, ABILITY_MAGMA_ARMOR, ABILITY_FLASH_FIRE, ABILITY_NONE};
static const u16 sSixthSenseAbilities[] = {ABILITY_FOREWARN, ABILITY_ANTICIPATION, ABILITY_TELEPATHY, ABILITY_NONE};
static const u16 sFriskAbilities[] = {ABILITY_FRISK, ABILITY_PICKPOCKET, ABILITY_MAGICIAN, ABILITY_NONE};
static const u16 sSwimAbilities[] = {ABILITY_SWIFT_SWIM, ABILITY_WATER_VEIL, ABILITY_NONE};
static const u16 sHealerAbilities[] = {ABILITY_HEALER, ABILITY_NATURAL_CURE, ABILITY_NONE};
static const u16 sHeatAbilities[] = {ABILITY_FLAME_BODY, ABILITY_MAGMA_ARMOR, ABILITY_FLASH_FIRE, ABILITY_NONE};
static const u16 sChargeAbilities[] = {ABILITY_STATIC, ABILITY_VOLT_ABSORB, ABILITY_MOTOR_DRIVE, ABILITY_LIGHTNING_ROD, ABILITY_PLUS, ABILITY_MINUS, ABILITY_NONE};
static const u16 sDigAbilities[] = {ABILITY_SAND_VEIL, ABILITY_SAND_RUSH, ABILITY_SAND_FORCE, ABILITY_NONE};
static const u16 sStrongAbilities[] = {ABILITY_GUTS, ABILITY_HUGE_POWER, ABILITY_PURE_POWER, ABILITY_NONE};

// The Pokémon walking with the player.
static struct Pokemon *GetLead(void)
{
    return GetFirstLiveMon();
}

static bool32 LeadQualifies(enum Type type, const u16 *abilities)
{
    struct Pokemon *lead = GetLead();

    return lead != NULL && ((type != TYPE_NONE && MonHasType(lead, type)) || HasAbility(lead, abilities));
}

static bool32 IsDarkCave(void)
{
    return gMapHeader.cave;
}

bool32 FieldAbility_IsCaveLit(void)
{
    u32 i, j;

    for (i = 0; i < ARRAY_COUNT(sFieldSpots); i++)
    {
        if (sFieldSpots[i].kind != SPOT_GENERATOR || !FlagGet(FLAG_FIELD_ABILITY_FIRST + i))
            continue;
        for (j = 0; j < ARRAY_COUNT(sFieldSpots[i].litMaps); j++)
        {
            if (sFieldSpots[i].litMaps[j] != 0 && IsMap(sFieldSpots[i].litMaps[j]))
                return TRUE;
        }
    }
    return FALSE;
}

bool32 FieldAbility_HasLantern(void)
{
    return LeadQualifies(TYPE_FIRE, sLanternAbilities);
}

bool32 FieldAbility_SurfsFast(void)
{
    return LeadQualifies(TYPE_NONE, sSwimAbilities);
}

u32 FieldAbility_ExtraBerries(void)
{
    static const u16 harvest[] = {ABILITY_HARVEST, ABILITY_NONE};
    struct Pokemon *lead = GetLead();

    if (lead == NULL)
        return 0;
    if (HasAbility(lead, harvest))
        return 2;
    if (MonHasType(lead, TYPE_GRASS))
        return 1;
    return 0;
}

// Healer / Natural Cure: every 64 steps, a 50% chance to cure the first
// party member with a status problem.
bool32 FieldAbility_OnStep(void)
{
    struct Pokemon *lead;
    u32 i;

    if (++sState.steps < 64 && !gFieldAbilityShowcase)
        return FALSE;
    sState.steps = 0;
    if (!LeadQualifies(TYPE_NONE, sHealerAbilities) || (Random() % 2 && !gFieldAbilityShowcase))
        return FALSE;
    lead = GetLead();
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 none = 0;

        if (!IsUsable(mon) || GetMonData(mon, MON_DATA_STATUS) == 0 || GetMonData(mon, MON_DATA_HP) == 0)
            continue;
        SetMonData(mon, MON_DATA_STATUS, &none);
        GetMonNickname(lead, gStringVar1);
        StringCopy(gStringVar2, gAbilitiesInfo[GetMonAbility(lead)].name);
        GetMonNickname(mon, gStringVar3);
        gFieldAbilityShowcase = 0;
        ScriptContext_SetupScript(FieldAbility_EventScript_Healer);
        return TRUE;
    }
    return FALSE;
}

void FieldAbility_OnBattleStart(void)
{
    struct Pokemon *foe = &gParties[B_TRAINER_OPPONENT_A][0];

    sState.friskItem = ITEM_NONE;
    if (gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_SECRET_BASE))
        return;
    sState.friskItem = GetMonData(foe, MON_DATA_HELD_ITEM);
    sState.friskSpecies = GetMonData(foe, MON_DATA_SPECIES);
}

void FieldAbility_OnWildBattleEnd(void)
{
    if (sState.friskItem == ITEM_NONE || gBattleOutcome != B_OUTCOME_WON || !LeadQualifies(TYPE_NONE, sFriskAbilities))
        return;
    if (AddBagItem(sState.friskItem, 1))
        sState.friskPending = TRUE;
}

void FieldAbility_ResetDaily(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sFieldSpots); i++)
    {
        if (sFieldSpots[i].kind == SPOT_DIG)
            FlagClear(FLAG_FIELD_ABILITY_FIRST + i);
    }
}

static u16 GetSpotGfx(u32 i)
{
    switch (sFieldSpots[i].kind)
    {
    case SPOT_ICE:
        return OBJ_EVENT_GFX_FIELD_ICE_BLOCK;
    case SPOT_GENERATOR:
        return FlagGet(FLAG_FIELD_ABILITY_FIRST + i) ? OBJ_EVENT_GFX_FIELD_GENERATOR_ON : OBJ_EVENT_GFX_FIELD_GENERATOR;
    case SPOT_DIG:
        return OBJ_EVENT_GFX_FIELD_DIG_SPOT;
    default:
        return OBJ_EVENT_GFX_PUSHABLE_BOULDER;
    }
}

static bool32 IsSpotWanted(u32 i)
{
    if (!IsMap(sFieldSpots[i].map))
        return FALSE;
    switch (sFieldSpots[i].kind)
    {
    case SPOT_ICE:
    case SPOT_DIG:
        return !FlagGet(FLAG_FIELD_ABILITY_FIRST + i);
    default:
        return TRUE;
    }
}

static u8 GetSpotObjectId(u32 i)
{
    return GetObjectEventIdByLocalIdAndMap(LOCALID_FIELD_ABILITY_FIRST + i, gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup);
}

static void RemoveSpotObject(u32 i)
{
    RemoveObjectEventByLocalIdAndMap(LOCALID_FIELD_ABILITY_FIRST + i, gSaveBlock1Ptr->location.mapNum,
                                     gSaveBlock1Ptr->location.mapGroup);
}

static void UpdateSpots(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sFieldSpots); i++)
    {
        bool32 wanted = IsSpotWanted(i);
        u8 objectId = GetSpotObjectId(i);

        if (objectId != OBJECT_EVENTS_COUNT && (!wanted || gObjectEvents[objectId].graphicsId != GetSpotGfx(i)))
        {
            RemoveSpotObject(i);
            objectId = OBJECT_EVENTS_COUNT;
        }
        if (wanted && objectId == OBJECT_EVENTS_COUNT)
        {
            s16 x = sFieldSpots[i].x + MAP_OFFSET, y = sFieldSpots[i].y + MAP_OFFSET;

            SpawnSpecialObjectEventParameterized(GetSpotGfx(i), MOVEMENT_TYPE_NONE, LOCALID_FIELD_ABILITY_FIRST + i,
                                                 x, y, MapGridGetElevationAt(x, y));
        }
    }
}

// A boulder pushed off its spot shows what was under it.
static bool32 TryBoulderPrize(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sFieldSpots); i++)
    {
        u8 objectId;
        struct ObjectEvent *boulder;

        if (sFieldSpots[i].kind != SPOT_BOULDER || !IsMap(sFieldSpots[i].map) || FlagGet(FLAG_FIELD_ABILITY_FIRST + i))
            continue;
        objectId = GetSpotObjectId(i);
        if (objectId == OBJECT_EVENTS_COUNT)
            continue;
        boulder = &gObjectEvents[objectId];
        if (boulder->heldMovementActive && !boulder->heldMovementFinished)
            continue;
        if (boulder->currentCoords.x == sFieldSpots[i].x + MAP_OFFSET && boulder->currentCoords.y == sFieldSpots[i].y + MAP_OFFSET)
            continue;
        FlagSet(FLAG_FIELD_ABILITY_FIRST + i);
        gSpecialVar_0x8005 = sFieldSpots[i].item;
        ScriptContext_SetupScript(FieldAbility_EventScript_BoulderPrize);
        return TRUE;
    }
    return FALSE;
}

// Sixth sense: hidden items within a screen of the player sparkle.
static void UpdateSixthSense(void)
{
    const struct MapEvents *events = gMapHeader.events;
    s16 px, py;
    u32 i, found = 0;

    if (!LeadQualifies(TYPE_PSYCHIC, sSixthSenseAbilities))
    {
        sState.sparkles = 0;
        return;
    }
    PlayerGetDestCoords(&px, &py);
    px -= MAP_OFFSET;
    py -= MAP_OFFSET;
    for (i = 0; i < events->bgEventCount && found < 3; i++)
    {
        const struct BgEvent *bg = &events->bgEvents[i];

        if (bg->kind != BG_EVENT_HIDDEN_ITEM || FlagGet(FLAG_HIDDEN_ITEMS_START + bg->bgUnion.hiddenItem.hiddenItemId))
            continue;
        if (abs(bg->x - px) > 7 || abs(bg->y - py) > 5)
            continue;
        gFieldEffectArguments[0] = bg->x;
        gFieldEffectArguments[1] = bg->y;
        gFieldEffectArguments[2] = 2;
        FieldEffectStart(FLDEFF_SPARKLE);
        found++;
    }
    if (found > sState.sparkles)
        PlaySE(SE_ITEMFINDER);
    sState.sparkles = found;
}

static bool32 TryLanternMessage(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;

    if (!IsDarkCave() || FlagGet(FLAG_SYS_USE_FLASH) || FieldAbility_IsCaveLit() || !FieldAbility_HasLantern())
    {
        if (!IsDarkCave())
            sState.lastMap = 0xFFFF;
        return FALSE;
    }
    // Entering a dark cave already lit by the lantern, or the lead changed
    // to a lantern while in the dark.
    if (sState.lastMap == map && GetFlashLevel() <= 1)
        return FALSE;
    sState.lastMap = map;
    GetMonNickname(GetLead(), gStringVar1);
    gSpecialVar_0x8004 = GetFlashLevel() > 1;
    ScriptContext_SetupScript(FieldAbility_EventScript_Lantern);
    return TRUE;
}

void FieldAbility_Update(void)
{
    if (ArePlayerFieldControlsLocked())
        return;
    if (sState.friskPending)
    {
        sState.friskPending = FALSE;
        GetMonNickname(GetLead(), gStringVar1);
        StringCopy(gStringVar2, GetSpeciesName(sState.friskSpecies));
        CopyItemName(sState.friskItem, gStringVar3);
        ScriptContext_SetupScript(FieldAbility_EventScript_Frisk);
        return;
    }
    if (++sState.timer % 8)
        return;
    UpdateSpots();
    if (gPlayerAvatar.tileTransitionState == T_NOT_MOVING && (TryBoulderPrize() || TryLanternMessage()))
        return;
    if (sState.timer >= 64)
    {
        sState.timer = 0;
        UpdateSixthSense();
    }
}

bool32 FieldAbility_TryInteract(s16 x, s16 y)
{
    static const u8 *const scripts[] =
    {
        [SPOT_ICE] = FieldAbility_EventScript_Ice,
        [SPOT_GENERATOR] = FieldAbility_EventScript_Generator,
        [SPOT_DIG] = FieldAbility_EventScript_Dig,
        [SPOT_BOULDER] = EventScript_StrengthBoulder,
    };
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sFieldSpots); i++)
    {
        const struct FieldSpot *spot = &sFieldSpots[i];
        u8 objectId = GetSpotObjectId(i);

        if (objectId == OBJECT_EVENTS_COUNT || gObjectEvents[objectId].currentCoords.x != x || gObjectEvents[objectId].currentCoords.y != y)
            continue;
        gSelectedObjectEvent = objectId;
        gSpecialVar_LastTalked = LOCALID_FIELD_ABILITY_FIRST + i;
        gSpecialVar_Facing = GetPlayerFacingDirection();
        gSpecialVar_0x8004 = i;
        if (spot->kind == SPOT_GENERATOR && FlagGet(FLAG_FIELD_ABILITY_FIRST + i))
            ScriptContext_SetupScript(FieldAbility_EventScript_GeneratorOn);
        else
            ScriptContext_SetupScript(scripts[spot->kind]);
        return TRUE;
    }
    return FALSE;
}

// Picks the party member who can deal with spot VAR_0x8004: VAR_RESULT is
// its slot (PARTY_SIZE if none), STR_VAR_1 its name, STR_VAR_2 what it uses.
static u32 FindHelper(enum Type type, const u16 *abilities, const u8 *what)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        bool32 byAbility;

        if (!IsUsable(mon))
            continue;
        byAbility = HasAbility(mon, abilities);
        if (!byAbility && !MonHasType(mon, type))
            continue;
        GetMonNickname(mon, gStringVar1);
        StringCopy(gStringVar2, byAbility ? gAbilitiesInfo[GetMonAbility(mon)].name : what);
        return i;
    }
    return PARTY_SIZE;
}

void FieldAbility_FindHelper(void)
{
    switch (sFieldSpots[gSpecialVar_0x8004].kind)
    {
    case SPOT_ICE:
        gSpecialVar_Result = FindHelper(TYPE_FIRE, sHeatAbilities, sText_Flames);
        break;
    case SPOT_GENERATOR:
        gSpecialVar_Result = FindHelper(TYPE_ELECTRIC, sChargeAbilities, sText_Charge);
        break;
    case SPOT_DIG:
        gSpecialVar_Result = FindHelper(TYPE_GROUND, sDigAbilities, sText_Claws);
        break;
    default:
        gSpecialVar_Result = PARTY_SIZE;
        break;
    }
}

// Strength boulders: a Fighting type, or Guts / Huge Power / Pure Power,
// pushes them without the move.
void FieldAbility_FindStrongMon(void)
{
    gSpecialVar_Result = FindHelper(TYPE_FIGHTING, sStrongAbilities, sText_Muscles);
}

// Spot VAR_0x8004 is done: VAR_0x8005 is what the player finds there.
void FieldAbility_Solve(void)
{
    u32 i = gSpecialVar_0x8004;

    FlagSet(FLAG_FIELD_ABILITY_FIRST + i);
    gSpecialVar_0x8005 = sFieldSpots[i].item;
    if (sFieldSpots[i].kind == SPOT_DIG)
        gSpecialVar_0x8005 = sDigTreasures[Random() % ARRAY_COUNT(sDigTreasures)];
    if (sFieldSpots[i].kind != SPOT_BOULDER)
        RemoveSpotObject(i);
    UpdateSpots();
}
