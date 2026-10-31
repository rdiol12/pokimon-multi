#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "malloc.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "script_menu.h"
#include "string_util.h"
#include "world.h"
#include "constants/battle.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/pokemon.h"

// Pokémon cafe: CAFÉ SOLEIL in Lilycove. Its chef cooks a dish from Berries
// and drinks you bring; eating it raises a stat of your lead Pokémon (and
// its partner in doubles) by one stage at the start of each of your next few
// battles against wild Pokémon and trainers (not online or Frontier ones).
// R > PASTIMES > CAFE shows the dish still in effect and the recipes.

extern const u8 Cafe_EventScript_Chef[];

struct Ingredient
{
    u16 item;
    u8 count;
};

struct Dish
{
    const u8 *name;
    const u8 *taste;  // what the chef says it does
    struct Ingredient ingredients[3];
    u8 stats;         // a bit per STAT_*
    u8 battles;
};

static const struct Dish sDishes[CAFE_DISH_COUNT] =
{
    [1] =
    {
        .name = COMPOUND_STRING("SPICY CURRY"),
        .taste = COMPOUND_STRING("Fiery! ATTACK +1"),
        .ingredients = {{ITEM_CHERI_BERRY, 2}, {ITEM_ORAN_BERRY, 1}},
        .stats = 1 << STAT_ATK,
        .battles = 5,
    },
    [2] =
    {
        .name = COMPOUND_STRING("SOUR SALAD"),
        .taste = COMPOUND_STRING("Crisp! DEFENSE +1"),
        .ingredients = {{ITEM_ASPEAR_BERRY, 2}, {ITEM_ORAN_BERRY, 1}},
        .stats = 1 << STAT_DEF,
        .battles = 5,
    },
    [3] =
    {
        .name = COMPOUND_STRING("DRY RICE BALL"),
        .taste = COMPOUND_STRING("Hearty! SP. ATK +1"),
        .ingredients = {{ITEM_CHESTO_BERRY, 2}, {ITEM_ORAN_BERRY, 1}},
        .stats = 1 << STAT_SPATK,
        .battles = 5,
    },
    [4] =
    {
        .name = COMPOUND_STRING("BITTER SOUP"),
        .taste = COMPOUND_STRING("Warming! SP. DEF +1"),
        .ingredients = {{ITEM_RAWST_BERRY, 2}, {ITEM_ORAN_BERRY, 1}},
        .stats = 1 << STAT_SPDEF,
        .battles = 5,
    },
    [5] =
    {
        .name = COMPOUND_STRING("SWEET PANCAKES"),
        .taste = COMPOUND_STRING("Fluffy! SPEED +1"),
        .ingredients = {{ITEM_PECHA_BERRY, 2}, {ITEM_MOOMOO_MILK, 1}},
        .stats = 1 << STAT_SPEED,
        .battles = 5,
    },
    [6] =
    {
        .name = COMPOUND_STRING("RAINBOW FEAST"),
        .taste = COMPOUND_STRING("A feast! All stats +1"),
        .ingredients = {{ITEM_SITRUS_BERRY, 1}, {ITEM_LUM_BERRY, 1}, {ITEM_MOOMOO_MILK, 1}},
        .stats = (1 << STAT_ATK) | (1 << STAT_DEF) | (1 << STAT_SPEED) | (1 << STAT_SPATK) | (1 << STAT_SPDEF),
        .battles = 3,
    },
};

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gCafeSpots[1] =
{
    {MAP_LILYCOVE_CITY, 38, 27, OBJ_EVENT_GFX_COOK, MOVEMENT_TYPE_FACE_DOWN, 0, Cafe_EventScript_Chef},
};

bool32 Cafe_HasIngredients(u32 dish)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sDishes[dish].ingredients) && sDishes[dish].ingredients[i].count != 0; i++)
    {
        if (!CheckBagHasItem(sDishes[dish].ingredients[i].item, sDishes[dish].ingredients[i].count))
            return FALSE;
    }
    return TRUE;
}

// inMessage: breaks the line before a third ingredient.
static u8 *AppendRecipe(u8 *dest, u32 dish, bool32 inMessage)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sDishes[dish].ingredients) && sDishes[dish].ingredients[i].count != 0; i++)
    {
        const struct Ingredient *ing = &sDishes[dish].ingredients[i];
        if (i != 0)
            dest = StringCopy(dest, (inMessage && i == 2) ? COMPOUND_STRING(",\n") : COMPOUND_STRING(", "));
        dest = ConvertIntToDecimalStringN(dest, ing->count, STR_CONV_MODE_LEFT_ALIGN, 1);
        *dest++ = CHAR_SPACE;
        CopyItemName(ing->item, dest);
        dest += StringLength(dest);
    }
    return dest;
}

