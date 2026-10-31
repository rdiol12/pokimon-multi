#ifndef GUARD_RESERVE_SAVE_H
#define GUARD_RESERVE_SAVE_H

#include "constants/reserve.h"

// A Pokémon left at the Haven Reserve while offline: enough to make it
// again (same species, level, nature, gender, shininess and IVs).
struct ReserveLocalMon
{
    u32 personality;
    u32 ivs;     // 5 bits each: HP, Atk, Def, Speed, SpAtk, SpDef
    u16 species;
    u8 level;
    u8 unused;
};

// The Haven Reserve's save data (src/reserve.c), in SaveBlock3.
struct ReserveSaveData
{
    struct ReserveLocalMon mons[RESERVE_LOCAL_MAX];
    u16 strayDay;   // the day the strays below are for
    u8 straysGone;  // strays caught or knocked out that day, one bit each
    u8 count;       // Pokémon in mons[]
    u16 left;       // Pokémon left at the reserve, ever
    u16 caught;     // Pokémon caught there, ever
};

#endif // GUARD_RESERVE_SAVE_H
