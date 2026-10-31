#include "global.h"
#include "load_save.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save_ext.h"
#include "string_util.h"
#include "test/test.h"
#include "constants/characters.h"

TEST("PC boxes 15-32 are kept in the extended save")
{
    struct Pokemon mon;
    u32 i;

    ResetPokemonStorageSystem();
    SaveExt_Clear();
    CreateMon(&mon, SPECIES_ZIGZAGOON, 7, 0x1234, OTID_STRUCT_PLAYER_ID);
    SetBoxMonAt(0, 0, &mon.box);
    CreateMon(&mon, SPECIES_RALTS, 9, 0x5678, OTID_STRUCT_PLAYER_ID);
    SetBoxMonAt(TOTAL_BOXES_COUNT - 1, IN_BOX_COUNT - 1, &mon.box);
    StringCopy(GetBoxNamePtr(20), COMPOUND_STRING("FAVES"));
    gPokemonStoragePtr->currentBox = 20;
    CopyPartyAndObjectsToSave();

    EXPECT_EQ(gPokemonStorageSave.currentBox, 0);
    EXPECT_EQ(GetBoxMonData(&gPokemonStorageSave.boxes[0][0], MON_DATA_SPECIES), SPECIES_ZIGZAGOON);
    EXPECT_EQ(GetBoxMonData(&gSaveBlockExt.pcMoreBoxes.boxes[MORE_BOXES_COUNT - 1][IN_BOX_COUNT - 1], MON_DATA_SPECIES), SPECIES_RALTS);

    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
        ZeroBoxMonAt(i, 0);
    ZeroBoxMonAt(TOTAL_BOXES_COUNT - 1, IN_BOX_COUNT - 1);
    gPokemonStoragePtr->currentBox = 0;
    CopyPartyAndObjectsFromSave();
    LoadMorePcBoxes();
    EXPECT_EQ(GetBoxMonDataAt(0, 0, MON_DATA_SPECIES), SPECIES_ZIGZAGOON);
    EXPECT_EQ(GetBoxMonDataAt(TOTAL_BOXES_COUNT - 1, IN_BOX_COUNT - 1, MON_DATA_SPECIES), SPECIES_RALTS);
    EXPECT_EQ(gPokemonStoragePtr->currentBox, 20);
    EXPECT_EQ(GetBoxNamePtr(20)[0], CHAR_F);
}

TEST("PC boxes 15-32 start empty with default names for an older save")
{
    struct Pokemon mon;

    ResetPokemonStorageSystem();
    CreateMon(&mon, SPECIES_RALTS, 9, 0x5678, OTID_STRUCT_PLAYER_ID);
    SetBoxMonAt(20, 0, &mon.box);
    StringCopy(GetBoxNamePtr(20), COMPOUND_STRING("JUNK"));
    SaveExt_Clear();
    LoadMorePcBoxes();
    EXPECT_EQ(GetBoxMonDataAt(20, 0, MON_DATA_SPECIES), SPECIES_NONE);
    EXPECT_EQ(GetBoxNamePtr(20)[0], CHAR_B);
}
