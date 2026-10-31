// Pokémon injuries.
//
// A body part (eye, limb, tail, wing, horn) can take damage in battle. The
// damage is a percentage that keeps rising while the trainer walks around
// without treating it. At 100% the part is lost for good: the stat penalty
// becomes permanent and the sprite shows a bandaged stump (or an eye patch).
// The Pokémon Center clinic treats the damage for money. Pokémon can also get
// a cosmetic scar.
//
// All state lives in spare bits of the Pokémon's own box data (see
// include/pokemon.h), so it survives the PC, trades and online battles, and it
// needs no EWRAM.
#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "injury.h"
#include "link.h"
#include "money.h"
#include "party_menu.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/battle.h"
#include "constants/pokemon.h"

#include "data/injury_shapes.h"

// Odds (percent).
#define FAINT_INJURY_CHANCE   30  // When one of your Pokémon faints.
#define FAINT_DAMAGE_MIN      20
#define FAINT_DAMAGE_MAX      40
#define CRIT_INJURY_CHANCE    20  // When one of your Pokémon takes a critical hit.
#define CRIT_DAMAGE_MIN       10
#define CRIT_DAMAGE_MAX       20
#define CRIT_SCAR_CHANCE      10
#define WORSEN_PER_128_STEPS  2   // In 2% units: +4% every 128 steps.
#define TREAT_SCAR_CHANCE     30
#define TREAT_LOST_SCAR_CHANCE 60
#define OPEN_WOUND_PENALTY    10  // Extra % off every stat until a lost part's wound is treated.
#define MAX_PENALTY           60

struct InjuryPartInfo
{
    u8 stat;
    u8 penalty; // % off the stat once the part is lost
};

static const struct InjuryPartInfo sPartInfo[INJURY_PART_COUNT] =
{
    [INJURY_PART_EYE]  = { STAT_SPATK, 20 },
    [INJURY_PART_LIMB] = { STAT_SPEED, 20 },
    [INJURY_PART_TAIL] = { STAT_DEF,   15 },
    [INJURY_PART_WING] = { STAT_SPEED, 25 },
    [INJURY_PART_HORN] = { STAT_ATK,   15 },
};

static const u8 *const sStatNames[NUM_STATS] =
{
    [STAT_HP]    = COMPOUND_STRING("HP"),
    [STAT_ATK]   = COMPOUND_STRING("ATTACK"),
    [STAT_DEF]   = COMPOUND_STRING("DEFENSE"),
    [STAT_SPEED] = COMPOUND_STRING("SPEED"),
    [STAT_SPATK] = COMPOUND_STRING("SP. ATK"),
    [STAT_SPDEF] = COMPOUND_STRING("SP. DEF"),
};

static const u8 *const sStatShortNames[NUM_STATS] =
{
    [STAT_HP]    = COMPOUND_STRING("HP"),
    [STAT_ATK]   = COMPOUND_STRING("ATK"),
    [STAT_DEF]   = COMPOUND_STRING("DEF"),
    [STAT_SPEED] = COMPOUND_STRING("SPE"),
    [STAT_SPATK] = COMPOUND_STRING("SPA"),
    [STAT_SPDEF] = COMPOUND_STRING("SPD"),
};

// ---------------------------------------------------------------------------
// Species body parts

static u32 GetBodyShape(u32 species)
{
    u32 dex = SpeciesToNationalPokedexNum(species);
    if (dex < ARRAY_COUNT(sInjuryBodyShapes))
        return sInjuryBodyShapes[dex];
    return BODY_SHAPE_NONE;
}

#define BIT(part) INJURY_PART_BIT(INJURY_PART_##part)

