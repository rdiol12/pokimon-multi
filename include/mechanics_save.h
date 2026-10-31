#ifndef GUARD_MECHANICS_SAVE_H
#define GUARD_MECHANICS_SAVE_H

// Save data of the battle and Pokémon mechanics, at the end of SaveBlock3.
struct MechanicsSaveData
{
    u8 rulesWon; // bit per RULESET_* beaten at the Slateport Battle Tent
    u8 cupsWon;  // bit per CUP_* won at the Verdanturf Battle Tent
    u8 filler[2];
};

#endif // GUARD_MECHANICS_SAVE_H
