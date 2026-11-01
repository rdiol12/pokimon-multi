#ifndef GUARD_CONSTANTS_ROVERS_H
#define GUARD_CONSTANTS_ROVERS_H

// Roaming trainers (src/rovers.c, docs/rovers.md).

#define ROVER_COUNT       150
#define ROVER_OT_ID_BASE  70000 // trainer ids of rovers (server/rovers.py TID_BASE)
#define ROVERS_MAX_HERE   10 // was 4 on the GBA's 16 objects; the extended GBA has 64

#define ROVER_STYLE_COLLECTOR 0 // catches a lot, swaps team members
#define ROVER_STYLE_TRAINER   1 // trains hard, keeps a small team
#define ROVER_STYLE_BALANCED  2
#define ROVER_STYLE_BREEDER   3 // likes the Day Care

// Object local ids, spawned in C (no map.json edits). 126-129 aren't
// rovers' (127 is the camera).
#define LOCALID_ROVER_FIRST     120
#define LOCALID_ROVER_WILD      120 // a wild Pokémon a rover is fighting
#define LOCALID_ROVER_BALL      121 // a thrown Poké Ball
#define LOCALID_ROVER_FOE_MON   122 // a game trainer's Pokémon in a fight with a rover
#define LOCALID_ROVER_CHARGER   123 // a wild Pokémon out of the grass
#define LOCALID_ROVER_MAJOR     124 // the quest board's rampaging Pokémon on this map
#define LOCALID_ROVER_BOARD     125 // the quest board in a town
#define LOCALID_ROVER_MISC_LAST 125
#define LOCALID_ROVER_TRAINER   130 // + actor 0-9
#define LOCALID_ROVER_MON       (LOCALID_ROVER_TRAINER + ROVERS_MAX_HERE) // + actor 0-9: the Pokémon walking with it
#define LOCALID_ROVER_LAST      (LOCALID_ROVER_MON + ROVERS_MAX_HERE - 1) // 149

#endif // GUARD_CONSTANTS_ROVERS_H
