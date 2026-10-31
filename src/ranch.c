#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "ranch.h"
#include "rtc.h"
#include "sound.h"
#include "string_util.h"
#include "trainer_level.h"
#include "world.h"
#include "constants/abilities.h"
#include "constants/event_objects.h"
#include "constants/items.h"

// Pokémon ranching: the Pokémon in the last PC box (named RANCH) live at the
// Hoenn Ranch behind the Route 117 Day Care. The first six wander the
// pasture; all of them gather things while the player is away, one find
// every RANCH_HOURS_PER_FIND hours of the clock each, kept for at most a
// day. The rancher hands them over: Berries from Grass and Bug types,
// Apricorns, Shards, Mushrooms, Pearls and Stardust by type, Pickup items,
// Honey from Honey Gather, and a few species' own specialities (Shuckle's
// Berry Juice, Miltank's Moomoo Milk, Chansey's Lucky Egg...). Most of it
// is what crafting needs.

#define RANCH_BOX            (TOTAL_BOXES_COUNT - 1)
#define RANCH_HOURS_PER_FIND 6
#define RANCH_MAX_HOURS      24
#define RANCH_MAX_HAUL       40
#define XP_RANCH             10

#define sSave (gSaveBlock3Ptr->adventure)

struct Find
{
    u16 item;
    u8 count;
    u8 mon; // box position of the first Pokémon that found it
};

static EWRAM_DATA struct Find sHaul[RANCH_MAX_HAUL] = {0};
static EWRAM_DATA u8 sHaulCount = 0;

static const u16 sBerries[] =
{
    ITEM_ORAN_BERRY, ITEM_PECHA_BERRY, ITEM_CHERI_BERRY, ITEM_CHESTO_BERRY,
    ITEM_RAWST_BERRY, ITEM_ASPEAR_BERRY, ITEM_LEPPA_BERRY, ITEM_ORAN_BERRY,
};

// Emerald's Pickup table, the common half and the rare half.
static const u16 sPickupItems[] =
{
    ITEM_SUPER_POTION, ITEM_FULL_HEAL, ITEM_ULTRA_BALL, ITEM_REVIVE,
    ITEM_HYPER_POTION, ITEM_ETHER, ITEM_TINY_MUSHROOM, ITEM_REPEL,
};
static const u16 sPickupRare[] =
{
    ITEM_RARE_CANDY, ITEM_NUGGET, ITEM_PP_UP, ITEM_FULL_RESTORE,
};

struct TypeFinds
{
    u16 common[2];
    u16 rare;
};

static const struct TypeFinds sTypeFinds[NUMBER_OF_MON_TYPES] =
{
    [TYPE_NORMAL]   = {{ITEM_WHITE_APRICORN,  ITEM_GREEN_APRICORN},  ITEM_PRETTY_FEATHER},
    [TYPE_FIGHTING] = {{ITEM_WHITE_APRICORN,  ITEM_RED_APRICORN},    ITEM_PROTEIN},
    [TYPE_FLYING]   = {{ITEM_GREEN_APRICORN,  ITEM_WHITE_APRICORN},  ITEM_PRETTY_FEATHER},
    [TYPE_POISON]   = {{ITEM_BLACK_APRICORN,  ITEM_TINY_MUSHROOM},   ITEM_BIG_MUSHROOM},
    [TYPE_GROUND]   = {{ITEM_RED_SHARD,       ITEM_YELLOW_SHARD},    ITEM_SOFT_SAND},
    [TYPE_ROCK]     = {{ITEM_BLUE_SHARD,      ITEM_GREEN_SHARD},     ITEM_HARD_STONE},
    [TYPE_BUG]      = {{ITEM_TINY_MUSHROOM,   ITEM_GREEN_APRICORN},  ITEM_SILVER_POWDER},
    [TYPE_GHOST]    = {{ITEM_BLACK_APRICORN,  ITEM_TINY_MUSHROOM},   ITEM_SPELL_TAG},
    [TYPE_STEEL]    = {{ITEM_RED_SHARD,       ITEM_BLUE_SHARD},      ITEM_METAL_COAT},
    [TYPE_FIRE]     = {{ITEM_RED_APRICORN,    ITEM_RED_SHARD},       ITEM_CHARCOAL},
    [TYPE_WATER]    = {{ITEM_BLUE_APRICORN,   ITEM_HEART_SCALE},     ITEM_PEARL},
    [TYPE_GRASS]    = {{ITEM_GREEN_APRICORN,  ITEM_TINY_MUSHROOM},   ITEM_SITRUS_BERRY},
    [TYPE_ELECTRIC] = {{ITEM_YELLOW_APRICORN, ITEM_YELLOW_SHARD},    ITEM_MAGNET},
    [TYPE_PSYCHIC]  = {{ITEM_PINK_APRICORN,   ITEM_STARDUST},        ITEM_STAR_PIECE},
    [TYPE_ICE]      = {{ITEM_BLUE_APRICORN,   ITEM_BLUE_SHARD},      ITEM_NEVER_MELT_ICE},
    [TYPE_DRAGON]   = {{ITEM_BLACK_APRICORN,  ITEM_YELLOW_APRICORN}, ITEM_DRAGON_SCALE},
    [TYPE_DARK]     = {{ITEM_BLACK_APRICORN,  ITEM_BLACK_APRICORN},  ITEM_BLACK_GLASSES},
    [TYPE_FAIRY]    = {{ITEM_PINK_APRICORN,   ITEM_STARDUST},        ITEM_STAR_PIECE},
};

