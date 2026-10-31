#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "pokemon.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/party_menu.h"

// Museum donations: the Lilycove Museum opens a natural history wing. Its
// curator takes fossils, records rare Pokémon you show her (they stay
// yours) and collects photos from photo mode. Donated fossils and rare
// Pokémon stand in the hall as models. Every few donations earn a reward.
// R > PASTIMES > MUSEUM lists the exhibits.

extern const u8 Museum_EventScript_Curator[];
extern const u8 Museum_EventScript_Statue[];

struct Fossil
{
    u16 item;
    u16 species; // the model in the hall
};

static const struct Fossil sFossils[MUSEUM_FOSSIL_COUNT] =
{
    {ITEM_ROOT_FOSSIL,      SPECIES_LILEEP},
    {ITEM_CLAW_FOSSIL,      SPECIES_ANORITH},
    {ITEM_HELIX_FOSSIL,     SPECIES_OMANYTE},
    {ITEM_DOME_FOSSIL,      SPECIES_KABUTO},
    {ITEM_OLD_AMBER,        SPECIES_AERODACTYL},
    {ITEM_SKULL_FOSSIL,     SPECIES_CRANIDOS},
    {ITEM_ARMOR_FOSSIL,     SPECIES_SHIELDON},
    {ITEM_COVER_FOSSIL,     SPECIES_TIRTOUGA},
    {ITEM_PLUME_FOSSIL,     SPECIES_ARCHEN},
    {ITEM_JAW_FOSSIL,       SPECIES_TYRUNT},
    {ITEM_SAIL_FOSSIL,      SPECIES_AMAURA},
    {ITEM_FOSSILIZED_BIRD,  SPECIES_ARCTOZOLT},
    {ITEM_FOSSILIZED_FISH,  SPECIES_ARCTOVISH},
    {ITEM_FOSSILIZED_DRAKE, SPECIES_DRACOZOLT},
    {ITEM_FOSSILIZED_DINO,  SPECIES_DRACOVISH},
};

// Final forms of the pseudo-legendaries count as rare too.
static const u16 sPseudoLegends[] =
{
    SPECIES_DRAGONITE, SPECIES_TYRANITAR, SPECIES_SALAMENCE, SPECIES_METAGROSS, SPECIES_GARCHOMP,
    SPECIES_HYDREIGON, SPECIES_GOODRA, SPECIES_KOMMO_O, SPECIES_DRAGAPULT, SPECIES_BAXCALIBUR,
};

// Exhibit points (fossils + rare Pokémon + photo tiers) at which the
// curator hands out each reward.
static const struct { u8 points; u16 item; } sRewards[] =
{
    {1,  ITEM_NUGGET},
    {3,  ITEM_RARE_CANDY},
    {5,  ITEM_PP_MAX},
    {8,  ITEM_BIG_NUGGET},
    {12, ITEM_COMET_SHARD},
    {16, ITEM_ABILITY_PATCH},
    {22, ITEM_MASTER_BALL},
};

// Photos donated for each tier of the photo wall.
static const u16 sPhotoTiers[] = {5, 15, 30};

#define sSave (gSaveBlock3Ptr->pastime)

// The curator, then the fossil models, then the rare Pokémon models.
const struct PastimeSpot gMuseumSpots[MUSEUM_SPOT_COUNT] =
{
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 20, 5, OBJ_EVENT_GFX_SCIENTIST_2, MOVEMENT_TYPE_FACE_LEFT, 0, Museum_EventScript_Curator},
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 16, 6, 0, MOVEMENT_TYPE_FACE_DOWN, 0, Museum_EventScript_Statue, Museum_IsStatueShown, Museum_GetStatueGraphicsId},
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 17, 6, 0, MOVEMENT_TYPE_FACE_DOWN, 1, Museum_EventScript_Statue, Museum_IsStatueShown, Museum_GetStatueGraphicsId},
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 18, 6, 0, MOVEMENT_TYPE_FACE_DOWN, 2, Museum_EventScript_Statue, Museum_IsStatueShown, Museum_GetStatueGraphicsId},
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 19, 6, 0, MOVEMENT_TYPE_FACE_DOWN, 3, Museum_EventScript_Statue, Museum_IsStatueShown, Museum_GetStatueGraphicsId},
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 10, 8, 0, MOVEMENT_TYPE_FACE_DOWN, 4, Museum_EventScript_Statue, Museum_IsStatueShown, Museum_GetStatueGraphicsId},
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 11, 8, 0, MOVEMENT_TYPE_FACE_DOWN, 5, Museum_EventScript_Statue, Museum_IsStatueShown, Museum_GetStatueGraphicsId},
    {MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 12, 8, 0, MOVEMENT_TYPE_FACE_DOWN, 6, Museum_EventScript_Statue, Museum_IsStatueShown, Museum_GetStatueGraphicsId},
};

