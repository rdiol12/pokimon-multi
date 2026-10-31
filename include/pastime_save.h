#ifndef GUARD_PASTIME_SAVE_H
#define GUARD_PASTIME_SAVE_H

#include "constants/pastime.h"

// Saved state of the minigames and side activities (docs/pastimes.md).
// Lives at the end of SaveBlock3. Keep it a multiple of 4 bytes.

struct PastimeSaveData
{
    // Lost and found: a bit per owner.
    u8 lostFound;    // the item was picked up
    u8 lostReturned; // and given back
    u8 lostHeard;    // the owner was met
    u8 filler;
    // Museum donations.
    u16 museumFossils;                  // a bit per fossil (sFossils)
    u16 museumPhotos;                   // photos donated
    u16 museumRare[MUSEUM_RARE_SLOTS];  // rare Pokémon shown, by species
    u8 museumRewards;                   // milestone rewards given
    u8 museumFiller[3];
    // Delivery jobs: the parcel being carried, if any.
    u8 deliveryFrom;    // town index, DELIVERY_NONE if no parcel
    u8 deliveryTo;
    u8 deliveryParcel;  // what's in it
    bool8 deliveryLate; // the "late" notice was shown
    u32 deliveryStart;  // play time in seconds when it was picked up
    u16 deliveryLimit;  // seconds allowed
    u16 deliveries;     // parcels delivered
};

#endif // GUARD_PASTIME_SAVE_H
