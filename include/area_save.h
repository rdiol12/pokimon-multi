#ifndef GUARD_AREA_SAVE_H
#define GUARD_AREA_SAVE_H

#include "constants/areas.h"

// Saved state of the new areas (docs/areas.md). Lives at the end of
// SaveBlock3.
struct AreaSaveData
{
    u8 flags[AREA_FLAGS_COUNT / 8];
};

#endif // GUARD_AREA_SAVE_H
