#ifndef GUARD_CONSTANTS_ADVENTURE_H
#define GUARD_CONSTANTS_ADVENTURE_H

// Fourth-batch game features (docs/adventure.md): trainer levels, daily
// quests, crafting, ranching, dungeons, treasure hunts, the online Battle
// Frontier and the Hall of Fame wall.

// R > ADVENTURE submenu (data/scripts/adventure/menu.inc).
#define ADVENTURE_MENU_TRAINER_LEVEL 0
#define ADVENTURE_MENU_OUTFITS       1
#define ADVENTURE_MENU_CLOSE         15

// Trainer levels (src/trainer_level.c).
#define TRAINER_LEVEL_MAX 30

// Outfits: 0 is the style's own clothes.
#define OUTFIT_NONE      0
#define OUTFIT_AUTUMN    1
#define OUTFIT_STARLIGHT 2
#define OUTFIT_ACE       3
#define OUTFIT_SHADOW    4
#define OUTFIT_CHAMPION  5
#define OUTFIT_COUNT     6
#define OUTFIT_STATES    7 // normal, Mach Bike, Acro Bike, surfing, field move, fishing, watering

// Perks, each unlocked at a trainer level.
#define PERK_PRIZE_MONEY  0 // +10% prize money from trainers
#define PERK_DISCOUNT     1 // 10% off in Poké Marts
#define PERK_HATCHING     2 // Eggs hatch twice as fast
#define PERK_PRIZE_MONEY2 3 // +25% prize money instead
#define PERK_SHINY        4 // one more shiny roll for wild Pokémon
#define PERK_COUNT        5

#endif // GUARD_CONSTANTS_ADVENTURE_H
