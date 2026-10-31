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
    u16 (*getGraphicsId)(u32 arg); // NULL = graphicsId
    void (*onSpawn)(u32 arg, u8 objectEventId); // NULL = nothing to set up
};

struct PastimeSpotList
{
    const struct PastimeSpot *spots;
    u32 count;
};

extern const struct PastimeSpot gLostFoundSpots[]; // an owner and an item each
extern const struct PastimeSpot gMuseumSpots[];
extern const struct PastimeSpot gDeliverySpots[];
extern const struct PastimeSpot gMailSpots[];
extern const struct PastimeSpot gQuizSpots[];
extern const struct PastimeSpot gHotelSpots[];
extern const struct PastimeSpot gCafeSpots[];
extern const struct PastimeSpot gCookingSpots[];
extern const struct PastimeSpot gBugContestSpots[];
extern const struct PastimeSpot gTradingSpots[];
extern const struct PastimeSpot gDojoSpots[];
extern const struct PastimeSpot gRangerSpots[];
extern const struct PastimeSpot gSchoolSpots[];
extern const struct PastimeSpot gGardenSpots[];
extern const struct PastimeSpot gWeatherSpots[];
extern const struct PastimeSpot gStarSpots[];
extern const struct PastimeSpot gOrchestraSpots[];
extern const struct PastimeSpot gArtSpots[];
extern const struct PastimeSpot gDetectiveSpots[];

#endif // GUARD_PASTIME_SPOTS_H
