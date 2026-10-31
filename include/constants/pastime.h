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
#define PASTIME_MENU_SCHOOL     12
#define PASTIME_MENU_GARDEN     13
#define PASTIME_MENU_WEATHER    14
#define PASTIME_MENU_STARS      15
#define PASTIME_MENU_CARNIVAL   16
#define PASTIME_MENU_ORCHESTRA  17
#define PASTIME_MENU_ART        18
#define PASTIME_MENU_DETECTIVE  19
#define PASTIME_MENU_TRACK      20
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

// Trainer school in Rustboro (src/pastime_school.c).
#define SCHOOL_LESSONS      5
#define SCHOOL_QUESTIONS    5
#define SCHOOL_ANSWERS      4
#define SCHOOL_PASS_MARK    4
#define SCHOOL_MENU_CANCEL  15
#define SCHOOL_FAILED       0 // VAR_RESULT of School_Finish
#define SCHOOL_PASSED       1
#define SCHOOL_PASSED_FIRST 2

// Community garden on Route 104 (src/pastime_garden.c).
#define GARDEN_PLOTS          8
#define GARDEN_TREE_FIRST     112 // berry tree ids GARDEN_TREE_FIRST to + GARDEN_PLOTS - 1
#define GARDEN_RANK_COUNT     4
#define GARDEN_SHOW_OK        0 // VAR_RESULT of Garden_CheckShow
#define GARDEN_SHOW_DONE      1
#define GARDEN_SHOW_MISSING   2

// Weather lab at the Weather Institute (src/pastime_weather.c).
#define WEATHER_KIND_RAIN      0
#define WEATHER_KIND_STORM     1
#define WEATHER_KIND_SAND      2
#define WEATHER_KIND_SNOW      3
#define WEATHER_KIND_COUNT     4
#define WEATHER_LAB_JOINED     (1 << 6) // bits of weatherSeen
#define WEATHER_LAB_BONUS      (1 << 7)
#define WEATHER_LAB_MAX_READINGS 10
#define WEATHER_LAB_FULL_SET_POINTS 10
#define WEATHER_RANK_COUNT     4
#define WEATHER_MENU_FORECAST  0
#define WEATHER_MENU_REPORT    1
#define WEATHER_MENU_CANCEL    2

// Star map and meteor shower nights (src/pastime_stars.c).
#define STAR_CONSTELLATIONS    8
#define STAR_METEORITES        5
#define STAR_NIGHT_START       20 // hours
#define STAR_NIGHT_END         5
#define STAR_METEOR_EVERY      5  // nights
#define STAR_METEOR_MON_PERCENT 25
#define STAR_METEOR_RATE       6  // a shooting star every this many updates, on average

// Carnival games at the Sunfair Carnival (src/pastime_carnival.c).
#define CARNIVAL_PERFECT_PIXELS 2 // needle this close to the zone's middle
#define CARNIVAL_CLOSE_PIXELS  14
#define BOWLING_FRAMES         5
#define BOWLING_PINS           10
#define BOWLING_STRIKE_BONUS   10
#define BOWLING_SPARE_BONUS    5
#define BOWLING_GREAT          80
#define BOWLING_GOOD           50
#define GOLF_HOLES             3
#define GOLF_PAR               3
#define GOLF_POND_HOLE         2  // the third hole has the pond
#define GOLF_PUTT_YARDS        8
#define GOLF_ACE_YARDS         130 // a perfect shot from this close goes in
#define GOLF_MAX_STROKES       7
#define SOCCER_KICKS           5
#define SOCCER_SIDES           3
#define SOCCER_CPU_MISS_PERCENT 15
#define SOCCER_OVER            0 // VAR_RESULT of Soccer_Prepare
#define SOCCER_SHOOT           1
#define SOCCER_KEEP            2
#define SOCCER_WON             0 // VAR_RESULT of Soccer_Finish
#define SOCCER_DRAW            1
#define SOCCER_LOST            2
#define FERRIS_WHEEL_FARE      100

// Pokémon orchestra in Verdanturf (src/pastime_orchestra.c).
#define ORCHESTRA_FIRST_NOTES  3
#define ORCHESTRA_MAX_NOTES    12

// Art studio at the Lilycove Museum (src/pastime_art.c).
#define ART_PAINTINGS          5
#define ART_SIZE               12 // cells a side
#define ART_MENU_CANCEL        15

// Detective cases from Mauville (src/pastime_detective.c).
#define DETECTIVE_CASES        4
#define DETECTIVE_CLUES        3
#define DETECTIVE_SUSPECTS     3
#define DETECTIVE_MENU_NOT_YET 15
#define DETECTIVE_OPEN         0 // VAR_RESULT of Detective_GetState
#define DETECTIVE_NEW          1
#define DETECTIVE_ALL_SOLVED   2

// Monthly reward track (src/pastime_track.c).
#define TRACK_TIERS            10
#define TRACK_COUNT            3  // tracks that take turns, a month each
#define TRACK_MONTH_DAYS       30
#define TRACK_DAILY_POINTS     5
#define TRACK_CATCH_POINTS     2
#define TRACK_TRAINER_POINTS   3

#endif // GUARD_CONSTANTS_PASTIME_H