// Which parts a species has, as a mask of INJURY_PART_BIT().
static u32 GetSpeciesParts(u32 species)
{
    u32 mask;
    u32 shape = GetBodyShape(species);

    switch (shape)
    {
    case BODY_SHAPE_BALL:
    case BODY_SHAPE_BLOB:
    case BODY_SHAPE_HEADS:     mask = BIT(EYE); break;
    case BODY_SHAPE_SQUIGGLE:  mask = BIT(EYE) | BIT(TAIL); break;
    case BODY_SHAPE_FISH:      mask = BIT(EYE) | BIT(TAIL) | BIT(LIMB); break;
    case BODY_SHAPE_ARMS:
    case BODY_SHAPE_HUMANOID:
    case BODY_SHAPE_ARMOR:     mask = BIT(EYE) | BIT(LIMB) | BIT(HORN); break;
    case BODY_SHAPE_UPRIGHT:
    case BODY_SHAPE_QUADRUPED: mask = BIT(EYE) | BIT(LIMB) | BIT(TAIL) | BIT(HORN); break;
    case BODY_SHAPE_LEGS:
    case BODY_SHAPE_TENTACLES: mask = BIT(EYE) | BIT(LIMB); break;
    case BODY_SHAPE_WINGS:     mask = BIT(EYE) | BIT(LIMB) | BIT(TAIL) | BIT(WING); break;
    case BODY_SHAPE_BUG_WINGS: mask = BIT(EYE) | BIT(LIMB) | BIT(WING) | BIT(HORN); break;
    default:                   mask = BIT(EYE) | BIT(LIMB); break;
    }
    if (shape != BODY_SHAPE_SQUIGGLE && shape != BODY_SHAPE_FISH
     && (gSpeciesInfo[species].types[0] == TYPE_FLYING || gSpeciesInfo[species].types[1] == TYPE_FLYING))
        mask |= BIT(WING);
    return mask;
}

static const u8 *GetPartName(u32 species, u32 part)
{
    u32 shape = GetBodyShape(species);
    u32 type = gSpeciesInfo[species].types[0];

    switch (part)
    {
    case INJURY_PART_EYE:
        return COMPOUND_STRING("EYE");
    case INJURY_PART_LIMB:
        if (shape == BODY_SHAPE_FISH)
            return COMPOUND_STRING("FIN");
        if (shape == BODY_SHAPE_TENTACLES)
            return COMPOUND_STRING("TENTACLE");
        if (shape == BODY_SHAPE_ARMS)
            return COMPOUND_STRING("ARM");
        return COMPOUND_STRING("LEG");
    case INJURY_PART_TAIL:
        return COMPOUND_STRING("TAIL");
    case INJURY_PART_WING:
        return COMPOUND_STRING("WING");
    case INJURY_PART_HORN:
        if (shape == BODY_SHAPE_BUG_WINGS || shape == BODY_SHAPE_ARMOR || type == TYPE_BUG)
            return COMPOUND_STRING("ANTENNA");
        if (type == TYPE_NORMAL || type == TYPE_ELECTRIC || type == TYPE_FAIRY || type == TYPE_GRASS
         || type == TYPE_FIRE || type == TYPE_DARK || type == TYPE_PSYCHIC || type == TYPE_FIGHTING)
            return COMPOUND_STRING("EAR");
        return COMPOUND_STRING("HORN");
    }
    return COMPOUND_STRING("BODY");
}

// ---------------------------------------------------------------------------
// State helpers

struct InjuryState
{
    u8 part;
    u8 damage; // percent
    u8 lost;
    u8 flags;
};

static void GetState(struct BoxPokemon *boxMon, struct InjuryState *st)
{
    st->part = GetBoxMonData(boxMon, MON_DATA_INJURY_PART);
    st->damage = GetBoxMonData(boxMon, MON_DATA_INJURY_DAMAGE) * INJURY_DAMAGE_UNIT;
    st->lost = GetBoxMonData(boxMon, MON_DATA_LOST_PARTS);
    st->flags = GetBoxMonData(boxMon, MON_DATA_INJURY_FLAGS);
    if (st->part >= INJURY_PART_COUNT)
        st->part = INJURY_PART_NONE;
    if (st->damage > 100)
        st->damage = 100;
}

static void SetState(struct BoxPokemon *boxMon, const struct InjuryState *st)
{
    u32 part = st->part, damage = st->damage / INJURY_DAMAGE_UNIT, lost = st->lost, flags = st->flags;
    SetBoxMonData(boxMon, MON_DATA_INJURY_PART, &part);
    SetBoxMonData(boxMon, MON_DATA_INJURY_DAMAGE, &damage);
    SetBoxMonData(boxMon, MON_DATA_LOST_PARTS, &lost);
    SetBoxMonData(boxMon, MON_DATA_INJURY_FLAGS, &flags);
}

static bool32 IsOpenWound(const struct InjuryState *st)
{
    return st->part != INJURY_PART_NONE && (st->lost & INJURY_PART_BIT(st->part));
}

