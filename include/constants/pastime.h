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

#endif // GUARD_CONSTANTS_PASTIME_H
