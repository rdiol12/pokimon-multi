#ifndef GUARD_LIFE_SAVE_H
#define GUARD_LIFE_SAVE_H

#include "constants/life.h"

// Saved state of the fifth batch of features (docs/life.md). Lives in
// SaveBlock3 next to struct WorldSaveData.

// struct BoxPokemon isn't defined yet where global.h includes this, so a
// stored Pokémon is kept as raw words (life_jobs.c checks the size).
#define LIFE_BOXMON_WORDS 20

// A Pokémon out on a job: it's out of the party until it comes back.
struct LifeJob
{
    u32 mon[LIFE_BOXMON_WORDS]; // struct BoxPokemon
    u32 startMinute; // gLocalTime in minutes when it left
    u8 job;          // LIFE_JOB_*
    u8 active;
    u16 padding;
};

// The best photo of one species in the photo safari album.
struct LifePhoto
{
    u16 species;
    u16 score;
    u8 pose;  // LIFE_POSE_*
    u8 shiny;
    u16 padding;
};

struct LifeSaveData
{
    // Bonding: what each Pokémon got today, by personality.
    u16 bondDay;
    u8 bondPets[LIFE_BOND_SLOTS];
    u8 bondFeeds[LIFE_BOND_SLOTS];
    u32 bondPersonality[LIFE_BOND_SLOTS];
    // Seasons: the season the player last saw outdoors, plus 1 (0: none yet).
    u8 season;
    u8 padding[3];
    struct LifeJob jobs[LIFE_JOB_SLOTS];
    // Mining: the digger's tools, which spots were dug today, and a count.
    u16 digDay;
    u8 digSpotsToday; // bit per dig spot
    u8 hasDigKit;
    u16 treasuresDug;
    u16 padding2;
    // Photo safari: shots left on the camera (0: no safari), the safari's
    // running score, and the album.
    u8 photoFilm;
    u8 padding3;
    u16 photoTotal;
    struct LifePhoto album[LIFE_ALBUM_SIZE];
};

#endif // GUARD_LIFE_SAVE_H