// Current % off the injured part's stat while it is damaged but not lost yet.
static u32 GetDamagePenalty(const struct InjuryState *st)
{
    if (st->part == INJURY_PART_NONE || IsOpenWound(st))
        return 0;
    return sPartInfo[st->part].penalty * st->damage / 100;
}

s32 Injury_ModifyStat(struct BoxPokemon *boxMon, u32 stat, s32 value)
{
    struct InjuryState st;
    u32 penalty = 0, part;

    if (stat == STAT_HP)
        return value;
    GetState(boxMon, &st);
    if (st.part == INJURY_PART_NONE && st.lost == 0)
        return value;

    for (part = INJURY_PART_EYE; part < INJURY_PART_COUNT; part++)
    {
        if ((st.lost & INJURY_PART_BIT(part)) && sPartInfo[part].stat == stat)
            penalty += sPartInfo[part].penalty;
    }
    if (st.part != INJURY_PART_NONE && sPartInfo[st.part].stat == stat)
        penalty += GetDamagePenalty(&st);
    if (IsOpenWound(&st))
        penalty += OPEN_WOUND_PENALTY;
    if (penalty > MAX_PENALTY)
        penalty = MAX_PENALTY;

    value = value * (100 - penalty) / 100;
    return value > 0 ? value : 1;
}

// Adds damage to a part. Returns TRUE if anything changed.
static bool32 AddDamage(struct Pokemon *mon, u32 minPct, u32 maxPct)
{
    struct InjuryState st;
    u32 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 part, avail, count, pick;

    GetState(&mon->box, &st);
    if (IsOpenWound(&st))
        return FALSE; // Already as bad as it gets until treated.

    if (st.part == INJURY_PART_NONE)
    {
        avail = GetSpeciesParts(species) & ~st.lost;
        for (count = 0, part = INJURY_PART_EYE; part < INJURY_PART_COUNT; part++)
            if (avail & INJURY_PART_BIT(part))
                count++;
        if (count == 0)
            return FALSE;
        pick = Random2() % count;
        for (part = INJURY_PART_EYE; part < INJURY_PART_COUNT; part++)
        {
            if ((avail & INJURY_PART_BIT(part)) && pick-- == 0)
                break;
        }
        st.part = part;
        st.damage = 0;
    }

    st.damage += minPct + Random2() % (maxPct - minPct + 1);
    st.damage -= st.damage % INJURY_DAMAGE_UNIT;
    if (st.damage >= 100)
    {
        st.damage = 100;
        st.lost |= INJURY_PART_BIT(st.part);
    }
    st.flags |= INJURY_FLAG_NOTIFY;
    SetState(&mon->box, &st);
    return TRUE;
}

static void AddScar(struct Pokemon *mon)
{
    u32 flags = GetMonData(mon, MON_DATA_INJURY_FLAGS);
    if (!(flags & INJURY_FLAG_SCAR))
    {
        flags |= INJURY_FLAG_SCAR | INJURY_FLAG_NEW_SCAR;
        SetMonData(mon, MON_DATA_INJURY_FLAGS, &flags);
    }
}

void Injury_Set(struct Pokemon *mon, u32 part, u32 percent, u32 lostMask, u32 flags)
{
    struct InjuryState st = { .part = part, .damage = percent, .lost = lostMask, .flags = flags };
    if (part != INJURY_PART_NONE && percent >= 100)
        st.lost |= INJURY_PART_BIT(part);
    SetState(&mon->box, &st);
    CalculateMonStats(mon);
}

// ---------------------------------------------------------------------------
// Battle

static bool32 CanInjureInBattle(u32 battler)
{
#if TESTING
    return FALSE;
#else
    if (!IsOnPlayerSide(battler))
        return FALSE;
    // Temporary or borrowed parties, replays and tutorials.
    if (gBattleTypeFlags & (BATTLE_TYPE_SAFARI | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_RECORDED | BATTLE_TYPE_RECORDED_LINK
                          | BATTLE_TYPE_POKEDUDE | BATTLE_TYPE_FIRST_BATTLE | BATTLE_TYPE_CATCH_TUTORIAL
                          | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_TRAINER_HILL))
        return FALSE;
    if (GetBattlerParty(battler) != gParties[B_TRAINER_PLAYER])
        return FALSE;
    // In a link multi battle, only touch our own Pokémon.
    if ((gBattleTypeFlags & BATTLE_TYPE_LINK) && (gBattleTypeFlags & BATTLE_TYPE_MULTI)
     && GetBattlerMultiplayerId(battler) != GetMultiplayerId())
        return FALSE;
    return TRUE;
#endif
}

