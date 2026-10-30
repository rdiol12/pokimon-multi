#ifndef GUARD_ACHIEVEMENTS_H
#define GUARD_ACHIEVEMENTS_H

// Achievements (src/world_achievements.c). Saved as bits in
// gSaveBlock3Ptr->world.achievements; never renumber these.
enum Achievement
{
    ACH_FIRST_CATCH,
    ACH_DEX_25,
    ACH_DEX_100,
    ACH_DEX_250,
    ACH_BADGES_4,
    ACH_BADGES_8,
    ACH_CHAMPION,
    ACH_NIGHT_OWL,
    ACH_STORM_CHASER,
    ACH_RIDER,
    ACH_SHINY,
    ACH_CHAIN_20,
    ACH_HATCH_10,
    ACH_EVOLVE_20,
    ACH_TRADE,
    ACH_PVP_WIN,
    ACH_ANGLER,
    ACH_BIG_FISH,
    ACH_FARMER,
    ACH_PHOTOGRAPHER,
    ACH_HUNTER,
    ACH_HARDCORE_GYM,
    ACH_HARDCORE_CHAMP,
    ACH_LEGEND,
    ACH_ALL_LEGENDS,
    ACH_HOMEOWNER,
    ACH_RICH,
    ACH_STEPS,
    ACHIEVEMENT_COUNT, // at most 32
};

struct Pokemon;

bool32 Achievements_Has(u8 id);
u32 Achievements_Count(void);
void Achievements_Unlock(u8 id);
void Achievements_OnCatch(struct Pokemon *mon);
void Achievements_Update(void);
void Achievements_ShowList(void);

#endif // GUARD_ACHIEVEMENTS_H
