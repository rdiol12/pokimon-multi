#ifndef GUARD_CONSTANTS_AREAS_H
#define GUARD_CONSTANTS_AREAS_H

// New areas (docs/areas.md). Their flags live in SaveBlock3 (struct
// AreaSaveData) instead of the full SaveBlock1 flag array, at ids from
// AREA_FLAGS_START up. FlagGet/FlagSet, object event flags and hidden items
// all work with them.
#define AREA_FLAGS_START          0x6000
#define AREA_FLAGS_COUNT          1024
#define AREA_FLAG(n)              (AREA_FLAGS_START + (n))

// Area trainers: a trainer table of their own (src/data/area_trainers.party),
// so new maps get trainers without raising MAX_TRAINERS_COUNT. Their ids
// start above the partner ids; their beaten flags are the last area flags.
#define AREA_TRAINERS_START       0x400
#define AREA_TRAINERS_MAX         256
#define AREA_TRAINER_FLAGS_START  AREA_FLAG(AREA_FLAGS_COUNT - AREA_TRAINERS_MAX)
#define AREA_TRAINER(n)           (AREA_TRAINERS_START + (n))

#include "constants/area_ids.h"

#endif // GUARD_CONSTANTS_AREAS_H