// Random2() is used on purpose: it is not part of the battle RNG, so rolling
// for our own Pokémon in a link battle can't desync the two games.
void Injury_OnCriticalHit(u32 battler)
{
    struct Pokemon *mon;

    if (!CanInjureInBattle(battler))
        return;
    mon = GetBattlerMon(battler);
    if (Random2() % 100 < CRIT_INJURY_CHANCE)
        AddDamage(mon, CRIT_DAMAGE_MIN, CRIT_DAMAGE_MAX);
    if (Random2() % 100 < CRIT_SCAR_CHANCE)
        AddScar(mon);
}

void Injury_OnBattlerFainted(u32 battler)
{
    if (!CanInjureInBattle(battler))
        return;
    if (Random2() % 100 < FAINT_INJURY_CHANCE)
        AddDamage(GetBattlerMon(battler), FAINT_DAMAGE_MIN, FAINT_DAMAGE_MAX);
}

// ---------------------------------------------------------------------------
// Field

void Injury_OnWalk128Steps(void)
{
    u32 i;
    struct InjuryState st;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 before;

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        GetState(&mon->box, &st);
        if (st.part == INJURY_PART_NONE || IsOpenWound(&st))
            continue;

        before = st.damage;
        st.damage += WORSEN_PER_128_STEPS * INJURY_DAMAGE_UNIT;
        if (st.damage >= 100)
        {
            st.damage = 100;
            st.lost |= INJURY_PART_BIT(st.part);
            st.flags |= INJURY_FLAG_NOTIFY;
        }
        else if ((before < 50 && st.damage >= 50) || (before < 80 && st.damage >= 80))
        {
            st.flags |= INJURY_FLAG_NOTIFY;
        }
        SetState(&mon->box, &st);
        CalculateMonStats(mon);
    }
}

static u8 *AppendNum(u8 *dest, u32 num)
{
    return ConvertIntToDecimalStringN(dest, num, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Writes the after-battle (or worsening) message for one Pokémon to gStringVar4.
static void BufferInjuryMessage(struct Pokemon *mon, const struct InjuryState *st)
{
    u32 species = GetMonData(mon, MON_DATA_SPECIES);
    const u8 *partName = GetPartName(species, st->part);
    const u8 *statName = sStatNames[sPartInfo[st->part].stat];
    u8 *s = gStringVar4;

    GetMonNickname(mon, gStringVar1);
    s = StringCopy(s, gStringVar1);
    s = StringCopy(s, COMPOUND_STRING("'s "));
    s = StringCopy(s, partName);
    if (IsOpenWound(st))
    {
        s = StringCopy(s, COMPOUND_STRING(" was lost!\nIt will never grow back.\p"));
        s = StringCopy(s, statName);
        s = StringCopy(s, COMPOUND_STRING(" is down "));
        s = AppendNum(s, sPartInfo[st->part].penalty);
        s = StringCopy(s, COMPOUND_STRING("% for good.\nThe open wound weakens it even more.\p"));
        s = StringCopy(s, COMPOUND_STRING("Rush it to a POKéMON CENTER\nto have the wound treated!"));
    }
    else
    {
        s = StringCopy(s, COMPOUND_STRING(" is "));
        s = AppendNum(s, st->damage);
        s = StringCopy(s, COMPOUND_STRING("% damaged!\n"));
        s = StringCopy(s, statName);
        s = StringCopy(s, COMPOUND_STRING(" is down "));
        s = AppendNum(s, GetDamagePenalty(st));
        s = StringCopy(s, COMPOUND_STRING("%.\pHeal it at a POKéMON CENTER soon,\nor the "));
        s = StringCopy(s, partName);
        s = StringCopy(s, COMPOUND_STRING(" will be lost!"));
    }
}

bool32 Injury_TryStartNotify(void)
{
    u32 i;
    struct InjuryState st;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        GetState(&mon->box, &st);
        if (st.flags & INJURY_FLAG_NOTIFY)
        {
            st.flags &= ~INJURY_FLAG_NOTIFY;
            SetState(&mon->box, &st);
            CalculateMonStats(mon);
            if (st.part != INJURY_PART_NONE)
            {
                BufferInjuryMessage(mon, &st);
                return TRUE;
            }
        }
        if (st.flags & INJURY_FLAG_NEW_SCAR)
        {
            st.flags &= ~INJURY_FLAG_NEW_SCAR;
            SetState(&mon->box, &st);
            GetMonNickname(mon, gStringVar1);
            StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("That hard battle left {STR_VAR_1}\nwith a scar. It will never fade."));
            return TRUE;
        }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// Pokémon Center clinic

static u32 GetTreatmentCost(struct Pokemon *mon, const struct InjuryState *st)
{
    u32 cost = st->damage * (GetMonData(mon, MON_DATA_LEVEL) + 10);
    if (IsOpenWound(st))
        cost = cost * 3 / 2;
    return cost;
}

bool32 Injury_PartyHasInjury(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_INJURY_PART) != INJURY_PART_NONE)
            return TRUE;
    }
    return FALSE;
}

