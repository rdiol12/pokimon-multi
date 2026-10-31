#ifndef GUARD_ROVERS_H
#define GUARD_ROVERS_H

#include "constants/rovers.h"

// Roaming trainers (src/rovers.c, docs/rovers.md).

struct RoverCast
{
    u8 name[PLAYER_NAME_LENGTH + 1];
    u8 look;
    u8 favType;
    u8 style;
    u16 starter;
};

struct RoverLook
{
    u16 gfx;
    u16 pic;
    u8 trainerClass;
    u8 female;
    u8 music;
};

extern const struct RoverCast gRoverCast[ROVER_COUNT];

// Screenshot showcase settings, written by tools/rovers/shots.
struct RoverShowcase
{
    u8 pending;
    u8 scene;     // ROVER_SCENE_*
    u8 ids[ROVERS_MAX_HERE];
    u8 count;
    u8 level;     // their Pokémon's level
    u8 teamSize;
    u8 offline;   // 1: don't ask the server
    u8 talk;      // actor + 1: talk to it after a moment
    u8 levelB;    // if not 0, the second and later rovers' level
    u8 bond;      // ROVER_BOND_* for the first rover (rival, friend)
};
extern struct RoverShowcase gRoverShowcase;

#define ROVER_SCENE_WANDER   1
#define ROVER_SCENE_DUEL     2 // two rovers fighting
#define ROVER_SCENE_TRAINER  3 // a rover fighting the map's own trainer
#define ROVER_SCENE_WILD     4 // a rover meeting a wild Pokémon (then catch or knock out)
#define ROVER_SCENE_CHARGER  5 // a wild Pokémon charges the player
#define ROVER_SCENE_MAJOR    6 // the map's rampaging Pokémon
#define ROVER_SCENE_CATCH    7 // like WILD, always throws a ball and catches it

struct Trainer;
struct ObjectEvent;
bool32 Rovers_OverworldUpdate(void);
void Rovers_OnMapTransition(void);
const u8 *Rovers_GetTalkScript(struct ObjectEvent *objectEvent);
bool32 Rovers_IsRoverObject(struct ObjectEvent *objectEvent);
bool32 Rovers_IsWatching(void);
bool32 Rovers_IsTrading(void);
const u8 *Rovers_TradeName(void);
u16 Rovers_AiVsAiTrainerPic(void);
bool32 Rovers_InjuresBattler(u32 battler);
void Rovers_OnLifeBattleEnd(const struct Trainer *trainer, bool32 playerWon);

#endif // GUARD_ROVERS_H
