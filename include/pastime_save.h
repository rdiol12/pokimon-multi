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
    // Trainer school (src/pastime_school.c)
    u8 schoolPassed;    // a bit per lesson
    u8 schoolFiller;
    u16 schoolTests;
    // Community garden (src/pastime_garden.c)
    u16 gardenDay;      // day + 1 of the last GARDEN SHOW entry
    u8 gardenGift;      // TRUE once the starter Berries were given
    u8 gardenFiller;
    u16 gardenPoints;
    u16 gardenShows;
    // Weather lab (src/pastime_weather.c)
    u16 weatherHour;     // hour + 1 (days * 24 + hours) of the last reading
    u8 weatherReadings;  // taken, not yet reported
    u8 weatherPending;   // their Research Points
    u8 weatherSeen;      // a bit per WEATHER_KIND_*, and WEATHER_LAB_JOINED, _BONUS
    u8 weatherFiller;
    u16 weatherPoints;
    u16 weatherTotal;    // readings reported
    // Star map and meteor showers (src/pastime_stars.c)
    u16 starSeen;        // a bit per constellation
    u8 starRewards;      // the astronomer's prizes given
    u8 meteorPicked;     // a bit per meteorite, on night meteorNight - 1
    u16 meteorNight;
    u16 meteorFound;
    // Carnival games (src/pastime_carnival.c)
    u8 bowlBest;
    u8 golfBest;         // fewest strokes, 0: never played
    u8 soccerWins;
    u8 carnivalPrizes;   // a bit per game's big prize
    u16 ferrisRides;
    u16 carnivalPlays;
    // Pokémon orchestra (src/pastime_orchestra.c)
    u8 orchestraBest;    // longest tune played back
    u8 orchestraPrizes;  // a bit per prize given
    u16 orchestraPlays;
    // Art studio (src/pastime_art.c)
    u8 artBest[ART_PAINTINGS]; // best percent right, per painting
    u8 artFiller;
    u16 artPaintings;
    // Detective cases (src/pastime_detective.c)
    u8 detectiveCase;    // index + 1 of the open case, 0: none
    u8 detectiveClues;   // a bit per clue of the open case
    u8 detectiveSolved;  // a bit per case
    u8 detectiveFiller;
    // Monthly reward track (src/pastime_track.c)
    u16 trackMonth;      // month + 1 of trackPoints
    u16 trackPoints;
    u8 trackTier;        // tiers paid out
    u8 trackFiller;
    u16 trackDay;        // day + 1 of the last daily visit points
    // New Game+ (src/pastime_new_game_plus.c)
    u8 newGamePlusCount;
    u8 newGamePlusFiller[3];
    // Seasonal festivals (src/pastime_festival.c)
    u16 festivalWeek;    // the festival's week + 1 of the stamps
    u8 festivalStamps;   // a bit per stall
    u8 festivalPrize;    // TRUE once this festival's prize is given
    u16 festivalsDone;
    u16 festivalFiller;
    // Archaeology digs (src/pastime_dig.c)
    u16 digDay[DIG_SITES]; // day + 1 each site was last dug
    u16 digFinds;          // everything unearthed
    u16 digFossils;
    u8 digRewards;         // the archaeologist's rewards given
    u8 digFiller[3];
    // Alpha taming (src/pastime_alpha.c)
    u16 alphaDay;        // day + 1 of alphaDoneToday
    u8 alphaDoneToday;   // a bit per alpha met today
    u8 alphaTamedMask;   // a bit per alpha ever tamed
    u16 alphaTamed;
    u16 alphaBattled;
    // Lighthouse keeper (src/pastime_lighthouse.c)
    u16 lighthouseNight;  // night + 1 of the last shift
    u16 lighthouseShifts;
    u16 lighthouseShips;  // ships guided past the rocks
    u8 lighthouseBest;
    u8 lighthouseFiller;
    // Bottle treasure maps (src/pastime_bottle.c)
    u16 bottleDay;       // day + 1 of the last bottle picked up
    u16 bottlePieces;    // a bit per map piece found
    u8 bottleTreasures;  // a bit per treasure dug up
    u8 bottleFiller;
    // Pokémon fashion (src/pastime_fashion.c)
    u16 fashionJudgeDay;    // day + 1 of the last look judged a match
    u32 fashionPersonality; // the Pokémon wearing fashionWorn
    u8 fashionOwned;        // a bit per accessory bought
    u8 fashionWorn;         // the accessory + 1, or 0
};

#endif // GUARD_PASTIME_SAVE_H