void Injury_ScriptPartyHasInjury(void)
{
    gSpecialVar_Result = Injury_PartyHasInjury();
}

// VAR_0x8005: party slot to start from. Sets VAR_RESULT and gStringVar4.
void Injury_ClinicFindNext(void)
{
    u32 i;
    struct InjuryState st;

    gSpecialVar_Result = FALSE;
    for (i = gSpecialVar_0x8005; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        const u8 *partName;

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        GetState(&mon->box, &st);
        if (st.part == INJURY_PART_NONE)
            continue;

        partName = GetPartName(GetMonData(mon, MON_DATA_SPECIES), st.part);
        GetMonNickname(mon, gStringVar1);
        StringCopy(gStringVar2, partName);
        ConvertIntToDecimalStringN(gStringVar3, GetTreatmentCost(mon, &st), STR_CONV_MODE_LEFT_ALIGN, 6);
        if (IsOpenWound(&st))
        {
            StringExpandPlaceholders(gStringVar4, COMPOUND_STRING(
                "Oh no! {STR_VAR_1} lost its {STR_VAR_2},\nand the wound is still open!\p"
                "Our clinic can close the wound\nfor ¥{STR_VAR_3}. Shall we treat it?"));
        }
        else
        {
            u8 *s = StringCopy(gStringVar4, gStringVar1);
            s = StringCopy(s, COMPOUND_STRING("'s "));
            s = StringCopy(s, partName);
            s = StringCopy(s, COMPOUND_STRING(" is "));
            s = AppendNum(s, st.damage);
            StringExpandPlaceholders(s, COMPOUND_STRING(
                "% damaged.\nIt needs treatment right away!\p"
                "Our clinic can treat it\nfor ¥{STR_VAR_3}. Shall we treat it?"));
        }
        gSpecialVar_0x8005 = i;
        gSpecialVar_Result = TRUE;
        return;
    }
}

// VAR_0x8005: party slot. Sets VAR_RESULT (TRUE if treated) and gStringVar4.
void Injury_ClinicTreat(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005];
    struct InjuryState st;
    u32 cost;
    bool32 wasLost, scar;
    u8 *s;

    gSpecialVar_Result = FALSE;
    GetState(&mon->box, &st);
    if (st.part == INJURY_PART_NONE)
        return;

    cost = GetTreatmentCost(mon, &st);
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, cost))
    {
        StringCopy(gStringVar4, COMPOUND_STRING("I'm sorry, you don't have enough\nmoney for the treatment…"));
        return;
    }
    RemoveMoney(&gSaveBlock1Ptr->money, cost);

    wasLost = IsOpenWound(&st);
    GetMonNickname(mon, gStringVar1);
    StringCopy(gStringVar2, GetPartName(GetMonData(mon, MON_DATA_SPECIES), st.part));
    scar = (Random2() % 100) < (wasLost ? TREAT_LOST_SCAR_CHANCE : TREAT_SCAR_CHANCE);
    st.part = INJURY_PART_NONE;
    st.damage = 0;
    st.flags &= ~INJURY_FLAG_NOTIFY;
    if (scar)
        st.flags |= INJURY_FLAG_SCAR;
    SetState(&mon->box, &st);
    CalculateMonStats(mon);

    s = StringExpandPlaceholders(gStringVar4, wasLost
        ? COMPOUND_STRING("The wound has closed. {STR_VAR_1}'s\n{STR_VAR_2} is gone, but it is out of danger.")
        : COMPOUND_STRING("{STR_VAR_1}'s {STR_VAR_2} was treated.\nIt will heal up completely!"));
    if (scar)
        StringExpandPlaceholders(s, COMPOUND_STRING("\pThe injury left a scar."));
    gSpecialVar_Result = TRUE;
}

