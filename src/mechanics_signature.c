#include "global.h"
#include "event_data.h"
#include "mechanics.h"
#include "move.h"
#include "party_menu.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "constants/moves.h"
#include "constants/species.h"

// Starter signature moves: Professor Birch's aide tells the player about an
// old statue of their starter's final form, in a place that suits its type.
// With the starter (any stage) in the party, the statue's guardian battles
// it; beat the guardian and the aide teaches the starter its signature
// move. Only that line can learn it (party_menu.c, CanTeachMove).

#define GUARDIAN_OT_ID 0x4D454348 // marks the guardian, which can't be caught

struct StarterLine
{
    u16 species[3];
    u16 move;
    const u8 *place;
};

static const struct StarterLine sLines[] =
{
    [STARTER_LINE_TREECKO] = {{SPECIES_TREECKO, SPECIES_GROVYLE, SPECIES_SCEPTILE}, MOVE_CANOPY_EDGE, COMPOUND_STRING("PETALBURG WOODS")},
    [STARTER_LINE_TORCHIC] = {{SPECIES_TORCHIC, SPECIES_COMBUSKEN, SPECIES_BLAZIKEN}, MOVE_BLAZE_RUSH, COMPOUND_STRING("the FIERY PATH")},
    [STARTER_LINE_MUDKIP]  = {{SPECIES_MUDKIP, SPECIES_MARSHTOMP, SPECIES_SWAMPERT}, MOVE_MIRE_CRASH, COMPOUND_STRING("ROUTE 117's pond")},
};

static u8 GetStarterLine(u16 species)
{
    u32 i, j;

    // Megas count as their base form.
    if (species == SPECIES_SCEPTILE_MEGA)
        return STARTER_LINE_TREECKO;
    if (species == SPECIES_BLAZIKEN_MEGA)
        return STARTER_LINE_TORCHIC;
    if (species == SPECIES_SWAMPERT_MEGA)
        return STARTER_LINE_MUDKIP;
    for (i = 1; i < ARRAY_COUNT(sLines); i++)
    {
        for (j = 0; j < ARRAY_COUNT(sLines[i].species); j++)
        {
            if (sLines[i].species[j] == species)
                return i;
        }
    }
    return STARTER_LINE_NONE;
}

bool32 Mechanics_CanLearnSignatureMove(u16 species, u16 move)
{
    u8 line = GetStarterLine(species);

    return line != STARTER_LINE_NONE && sLines[line].move == move;
}

bool32 Mechanics_IsCatchBlocked(void)
{
    return GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_OT_ID) == GUARDIAN_OT_ID;
}

// The first party Pokémon of a starter line, or NULL.
static struct Pokemon *FindStarter(u8 line)
{
    u32 i;

    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (line == STARTER_LINE_NONE ? GetStarterLine(GetMonData(mon, MON_DATA_SPECIES)) != STARTER_LINE_NONE
                                      : GetStarterLine(GetMonData(mon, MON_DATA_SPECIES)) == line)
            return mon;
    }
    return NULL;
}

// Script: VAR_RESULT = the STARTER_LINE_* of the first starter in the
// party (0: none). gStringVar1 = its nickname, gStringVar2 = its move,
// gStringVar3 = where its statue is.
void Mechanics_FindStarter(void)
{
    struct Pokemon *mon = FindStarter(STARTER_LINE_NONE);
    u8 line;

    gSpecialVar_Result = STARTER_LINE_NONE;
    if (mon == NULL)
        return;
    line = GetStarterLine(GetMonData(mon, MON_DATA_SPECIES));
    gSpecialVar_Result = line;
    GetMonNickname(mon, gStringVar1);
    StringCopy(gStringVar2, GetMoveName(sLines[line].move));
    StringCopy(gStringVar3, sLines[line].place);
}

// Script: VAR_0x8004 = the statue's STARTER_LINE_*. VAR_RESULT = TRUE if
// that line is in the party (gStringVar1 = its nickname); then the
// guardian, the line's final form, is ready for a scripted wild battle.
void Mechanics_SetupGuardian(void)
{
    const struct StarterLine *line = &sLines[gSpecialVar_0x8004];
    struct Pokemon *mon = FindStarter(gSpecialVar_0x8004);
    u32 level;

    StringCopy(gStringVar2, GetSpeciesName(line->species[2]));
    gSpecialVar_Result = (mon != NULL);
    if (mon == NULL)
        return;
    GetMonNickname(mon, gStringVar1);
    // A little stronger than the starter, and never a pushover.
    level = min(max(GetMonData(mon, MON_DATA_LEVEL) + 3, 30), MAX_LEVEL);
    ZeroEnemyPartyMons();
    CreateMonWithIVs(&gParties[B_TRAINER_OPPONENT_A][0], line->species[2], level, Random32(),
                     OTID_STRUCT_PRESET(GUARDIAN_OT_ID), 20);
    GiveMonInitialMoveset(&gParties[B_TRAINER_OPPONENT_A][0]);
    Mechanics_PrepareOpponentMon(&gParties[B_TRAINER_OPPONENT_A][0]);
}

// The statue shows only while the quest wants it.
bool32 Mechanics_IsStatueHere(u8 line)
{
    return FlagGet(FLAG_MECH_SIG_STARTED) && !FlagGet(FLAG_MECH_SIG_BEATEN)
        && FindStarter(STARTER_LINE_NONE) != NULL
        && GetStarterLine(GetMonData(FindStarter(STARTER_LINE_NONE), MON_DATA_SPECIES)) == line;
}
