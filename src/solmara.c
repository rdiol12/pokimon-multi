#include "global.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "save_ext.h"
#include "solmara.h"

// Solmara, the second region (docs/solmara.md): its badges, and trainers
// that grow with them so the gyms can be taken in any order.

bool32 Solmara_IsTrainerId(u16 trainerId)
{
    return trainerId >= SOLMARA_TRAINER(0) && trainerId < SOLMARA_TRAINER(SOLMARA_TRAINERS_COUNT);
}

u32 Solmara_CountBadges(void)
{
#ifdef FLAG_SOLMARA_BADGE_1
    static const u16 sBadges[] = {
        FLAG_SOLMARA_BADGE_1, FLAG_SOLMARA_BADGE_2, FLAG_SOLMARA_BADGE_3, FLAG_SOLMARA_BADGE_4,
        FLAG_SOLMARA_BADGE_5, FLAG_SOLMARA_BADGE_6, FLAG_SOLMARA_BADGE_7, FLAG_SOLMARA_BADGE_8,
    };
    u32 i, count = 0;

    for (i = 0; i < ARRAY_COUNT(sBadges); i++)
        if (FlagGet(sBadges[i]))
            count++;
    return count;
#else
    return 0;
#endif
}

// Script: VAR_RESULT = how many Solmara badges the player has.
void Solmara_GetBadgeCount(void)
{
    gSpecialVar_Result = Solmara_CountBadges();
}

// Raises a Solmara trainer's Pokémon by SOLMARA_LEVELS_PER_BADGE levels per
// Solmara badge the player holds. Their moves stay as written.
void Solmara_ScaleTrainerParty(struct Pokemon *party, u16 trainerId)
{
    u32 i, bonus;

    if (!Solmara_IsTrainerId(trainerId))
        return;
    bonus = Solmara_CountBadges() * SOLMARA_LEVELS_PER_BADGE;
    if (bonus == 0)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &party[i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 level;

        if (species == SPECIES_NONE)
            continue;
        level = GetMonData(mon, MON_DATA_LEVEL) + bonus;
        if (level > MAX_LEVEL)
            level = MAX_LEVEL;
        SetMonData(mon, MON_DATA_EXP, &gExperienceTables[gSpeciesInfo[species].growthRate][level]);
        CalculateMonStats(mon);
    }
}

// Script: the first time the player becomes Solmara's champion, remembers
// when (frames played); counts every League clear.
void Solmara_RecordLeagueClear(void)
{
    if (gSaveBlockExt.solmara.championTime == 0)
        gSaveBlockExt.solmara.championTime = gSaveBlock2Ptr->playTimeHours * 216000u
                                           + gSaveBlock2Ptr->playTimeMinutes * 3600u
                                           + gSaveBlock2Ptr->playTimeSeconds * 60u + 1;
    if (gSaveBlockExt.solmara.leagueClears < 0xFFFF)
        gSaveBlockExt.solmara.leagueClears++;
    gSpecialVar_Result = gSaveBlockExt.solmara.leagueClears;
}