// ---------------------------------------------------------------------------
// Summary screen

bool32 Injury_BufferSummaryMemo(struct BoxPokemon *boxMon, u8 *dest)
{
    struct InjuryState st;
    u32 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);
    u32 part, n = 0;
    u8 *s = dest;

    GetState(boxMon, &st);
    if (st.part == INJURY_PART_NONE && st.lost == 0 && !(st.flags & INJURY_FLAG_SCAR))
        return FALSE;

    if (st.part != INJURY_PART_NONE)
    {
        s = StringCopy(s, GetPartName(species, st.part));
        if (IsOpenWound(&st))
        {
            s = StringCopy(s, COMPOUND_STRING(" wound open!"));
        }
        else
        {
            s = StringCopy(s, COMPOUND_STRING(" "));
            s = AppendNum(s, st.damage);
            s = StringCopy(s, COMPOUND_STRING("% hurt: "));
            s = StringCopy(s, sStatShortNames[sPartInfo[st.part].stat]);
            s = StringCopy(s, COMPOUND_STRING(" -"));
            s = AppendNum(s, GetDamagePenalty(&st));
            s = StringCopy(s, COMPOUND_STRING("%"));
        }
        s = StringCopy(s, COMPOUND_STRING("\n"));
    }
    if (st.lost)
    {
        s = StringCopy(s, COMPOUND_STRING("Lost: "));
        for (part = INJURY_PART_EYE; part < INJURY_PART_COUNT; part++)
        {
            if (!(st.lost & INJURY_PART_BIT(part)))
                continue;
            if (n++)
                s = StringCopy(s, COMPOUND_STRING(", "));
            s = StringCopy(s, GetPartName(species, part));
        }
        s = StringCopy(s, COMPOUND_STRING("\n"));
    }
    if (st.flags & INJURY_FLAG_SCAR)
        s = StringCopy(s, COMPOUND_STRING("Has a battle scar."));
    else if (s > dest && s[-1] == CHAR_NEWLINE)
        s[-1] = EOS;
    return TRUE;
}

// ---------------------------------------------------------------------------
// Sprites
//
// Works on a decompressed 4bpp 64x64 pic. Parts are found from the sprite's
// silhouette: the tip of the silhouette in the part's direction is cut off and
// the stump is wrapped in a bandage. A lost eye gets an eye patch on the head.
// Colors come from the Pokémon's own palette (its darkest, lightest and most
// neutral colors), so it needs no extra palette slots.

#define PIC_W 64

static inline u8 *PixelPtr(u8 *frame, s32 x, s32 y)
{
    return &frame[((y >> 3) * 8 + (x >> 3)) * 32 + (y & 7) * 4 + ((x & 7) >> 1)];
}

static u32 GetPx(u8 *frame, s32 x, s32 y)
{
    if (x < 0 || y < 0 || x >= PIC_W || y >= PIC_W)
        return 0;
    return (x & 1) ? (*PixelPtr(frame, x, y) >> 4) : (*PixelPtr(frame, x, y) & 0xF);
}

static void SetPx(u8 *frame, s32 x, s32 y, u32 c)
{
    u8 *p;
    if (x < 0 || y < 0 || x >= PIC_W || y >= PIC_W)
        return;
    p = PixelPtr(frame, x, y);
    if (x & 1)
        *p = (*p & 0x0F) | (c << 4);
    else
        *p = (*p & 0xF0) | c;
}

struct InjuryColors
{
    u8 dark, light, mid;
};

static void PickColors(const u16 *pal, struct InjuryColors *c)
{
    u32 i, bestDark = 0xFFFF, bestLight = 0, bestMid = 0xFFFF;

    c->dark = c->light = c->mid = 1;
    for (i = 1; i < 16; i++)
    {
        u32 r = pal[i] & 31, g = (pal[i] >> 5) & 31, b = (pal[i] >> 10) & 31;
        u32 sum = r + g + b;
        u32 hi = max(r, max(g, b)), lo = min(r, min(g, b));
        u32 v = sum / 3;
        u32 mid = (hi - lo) * 2 + (v > 22 ? v - 22 : 22 - v);
        if (sum < bestDark)  { bestDark = sum;  c->dark = i; }
        if (sum > bestLight) { bestLight = sum; c->light = i; }
        if (mid < bestMid)   { bestMid = mid;   c->mid = i; }
    }
}

