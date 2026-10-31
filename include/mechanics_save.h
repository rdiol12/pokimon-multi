#ifndef GUARD_MECHANICS_SAVE_H
#define GUARD_MECHANICS_SAVE_H

// Save data of the battle and Pokémon mechanics, at the end of SaveBlock3.
struct MechanicsSaveData
{
    u8 rulesWon; // bit per RULESET_* beaten at the Slateport Battle Tent
    u8 filler[3];
};

#endif // GUARD_MECHANICS_SAVE_H
