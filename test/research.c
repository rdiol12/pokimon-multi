#include "global.h"
#include "battle.h"
#include "item.h"
#include "money.h"
#include "research.h"
#include "save_ext.h"
#include "test/test.h"

TEST("Research Pokédex: tasks raise the level, level 10 completes the species and pays")
{
    u32 i;

    SaveExt_Clear();
    ClearBag();
    SetMoney(&gSaveBlock1Ptr->money, 0);
    gBattleResults.caughtMonSpecies = SPECIES_ZIGZAGOON;
    for (i = 0; i < 7; i++)
        Research_OnCatch();
    EXPECT_EQ(Research_GetLevel(SPECIES_ZIGZAGOON), 4); // caught 1, 2, 4 and 7
    for (i = 0; i < 15; i++)
        Research_OnDefeat(SPECIES_ZIGZAGOON);
    EXPECT_EQ(Research_GetLevel(SPECIES_ZIGZAGOON), 9); // defeated 1, 3, 6, 10 and 15
    EXPECT_EQ(gSaveBlockExt.research.completed, 0);
    Research_OnEvolve(SPECIES_ZIGZAGOON);
    EXPECT_EQ(Research_GetLevel(SPECIES_ZIGZAGOON), 10);
    EXPECT_EQ(gSaveBlockExt.research.completed, 1);
    EXPECT_EQ(gSaveBlockExt.research.rank, 1);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 50 * (1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10) + 1000);
    EXPECT(CheckBagHasItem(ITEM_ULTRA_BALL, 3));
    EXPECT(CheckBagHasItem(ITEM_GREAT_BALL, 5));

    // Wild Zigzagoon get extra shiny rolls now; others don't.
    Research_OnWildSpecies(SPECIES_ZIGZAGOON);
    EXPECT_EQ(Research_ShinyRolls(), 2);
    EXPECT_EQ(Research_ShinyRolls(), 0);
    Research_OnWildSpecies(SPECIES_POOCHYENA);
    EXPECT_EQ(Research_ShinyRolls(), 0);

    // A form shares its species' research.
    EXPECT_EQ(Research_GetLevel(SPECIES_ZIGZAGOON_GALAR), 10);
}
