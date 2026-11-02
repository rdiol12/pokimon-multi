#ifndef GUARD_STORMWILD_H
#define GUARD_STORMWILD_H

// The Stormwild, the extraction run (src/stormwild.c, server/online_xrun.py,
// docs/stormwild.md).

#define STORMWILD_POUCH_KINDS 10  // kinds of items a runner may bring
#define STORMWILD_POUCH_EACH  20  // ... and how many of each
#define STORMWILD_MAX_TEAM    3
#define STORMWILD_MAX_CATCHES 12
#define STORMWILD_STASH_SLOTS (BAG_ITEMS_COUNT + BAG_POKEBALLS_COUNT + BAG_BERRIES_COUNT)

enum
{
    STORMWILD_NONE,
    STORMWILD_PACKED, // the pouch is packed (out of the bag), not in yet
    STORMWILD_IN_RUN, // the rest of the bag waits at the gate
};

// In the extended save (include/save_ext.h).
struct StormwildSave
{
    u8 state;
    u8 numPouch;
    u8 numCatches;
    u8 drop;
    u16 runId;
    u16 runs;
    u32 activeCaches;
    u32 bestHaul;
    u16 escapes;
    u16 filler;
    struct ItemSlot pouch[STORMWILD_POUCH_KINDS];
    struct ItemSlot stash[STORMWILD_STASH_SLOTS];
    u32 catches[STORMWILD_MAX_CATCHES]; // personalities of the Pokémon caught in the run
};

bool32 Stormwild_IsOn(void);
bool32 Stormwild_Update(void);
void Stormwild_OnWildBattleStart(void);
void Stormwild_OnCatch(void);
u32 Stormwild_ShinyRolls(void);

// Requests' build and after hooks (src/online_requests.c).
void Stormwild_AddRobbed(void);
void Stormwild_AfterRob(void);
void Stormwild_AddHaul(void);

#endif // GUARD_STORMWILD_H
