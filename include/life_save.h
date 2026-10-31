#ifndef GUARD_LIFE_SAVE_H
#define GUARD_LIFE_SAVE_H

#include "constants/life.h"

// Saved state of the fifth batch of features (docs/life.md). Lives in
// SaveBlock3 next to struct WorldSaveData.

struct LifeSaveData
{
    // Bonding: what each Pokémon got today, by personality.
    u16 bondDay;
    u8 bondPets[LIFE_BOND_SLOTS];
    u8 bondFeeds[LIFE_BOND_SLOTS];
    u32 bondPersonality[LIFE_BOND_SLOTS];
    // Seasons: the season the player last saw outdoors, plus 1 (0: none yet).
    u8 season;
};

#endif // GUARD_LIFE_SAVE_H
