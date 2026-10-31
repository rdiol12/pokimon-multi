#ifndef GUARD_CONSTANTS_PASTIME_H
#define GUARD_CONSTANTS_PASTIME_H

// Minigames and side activities (docs/pastimes.md).

// Row of the R menu (data/scripts/world/menu.inc) that opens R > PASTIMES.
#define WORLD_MENU_PASTIMES 20

// R > PASTIMES submenu (data/scripts/pastime/menu.inc).
#define PASTIME_MENU_LOST_FOUND 0
#define PASTIME_MENU_MUSEUM     1
#define PASTIME_MENU_DELIVERY   2
#define PASTIME_MENU_MAIL       3
#define PASTIME_MENU_QUIZ       4
#define PASTIME_MENU_HOTEL      5
#define PASTIME_MENU_CAFE       6
#define PASTIME_MENU_COOKING    7
#define PASTIME_MENU_BUG_CONTEST 8
#define PASTIME_MENU_TRADING    9
#define PASTIME_MENU_DOJO       10
#define PASTIME_MENU_RANGER     11
#define PASTIME_MENU_CLOSE      31

// NPCs and objects the activities place on maps from code (src/pastime.c)
// use these local ids, so they never clash with a map's own objects.
#define LOCALID_PASTIME_FIRST 170
#define LOCALID_PASTIME_LAST  189

// Lost and found (src/pastime_lost.c).
#define LOST_FOUND_COUNT 8
#define LOST_FOUND_MISSING  0
#define LOST_FOUND_CARRYING 1
#define LOST_FOUND_RETURNED 2

// Museum donations (src/pastime_museum.c).
#define MUSEUM_FOSSIL_COUNT   15
#define MUSEUM_RARE_SLOTS     12
#define MUSEUM_FOSSIL_STATUES 4
#define MUSEUM_RARE_STATUES   3
#define MUSEUM_SPOT_COUNT     (1 + MUSEUM_FOSSIL_STATUES + MUSEUM_RARE_STATUES)
#define MUSEUM_RARE_OK        0
#define MUSEUM_RARE_NOT_RARE  1
#define MUSEUM_RARE_ALREADY   2
#define MUSEUM_RARE_FULL      3
#define MUSEUM_RARE_EGG       4

// Delivery jobs (src/pastime_delivery.c).
#define DELIVERY_TOWN_COUNT 8
#define DELIVERY_NONE       0xFF
#define DELIVERY_OFFER      0 // VAR_RESULT of Delivery_Talk
#define DELIVERY_NO_TOWNS   1
#define DELIVERY_DELIVERED  2
#define DELIVERY_ELSEWHERE  3

// Mail courier routes (src/pastime_mail.c).
#define MAIL_TOWN_COUNT   4
#define MAIL_LETTERS      3
#define MAIL_ACTIVE       0x40 // in mailLetters: today's route was taken
#define MAIL_PAID         0x80 // and paid
#define MAIL_OTHER_TOWN   0 // VAR_RESULT of Mail_TalkToPostmaster
#define MAIL_NEW_ROUTE    1
#define MAIL_IN_PROGRESS  2
#define MAIL_COMPLETE     3
#define MAIL_DONE_TODAY   4

// Battle quiz hall (src/pastime_quiz.c).
#define QUIZ_QUESTIONS   10
#define QUIZ_SUPER       0 // answers, in menu order
#define QUIZ_NORMAL      1
#define QUIZ_NOT_VERY    2
#define QUIZ_NO_EFFECT   3
#define WORLD_BOARD_QUIZ 16 // server leaderboard (server/world.py)

// Pokémon hotel on Starfall Isle (src/pastime_hotel.c).
#define HOTEL_MAX_STARS     5
#define HOTEL_MAX_GUESTS    6
#define HOTEL_SPOT_COUNT    (2 + HOTEL_MAX_GUESTS) // bellhop outside, manager desk, guests
#define HOTEL_GUEST_SERVED  0 // VAR_RESULT of Hotel_TalkToGuest
#define HOTEL_GUEST_ASKS    1
#define HOTEL_GUEST_CANT    2
#define HOTEL_DESK_COLLECT  0 // VAR_RESULT of Hotel_TalkToDesk
#define HOTEL_DESK_WAITING  1
#define HOTEL_DESK_DONE     2

// Pokémon cafe (src/pastime_cafe.c).
#define CAFE_DISH_NONE    0
#define CAFE_DISH_COUNT   7 // dishes 1 to 6
#define CAFE_MENU_CANCEL  15

// Cooking contest (src/pastime_cooking.c), next to the cafe.
#define COOK_RANK_NORMAL  0
#define COOK_RANK_GREAT   1
#define COOK_RANK_ULTRA   2
#define COOK_RANK_MASTER  3
#define COOK_RANK_COUNT   4
#define COOK_RIVALS       3
#define COOK_STEPS        3 // chop, stir, plate: one timing meter round each
#define WORLD_BOARD_COOKING 17 // server leaderboard (server/world.py)

// Bug-catching contest on Route 119 (src/pastime_bug_contest.c).
#define BUG_CONTEST_OPEN       0 // VAR_RESULT of BugContest_GetState
#define BUG_CONTEST_RUNNING    1
#define BUG_CONTEST_DONE_TODAY 2
#define WORLD_BOARD_BUG_CONTEST 18 // server leaderboard (server/world.py)

// Trading post in the Slateport market (src/pastime_trading.c).
#define TRADING_OFFERS      3    // barters a day
#define TRADING_SWAP_DONE   0x80 // tradeDone bit
#define TRADING_MENU_SWAP   13
#define TRADING_MENU_CANCEL 14
#define TRADING_OK          0    // VAR_RESULT of the checks
#define TRADING_DONE        1
#define TRADING_MISSING     2
#define TRADING_EGG         3

// Battle dojo in Dewford (src/pastime_dojo.c).
#define DOJO_SESSIONS     3  // training sessions a day
#define DOJO_DRILLS       5  // timing meter rounds a session
#define DOJO_BELT_COUNT   5
#define DOJO_MOVE_COUNT   16
#define DOJO_MENU_TRAIN   0
#define DOJO_MENU_LEARN   1
#define DOJO_MENU_CANCEL  31
#define DOJO_OK           0  // VAR_RESULT of Dojo_CheckMove
#define DOJO_NO_BELT      1
#define DOJO_NO_POINTS    2

// Ranger missions from Fortree (src/pastime_ranger.c).
#define RANGER_RESCUE            0
#define RANGER_CALM              1
#define RANGER_SURVEY            2
#define RANGER_LOST_COUNT        5
#define RANGER_RANK_COUNT        4
#define RANGER_MISSIONS_PER_DAY  3
#define RANGER_STATE_OPEN        0 // VAR_RESULT of Ranger_GetState
#define RANGER_STATE_ACTIVE      1
#define RANGER_STATE_COMPLETE    2
#define RANGER_STATE_DONE_TODAY  3

#endif // GUARD_CONSTANTS_PASTIME_H
