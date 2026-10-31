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

struct SaveBlockExt
{
    u32 version;
    u32 reserved[7];
    // New saved data goes below.
    struct ReserveLocalMon reserveMoreMons[RESERVE_LOCAL_MAX - RESERVE_LOCAL_SAVED]; // Haven Reserve drop-offs 7-40
    struct FusionRecipe fusionMoreRecipes[FUSION_RECIPE_TOTAL - FUSION_RECIPE_COUNT]; // fusion colors 13-252
    struct PokemonStorageMore pcMoreBoxes; // PC boxes 15-32
};

extern struct SaveBlockExt gSaveBlockExt;

bool32 SaveExt_IsAvailable(void);
void SaveExt_Clear(void);
void SaveExt_Write(void);
void SaveExt_Load(void);

#endif // GUARD_SAVE_EXT_H
