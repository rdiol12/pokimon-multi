#ifndef GUARD_CONSTANTS_ROVERS_H
#define GUARD_CONSTANTS_ROVERS_H

// Roaming trainers (src/rovers.c, docs/rovers.md).

#define ROVER_COUNT       150
#define ROVERS_MAX_HERE   4

#define ROVER_STYLE_COLLECTOR 0 // catches a lot, swaps team members
#define ROVER_STYLE_TRAINER   1 // trains hard, keeps a small team
#define ROVER_STYLE_BALANCED  2
#define ROVER_STYLE_BREEDER   3 // likes the Day Care

// Object local ids, spawned in C (no map.json edits).
#define LOCALID_ROVER_FIRST     130
#define LOCALID_ROVER_TRAINER   130 // + actor 0-3
#define LOCALID_ROVER_MON       134 // + actor 0-3: the Pokémon walking with it
#define LOCALID_ROVER_WILD      138 // a wild Pokémon a rover is fighting
#define LOCALID_ROVER_BALL      139 // a thrown Poké Ball
#define LOCALID_ROVER_FOE_MON   140 // a game trainer's Pokémon in a fight with a rover
#define LOCALID_ROVER_CHARGER   141 // a wild Pokémon out of the grass
#define LOCALID_ROVER_MAJOR     142 // the quest board's rampaging Pokémon on this map
#define LOCALID_ROVER_BOARD     143 // the quest board in a town
#define LOCALID_ROVER_LAST      149

#endif // GUARD_CONSTANTS_ROVERS_H
