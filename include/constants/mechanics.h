#ifndef GUARD_CONSTANTS_MECHANICS_H
#define GUARD_CONSTANTS_MECHANICS_H

// Battle and Pokémon mechanics (src/mechanics*.c, docs/mechanics.md).

// People and Pokémon the mechanics put on maps from C (190-199; other
// threads use 200 and up). One per entry of sMechNpcs in src/mechanics.c.
#define LOCALID_MECH_FIRST 190
#define LOCALID_MECH_TUTOR 190 // the move tutor in every Pokémon Center
#define LOCALID_MECH_REFEREE 191 // the rules referee in the Slateport Battle Tent

// Battle rule sets (Mechanics_SetBattleRules). The Slateport Battle Tent's
// referee offers them against Battle Frontier trainers; online PvP can use
// them too, if both games set the same one before the link battle.
#define RULESET_NONE        0
#define RULESET_INVERSE     1 // the type chart is turned around
#define RULESET_TRICK_ROOM  2 // Trick Room for the whole battle
#define RULESET_NO_ITEMS    3 // Magic Room for the whole battle, and no Bag
#define RULESET_WONDER_ROOM 4 // Wonder Room for the whole battle
#define RULESET_MISTY       5 // Misty Terrain for the whole battle
#define RULESET_COUNT       5

// Flags (FLAG_UNUSED_0x881-0x887 and 0x8E5-0x8EF belong to the mechanics).
#define FLAG_MECH_MET_TUTOR    FLAG_UNUSED_0x881
#define FLAG_MECH_RULE_INVERSE FLAG_UNUSED_0x882 // B_FLAG_INVERSE_BATTLE
#define FLAG_MECH_MET_REFEREE  FLAG_UNUSED_0x883

#endif // GUARD_CONSTANTS_MECHANICS_H
