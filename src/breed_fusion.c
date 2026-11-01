#include "global.h"
#include "save_ext.h"
#include "breed_fusion.h"
#include "community.h"
#include "main.h"
#include "constants/rgb.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "script.h"
#include "daycare.h"
#include "decompress.h"
#include "malloc.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/moves.h"

// Fusion breeding. See include/breed_fusion.h for the rules. Nothing here
// needs save data beyond the partner species kept in each Pokémon, and
// the only EWRAM is a few caches.
//
// Colors: both parents' front sprites are measured (how many pixels use
// each palette color) and each palette is split into color families:
// hues that belong together, plus one family for greys and whites. The
// darkest colors (outlines) are left alone. The baby's biggest family
// takes the father's biggest family's color, the second the second, and
// so on, keeping the baby's own shading inside each family.

#define NUM_COLORS       16
#define MAX_FAMILIES     4
#define KEEP             0xFF
#define HUE_RANGE        1536 // 6 sectors of 256
#define HUE_NEAR         160  // hues closer than this (about 37°) join one family
#define DARK_LIMIT       44   // lightness below this is an outline, kept as is
#define GREY_LIMIT       40   // saturation below this is a grey/white
// Caches: blended palettes made (each needs a sprite decompressed and
// measured) and partners looked up in the party and boxes. 3 and 4 on the
// GBA; extended EWRAM fits enough for a full double battle, party and box.
#define PALETTE_SLOTS    32
#define PARTNER_SLOTS    48
#define FUSION_FOLLOWER_PAL_TAG  0xF000 // + a hash of the pair, below TAG_NONE
#define FUSION_FOLLOWER_PAL_TAGS 0x0F00

struct Hsl
{
    s16 h, s, l;
};

struct Families
{
    u8 of[NUM_COLORS];        // family of each color, or KEEP
    u8 rep[MAX_FAMILIES];     // the color that stands for each family (its most used)
    u8 count;                 // number of families, biggest first
};

struct FusionPalette
{
    const u16 *source; // the Pokémon's own palette (normal, shiny or female)
    u16 species;
    u16 partner;
    u16 colors[NUM_COLORS];
};

struct PartnerCacheEntry
{
    u32 personality;
    u16 partner;
};

static void CountPicPixels(const u8 *pic, u32 size, u16 *counts);

static EWRAM_EXT_DATA struct FusionPalette sPalettes[PALETTE_SLOTS] = {0};
static EWRAM_DATA u8 sNextPalette = 0;
static EWRAM_EXT_DATA struct PartnerCacheEntry sPartners[PARTNER_SLOTS] = {0};
static EWRAM_DATA u8 sNextPartner = 0;
static EWRAM_DATA u8 sRequestState = 0;   // REQUEST_*
static EWRAM_DATA u8 sRequestFailures = 0;
static EWRAM_DATA u16 sRequestTimer = 0;
static EWRAM_DATA u32 sRefusedPair = 0;   // species | partner << 16 the server said can't fuse

#define REQUEST_IDLE     0
#define REQUEST_SENT     1
#define REQUEST_TIMEOUT  (60 * 15)
#define MAX_FAILURES     3
#define FUSION_CANT_FUSE 2 // the server's answer for a pair that can't fuse

u32 BreedFusion_GetPartner(struct BoxPokemon *boxMon)
{
    u32 partner = GetBoxMonData(boxMon, MON_DATA_FUSION_PARTNER);

    if (partner == SPECIES_NONE || partner >= NUM_SPECIES)
        return SPECIES_NONE;
    return partner;
}

// ---------------------------------------------------------------------------
// Breeding

static bool32 IsDitto(u32 species)
{
    return gSpeciesInfo[species].eggGroups[0] == EGG_GROUP_DITTO
        || gSpeciesInfo[species].eggGroups[1] == EGG_GROUP_DITTO;
}