// Hours on the clock. Starts a day in, so "a day ago" exists on day 0.
static u32 NowHours(void)
{
    RtcCalcLocalTime();
    return (gLocalTime.days + 1) * 24 + gLocalTime.hours;
}

// One find for a Pokémon; `roll` is its own random number for this find.
static u16 FindFor(struct BoxPokemon *mon, u32 roll)
{
    u32 species = GetBoxMonData(mon, MON_DATA_SPECIES);
    u32 ability = GetAbilityBySpecies(species, GetBoxMonData(mon, MON_DATA_ABILITY_NUM));
    u32 type = GetSpeciesType(species, roll & 1);
    u32 pick = (roll >> 8) % 16;

    // Species with something of their own (as in the main series).
    switch (species)
    {
    case SPECIES_SHUCKLE:
        return ITEM_BERRY_JUICE;
    case SPECIES_MILTANK:
        return ITEM_MOOMOO_MILK;
    case SPECIES_CHANSEY:
    case SPECIES_BLISSEY:
    case SPECIES_HAPPINY:
        return pick == 0 ? ITEM_LUCKY_EGG : ITEM_OVAL_STONE;
    case SPECIES_COMBEE:
    case SPECIES_VESPIQUEN:
        return ITEM_HONEY;
    default:
        break;
    }
    if (ability == ABILITY_HONEY_GATHER)
        return ITEM_HONEY;
    if (ability == ABILITY_PICKUP)
        return pick == 0 ? sPickupRare[(roll >> 16) % ARRAY_COUNT(sPickupRare)] : sPickupItems[(roll >> 16) % ARRAY_COUNT(sPickupItems)];
    if ((type == TYPE_GRASS || type == TYPE_BUG) && pick < 8)
        return sBerries[(roll >> 16) % ARRAY_COUNT(sBerries)];
    if (type >= NUMBER_OF_MON_TYPES || sTypeFinds[type].rare == ITEM_NONE)
        type = TYPE_NORMAL;
    if (pick == 0)
        return sTypeFinds[type].rare;
    return sTypeFinds[type].common[(roll >> 16) & 1];
}

static void AddFind(u16 item, u8 mon)
{
    u32 i;

    for (i = 0; i < sHaulCount; i++)
    {
        if (sHaul[i].item == item)
        {
            sHaul[i].count++;
            return;
        }
    }
    if (sHaulCount < RANCH_MAX_HAUL)
    {
        sHaul[sHaulCount].item = item;
        sHaul[sHaulCount].count = 1;
        sHaul[sHaulCount].mon = mon;
        sHaulCount++;
    }
}

u32 Ranch_CountMons(void)
{
    u32 i, n = 0;

    for (i = 0; i < IN_BOX_COUNT; i++)
    {
        u32 species = GetBoxMonDataAt(RANCH_BOX, i, MON_DATA_SPECIES_OR_EGG);
        n += (species != SPECIES_NONE && species != SPECIES_EGG);
    }
    return n;
}

