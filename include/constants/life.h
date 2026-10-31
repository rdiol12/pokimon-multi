#ifndef GUARD_CONSTANTS_LIFE_H
#define GUARD_CONSTANTS_LIFE_H

// Fifth batch of features (docs/life.md): bonding, seasons, Pokémon jobs,
// underground mining, the photo safari, the Team Eclipse chapter, the
// auction house and ghost trainers.
//
// Numbers this batch owns (agreed with the other feature threads):
// flags 0x290–0x2AF, vars 0x4091, 0x409B, 0x409D, 0x40A1, object local
// ids 216–229, and its own EWRAM mailbox (gLifeMailbox, magic PKMNLIF1).

// Rows this batch adds to the R button menu (data/scripts/world/menu.inc);
// numbered after the world features' own WORLD_MENU_* values.
#define LIFE_MENU_BOND 16
#define LIFE_MENU_JOBS 17
#define LIFE_MENU_ALBUM 18

// Bonding (src/life_bond.c).
#define LIFE_BOND_SLOTS        PARTY_SIZE
#define LIFE_BOND_PET_LIMIT    20 // friendship from petting per Pokémon per day
#define LIFE_BOND_FEED_LIMIT   3  // treats per Pokémon per day

// Pokémon jobs (src/life_jobs.c).
#define LIFE_JOB_SLOTS 3
#define LIFE_JOB_BERRY_FARM  0
#define LIFE_JOB_DELIVERY    1
#define LIFE_JOB_LIFEGUARD   2
#define LIFE_JOB_POWER_PLANT 3
#define LIFE_JOB_CAVE_SURVEY 4
#define LIFE_JOB_NIGHT_WATCH 5
#define LIFE_JOB_COUNT       6
// LifeJobs_Send results.
#define LIFE_JOB_SENT     0
#define LIFE_JOB_LAST_MON 1
#define LIFE_JOB_EGG      2

// Underground mining (src/life_mining.c): dig spots in caves (VAR_0x8005),
// and LifeMining_CheckSpot results.
#define LIFE_DIG_SPOTS        6
#define LIFE_DIG_NO_KIT       0
#define LIFE_DIG_DONE_TODAY   1
#define LIFE_DIG_READY        2

// Photo safari (src/life_photo.c): roaming Pokémon use object local ids
// 216-219, and LifePhoto_RangerState results.
#define LIFE_PHOTO_MONS          4
#define LOCALID_LIFE_PHOTO_MON   216
#define LIFE_ALBUM_SIZE          24
#define LIFE_POSE_NONE    0
#define LIFE_POSE_LOOKING 1
#define LIFE_POSE_MOVING  2
#define LIFE_POSE_SIDE    3
#define LIFE_POSE_BEHIND  4
#define LIFE_PHOTO_RANGER_OFFER     0
#define LIFE_PHOTO_RANGER_STILL_OUT 1
#define LIFE_PHOTO_RANGER_PAYOUT    2

// Team Eclipse (src/life_eclipse.c): the postgame chapter. VAR_LIFE_ECLIPSE
// is the step; each trainer has a flag that hides its object and one that
// says it's been beaten.
#define VAR_LIFE_ECLIPSE            0x4091
#define LIFE_ECLIPSE_NOT_STARTED    0
#define LIFE_ECLIPSE_METEOR_FALLS   1
#define LIFE_ECLIPSE_MT_PYRE        2
#define LIFE_ECLIPSE_SPACE_CENTER   3
#define LIFE_ECLIPSE_REWARD         4
#define LIFE_ECLIPSE_DONE           5
#define LIFE_ECLIPSE_TRAINERS       6
#define FLAG_LIFE_ECLIPSE_HIDE_0    0x290
#define FLAG_LIFE_ECLIPSE_HIDE_1    0x291
#define FLAG_LIFE_ECLIPSE_HIDE_2    0x292
#define FLAG_LIFE_ECLIPSE_HIDE_3    0x293
#define FLAG_LIFE_ECLIPSE_HIDE_4    0x294
#define FLAG_LIFE_ECLIPSE_HIDE_5    0x295
#define FLAG_LIFE_ECLIPSE_HIDE_SCIENTIST 0x296
#define FLAG_LIFE_ECLIPSE_BEATEN_0  0x298 // to 0x29D

// Auction house (src/life_auction.c, server/life_auction.py). VAR_0x8004 of
// LifeAuction_Request.
#define LIFE_AUC_REQ_LIST     0
#define LIFE_AUC_REQ_POST     1 // VAR_0x8005: party slot, VAR_0x8006: hours; the price is LifeAuction_PickPrice's
#define LIFE_AUC_REQ_BID      2 // the listing and amount LifeAuction_Browse picked
#define LIFE_AUC_REQ_CANCEL   3 // the listing LifeAuction_Browse picked
#define LIFE_AUC_REQ_COLLECT  4
#define LIFE_AUC_REQ_COUNT    5
// VAR_RESULT of LifeAuction_Browse.
#define LIFE_AUC_PICK_NONE    0
#define LIFE_AUC_PICK_BID     1
#define LIFE_AUC_PICK_CANCEL  2
// VAR_0x8005 after LIFE_AUC_REQ_COLLECT: what was waiting.
#define LIFE_AUC_GOT_NOTHING  0
#define LIFE_AUC_GOT_WON      1
#define LIFE_AUC_GOT_SOLD     2
#define LIFE_AUC_GOT_OUTBID   3
#define LIFE_AUC_GOT_UNSOLD   4
#define LIFE_AUC_GOT_CANCELED 5
#define LIFE_AUC_LISTINGS     20

// Seasons (src/life_seasons.c), from the real-world month.
#define SEASON_SPRING 0 // March to May
#define SEASON_SUMMER 1 // June to August
#define SEASON_AUTUMN 2 // September to November
#define SEASON_WINTER 3 // December to February
#define SEASON_COUNT  4

#endif // GUARD_CONSTANTS_LIFE_H