// Any two Pokémon of different evolution families can fuse, except Ditto
// (it makes ordinary eggs) and Pokémon that can't breed at all.
bool32 BreedFusion_CanFuse(u32 motherSpecies, u32 fatherSpecies)
{
    if (motherSpecies == SPECIES_NONE || fatherSpecies == SPECIES_NONE)
        return FALSE;
    if (IsDitto(motherSpecies) || IsDitto(fatherSpecies))
        return FALSE;
    if (gSpeciesInfo[motherSpecies].eggGroups[0] == EGG_GROUP_NO_EGGS_DISCOVERED
     || gSpeciesInfo[fatherSpecies].eggGroups[0] == EGG_GROUP_NO_EGGS_DISCOVERED)
        return FALSE;
    return GetEggSpecies(motherSpecies) != GetEggSpecies(fatherSpecies);
}

// The two Pokémon in this Day Care would make a fusion egg.
bool32 BreedFusion_CanFuseDaycare(struct DayCare *daycare)
{
    struct BoxPokemon *a = &daycare->mons[0].mon, *b = &daycare->mons[1].mon;

    if (GetBoxMonGender(a) != MON_FEMALE)
    {
        struct BoxPokemon *t = a;
        a = b;
        b = t;
    }
    if (GetBoxMonGender(a) != MON_FEMALE || GetBoxMonGender(b) != MON_MALE)
        return FALSE;
    return BreedFusion_CanFuse(GetBoxMonData(a, MON_DATA_SPECIES), GetBoxMonData(b, MON_DATA_SPECIES));
}

static bool32 KnowsMove(struct Pokemon *mon, u32 move)
{
    u32 i;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == move)
            return TRUE;
    }
    return FALSE;
}

// Marks the egg as a fusion with the father, and passes on the father's
// first two moves, so the baby starts with something of both parents.
void BreedFusion_MakeEgg(struct Pokemon *egg, struct BoxPokemon *father)
{
    u16 partner = GetBoxMonData(father, MON_DATA_SPECIES);
    u32 i, given = 0;

    SetMonData(egg, MON_DATA_FUSION_PARTNER, &partner);
    for (i = 0; i < MAX_MON_MOVES && given < 2; i++)
    {
        u32 move = GetBoxMonData(father, MON_DATA_MOVE1 + i);

        if (move == MOVE_NONE || KnowsMove(egg, move))
            continue;
        if (GiveMoveToMon(egg, move) == MON_HAS_MAX_MOVES)
            DeleteFirstMoveAndGiveMoveToMon(egg, move);
        given++;
    }
}

// ---------------------------------------------------------------------------
// Types, stats and name

void BreedFusion_GetTypes(u32 species, u32 partner, enum Type *types)
{
    types[0] = GetSpeciesType(species, 0);
    types[1] = GetSpeciesType(species, 1);
    if (partner == SPECIES_NONE)
        return;
    // Mother's first type, father's first type (or his second, if the first matches).
    if (GetSpeciesType(partner, 0) != types[0])
        types[1] = GetSpeciesType(partner, 0);
    else if (GetSpeciesType(partner, 1) != types[0])
        types[1] = GetSpeciesType(partner, 1);
}

void BreedFusion_SetBattlerTypes(struct BoxPokemon *boxMon, u32 species, enum Type *types)
{
    u32 partner = BreedFusion_GetPartner(boxMon);

    if (partner != SPECIES_NONE)
        BreedFusion_GetTypes(species, partner, types);
}

u32 BreedFusion_GetBaseStat(struct BoxPokemon *boxMon, u32 species, u32 stat)
{
    u32 partner = BreedFusion_GetPartner(boxMon);
    u32 value = GetSpeciesBaseStat(species, stat);

    if (partner == SPECIES_NONE)
        return value;
    return (value + GetSpeciesBaseStat(partner, stat) + 1) / 2;
}

static u32 NameLength(const u8 *s)
{
    u32 n = 0;

    while (s[n] != EOS && n < POKEMON_NAME_LENGTH)
        n++;
    return n;
}

// The first half of the mother's name and the last half of the father's:
// Wingull + Pikachu = Wingchu.
void BreedFusion_BufferName(u8 *dest, u32 species, u32 partner)
{
    const u8 *a = GetSpeciesName(species), *b = GetSpeciesName(partner);
    u32 lenA = NameLength(a), lenB = NameLength(b);
    u32 headLen = (lenA + 1) / 2, tailStart = lenB - lenB / 2, i, n = 0;

    for (i = 0; i < headLen; i++)
        dest[n++] = a[i];
    // Skip spaces and punctuation at the joint ("Mr. Mime" gives "Mime").
    while (tailStart < lenB && !(b[tailStart] >= CHAR_A && b[tailStart] <= CHAR_z))
        tailStart++;
    for (i = tailStart; i < lenB && n < POKEMON_NAME_LENGTH; i++)
    {
        u8 c = b[i];

        if (c >= CHAR_A && c <= CHAR_Z)
            c += CHAR_a - CHAR_A;
        dest[n++] = c;
    }
    dest[n] = EOS;
}