// The rancher: what the Pokémon found since the last visit. VAR_RESULT =
// RANCH_TALK_* ; STR_VAR_1 = how many things.
void Ranch_Collect(void)
{
    u32 now = NowHours(), hours, finds, pos, n, total = 0;

    sHaulCount = 0;
    if (!sSave.ranchStarted)
    {
        // First visit: name the box and start the clock.
        StringCopy(GetBoxNamePtr(RANCH_BOX), COMPOUND_STRING("RANCH"));
        sSave.ranchStarted = TRUE;
        sSave.ranchLastHour = now;
        gSpecialVar_Result = RANCH_TALK_FIRST;
        return;
    }
    if (Ranch_CountMons() == 0)
    {
        sSave.ranchLastHour = now;
        gSpecialVar_Result = RANCH_TALK_EMPTY;
        return;
    }
    hours = now >= sSave.ranchLastHour ? now - sSave.ranchLastHour : 0;
    hours = min(hours, RANCH_MAX_HOURS);
    finds = hours / RANCH_HOURS_PER_FIND;
    if (finds == 0)
    {
        gSpecialVar_Result = RANCH_TALK_NOTHING_YET;
        return;
    }
    sSave.ranchLastHour = now - hours % RANCH_HOURS_PER_FIND;
    for (pos = 0; pos < IN_BOX_COUNT && total < RANCH_MAX_HAUL; pos++)
    {
        struct BoxPokemon *mon = GetBoxedMonPtr(RANCH_BOX, pos);
        u32 species = GetBoxMonData(mon, MON_DATA_SPECIES_OR_EGG);
        if (species == SPECIES_NONE || species == SPECIES_EGG)
            continue;
        for (n = 0; n < finds && total < RANCH_MAX_HAUL; n++, total++)
        {
            // Random, but the same for the same Pokémon and hour.
            u32 roll = GetBoxMonData(mon, MON_DATA_PERSONALITY) ^ ((now / RANCH_HOURS_PER_FIND - n) * 0x9E3779B1u);
            roll ^= roll >> 15;
            roll *= 0x2C1B3C6Du;
            roll ^= roll >> 12;
            AddFind(FindFor(mon, roll), pos);
        }
    }
    for (n = 0; n < sHaulCount; n++)
    {
        if (!AddBagItem(sHaul[n].item, sHaul[n].count))
            sHaul[n].count = 0; // no room: it's lost, like a full Pickup
    }
    sSave.ranchItemsGathered += total;
    TrainerLevel_AddXp(XP_RANCH);
    ConvertIntToDecimalStringN(gStringVar1, total, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = RANCH_TALK_HAUL;
}

static void GetHaulLine(u16 index, u8 *dest)
{
    u8 name[POKEMON_NAME_BUFFER_SIZE];

    ConvertIntToDecimalStringN(dest, sHaul[index].count, STR_CONV_MODE_RIGHT_ALIGN, 2);
    StringAppend(dest, COMPOUND_STRING(" "));
    CopyItemNameHandlePlural(sHaul[index].item, dest + StringLength(dest), sHaul[index].count);
    if (sHaul[index].count == 0)
    {
        StringAppend(dest, COMPOUND_STRING(" (BAG full)"));
        return;
    }
    GetBoxMonData(GetBoxedMonPtr(RANCH_BOX, sHaul[index].mon), MON_DATA_NICKNAME, name);
    StringAppend(dest, COMPOUND_STRING("{CLEAR_TO 120}by "));
    StringAppend(dest, name);
}

void Ranch_ShowHaul(void)
{
    WorldBoard_Show(COMPOUND_STRING("WHAT YOUR POKéMON FOUND"), sHaulCount, GetHaulLine, NULL);
}

// For the screenshot showcase (src/adventure.c): fills the RANCH box with
// these species and pretends the last visit was `hoursAgo` hours ago.
void Ranch_Showcase(const u16 *species, u32 count, u32 hoursAgo)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        struct BoxPokemon mon;
        if (species[i] == SPECIES_NONE)
            continue;
        CreateBoxMon(&mon, species[i], 20, Random32(), OTID_STRUCT_PLAYER_ID);
        SetBoxMonAt(RANCH_BOX, i, &mon);
    }
    if (hoursAgo != 0)
    {
        StringCopy(GetBoxNamePtr(RANCH_BOX), COMPOUND_STRING("RANCH"));
        sSave.ranchStarted = TRUE;
        sSave.ranchLastHour = NowHours() - hoursAgo;
    }
}

// The pasture shows the first RANCH_SHOWN Pokémon in the box; called when
// the ranch map loads. Empty spots hide their object.
void Ranch_SetupMap(void)
{
    u32 i, shown = 0;

    for (i = 0; i < IN_BOX_COUNT && shown < RANCH_SHOWN; i++)
    {
        struct BoxPokemon *mon = GetBoxedMonPtr(RANCH_BOX, i);
        u32 species = GetBoxMonData(mon, MON_DATA_SPECIES_OR_EGG);
        u32 gfx;
        if (species == SPECIES_NONE || species == SPECIES_EGG)
            continue;
        gfx = OBJ_EVENT_MON + species;
        if (GetBoxMonData(mon, MON_DATA_IS_SHINY))
            gfx |= OBJ_EVENT_MON_SHINY;
        if (GetBoxMonGender(mon) == MON_FEMALE)
            gfx |= OBJ_EVENT_MON_FEMALE;
        VarSet(VAR_OBJ_GFX_ID_0 + shown, gfx);
        sSave.ranchShown[shown] = i;
        FlagClear(FLAG_HIDE_RANCH_MON_1 + shown);
        shown++;
    }
    for (; shown < RANCH_SHOWN; shown++)
        FlagSet(FLAG_HIDE_RANCH_MON_1 + shown);
}

// Talking to a Pokémon in the pasture (VAR_0x8004 = which one): its cry,
// STR_VAR_1 = its nickname, STR_VAR_2 = what it's up to.
void Ranch_TalkToMon(void)
{
    struct BoxPokemon *mon = GetBoxedMonPtr(RANCH_BOX, sSave.ranchShown[gSpecialVar_0x8004 % RANCH_SHOWN]);
    u32 species = GetBoxMonData(mon, MON_DATA_SPECIES);
    static const u8 *const sDoing[] =
    {
        COMPOUND_STRING("is sniffing around for something."),
        COMPOUND_STRING("is having a nap in the sun."),
        COMPOUND_STRING("is digging happily."),
        COMPOUND_STRING("looks very pleased to see you!"),
    };

    GetBoxMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringCopy(gStringVar2, sDoing[(GetBoxMonData(mon, MON_DATA_PERSONALITY) + gLocalTime.hours) % ARRAY_COUNT(sDoing)]);
    PlayCry_Normal(species, 0);
}