static u32 CountFossils(void)
{
    u32 i, count = 0;

    for (i = 0; i < MUSEUM_FOSSIL_COUNT; i++)
        count += (sSave.museumFossils >> i) & 1;
    return count;
}

static u32 CountRare(void)
{
    u32 i;

    for (i = 0; i < MUSEUM_RARE_SLOTS && sSave.museumRare[i] != SPECIES_NONE; i++)
        ;
    return i;
}

static u32 PhotoTier(void)
{
    u32 tier = 0;

    while (tier < ARRAY_COUNT(sPhotoTiers) && sSave.museumPhotos >= sPhotoTiers[tier])
        tier++;
    return tier;
}

static u32 Points(void)
{
    return CountFossils() + CountRare() + PhotoTier();
}

// Statue arg: 0-3 the first donated fossils, 4-6 the first rare Pokémon.
static u16 StatueSpecies(u32 arg)
{
    u32 i, n = 0;

    if (arg >= MUSEUM_FOSSIL_STATUES)
        return sSave.museumRare[arg - MUSEUM_FOSSIL_STATUES];
    for (i = 0; i < MUSEUM_FOSSIL_COUNT; i++)
    {
        if ((sSave.museumFossils >> i) & 1 && n++ == arg)
            return sFossils[i].species;
    }
    return SPECIES_NONE;
}

bool32 Museum_IsStatueShown(u32 arg)
{
    return StatueSpecies(arg) != SPECIES_NONE;
}

u16 Museum_GetStatueGraphicsId(u32 arg)
{
    return OBJ_EVENT_MON + StatueSpecies(arg);
}

// Script: the first fossil in the bag not yet in the museum. VAR_RESULT =
// TRUE if there is one, VAR_0x8005 = its index, STR_VAR_1 = its name.
void Museum_FindFossil(void)
{
    u32 i;

    gSpecialVar_Result = FALSE;
    for (i = 0; i < MUSEUM_FOSSIL_COUNT; i++)
    {
        if (!((sSave.museumFossils >> i) & 1) && CheckBagHasItem(sFossils[i].item, 1))
        {
            gSpecialVar_Result = TRUE;
            gSpecialVar_0x8005 = i;
            CopyItemName(sFossils[i].item, gStringVar1);
            StringCopy(gStringVar2, GetSpeciesName(sFossils[i].species));
            return;
        }
    }
}

void Museum_DonateFossil(void)
{
    u32 i = gSpecialVar_0x8005;

    RemoveBagItem(sFossils[i].item, 1);
    sSave.museumFossils |= 1 << i;
}

static bool32 IsRare(struct Pokemon *mon, u16 species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    u32 i;

    if (IsMonShiny(mon) || info->isRestrictedLegendary || info->isSubLegendary || info->isMythical
     || info->isUltraBeast || info->isParadox)
        return TRUE;
    for (i = 0; i < ARRAY_COUNT(sPseudoLegends); i++)
    {
        if (sPseudoLegends[i] == species)
            return TRUE;
    }
    return FALSE;
}

// Script: after ChoosePartyMon (VAR_0x8004 = the slot). VAR_RESULT =
// MUSEUM_RARE_*, STR_VAR_1 = the species.
void Museum_ShowRare(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 i, count = CountRare();

    StringCopy(gStringVar1, GetSpeciesName(species));
    if (GetMonData(mon, MON_DATA_IS_EGG))
    {
        gSpecialVar_Result = MUSEUM_RARE_EGG;
        return;
    }
    for (i = 0; i < count; i++)
    {
        if (sSave.museumRare[i] == species)
        {
            gSpecialVar_Result = MUSEUM_RARE_ALREADY;
            return;
        }
    }
    if (!IsRare(mon, species))
        gSpecialVar_Result = MUSEUM_RARE_NOT_RARE;
    else if (count >= MUSEUM_RARE_SLOTS)
        gSpecialVar_Result = MUSEUM_RARE_FULL;
    else
    {
        sSave.museumRare[count] = species;
        gSpecialVar_Result = MUSEUM_RARE_OK;
    }
}