void BreedFusion_SetHatchedName(struct Pokemon *mon)
{
    u8 name[POKEMON_NAME_BUFFER_SIZE];
    u32 partner = BreedFusion_GetPartner(&mon->box);

    if (partner == SPECIES_NONE)
        return;
    BreedFusion_BufferName(name, GetMonData(mon, MON_DATA_SPECIES), partner);
    SetMonData(mon, MON_DATA_NICKNAME, name);
}

u8 *BreedFusion_BufferSummaryLine(struct BoxPokemon *boxMon, u8 *dest)
{
    u32 partner = BreedFusion_GetPartner(boxMon);

    if (partner == SPECIES_NONE)
        return dest;
    dest = StringCopy(dest, COMPOUND_STRING("Fused with "));
    dest = StringCopy(dest, GetSpeciesName(partner));
    dest = StringCopy(dest, COMPOUND_STRING("."));
    dest = StringCopy(dest, COMPOUND_STRING("\n"));
    return dest;
}

// ---------------------------------------------------------------------------
// Finding the partner from a personality value
//
// Sprite code often only has a species and personality value. The fusion
// is found by looking for the Pokémon with that personality in the
// parties, the PC and the Day Care (the personality is stored in the
// clear, so this is quick); recent answers are cached.

static bool32 CheckBoxMon(struct BoxPokemon *boxMon, u32 personality, u32 *partner)
{
    if (boxMon->personality != personality || !boxMon->hasSpecies)
        return FALSE;
    *partner = BreedFusion_GetPartner(boxMon);
    return TRUE;
}

static u32 SearchPartner(u32 personality)
{
    u32 i, j, partner = SPECIES_NONE;

    for (i = 0; i < MAX_BATTLE_TRAINERS; i++)
    {
        for (j = 0; j < PARTY_SIZE; j++)
        {
            if (CheckBoxMon(&gParties[i][j].box, personality, &partner))
                return partner;
        }
    }
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        if (CheckBoxMon(&gSaveBlock1Ptr->daycare.mons[i].mon, personality, &partner))
            return partner;
    }
    if (gPokemonStoragePtr != NULL)
    {
        for (i = 0; i < TOTAL_BOXES_COUNT; i++)
        {
            for (j = 0; j < IN_BOX_COUNT; j++)
            {
                if (CheckBoxMon(&gPokemonStoragePtr->boxes[i][j], personality, &partner))
                    return partner;
            }
        }
    }
    return SPECIES_NONE;
}

u32 BreedFusion_FindPartner(u32 personality)
{
    u32 i, partner;

    if (personality == 0)
        return SPECIES_NONE;
    for (i = 0; i < PARTNER_SLOTS; i++)
    {
        if (sPartners[i].partner != SPECIES_NONE && sPartners[i].personality == personality)
            return sPartners[i].partner;
    }
    partner = SearchPartner(personality);
    if (partner != SPECIES_NONE)
    {
        sPartners[sNextPartner].personality = personality;
        sPartners[sNextPartner].partner = partner;
        sNextPartner = (sNextPartner + 1) % PARTNER_SLOTS;
    }
    return partner;
}

u32 BreedFusion_GetIconPalIndex(u32 species, u32 personality, u32 palIndex)
{
    u32 partner = BreedFusion_FindPartner(personality);

    if (partner == SPECIES_NONE)
        return palIndex;
    return gSpeciesInfo[partner].iconPalIndex;
}

// ---------------------------------------------------------------------------
// Fusions built by the server

// The saved recipes: the first few in SaveBlock3, the rest in the extended
// save, which only an emulator with hardware v3 keeps.
static u32 RecipeCount(void)
{
    return SaveExt_IsAvailable() ? FUSION_RECIPE_TOTAL : FUSION_RECIPE_COUNT;
}