struct Silhouette
{
    s16 x0, y0, x1, y1;
    s16 cx, cy;
    bool8 empty;
};

static void MeasureSilhouette(u8 *frame, struct Silhouette *sil)
{
    s32 x, y, n = 0, sx = 0, sy = 0;

    sil->x0 = sil->y0 = PIC_W;
    sil->x1 = sil->y1 = -1;
    for (y = 0; y < PIC_W; y++)
    {
        for (x = 0; x < PIC_W; x++)
        {
            if (!GetPx(frame, x, y))
                continue;
            n++; sx += x; sy += y;
            if (x < sil->x0) sil->x0 = x;
            if (x > sil->x1) sil->x1 = x;
            if (y < sil->y0) sil->y0 = y;
            if (y > sil->y1) sil->y1 = y;
        }
    }
    sil->empty = (n == 0);
    if (n)
    {
        sil->cx = sx / n;
        sil->cy = sy / n;
    }
}

// Direction of each part on a front sprite (Pokémon face left), scaled to 64.
static const s8 sPartDirs[INJURY_PART_COUNT][2] =
{
    [INJURY_PART_TAIL] = {  63,  13 },
    [INJURY_PART_WING] = {  29, -57 },
    [INJURY_PART_HORN] = { -13, -63 },
    [INJURY_PART_LIMB] = { -13,  63 },
};

#define STUMP_RADIUS 12

static bool32 InStump(s32 x, s32 y, s32 dx, s32 dy, s32 P, s32 ex, s32 ey)
{
    s32 p = x * dx + y * dy;
    s32 ddx = x - ex, ddy = y - ey;
    return p > P - 9 * 64 && p <= P - 2 * 64 && ddx * ddx + ddy * ddy <= STUMP_RADIUS * STUMP_RADIUS;
}

static void DrawLostPart(u8 *frame, u32 part, bool32 isFront, const struct InjuryColors *c)
{
    s32 dx = sPartDirs[part][0], dy = sPartDirs[part][1];
    s32 x, y, P = -0x7FFFFFFF, ex = 0, ey = 0;

    if (!isFront)
        dx = -dx;
    for (y = 0; y < PIC_W; y++)
    {
        for (x = 0; x < PIC_W; x++)
        {
            if (GetPx(frame, x, y) && x * dx + y * dy > P)
            {
                P = x * dx + y * dy;
                ex = x; ey = y;
            }
        }
    }
    if (P == -0x7FFFFFFF)
        return;

    // Cut off the tip and wrap the stump.
    for (y = max(0, ey - STUMP_RADIUS); y <= min(PIC_W - 1, ey + STUMP_RADIUS); y++)
    {
        for (x = max(0, ex - STUMP_RADIUS); x <= min(PIC_W - 1, ex + STUMP_RADIUS); x++)
        {
            s32 ddx = x - ex, ddy = y - ey;
            if (!GetPx(frame, x, y) || ddx * ddx + ddy * ddy > STUMP_RADIUS * STUMP_RADIUS)
                continue;
            if (x * dx + y * dy > P - 2 * 64)
                SetPx(frame, x, y, 0);
            else if (InStump(x, y, dx, dy, P, ex, ey))
                SetPx(frame, x, y, ((x - y + 99) % 3 == 0) ? c->mid : c->light);
        }
    }
    // Outline the bandage.
    for (y = max(0, ey - STUMP_RADIUS); y <= min(PIC_W - 1, ey + STUMP_RADIUS); y++)
    {
        for (x = max(0, ex - STUMP_RADIUS); x <= min(PIC_W - 1, ex + STUMP_RADIUS); x++)
        {
            if (!GetPx(frame, x, y) || !InStump(x, y, dx, dy, P, ex, ey))
                continue;
            if (!GetPx(frame, x + 1, y) || !InStump(x + 1, y, dx, dy, P, ex, ey)
             || !GetPx(frame, x - 1, y) || !InStump(x - 1, y, dx, dy, P, ex, ey)
             || !GetPx(frame, x, y + 1) || !InStump(x, y + 1, dx, dy, P, ex, ey)
             || !GetPx(frame, x, y - 1) || !InStump(x, y - 1, dx, dy, P, ex, ey))
                SetPx(frame, x, y, c->dark);
        }
    }
}

