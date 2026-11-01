#ifndef GUARD_SAVE_EXT_H
#define GUARD_SAVE_EXT_H

// Extended save (extended GBA v3, emulator/README.md): 1 MiB of battery-backed RAM
// next to the 128 KiB flash save. gSaveBlockExt is written there on every save and read
// back on load, alongside the normal save blocks.
//
// To save something new, add a field to struct SaveBlockExt. Saves made before the field
// existed load it as zero, so put new fields at the end and treat zero as "not set".
// Never remove or reorder fields; bump SAVE_EXT_VERSION if old data needs migrating.

#define SAVE_EXT_VERSION 1
#define SAVE_EXT_SLOT_SIZE 0x80000 // two slots, so a save cut short keeps the previous one
#define SAVE_EXT_HEADER_SIZE 32
#define SAVE_EXT_MAX_SIZE (SAVE_EXT_SLOT_SIZE - SAVE_EXT_HEADER_SIZE)

#include "breed_fusion_save.h"
#include "reserve_save.h"
#include "pokemon_storage_system.h"
#include "research.h"
#include "solmara.h"

// The towns' quest boards when playing offline (src/rovers.c): today's
// rampaging Pokémon the player cleared and the reward not yet collected.
#define ROVER_OFFLINE_CLEARS 6
struct RoverOfflineClear
{
    u16 species;
    u8 level;
    u8 mapGroup;
    u8 mapNum;
    u8 filler[3];
};

struct RoverOfflineBoard
{
    u16 day;     // 1 + the day these clears are for (0: none yet)
    u8 count;
    u8 filler;
    u32 owed;    // money to collect at a board
    struct RoverOfflineClear clears[ROVER_OFFLINE_CLEARS];
};

// Maps the player has been to (src/hoenn_multi.c): one bit per map.
#define HOENN_MULTI_MAP_GROUPS 128
#define HOENN_MULTI_MAPS_PER_GROUP 256

struct SaveBlockExt
{
    u32 version;
    u32 reserved[7];
    // New saved data goes below.
    struct ReserveLocalMon reserveMoreMons[RESERVE_LOCAL_MAX - RESERVE_LOCAL_SAVED]; // Haven Reserve drop-offs 7-40
    struct FusionRecipe fusionMoreRecipes[FUSION_RECIPE_TOTAL - FUSION_RECIPE_COUNT]; // fusion colors 13-252
    struct PokemonStorageMore pcMoreBoxes; // PC boxes 15-32
    struct RoverOfflineBoard roverBoard; // offline quest boards
    struct ResearchSave research; // research Pokédex (src/research.c)
    u8 visitedMaps[HOENN_MULTI_MAP_GROUPS][HOENN_MULTI_MAPS_PER_GROUP / 8]; // friend travel (src/hoenn_multi.c)
    struct SolmaraSaveData solmara; // Solmara, the second region (docs/solmara.md)
};

extern struct SaveBlockExt gSaveBlockExt;

bool32 SaveExt_IsAvailable(void);
void SaveExt_Clear(void);
void SaveExt_Write(void);
void SaveExt_Load(void);

#endif // GUARD_SAVE_EXT_H
