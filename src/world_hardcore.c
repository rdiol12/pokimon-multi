#include "global.h"
#include "achievements.h"
#include "battle.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "world.h"
#include "constants/battle.h"

// Hardcore mode: the classic Nuzlocke rules, picked once from the Challenge
// Clerk in Oldale Town before the first badge and never turned off.
//
// - Only the first wild Pokémon you meet in each area (region map section)
//   can be caught. Shiny Pokémon can always be caught.
// - A Pokémon that faints is gone: after the battle it leaves the party.
// - If the whole party faints, the run is over (the save carries on as a
//   normal game, marked as a failed run).
//
// Runs are ranked on the server's hardcore ladder (badges, then fewest
// losses, then fastest), see netplay_world.c.

extern const u8 World_EventScript_HardcoreLosses[];
extern const u8 World_EventScript_HardcoreOver[];

static EWRAM_DATA u8 sLostThisBattle = 0;
static EWRAM_DATA bool8 sRunJustEnded = FALSE;

bool32 Hardcore_IsOn(void)
{
    return gSaveBlock3Ptr->world.hardcore == HARDCORE_ON;
}

static bool32 IsAreaUsed(u32 mapsec)
{
    if (mapsec / 8 >= WORLD_NUZLOCKE_AREA_BYTES)
        return FALSE;
    return (gSaveBlock3Ptr->world.nuzlockeAreas[mapsec / 8] >> (mapsec % 8)) & 1;
}

static void MarkAreaUsed(u32 mapsec)
{
    if (mapsec / 8 < WORLD_NUZLOCKE_AREA_BYTES)
        gSaveBlock3Ptr->world.nuzlockeAreas[mapsec / 8] |= 1 << (mapsec % 8);
}

bool32 Hardcore_IsCatchBlocked(void)
{
    if (!Hardcore_IsOn() || (gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return FALSE;
    if (GetMonData(&gEnemyParty[0], MON_DATA_IS_SHINY))
        return FALSE;
    return IsAreaUsed(gMapHeader.regionMapSectionId);
}

static void RemoveFaintedMons(void)
{
    u32 i;

    sLostThisBattle = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_HP) == 0)
        {
            if (sLostThisBattle == 0)
            {
                GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
                StringGet_Nickname(gStringVar1);
            }
            ZeroMonData(mon);
            sLostThisBattle++;
            if (gSaveBlock3Ptr->world.hardcoreDeaths < 255)
                gSaveBlock3Ptr->world.hardcoreDeaths++;
        }
    }
    if (sLostThisBattle != 0)
        CompactPartySlots();
}

static bool32 WasPlayerDefeated(void)
{
    return gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW || gBattleOutcome == B_OUTCOME_FORFEITED;
}

// Called after every wild and trainer battle.
void Hardcore_OnBattleEnd(bool32 wild)
{
    if (!Hardcore_IsOn())
        return;
    // Link battles (online PvP) use a copy of the party; they never cost lives.
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        return;
    if (wild)
        MarkAreaUsed(gMapHeader.regionMapSectionId);
    if (WasPlayerDefeated())
    {
        gSaveBlock3Ptr->world.hardcore = HARDCORE_FAILED;
        sRunJustEnded = TRUE;
        World_OnHardcoreProgress();
        return;
    }
    RemoveFaintedMons();
}

// Called every overworld frame: tells the player what the last battle cost.
bool32 Hardcore_Update(void)
{
    if (ArePlayerFieldControlsLocked())
        return FALSE;
    if (sRunJustEnded)
    {
        sRunJustEnded = FALSE;
        ScriptContext_SetupScript(World_EventScript_HardcoreOver);
        return TRUE;
    }
    if (sLostThisBattle != 0)
    {
        ConvertIntToDecimalStringN(gStringVar2, sLostThisBattle - 1, STR_CONV_MODE_LEFT_ALIGN, 1);
        gSpecialVar_0x8004 = sLostThisBattle;
        sLostThisBattle = 0;
        ScriptContext_SetupScript(World_EventScript_HardcoreLosses);
        return TRUE;
    }
    return FALSE;
}

// Script: VAR_RESULT = TRUE if hardcore mode can still be turned on here.
void Hardcore_CanStart(void)
{
    gSpecialVar_Result = gSaveBlock3Ptr->world.hardcore == HARDCORE_OFF && !FlagGet(FLAG_BADGE01_GET);
}

void Hardcore_Start(void)
{
    RtcCalcLocalTime();
    gSaveBlock3Ptr->world.hardcore = HARDCORE_ON;
    gSaveBlock3Ptr->world.hardcoreDeaths = 0;
    gSaveBlock3Ptr->world.hardcoreStartDay = gLocalTime.days;
    memset(gSaveBlock3Ptr->world.nuzlockeAreas, 0, sizeof(gSaveBlock3Ptr->world.nuzlockeAreas));
    World_OnHardcoreProgress();
}

// Script: STR_VAR_1 = losses, VAR_RESULT = hardcore state.
void Hardcore_GetStatus(void)
{
    ConvertIntToDecimalStringN(gStringVar1, gSaveBlock3Ptr->world.hardcoreDeaths, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = gSaveBlock3Ptr->world.hardcore;
}