static struct FusionRecipe *Recipe(u32 i)
{
    if (i < FUSION_RECIPE_COUNT)
        return &gSaveBlock3Ptr->breedFusion.recipes[i];
    return &gSaveBlockExt.fusionMoreRecipes[i - FUSION_RECIPE_COUNT];
}

static struct FusionRecipe *FindRecipe(u32 species, u32 partner)
{
    u32 i, n = RecipeCount();

    for (i = 0; i < n; i++)
    {
        struct FusionRecipe *r = Recipe(i);

        if (r->count != 0 && r->species == species && r->partner == partner)
            return r;
    }
    return NULL;
}

static bool32 PartyNeedsRecipe(u32 species, u32 partner)
{
    u32 i;

    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct BoxPokemon *boxMon = &gParties[B_TRAINER_PLAYER][i].box;

        if (GetBoxMonData(boxMon, MON_DATA_SPECIES) == species && BreedFusion_GetPartner(boxMon) == partner)
            return TRUE;
    }
    return FALSE;
}

static void SaveRecipe(const u8 *data, u32 len)
{
    struct BreedFusionSaveData *save = &gSaveBlock3Ptr->breedFusion;
    struct FusionRecipe *recipe;
    u32 species, partner, count, i, n = RecipeCount(), slot = 0;

    if (len < 4 + 2 + FUSION_RECIPE_COLORS * 4)
        return;
    species = data[0] | (data[1] << 8);
    partner = data[2] | (data[3] << 8);
    count = data[4];
    if (count == 0 || count > FUSION_RECIPE_COLORS || species >= NUM_SPECIES || partner >= NUM_SPECIES)
        return;
    recipe = FindRecipe(species, partner);
    if (recipe == NULL)
    {
        // An empty slot, else the next one whose fusion isn't in the party.
        for (i = 0; i < n && recipe == NULL; i++)
        {
            if (Recipe(i)->count == 0)
            {
                recipe = Recipe(i);
                slot = i;
            }
        }
        for (i = 0; i < n && recipe == NULL; i++)
        {
            struct FusionRecipe *r = Recipe((save->nextSlot + i) % n);

            if (!PartyNeedsRecipe(r->species, r->partner))
            {
                recipe = r;
                slot = (save->nextSlot + i) % n;
            }
        }
        if (recipe == NULL)
        {
            slot = save->nextSlot % n;
            recipe = Recipe(slot);
        }
        save->nextSlot = (slot + 1) % n;
    }
    recipe->species = species;
    recipe->partner = partner;
    recipe->count = count;
    for (i = 0; i < FUSION_RECIPE_COLORS; i++)
    {
        recipe->colors[i] = data[6 + i * 2] | (data[7 + i * 2] << 8);
        recipe->shinyColors[i] = data[6 + FUSION_RECIPE_COLORS * 2 + i * 2] | (data[7 + FUSION_RECIPE_COLORS * 2 + i * 2] << 8);
    }
    memset(sPalettes, 0, sizeof(sPalettes)); // load the new colors next time
}

// A fusion in the party (or an egg about to hatch into one) that the
// server hasn't built yet.
static bool32 FindMissingRecipe(u32 *species, u32 *partner)
{
    u32 i;

    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct BoxPokemon *boxMon = &gParties[B_TRAINER_PLAYER][i].box;
        u32 p = BreedFusion_GetPartner(boxMon), s;

        if (p == SPECIES_NONE)
            continue;
        s = GetBoxMonData(boxMon, MON_DATA_SPECIES);
        if (FindRecipe(s, p) == NULL && sRefusedPair != (s | (p << 16)))
        {
            *species = s;
            *partner = p;
            return TRUE;
        }
    }
    return FALSE;
}

