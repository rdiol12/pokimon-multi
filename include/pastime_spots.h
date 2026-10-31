#ifndef GUARD_PASTIME_SPOTS_H
#define GUARD_PASTIME_SPOTS_H

// An NPC or object an activity places on a map from code (src/pastime.c),
// so activities don't need map.json edits or event flags. It shows up while
// isShown(arg) is TRUE (NULL = always); talking to it runs script with
// VAR_0x8004 = arg.
struct PastimeSpot
{
    u16 map;      // MAP_* (group << 8 | num)
    s16 x, y;     // map coordinates, without MAP_OFFSET
    u16 graphicsId;
    u8 movementType;
    u8 arg;
    const u8 *script;
    bool32 (*isShown)(u32 arg);
};

struct PastimeSpotList
{
    const struct PastimeSpot *spots;
    u32 count;
};

extern const struct PastimeSpot gLostFoundSpots[]; // an owner and an item each

#endif // GUARD_PASTIME_SPOTS_H
