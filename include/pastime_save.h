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
    // Pokémon cafe: the dish eaten last, and battles it still lasts.
    u8 cafeDish;
    u8 cafeBattles;
    u16 cafeDishesCooked;
    // Cooking contest (src/pastime_cooking.c)
    u16 cookDay;      // day + 1 of the last contest entered
    u8 cookRank;      // COOK_RANK_*
    u8 cookBest;
    u16 cookWins;
    u16 cookEntries;
    // Bug-catching contest (src/pastime_bug_contest.c)
    u16 bugDay;       // day + 1 of the last contest entered
    u16 bugBest;
    u16 bugBestSpecies;
    u16 bugWins;
    u16 bugEntries;
    // Trading post (src/pastime_trading.c)
    u16 tradeDay;     // day + 1 the offers below are for
    u8 tradeDone;     // a bit per barter done today, TRADING_SWAP_DONE
    u8 tradeFiller;
    u16 tradeBarters;
    u16 tradeSwaps;
    // Battle dojo (src/pastime_dojo.c)
    u16 dojoDay;      // day + 1 of dojoSessions
    u8 dojoSessions;
    u8 dojoFiller;
    u16 dojoPoints;
    u16 dojoEarned;
    u16 dojoTaught;
    // Ranger missions (src/pastime_ranger.c)
    u16 rangerDay;      // day + 1 of rangerToday
    u8 rangerToday;     // missions given today
    u8 rangerMission;   // index + 1 in sMissions, 0: none
    u8 rangerProgress;
    u8 rangerFiller;
    u16 rangerPoints;
    u16 rangerDone;
};

#endif // GUARD_PASTIME_SAVE_H