// Each overworld frame (src/community.c). Asks for one missing fusion at a
// time, only while the player walks freely and no other request is out.
void BreedFusion_OverworldUpdate(void)
{
    u32 species, partner;

    if (sRequestState == REQUEST_SENT)
    {
        if (Community_HasResponse(COMMUNITY_REQ_FUSION_BUILD))
        {
            sRequestState = REQUEST_IDLE;
            if (gCommunityMailbox.respStatus == COMMUNITY_OK)
            {
                SaveRecipe(gCommunityMailbox.respData, gCommunityMailbox.respLen);
                sRequestFailures = 0;
            }
            else if (gCommunityMailbox.respStatus == FUSION_CANT_FUSE)
            {
                sRefusedPair = gCommunityMailbox.respArgs[0] | (gCommunityMailbox.respArgs[1] << 16);
            }
            else
            {
                sRequestFailures++;
            }
        }
        else if (++sRequestTimer > REQUEST_TIMEOUT)
        {
            sRequestState = REQUEST_IDLE;
            sRequestFailures++;
        }
        return;
    }
    if ((gMain.vblankCounter1 & 63) != 0 || sRequestFailures >= MAX_FAILURES)
        return;
    if (!Community_IsOnline() || ArePlayerFieldControlsLocked())
        return;
    if (gCommunityMailbox.reqAck != gCommunityMailbox.reqSeq)
        return; // another request is still going out
    if (!FindMissingRecipe(&species, &partner))
        return;
    Community_Request(COMMUNITY_REQ_FUSION_BUILD, species, partner, 0, 0, NULL, 0);
    sRequestState = REQUEST_SENT;
    sRequestTimer = 0;
}

// ---------------------------------------------------------------------------
// Colors

static struct Hsl ToHsl(u16 color)
{
    s32 r = GET_R(color) * 8, g = GET_G(color) * 8, b = GET_B(color) * 8;
    s32 max = max(r, max(g, b)), min = min(r, min(g, b)), d = max - min;
    struct Hsl c;

    c.l = (max + min) / 2;
    if (d == 0)
    {
        c.h = 0;
        c.s = 0;
        return c;
    }
    c.s = (c.l < 128) ? d * 255 / (max + min) : d * 255 / (510 - max - min);
    if (max == r)
        c.h = ((g - b) * 256 / d + HUE_RANGE) % HUE_RANGE;
    else if (max == g)
        c.h = (b - r) * 256 / d + 512;
    else
        c.h = (r - g) * 256 / d + 1024;
    return c;
}

static u16 FromHsl(struct Hsl c)
{
    s32 chroma = (255 - abs(2 * c.l - 255)) * c.s / 255;
    s32 x = chroma * (256 - abs((c.h % 512) - 256)) / 256;
    s32 m = c.l - chroma / 2, r, g, b;

    switch (c.h / 256)
    {
    case 0:  r = chroma; g = x;      b = 0;      break;
    case 1:  r = x;      g = chroma; b = 0;      break;
    case 2:  r = 0;      g = chroma; b = x;      break;
    case 3:  r = 0;      g = x;      b = chroma; break;
    case 4:  r = x;      g = 0;      b = chroma; break;
    default: r = chroma; g = 0;      b = x;      break;
    }
    r = (r + m) / 8;
    g = (g + m) / 8;
    b = (b + m) / 8;
    return RGB(min(31, max(0, r)), min(31, max(0, g)), min(31, max(0, b)));
}

static s32 HueDistance(s32 a, s32 b)
{
    s32 d = abs(a - b);
    return min(d, HUE_RANGE - d);
}

// How many pixels of the first frame of the front sprite use each color.
static void CountPixels(u32 species, u32 personality, u16 *counts)
{
    u8 *pic = Alloc(MON_PIC_SIZE * MAX_MON_PIC_FRAMES);
    u32 i;

    if (pic == NULL)
    {
        for (i = 0; i < NUM_COLORS; i++)
            counts[i] = 1;
        return;
    }
    for (i = 0; i < NUM_COLORS; i++)
        counts[i] = 0;
    LoadSpecialPokePic(pic, species, personality, TRUE);
    CountPicPixels(pic, MON_PIC_SIZE, counts);
    Free(pic);
}

static u32 FindFamily(struct Hsl *hsl, struct Families *f, struct Hsl c, bool32 isGrey)
{
    u32 k, best = KEEP;
    s32 bestDist = HUE_RANGE;

    for (k = 0; k < f->count; k++)
    {
        struct Hsl rep = hsl[f->rep[k]];
        bool32 repGrey = rep.s < GREY_LIMIT;

        if (isGrey != repGrey)
            continue;
        if (isGrey)
            return k;
        if (HueDistance(rep.h, c.h) < bestDist)
        {
            bestDist = HueDistance(rep.h, c.h);
            best = k;
        }
    }
    if (best != KEEP && bestDist < HUE_NEAR)
        return best;
    return KEEP;
}

