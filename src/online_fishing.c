#include "global.h"
#include "battle.h"
#include "online.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "task.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "netplay_ext.h"
#include "world.h"

// Fishing tournaments and co-op fishing (server/online_fishing.py,
// server/online_coopfish.py). While one is on, the
// server's status says so ("S|FISHT|1|minutes left|rare|rare|rare|id") and a
// third of the Pokémon hooked are one of its three rare species, at the
// level of the Pokémon they replace. Landed Pokémon score by size, rare ones
// more (the FISH report in src/online.c).

#define RARE_CHANCE 3 // one bite in this many

bool32 Online_TournamentIsOn(void)
{
    return Online_StatusNum("FISHT", 0) == 1;
}

// A fishing encounter's Pokémon was just made: maybe swap in a rare one.
void Online_TournamentBite(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u16 species;
    u8 level;

    if (!Online_TournamentIsOn() || Random() % RARE_CHANCE != 0)
        return;
    species = Online_StatusNum("FISHT", 2 + Random() % 3);
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    level = GetMonData(mon, MON_DATA_LEVEL);
    CreateMonWithIVs(mon, species, level, GetMonPersonality(species, MON_GENDER_RANDOM, NATURE_RANDOM, RANDOM_UNOWN_LETTER),
        OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
}

// Co-op fishing (server/online_coopfish.py) -------------------------------
//
// Another player's invitation comes in the status ("S|COFISH|1|name|id");
// it's shown once per invitation, when the player is free.

#define REEL_ROUNDS 3
#define REEL_ZONE   2
#define REEL_SPEED  3

extern const u8 Online_EventScript_CoFishInvite[];

static EWRAM_DATA s32 sAnsweredInvite = 0;

bool32 Online_TryCoFishInvite(void)
{
    s32 id;

    if (Online_StatusNum("COFISH", 0) != 1)
        return FALSE;
    id = Online_StatusNum("COFISH", 2);
    if (id == sAnsweredInvite)
        return FALSE;
    sAnsweredInvite = id;
    Online_CopyText(gStringVar1, Online_Status("COFISH", 1), PLAYER_NAME_LENGTH);
    ScriptContext_SetupScript(Online_EventScript_CoFishInvite);
    return TRUE;
}

// VAR_RESULT: TRUE if the player faces water they could fish in.
void Online_CheckFacingWater(void)
{
    gSpecialVar_Result = IsPlayerFacingSurfableFishableWater();
}

// Plays the reeling meter. VAR_0x8005: the hits (sent with the REEL request).
static void Task_Reel(u8 taskId)
{
    s32 hits = TimingMeter_GetResult();

    if (hits < 0)
        return;
    gSpecialVar_0x8005 = hits;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

void Online_PlayReel(void)
{
    TimingMeter_Start(REEL_ROUNDS, REEL_ZONE, REEL_SPEED);
    CreateTask(Task_Reel, 80);
}
