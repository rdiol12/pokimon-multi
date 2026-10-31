#ifndef GUARD_CONSTANTS_LIFE_H
#define GUARD_CONSTANTS_LIFE_H

// Fifth batch of features (docs/life.md): bonding, seasons, Pokémon jobs,
// underground mining, the photo safari, the Team Eclipse chapter, the
// auction house and ghost trainers.
//
// Numbers this batch owns (agreed with the other feature threads):
// flags 0x290–0x2AF, vars 0x4091, 0x409B, 0x409D, 0x40A1, object local
// ids 216–229, and its own EWRAM mailbox (gLifeMailbox, magic PKMNLIF1).

// Rows this batch adds to the R button menu (data/scripts/world/menu.inc);
// numbered after the world features' own WORLD_MENU_* values.
#define LIFE_MENU_BOND 16
#define LIFE_MENU_JOBS 17

// Bonding (src/life_bond.c).
#define LIFE_BOND_SLOTS        PARTY_SIZE
#define LIFE_BOND_PET_LIMIT    20 // friendship from petting per Pokémon per day
#define LIFE_BOND_FEED_LIMIT   3  // treats per Pokémon per day

// Pokémon jobs (src/life_jobs.c).
#define LIFE_JOB_SLOTS 3
#define LIFE_JOB_BERRY_FARM  0
#define LIFE_JOB_DELIVERY    1
#define LIFE_JOB_LIFEGUARD   2
#define LIFE_JOB_POWER_PLANT 3
#define LIFE_JOB_CAVE_SURVEY 4
#define LIFE_JOB_NIGHT_WATCH 5
#define LIFE_JOB_COUNT       6
// LifeJobs_Send results.
#define LIFE_JOB_SENT     0
#define LIFE_JOB_LAST_MON 1
#define LIFE_JOB_EGG      2

// Seasons (src/life_seasons.c), from the real-world month.
#define SEASON_SPRING 0 // March to May
#define SEASON_SUMMER 1 // June to August
#define SEASON_AUTUMN 2 // September to November
#define SEASON_WINTER 3 // December to February
#define SEASON_COUNT  4

#endif // GUARD_CONSTANTS_LIFE_H