// Script: VAR_RESULT = how many new photos were donated (STR_VAR_1 too).
void Museum_DonatePhotos(void)
{
    u16 taken = gSaveBlock3Ptr->world.photosTaken;

    gSpecialVar_Result = taken > sSave.museumPhotos ? taken - sSave.museumPhotos : 0;
    sSave.museumPhotos = taken > sSave.museumPhotos ? taken : sSave.museumPhotos;
    ConvertIntToDecimalStringN(gStringVar1, gSpecialVar_Result, STR_CONV_MODE_LEFT_ALIGN, 5);
}

// Script: VAR_RESULT = TRUE if a reward is due, VAR_0x8005 = the item.
void Museum_CheckReward(void)
{
    gSpecialVar_Result = FALSE;
    if (sSave.museumRewards < ARRAY_COUNT(sRewards) && Points() >= sRewards[sSave.museumRewards].points)
    {
        gSpecialVar_Result = TRUE;
        gSpecialVar_0x8005 = sRewards[sSave.museumRewards++].item;
    }
}

// Script: talking to a model (VAR_0x8004 = the statue).
void Museum_DescribeStatue(void)
{
    u32 arg = gSpecialVar_0x8004, i, n = 0;
    u16 species = StatueSpecies(arg);
    u8 *p = gPastimeText;

    p = StringCopy(p, COMPOUND_STRING("A life-size model of "));
    p = StringCopy(p, GetSpeciesName(species));
    if (arg >= MUSEUM_FOSSIL_STATUES)
    {
        StringCopy(p, COMPOUND_STRING(".\pThe plaque reads: “A rare Pokémon,\nshown to the museum by {PLAYER}.”"));
        return;
    }
    for (i = 0; i < MUSEUM_FOSSIL_COUNT; i++)
    {
        if ((sSave.museumFossils >> i) & 1 && n++ == arg)
            break;
    }
    p = StringCopy(p, COMPOUND_STRING(".\pThe plaque reads: “Rebuilt from the\n"));
    CopyItemName(sFossils[i].item, p);
    p += StringLength(p);
    StringCopy(p, COMPOUND_STRING(",\lgiven by {PLAYER}.”"));
}

// R > PASTIMES > MUSEUM -------------------------------------------------

// Lines: the fossils, the rare Pokémon slots, then the photo wall.
static void GetLine(u16 index, u8 *dest)
{
    if (index < MUSEUM_FOSSIL_COUNT)
    {
        if ((sSave.museumFossils >> index) & 1)
        {
            CopyItemName(sFossils[index].item, dest);
            dest += StringLength(dest);
            dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 96}"));
            StringCopy(dest, GetSpeciesName(sFossils[index].species));
        }
        else
        {
            StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}Fossil ?????"));
        }
    }
    else if (index < MUSEUM_FOSSIL_COUNT + MUSEUM_RARE_SLOTS)
    {
        u16 species = sSave.museumRare[index - MUSEUM_FOSSIL_COUNT];
        dest = StringCopy(dest, COMPOUND_STRING("Rare Pokémon{CLEAR_TO 96}"));
        if (species != SPECIES_NONE)
            StringCopy(dest, GetSpeciesName(species));
        else
            StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}-----"));
    }
    else
    {
        dest = StringCopy(dest, COMPOUND_STRING("Photo wall{CLEAR_TO 96}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.museumPhotos, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringCopy(dest, COMPOUND_STRING(" photos"));
    }
}

static void GetDetail(u16 index, u8 *dest)
{
    if (index < MUSEUM_FOSSIL_COUNT)
        StringCopy(dest, COMPOUND_STRING("Fossils: bring them to the curator."));
    else if (index < MUSEUM_FOSSIL_COUNT + MUSEUM_RARE_SLOTS)
        StringCopy(dest, COMPOUND_STRING("Legends, shinies, Ultra Beasts, pseudo-legends…"));
    else if (PhotoTier() < ARRAY_COUNT(sPhotoTiers))
    {
        dest = StringCopy(dest, COMPOUND_STRING("Next wall section at "));
        dest = ConvertIntToDecimalStringN(dest, sPhotoTiers[PhotoTier()], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringCopy(dest, COMPOUND_STRING(" photos."));
    }
    else
        StringCopy(dest, COMPOUND_STRING("The photo wall is complete!"));
}

static EWRAM_DATA u8 sTitle[40] = {0};

void Museum_ShowList(void)
{
    u8 *p = StringCopy(sTitle, COMPOUND_STRING("MUSEUM   Exhibit points "));
    ConvertIntToDecimalStringN(p, Points(), STR_CONV_MODE_LEFT_ALIGN, 2);
    WorldBoard_Show(sTitle, MUSEUM_FOSSIL_COUNT + MUSEUM_RARE_SLOTS + 1, GetLine, GetDetail);
}