static void DrawEyePatch(u8 *frame, const struct Silhouette *sil, bool32 isFront, const struct InjuryColors *c)
{
    s32 w = sil->x1 - sil->x0 + 1, h = sil->y1 - sil->y0 + 1;
    s32 tx = sil->x0 + w * (isFront ? 32 : 68) / 100;
    s32 ty = sil->y0 + h * 22 / 100;
    s32 x, y, a, b, best = 0x7FFFFFFF, ex = -1, ey = -1;

    // Nearest point to the usual head spot that has room for a patch.
    for (y = sil->y0; y <= sil->y0 + h / 2; y++)
    {
        for (x = sil->x0; x <= sil->x1; x++)
        {
            s32 d;
            if (!GetPx(frame, x, y) || !GetPx(frame, x - 3, y - 3) || !GetPx(frame, x + 3, y - 3)
             || !GetPx(frame, x - 3, y + 3) || !GetPx(frame, x + 3, y + 3))
                continue;
            d = (x - tx) * (x - tx) * 4 + (y - ty) * (y - ty) * 4;
            if (d < best)
            {
                best = d;
                ex = x; ey = y;
            }
        }
    }
    if (ex < 0)
        return;

    // Strap across the head.
    for (a = -9; a <= 9; a++)
    {
        s32 yy = ey - 2 - (isFront ? (a * 35 + (a < 0 ? -50 : 50)) / 100 : -(a * 35 + (a < 0 ? -50 : 50)) / 100);
        if ((a > 2 || a < -2) && GetPx(frame, ex + a, yy))
            SetPx(frame, ex + a, yy, c->dark);
    }
    // Patch.
    for (a = -3; a <= 3; a++)
    {
        for (b = -3; b <= 3; b++)
        {
            if (a * a + b * b <= 6 && GetPx(frame, ex + a, ey + b))
                SetPx(frame, ex + a, ey + b, c->dark);
        }
    }
    SetPx(frame, ex - 1, ey - 1, c->mid);
}

static void DrawScar(u8 *frame, const struct Silhouette *sil, bool32 isFront, const struct InjuryColors *c)
{
    s32 k, t;
    for (k = -3; k <= 3; k += 3)
    {
        for (t = -6; t <= 6; t++)
        {
            s32 half = (t >= 0 ? t + 1 : t - 1) / 2;
            s32 x = sil->cx + k + (isFront ? half : -half);
            s32 y = sil->cy - 4 + t;
            if (GetPx(frame, x, y) && GetPx(frame, x + 1, y) && GetPx(frame, x - 2, y) && GetPx(frame, x + 3, y))
            {
                SetPx(frame, x, y, c->dark);
                SetPx(frame, x + 1, y, (t % 3) ? c->mid : c->light);
            }
        }
    }
}

static const u8 sDrawOrder[] = { INJURY_PART_TAIL, INJURY_PART_WING, INJURY_PART_HORN, INJURY_PART_LIMB };

void Injury_DrawOnPic(struct BoxPokemon *boxMon, u32 species, void *pic, const u16 *palette, bool32 isFrontPic, u32 frames)
{
    struct InjuryState st;
    struct InjuryColors colors;
    struct Silhouette sil;
    u32 f, i, parts;

    if (pic == NULL || palette == NULL || GetBoxMonData(boxMon, MON_DATA_IS_EGG))
        return;
    GetState(boxMon, &st);
    parts = st.lost & GetSpeciesParts(species);
    if (parts == 0 && !(st.flags & INJURY_FLAG_SCAR))
        return;

    PickColors(palette, &colors);
    for (f = 0; f < frames; f++)
    {
        u8 *frame = (u8 *)pic + f * MON_PIC_SIZE;

        MeasureSilhouette(frame, &sil);
        if (sil.empty)
            continue;
        if (st.flags & INJURY_FLAG_SCAR)
            DrawScar(frame, &sil, isFrontPic, &colors);
        for (i = 0; i < ARRAY_COUNT(sDrawOrder); i++)
        {
            if (parts & INJURY_PART_BIT(sDrawOrder[i]))
                DrawLostPart(frame, sDrawOrder[i], isFrontPic, &colors);
        }
        if (parts & INJURY_PART_BIT(INJURY_PART_EYE))
        {
            MeasureSilhouette(frame, &sil);
            DrawEyePatch(frame, &sil, isFrontPic, &colors);
        }
    }
}
