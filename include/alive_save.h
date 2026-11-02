#ifndef GUARD_ALIVE_SAVE_H
#define GUARD_ALIVE_SAVE_H

#include "constants/alive.h"

// What a townsperson remembers of the player (src/alive_talk.c).
struct AliveMemory
{
    u8 mapGroup;
    u8 mapNum;
    u8 localId;
    u8 times;     // times talked to (stops at 255)
    u16 lead;     // the lead Pokémon's species last time
    u8 badges;    // badges last time
    u8 lastDay;   // low byte of the day last talked to
};

#define ALIVE_MEMORIES 96

// The living world's saved data (src/alive*.c), in SaveBlockExt.
struct AliveSave
{
    struct AliveMemory memories[ALIVE_MEMORIES];
    u32 donated[ALIVE_PROJECT_COUNT]; // offline: money the player gave each town project
    u8 projectsDone;                  // one bit each, finished offline or online
    u8 classic;                       // 1: classic random encounters
    u16 helped;                       // surprise events the player helped with
    u16 eventDay;                     // 1 + the day of the last surprise event
    u8 eventsToday;
    u8 findsToday;                    // dried-pond finds taken today
    u16 findDay;                      // 1 + the day of findsToday
    u16 newsSeen;                     // the last headline the crier shouted
    u16 giftDay;                      // 1 + the day of giftTowns
    u32 giftTowns;                    // town Pokémon that gave the player a find today, one bit each
    u32 reserved[16];
};

#endif // GUARD_ALIVE_SAVE_H
