#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "money.h"
#include "palette.h"
#include "pastime.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "overworld.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// New Game+: once the player is CHAMPION, R > PASTIMES > NEW GAME+ starts
// the story over from the moving truck, keeping:
//  - every Pokémon (the PC boxes, and the party goes into them),
//  - the money, the Pokédex and the Trainer ID (so the Pokémon stay the
//    player's own),
//  - the pastimes' records and the world features' achievements and
//    records (struct PastimeSaveData, struct WorldSaveData).
// The first New Game+ also gives a Shiny Charm and an Exp. Charm. Nothing
// is written until the player saves, so the old save is there until then.

struct NewGamePlusCarry
{
    struct PokemonStorage storage;
    struct Pokemon party[PARTY_SIZE];
    u8 partyCount;
    u32 money;
    u8 trainerId[TRAINER_ID_LENGTH];
    u8 dexSeen[NUM_DEX_FLAG_BYTES];
    u8 dexCaught[NUM_DEX_FLAG_BYTES];
    struct PastimeSaveData pastime;
    struct WorldSaveData world;
};

static EWRAM_DATA struct NewGamePlusCarry *sCarry = NULL;

// Script: VAR_RESULT = TRUE if the player may start a New Game+;
// gStringVar1 = New Game+ played so far.
void NewGamePlus_GetState(void)
{
    gSpecialVar_Result = FlagGet(FLAG_SYS_GAME_CLEAR);
    ConvertIntToDecimalStringN(gStringVar1, gSaveBlock3Ptr->pastime.newGamePlusCount, STR_CONV_MODE_LEFT_ALIGN, 3);
}

static void Task_StartNewGamePlus(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    DestroyTask(taskId);
    SetMainCallback2(CB2_NewGame);
}

// Script: `callnative NewGamePlus_Start, waitstate=1` (it doesn't come back).
void NewGamePlus_Start(void)
{
    sCarry = AllocZeroed(sizeof(*sCarry));
    if (sCarry == NULL)
    {
        ScriptContext_Enable();
        return;
    }
    sCarry->storage = *gPokemonStoragePtr;
    memcpy(sCarry->party, gParties[B_TRAINER_PLAYER], sizeof(sCarry->party));
    sCarry->partyCount = gPartiesCount[B_TRAINER_PLAYER];
    sCarry->money = GetMoney(&gSaveBlock1Ptr->money);
    memcpy(sCarry->trainerId, gSaveBlock2Ptr->playerTrainerId, TRAINER_ID_LENGTH);
    memcpy(sCarry->dexSeen, gSaveBlock1Ptr->dexSeen, NUM_DEX_FLAG_BYTES);
    memcpy(sCarry->dexCaught, gSaveBlock1Ptr->dexCaught, NUM_DEX_FLAG_BYTES);
    sCarry->pastime = gSaveBlock3Ptr->pastime;
    sCarry->world = gSaveBlock3Ptr->world;
    PlaySE(SE_WARP_IN);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITE);
    CreateTask(Task_StartNewGamePlus, 80);
}

// CB2_NewGame, just after NewGameInitData: puts back what New Game+ keeps.
void NewGamePlus_AfterInit(void)
{
    u32 i;

    if (sCarry == NULL)
        return;
    *gPokemonStoragePtr = sCarry->storage;
    SetMoney(&gSaveBlock1Ptr->money, sCarry->money);
    memcpy(gSaveBlock2Ptr->playerTrainerId, sCarry->trainerId, TRAINER_ID_LENGTH);
    memcpy(gSaveBlock1Ptr->dexSeen, sCarry->dexSeen, NUM_DEX_FLAG_BYTES);
    memcpy(gSaveBlock1Ptr->dexCaught, sCarry->dexCaught, NUM_DEX_FLAG_BYTES);
    gSaveBlock3Ptr->pastime = sCarry->pastime;
    gSaveBlock3Ptr->world = sCarry->world;
    for (i = 0; i < sCarry->partyCount; i++)
        CopyMonToPC(&sCarry->party[i]);
    if (gSaveBlock3Ptr->pastime.newGamePlusCount == 0)
    {
        AddBagItem(ITEM_SHINY_CHARM, 1);
        AddBagItem(ITEM_EXP_CHARM, 1);
    }
    if (gSaveBlock3Ptr->pastime.newGamePlusCount < 0xFF)
        gSaveBlock3Ptr->pastime.newGamePlusCount++;
    Free(sCarry);
    sCarry = NULL;
}
