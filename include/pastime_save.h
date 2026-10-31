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
    // Mail courier routes.
    u16 mailDay;        // day the route was taken (gLocalTime.days)
    u8 mailTown;
    u8 mailLetters;     // a bit per letter still to deliver, | MAIL_ACTIVE or MAIL_PAID
    u16 mailRoutes;     // routes finished
    u16 mailLettersDelivered;
    // Battle quiz hall.
    u16 quizBest;       // best round score
    u16 quizPrizeDay;   // day the daily prize was won
    u16 quizRounds;
    u16 quizPerfect;    // rounds with every answer right
    // Pokémon hotel.
    u16 hotelDay;       // day the guests below are for
    u8 hotelStars;      // 1 to HOTEL_MAX_STARS (0 = a save from before the hotel)
    u8 hotelServed;     // a bit per guest served today
    bool8 hotelCollected; // today's earnings were collected
    u8 hotelFiller;
    u16 hotelTips;      // tips waiting at the desk
    u32 hotelEarnings;  // all-time
    u16 hotelGuests;    // all-time guests served
    u16 hotelFiller2;
};

#endif // GUARD_PASTIME_SAVE_H
