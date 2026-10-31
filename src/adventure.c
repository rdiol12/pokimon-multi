#include "global.h"
#include "adventure.h"
#include "battle.h"
#include "item.h"
#include "script.h"
#include "daily_quest.h"
#include "dungeon.h"
#include "ranch.h"
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
    u16 ranchSpecies[RANCH_SHOWN]; // put in the RANCH box
    u8 ranchHoursAgo;              // the last ranch visit, hours ago
    u8 dungeonFloor;               // the dungeon floor the next dive shows
    u8 dungeonLadderX, dungeonLadderY; // out: where that floor's ladder is
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
    if (gAdventureShowcase.ranchSpecies[0] != SPECIES_NONE || gAdventureShowcase.ranchHoursAgo != 0)
        Ranch_Showcase(gAdventureShowcase.ranchSpecies, RANCH_SHOWN, gAdventureShowcase.ranchHoursAgo);
    memset(gAdventureShowcase.ranchSpecies, 0, sizeof(gAdventureShowcase.ranchSpecies));
    gAdventureShowcase.ranchHoursAgo = 0;
    if (gAdventureShowcase.dungeonFloor != 0)
        Dungeon_Showcase(gAdventureShowcase.dungeonFloor, &gAdventureShowcase.dungeonLadderX, &gAdventureShowcase.dungeonLadderY);
    gAdventureShowcase.dungeonFloor = 0;
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
