#ifndef GUARD_CONSTANTS_ALIVE_H
#define GUARD_CONSTANTS_ALIVE_H

// The living world (src/alive*.c, docs/alive.md).

// Object local ids, spawned in C. Maps keep their own object events at 51
// or below.
#define LOCALID_ALIVE_FIRST      52
#define LOCALID_ALIVE_TOWN_MON   52 // + 0-3: the town's resident Pokémon
#define LOCALID_ALIVE_CRIER      56 // the town crier (living news)
#define LOCALID_ALIVE_FOREMAN    57 // the town project's foreman
#define LOCALID_ALIVE_COMMUTER   58 // + 0-1: townsfolk heading out or home
#define LOCALID_ALIVE_CRITTER    60 // + 0-3: birds and other critters
#define LOCALID_ALIVE_INCIDENT   64 // + 0-3: a surprise event's people and Pokémon
#define LOCALID_ALIVE_WEATHER    68 // a find in a dried-up pond
#define LOCALID_ALIVE_LOST_CHILD 69 // a lost child following the player
#define LOCALID_ALIVE_LAST       69

#define ALIVE_TOWN_MONS  4
#define ALIVE_CRITTERS   4
#define ALIVE_INCIDENT_OBJECTS 4

// Town projects (src/alive_projects.c, server/alive.py PROJECTS).
#define ALIVE_PROJECT_OLDALE_GARDEN     0
#define ALIVE_PROJECT_PETALBURG_PARK    1
#define ALIVE_PROJECT_RUSTBORO_STATUE   2
#define ALIVE_PROJECT_SLATEPORT_STALLS  3
#define ALIVE_PROJECT_MAUVILLE_PLAZA    4
#define ALIVE_PROJECT_FORTREE_BRIDGE    5
#define ALIVE_PROJECT_LILYCOVE_PROMENADE 6
#define ALIVE_PROJECT_DEWFORD_PIER      7
#define ALIVE_PROJECT_COUNT             8

// Surprise events (src/alive_events.c).
#define ALIVE_INCIDENT_NONE      0
#define ALIVE_INCIDENT_STAMPEDE  1
#define ALIVE_INCIDENT_LOST_CHILD 2
#define ALIVE_INCIDENT_STUCK_MON 3
#define ALIVE_INCIDENT_CRATE     4
#define ALIVE_INCIDENT_HURT_MON  5
#define ALIVE_INCIDENT_COUNT     6

#endif // GUARD_CONSTANTS_ALIVE_H
