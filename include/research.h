#ifndef GUARD_RESEARCH_H
#define GUARD_RESEARCH_H

#include "constants/pokedex.h"

// Research Pokédex (src/research.c, docs/research.md): every species has
// research tasks (catch it, defeat it, evolve it, find a big one, catch it
// by day and at night). Each task tier reached is a research level and pays
// a little money; level 10 completes the species, which pays more and makes
// wild ones of it likelier to be shiny. Completed species add up to
// research ranks with bigger rewards. Opened from the R menu (RESEARCH).

#define RESEARCH_SPECIES (NATIONAL_DEX_COUNT + 1) // by national dex number
// The first 1026 entries (dex numbers 0-1025) were saved before brand-new
// Pokémon got dex numbers, so that array keeps its size and new numbers go in
// SaveBlockExt.researchMore (room for RESEARCH_MORE_SPECIES of them).
#define RESEARCH_SAVED_SPECIES 1026
#define RESEARCH_MORE_SPECIES 64
#define RESEARCH_COMPLETE 10                     // the research level that completes a species

#define RESEARCH_FLAG_BIG      (1 << 0) // caught one at least 115% of the usual size
#define RESEARCH_FLAG_DAY      (1 << 1) // caught one by day
#define RESEARCH_FLAG_NIGHT    (1 << 2) // caught one at night
#define RESEARCH_FLAG_COMPLETE (1 << 3) // the completion reward was given
#define RESEARCH_FLAG_PERFECT  (1 << 4) // every task done to the last tier

struct ResearchEntry
{
    u8 caught;
    u8 defeated;
    u8 evolved;
    u8 flags;
};

// In the extended save (include/save_ext.h).
struct ResearchSave
{
    u16 completed; // species completed
    u8 rank;       // research ranks whose reward was given
    u8 filler;
    struct ResearchEntry entries[RESEARCH_SAVED_SPECIES];
};

struct ResearchSaveMore
{
    struct ResearchEntry entries[RESEARCH_MORE_SPECIES];
};

// Hooks.
void Research_OnWildBattleStart(void);
void Research_OnCatch(void);
void Research_OnDefeat(u16 species);
void Research_OnEvolve(u16 preEvoSpecies);
void Research_OnWildSpecies(u16 species);
u32 Research_ShinyRolls(void);
void Research_Update(void);

// The research level (0-10+) of a species.
u32 Research_GetLevel(u16 species);

// Scripts.
void Research_ShowScreen(void);       // callnative, waitstate=1
void Research_GetCompletedCount(void); // VAR_0x8005 = species completed

#endif // GUARD_RESEARCH_H
