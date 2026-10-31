#ifndef GUARD_RESERVE_H
#define GUARD_RESERVE_H

#include "constants/reserve.h"

// The Haven Reserve (docs/reserve.md). Everything else is called from
// scripts (data/scripts/reserve.inc).
void Reserve_SetupMap(void);
void Reserve_ForgetList(void);

#endif // GUARD_RESERVE_H
