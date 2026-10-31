#include "global.h"
#include "adventure.h"
#include "battle.h"
#include "item.h"
#include "script.h"
#include "daily_quest.h"
#include "trainer_level.h"

// Entry points for the fourth-batch game features (docs/adventure.md),
// called from the world features' overworld and battle hooks (src/world.c).

// The screenshot driver (tools/adventure_shots/) writes a scene here; the
// next overworld frame applies it.
struct AdventureShowcase
{
    u8 pending;
    u8 outfit;   // OUTFIT_* + 1, 0 = leave as is
    u8 filler[2];
    u32 setXp;   // trainer XP + 1, 0 = leave as is
    u16 items[8]; // 10 of each go in the bag
};

EWRAM_DATA struct AdventureShowcase gAdventureShowcase = {0};

static void RunShowcase(void)
{
    u32 i;

    gAdventureShowcase.pending = FALSE;
    for (i = 0; i < ARRAY_COUNT(gAdventureShowcase.items); i++)
    {
        if (gAdventureShowcase.items[i] != ITEM_NONE)
            AddBagItem(gAdventureShowcase.items[i], 10);
        gAdventureShowcase.items[i] = ITEM_NONE;
    }
    if (gAdventureShowcase.setXp != 0)
        TrainerLevel_SetXp(gAdventureShowcase.setXp - 1);
    if (gAdventureShowcase.outfit != 0)
        TrainerLevel_SetOutfit(gAdventureShowcase.outfit - 1);
    gAdventureShowcase.setXp = 0;
    gAdventureShowcase.outfit = 0;
}

// Called once per overworld frame.
void Adventure_Update(void)
{
    if (gAdventureShowcase.pending && !ArePlayerFieldControlsLocked())
        RunShowcase();
    TrainerLevel_Update();
    DailyQuest_Update();
}

// Called when the player caught a wild Pokémon.
void Adventure_OnCatch(u16 species)
{
    TrainerLevel_OnCatch();
    DailyQuest_OnCatch(species);
}

// Back from a trainer battle.
void Adventure_OnTrainerBattleEnd(void)
{
    if (gBattleOutcome == B_OUTCOME_WON)
    {
        TrainerLevel_OnTrainerWin();
        DailyQuest_OnTrainerWin();
    }
}
