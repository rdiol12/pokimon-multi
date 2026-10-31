#ifndef GUARD_CONSTANTS_MECHANICS_H
#define GUARD_CONSTANTS_MECHANICS_H

// Battle and Pokémon mechanics (src/mechanics*.c, docs/mechanics.md).

// People and Pokémon the mechanics put on maps from C (190-199; other
// threads use 200 and up). One per entry of sMechNpcs in src/mechanics.c.
#define LOCALID_MECH_FIRST 190
#define LOCALID_MECH_TUTOR 190 // the move tutor in every Pokémon Center
#define LOCALID_MECH_REFEREE 191 // the rules referee in the Slateport Battle Tent
#define LOCALID_MECH_CUP_HOST 192 // the tournament host in the Verdanturf Battle Tent
#define LOCALID_MECH_STYLE_MASTER 193 // the battle style master in the Fallarbor Battle Tent

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

// Level-capped tournaments (src/mechanics_cups.c).
#define CUP_LITTLE 0 // level 5, Pokémon that haven't evolved yet
#define CUP_RISING 1 // level 30
#define CUP_PRIME  2 // level 50
#define CUP_COUNT  3

// Rare color variants (src/mechanics_colors.c).
#define COLOR_NORMAL        0
#define COLOR_AURORA        1
#define COLOR_DUSK          2
#define COLOR_MIDNIGHT      3
#define COLOR_VARIANT_COUNT 3

// Pokémon personalities (src/mechanics_personality.c).
#define PERSONA_PLAYFUL 0
#define PERSONA_SHY     1
#define PERSONA_PROUD   2
#define PERSONA_LAZY    3
#define PERSONA_CURIOUS 4
#define PERSONA_BRAVE   5
#define PERSONA_GENTLE  6
#define PERSONA_GRUMPY  7
#define PERSONA_COUNT   8

// Battle styles (src/mechanics_styles.c). The player's choice travels with
// the chosen move in the controller's return value, in these bits (move
// index is bits 0-1, RET_GIMMICK bit 7).
#define STYLE_NONE   0
#define STYLE_AGILE  1 // weaker, but goes first among moves of its priority
#define STYLE_STRONG 2 // stronger, but goes last among moves of its priority
#define STYLE_RET_SHIFT 5
#define STYLE_RET_MASK  (3 << STYLE_RET_SHIFT)

// Messages printed with STRINGID_MECH_MESSAGE; the id goes in
// gBattleTextBuff3[0] so it reaches both games in a link battle.
#define MECH_MSG_STYLE_AGILE  0
#define MECH_MSG_STYLE_STRONG 1

// Flags (FLAG_UNUSED_0x881-0x887 and 0x8E5-0x8EF belong to the mechanics).
#define FLAG_MECH_MET_TUTOR    FLAG_UNUSED_0x881
#define FLAG_MECH_RULE_INVERSE FLAG_UNUSED_0x882 // B_FLAG_INVERSE_BATTLE
#define FLAG_MECH_MET_REFEREE  FLAG_UNUSED_0x883
#define FLAG_MECH_MET_CUP_HOST FLAG_UNUSED_0x884
#define FLAG_MECH_BATTLE_STYLES FLAG_UNUSED_0x885 // learned from the style master

#endif // GUARD_CONSTANTS_MECHANICS_H
