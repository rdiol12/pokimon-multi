#include "global.h"
#include "life.h"
#include "pokemon.h"

// Entry points the overworld calls for the fifth batch of features
// (docs/life.md), and the showcase settings the screenshot driver uses
// (tools/life_shots/).

// The screenshot driver writes here; the next overworld frame applies it.
struct LifeShowcase
{
    u8 pending;
    u8 friendship; // the lead's friendship, if not 0
};

EWRAM_DATA struct LifeShowcase gLifeShowcase = {0};

static void RunShowcase(void)
{
    gLifeShowcase.pending = FALSE;
    if (gLifeShowcase.friendship != 0)
    {
        u32 friendship = gLifeShowcase.friendship;
        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_FRIENDSHIP, &friendship);
    }
}

bool32 Life_OverworldUpdate(void)
{
    if (gLifeShowcase.pending)
        RunShowcase();
    return FALSE;
}

void Life_OnMapTransition(void)
{
}