// Queues the dish's stat boosts for the player's battlers, like a totem's
// aura. Called as a battle is set up (Pastime_OnBattleStart).
void Cafe_OnBattleStart(void)
{
    const struct Dish *dish;
    u32 stat, i;
    static const u8 sPlayerBattlers[] = {B_POSITION_PLAYER_LEFT, B_POSITION_PLAYER_RIGHT};

    if (sSave.cafeBattles == 0 || sSave.cafeDish == CAFE_DISH_NONE || sSave.cafeDish >= CAFE_DISH_COUNT)
        return;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_SAFARI))
        return;
    dish = &sDishes[sSave.cafeDish];
    for (i = 0; i < ARRAY_COUNT(sPlayerBattlers); i++)
    {
        for (stat = STAT_ATK; stat < NUM_BATTLE_STATS; stat++)
        {
            if (dish->stats & (1 << stat))
            {
                gQueuedStatBoosts[sPlayerBattlers[i]].stats |= 1 << stat;
                gQueuedStatBoosts[sPlayerBattlers[i]].statChanges[stat] = 1;
            }
        }
    }
    sSave.cafeBattles--;
}

// The menu frees each name when it closes, so they are copied to the heap.
static void PushMenuItem(const u8 *name, u32 id)
{
    struct ListMenuItem item;
    u8 *buffer = Alloc(StringLength(name) + 1);

    StringCopy(buffer, name);
    item.name = buffer;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

// Script: pushes the dishes onto the dynmultichoice stack.
void Cafe_PushMenu(void)
{
    u32 dish;

    for (dish = 1; dish < CAFE_DISH_COUNT; dish++)
        PushMenuItem(sDishes[dish].name, dish);
    PushMenuItem(COMPOUND_STRING("CANCEL"), CAFE_MENU_CANCEL);
}

// Script: VAR_0x8005 = the dish picked. gPastimeText = the recipe;
// VAR_RESULT = TRUE if the player has what it needs.
void Cafe_CheckDish(void)
{
    u32 dish = gSpecialVar_0x8005;
    u8 *p = gPastimeText;

    p = StringCopy(p, sDishes[dish].name);
    p = StringCopy(p, COMPOUND_STRING("!\n"));
    p = StringCopy(p, sDishes[dish].taste);
    p = StringCopy(p, COMPOUND_STRING(", "));
    p = ConvertIntToDecimalStringN(p, sDishes[dish].battles, STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, COMPOUND_STRING(" battles.\pIt takes:\n"));
    p = AppendRecipe(p, dish, TRUE);
    StringCopy(p, COMPOUND_STRING("."));
    gSpecialVar_Result = Cafe_HasIngredients(dish);
    StringCopy(gStringVar1, sDishes[dish].name);
}

void Cafe_UseIngredients(u32 dish)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sDishes[dish].ingredients) && sDishes[dish].ingredients[i].count != 0; i++)
        RemoveBagItem(sDishes[dish].ingredients[i].item, sDishes[dish].ingredients[i].count);
}

const u8 *Cafe_GetDishName(u32 dish)
{
    return sDishes[dish].name;
}

// Script: cooks and eats the dish (VAR_0x8005).
void Cafe_Cook(void)
{
    u32 dish = gSpecialVar_0x8005;

    Cafe_UseIngredients(dish);
    sSave.cafeDish = dish;
    sSave.cafeBattles = sDishes[dish].battles;
    if (sSave.cafeDishesCooked < 0xFFFF)
        sSave.cafeDishesCooked++;
    StringCopy(gStringVar1, sDishes[dish].name);
    StringCopy(gStringVar2, sDishes[dish].taste);
}

// R > PASTIMES > CAFE -----------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index == 0)
    {
        if (sSave.cafeBattles == 0 || sSave.cafeDish == CAFE_DISH_NONE)
        {
            StringCopy(dest, COMPOUND_STRING("No dish in effect"));
            return;
        }
        dest = StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}"));
        dest = StringCopy(dest, sDishes[sSave.cafeDish].name);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 120}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.cafeBattles, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringCopy(dest, COMPOUND_STRING(" battles left"));
        return;
    }
    if (!Cafe_HasIngredients(index))
        dest = StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
    dest = StringCopy(dest, sDishes[index].name);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 96}"));
    StringCopy(dest, sDishes[index].taste);
}

static void GetDetail(u16 index, u8 *dest)
{
    if (index == 0)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Dishes cooked: "));
        ConvertIntToDecimalStringN(dest, sSave.cafeDishesCooked, STR_CONV_MODE_LEFT_ALIGN, 5);
        return;
    }
    AppendRecipe(dest, index, FALSE);
}

void Cafe_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("CAFÉ SOLEIL"), CAFE_DISH_COUNT, GetLine, GetDetail);
}
