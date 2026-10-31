#include "global.h"
#include "life.h"
#include "money.h"
#include "pokemon.h"
#include "rovers.h"

// Entry points the overworld calls for the fifth batch of features
// (docs/life.md), and the showcase settings the screenshot driver uses
// (tools/life_shots/).

// The screenshot driver writes here; the next overworld frame applies it.
EWRAM_DATA struct LifeShowcase gLifeShowcase = {0};

static void RunShowcase(void)
{
    gLifeShowcase.pending = FALSE;
    if (gLifeShowcase.friendship != 0)
    {
        u32 friendship = gLifeShowcase.friendship;
        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_FRIENDSHIP, &friendship);
    }
    if (gLifeShowcase.eclipseStep != 0)
    {
        LifeEclipse_ApplyShowcase(gLifeShowcase.eclipseStep - 1);
        gLifeShowcase.eclipseStep = 0;
    }
    if (gLifeShowcase.moneyThousands != 0)
    {
        SetMoney(&gSaveBlock1Ptr->money, gLifeShowcase.moneyThousands * 1000);
        gLifeShowcase.moneyThousands = 0;
    }
    if (gLifeShowcase.finishJobs)
    {
        LifeJobs_FinishAll();
        gLifeShowcase.finishJobs = FALSE;
    }
}

bool32 Life_OverworldUpdate(void)
{
    if (gLifeShowcase.pending)
        RunShowcase();
    Seasons_Update();
    LifePhoto_Update();
    LifeGhost_Update();
    return Rovers_OverworldUpdate();
}

void Life_OnMapTransition(void)
{
    Seasons_OnMapTransition();
    LifePhoto_OnMapTransition();
    LifeEclipse_UpdateObjects();
    LifeGhost_OnMapTransition();
    Rovers_OnMapTransition();
}

enum Species Life_ModifyWildSpecies(enum Species species, u8 level)
{
    return Seasons_ModifyWildSpecies(species, level);
}