static void SplitFamilies(const u16 *palette, const u16 *counts, struct Hsl *hsl, struct Families *f)
{
    u32 order[NUM_COLORS], totals[MAX_FAMILIES] = {0}, i, j, n = 0;
    u8 rank[MAX_FAMILIES], remap[MAX_FAMILIES];
    u8 reps[MAX_FAMILIES];

    f->count = 0;
    for (i = 0; i < NUM_COLORS; i++)
    {
        hsl[i] = ToHsl(palette[i]);
        f->of[i] = KEEP;
    }
    // Colors by use, most used first (index 0 is transparent).
    for (i = 1; i < NUM_COLORS; i++)
    {
        if (hsl[i].l < DARK_LIMIT)
            continue;
        for (j = n; j > 0 && counts[order[j - 1]] < counts[i]; j--)
            order[j] = order[j - 1];
        order[j] = i;
        n++;
    }
    for (i = 0; i < n; i++)
    {
        u32 c = order[i], k;
        bool32 isGrey = hsl[c].s < GREY_LIMIT;

        k = FindFamily(hsl, f, hsl[c], isGrey);
        if (k == KEEP && counts[c] != 0 && f->count < MAX_FAMILIES)
        {
            k = f->count++;
            f->rep[k] = c;
        }
        if (k == KEEP && f->count != 0)
        {
            // Unused or leftover color: nearest family of any kind.
            s32 bestDist = 0x7FFF;
            for (j = 0; j < f->count; j++)
            {
                s32 d = HueDistance(hsl[f->rep[j]].h, hsl[c].h) + abs(hsl[f->rep[j]].s - hsl[c].s) * 4;
                if (d < bestDist)
                {
                    bestDist = d;
                    k = j;
                }
            }
        }
        if (k != KEEP)
        {
            f->of[c] = k;
            totals[k] += counts[c];
        }
    }
    // Biggest family first.
    for (i = 0; i < f->count; i++)
        rank[i] = i;
    for (i = 1; i < f->count; i++)
    {
        for (j = i; j > 0 && totals[rank[j - 1]] < totals[rank[j]]; j--)
        {
            u8 t = rank[j];
            rank[j] = rank[j - 1];
            rank[j - 1] = t;
        }
    }
    for (i = 0; i < f->count; i++)
    {
        remap[rank[i]] = i;
        reps[i] = f->rep[rank[i]];
    }
    for (i = 0; i < f->count; i++)
        f->rep[i] = reps[i];
    for (i = 0; i < NUM_COLORS; i++)
    {
        if (f->of[i] != KEEP)
            f->of[i] = remap[f->of[i]];
    }
}

static void CountPicPixels(const u8 *pic, u32 size, u16 *counts)
{
    u32 i;

    for (i = 0; i < size; i++)
    {
        counts[pic[i] & 0xF]++;
        counts[pic[i] >> 4]++;
    }
}

// `counts`: how much of the baby's sprite uses each of `palette`'s colors.
static void MakeFusionPalette(u16 *dest, const u16 *palette, const u16 *counts, u32 species, u32 partner, bool32 isShiny)
{
    const u16 *partnerPal = GetMonSpritePalFromSpeciesIsEgg(partner, isShiny, FALSE, FALSE);
    const struct FusionRecipe *recipe = FindRecipe(species, partner);
    u16 partnerCounts[NUM_COLORS];
    struct Hsl hslA[NUM_COLORS], hslB[NUM_COLORS];
    struct Families famA, famB;
    u32 i;

    CpuCopy16(palette, dest, NUM_COLORS * 2);
    if (partnerPal == NULL && recipe == NULL)
        return;
    SplitFamilies(palette, counts, hslA, &famA);
    if (recipe != NULL)
    {
        // The server's colors stand for the father's families, biggest first.
        const u16 *colors = isShiny ? recipe->shinyColors : recipe->colors;

        famB.count = recipe->count;
        for (i = 0; i < famB.count; i++)
        {
            famB.rep[i] = i;
            hslB[i] = ToHsl(colors[i]);
        }
    }
    else
    {
        CountPixels(partner, 0, partnerCounts);
        SplitFamilies(partnerPal, partnerCounts, hslB, &famB);
    }
    if (famA.count == 0 || famB.count == 0)
        return;

    for (i = 1; i < NUM_COLORS; i++)
    {
        u32 fa = famA.of[i], fb;
        struct Hsl c = hslA[i], a, b;

        if (fa == KEEP)
            continue;
        fb = fa % famB.count;
        a = hslA[famA.rep[fa]];
        b = hslB[famB.rep[fb]];
        c.h = (b.h + (c.h - a.h) + HUE_RANGE * 2) % HUE_RANGE;
        c.s = min(255, max(0, b.s + (c.s - a.s)));
        c.l = min(244, max(DARK_LIMIT, c.l + (b.l - a.l) * 3 / 4));
        dest[i] = FromHsl(c);
    }
}

