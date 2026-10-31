#ifndef GUARD_BREED_FUSION_SAVE_H
#define GUARD_BREED_FUSION_SAVE_H

// Fusion colors the server built (src/breed_fusion.c), one per mother
// species + father species pair the player has had. When a pair has none
// yet (offline, or a slot was reused), the game builds the colors itself.
#define FUSION_RECIPE_COUNT   12
#define FUSION_RECIPE_COLORS  4

struct FusionRecipe
{
    u16 species;
    u16 partner;
    u8 count;   // colors used (0 = empty slot)
    u8 filler;
    u16 colors[FUSION_RECIPE_COLORS];      // for the baby's color families, biggest first
    u16 shinyColors[FUSION_RECIPE_COLORS];
};

struct BreedFusionSaveData
{
    struct FusionRecipe recipes[FUSION_RECIPE_COUNT];
    u8 nextSlot;
    u8 filler[3];
};

#endif // GUARD_BREED_FUSION_SAVE_H
