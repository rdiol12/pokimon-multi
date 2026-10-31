#include "global.h"
#include "battle.h"
#include "online.h"
#include "pokemon.h"
#include "random.h"

// Fishing tournaments (server/online_fishing.py). While one is on, the
// server's status says so ("S|FISHT|1|minutes left|rare|rare|rare|id") and a
// third of the Pokémon hooked are one of its three rare species, at the
// level of the Pokémon they replace. Landed Pokémon score by size, rare ones
// more (the FISH report in src/online.c).

#define RARE_CHANCE 3 // one bite in this many

bool32 Online_TournamentIsOn(void)
{
    return Online_StatusNum("FISHT", 0) == 1;
}

// A fishing encounter's Pokémon was just made: maybe swap in a rare one.
void Online_TournamentBite(void)
{
    struct Pokemon *mon = &gEnemyParty[0];
    u16 species;
    u8 level;

    if (!Online_TournamentIsOn() || Random() % RARE_CHANCE != 0)
        return;
    species = Online_StatusNum("FISHT", 2 + Random() % 3);
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    level = GetMonData(mon, MON_DATA_LEVEL);
    CreateMonWithIVs(mon, species, level, GetMonPersonality(species, MON_GENDER_RANDOM, NATURE_RANDOM, RANDOM_UNOWN_LETTER),
        OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
}
