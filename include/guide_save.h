#ifndef GUARD_GUIDE_SAVE_H
#define GUARD_GUIDE_SAVE_H

#include "constants/guide.h"

// The guide's save data (src/guide.c), in SaveBlock3.
struct GuideSaveData
{
    u8 seen[GUIDE_MAX_ENTRIES / 8]; // entries the player has looked at or been to
};

#endif // GUARD_GUIDE_SAVE_H
