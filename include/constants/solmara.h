#ifndef GUARD_CONSTANTS_SOLMARA_H
#define GUARD_CONSTANTS_SOLMARA_H

// Solmara, the second region (docs/solmara.md). Its flags live in the
// extended save (struct SolmaraSaveData in SaveBlockExt), at ids from
// SOLMARA_FLAGS_START up; FlagGet/FlagSet, object event flags and hidden
// items all work with them.
#define SOLMARA_FLAGS_START         0x7000
#define SOLMARA_FLAGS_COUNT         2048
#define SOLMARA_FLAG(n)             (SOLMARA_FLAGS_START + (n))

// Solmara's trainers are area trainers (src/data/area_trainers.party) from
// this index up. Their beaten flags are the last 256 Solmara flags.
#define SOLMARA_TRAINERS_FIRST      256
#define SOLMARA_TRAINERS_MAX        256
#define SOLMARA_TRAINER_FLAGS_START SOLMARA_FLAG(SOLMARA_FLAGS_COUNT - SOLMARA_TRAINERS_MAX)

// Every Solmara trainer's Pokémon gain this many levels per Solmara badge,
// so the gyms can be taken in any order (and friends at different points
// can travel together).
#define SOLMARA_LEVELS_PER_BADGE    2

#include "constants/solmara_ids.h"

#define SOLMARA_TRAINER(n)          (AREA_TRAINERS_START + SOLMARA_TRAINERS_FIRST + (n))
#define AREA_TRAINERS_TABLE_SIZE    (SOLMARA_TRAINERS_FIRST + SOLMARA_TRAINERS_COUNT)

#endif // GUARD_CONSTANTS_SOLMARA_H
