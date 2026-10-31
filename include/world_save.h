#ifndef GUARD_WORLD_SAVE_H
#define GUARD_WORLD_SAVE_H

#include "constants/world.h"

// Saved state of the world and gameplay features (docs/world.md). Lives in
// SaveBlock3, so it costs no space in the vanilla save blocks.

#define WORLD_NUZLOCKE_AREA_BYTES 32 // one bit per region map section
#define WORLD_HOUSE_SLOTS         16
#define WORLD_LEGEND_QUESTS       8

// hardcore: HARDCORE_OFF, HARDCORE_ON or HARDCORE_FAILED (the party was
// wiped out; the run is over), from constants/world.h.

struct WorldSaveData
{
    u32 achievements; // bit per enum Achievement
    // Hardcore (Nuzlocke) mode.
    u8 hardcore;
    u8 hardcoreDeaths;
    u16 hardcoreStartDay;
    u8 nuzlockeAreas[WORLD_NUZLOCKE_AREA_BYTES];
    // Fishing records.
    u16 bestFishSpecies;
    u16 bestFishSize;      // in millimetres
    u16 fishLanded;
    // Berry farming.
    u16 berriesHarvested;
    u16 bestBerryHarvest;  // most berries from one plot
    // Player house on Starfall Isle: furniture id per slot (0 = empty).
    u8 houseDecor[WORLD_HOUSE_SLOTS];
    // Legendary quest chains: the step each one has reached.
    u8 legendStage[WORLD_LEGEND_QUESTS];
    // Shiny hunting: the current chain.
    u16 chainSpecies;
    u16 chainCount;
    u16 photosTaken;
    u16 huntsEntered;
};

#endif // GUARD_WORLD_SAVE_H
