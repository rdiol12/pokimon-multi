#ifndef GUARD_CONSTANTS_MECHANICS_H
#define GUARD_CONSTANTS_MECHANICS_H

// Battle and Pokémon mechanics (src/mechanics*.c, docs/mechanics.md).

// People and Pokémon the mechanics put on maps from C (190-199; other
// threads use 200 and up). One per entry of sMechNpcs in src/mechanics.c.
#define LOCALID_MECH_FIRST 190
#define LOCALID_MECH_TUTOR 190 // the move tutor in every Pokémon Center

// Flags (FLAG_UNUSED_0x881-0x887 and 0x8E5-0x8EF belong to the mechanics).
#define FLAG_MECH_MET_TUTOR FLAG_UNUSED_0x881

#endif // GUARD_CONSTANTS_MECHANICS_H