static struct FusionPalette *FindCachedPalette(const u16 *palette, u32 species, u32 partner)
{
    u32 i;

    for (i = 0; i < PALETTE_SLOTS; i++)
    {
        if (sPalettes[i].source == palette && sPalettes[i].species == species && sPalettes[i].partner == partner)
            return &sPalettes[i];
    }
    return NULL;
}

static struct FusionPalette *NewCachedPalette(const u16 *palette, u32 species, u32 partner)
{
    struct FusionPalette *slot = &sPalettes[sNextPalette];

    sNextPalette = (sNextPalette + 1) % PALETTE_SLOTS;
    slot->source = palette;
    slot->species = species;
    slot->partner = partner;
    return slot;
}

// The follower (src/event_object_movement.c): the overworld sprite has its
// own palette, so its colors are made from it and the overworld pic. Sets
// *tag to a palette tag for this fusion.
const u16 *BreedFusion_GetFollowerPalette(const u16 *palette, u32 species, bool32 isShiny, const struct ObjectEventGraphicsInfo *info, u16 *tag)
{
    struct Pokemon *lead = GetFirstLiveMon();
    struct FusionPalette *slot;
    u32 partner;
    u16 counts[NUM_COLORS] = {0};

    if (palette == NULL || lead == NULL || GetMonData(lead, MON_DATA_SPECIES) != species
     || GetMonData(lead, MON_DATA_IS_SHINY) != isShiny)
        return palette;
    partner = BreedFusion_GetPartner(&lead->box);
    if (partner == SPECIES_NONE)
        return palette;
    *tag = FUSION_FOLLOWER_PAL_TAG + ((species * 7 + partner * 3 + isShiny) % FUSION_FOLLOWER_PAL_TAGS);
    slot = FindCachedPalette(palette, species, partner);
    if (slot != NULL)
        return slot->colors;
    if (info->compressed)
    {
        u8 *pic = Alloc(GetDecompressedDataSize((const u32 *)info->images[0].data));

        if (pic != NULL)
        {
            DecompressDataWithHeaderWram((const u32 *)info->images[0].data, pic);
            CountPicPixels(pic, info->size, counts);
            Free(pic);
        }
    }
    else
    {
        CountPicPixels(info->images[0].data, info->size, counts);
    }
    slot = NewCachedPalette(palette, species, partner);
    MakeFusionPalette(slot->colors, palette, counts, species, partner, isShiny);
    return slot->colors;
}

// Called where every Pokémon sprite palette is picked: returns the fusion
// colors if the Pokémon with this personality value is a fusion.
const u16 *BreedFusion_ApplyPalette(const u16 *palette, u32 species, bool32 isShiny, u32 personality)
{
    struct FusionPalette *slot;
    u32 partner;
    u16 counts[NUM_COLORS];

    if (palette == NULL)
        return palette;
    partner = BreedFusion_FindPartner(personality);
    if (partner == SPECIES_NONE)
        return palette;
    slot = FindCachedPalette(palette, species, partner);
    if (slot != NULL)
        return slot->colors;
    CountPixels(species, personality, counts);
    slot = NewCachedPalette(palette, species, partner);
    MakeFusionPalette(slot->colors, palette, counts, species, partner, isShiny);
    return slot->colors;
}
