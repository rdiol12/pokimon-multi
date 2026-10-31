#include "global.h"
#include "crafting.h"
#include "event_data.h"
#include "item.h"
#include "item_icon.h"
#include "main.h"
#include "menu.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "trainer_level.h"
#include "window.h"
#include "constants/items.h"
#include "constants/songs.h"

// Crafting: turn things found around Hoenn into Poké Balls, medicine and
// other items. The crafter outside the Devon Corporation in Rustboro shows
// how (and sells Apricorns); after that R > ADVENTURE > CRAFTING opens the
// recipe list anywhere. A crafts one, holding A keeps crafting.
//
// Apricorn Balls follow Kurt's recipes from Gold and Silver; the
// evolution stones take 4 Shards like the Route 124 treasure hunter's
// trades. The rest use Berries and Mushrooms the way their in-game
// descriptions suggest.

#define MAX_INGREDIENTS 3
#define XP_CRAFT 3

struct Ingredient
{
    u16 item;
    u8 count;
};

struct Recipe
{
    u16 item;
    u8 count;
    struct Ingredient needs[MAX_INGREDIENTS];
};

static const struct Recipe sRecipes[] =
{
    // Apricorn Balls.
    {ITEM_LEVEL_BALL,   1, {{ITEM_RED_APRICORN, 2}}},
    {ITEM_LURE_BALL,    1, {{ITEM_BLUE_APRICORN, 2}}},
    {ITEM_MOON_BALL,    1, {{ITEM_YELLOW_APRICORN, 2}}},
    {ITEM_FRIEND_BALL,  1, {{ITEM_GREEN_APRICORN, 2}}},
    {ITEM_LOVE_BALL,    1, {{ITEM_PINK_APRICORN, 2}}},
    {ITEM_FAST_BALL,    1, {{ITEM_WHITE_APRICORN, 2}}},
    {ITEM_HEAVY_BALL,   1, {{ITEM_BLACK_APRICORN, 2}}},
    // Better Balls from plain ones.
    {ITEM_POKE_BALL,    2, {{ITEM_RED_APRICORN, 1}, {ITEM_WHITE_APRICORN, 1}}},
    {ITEM_GREAT_BALL,   1, {{ITEM_POKE_BALL, 2}, {ITEM_BLUE_APRICORN, 1}}},
    {ITEM_ULTRA_BALL,   1, {{ITEM_GREAT_BALL, 2}, {ITEM_BLACK_APRICORN, 1}, {ITEM_YELLOW_APRICORN, 1}}},
    // Medicine.
    {ITEM_POTION,       2, {{ITEM_ORAN_BERRY, 2}}},
    {ITEM_SUPER_POTION, 1, {{ITEM_ORAN_BERRY, 2}, {ITEM_SITRUS_BERRY, 1}}},
    {ITEM_FULL_HEAL,    1, {{ITEM_PECHA_BERRY, 1}, {ITEM_CHESTO_BERRY, 1}, {ITEM_RAWST_BERRY, 1}}},
    {ITEM_ETHER,        1, {{ITEM_LEPPA_BERRY, 2}}},
    {ITEM_REVIVE,       1, {{ITEM_TINY_MUSHROOM, 2}, {ITEM_SITRUS_BERRY, 1}}},
    {ITEM_MAX_REVIVE,   1, {{ITEM_REVIVE, 2}, {ITEM_BIG_MUSHROOM, 1}}},
    // Field items.
    {ITEM_REPEL,        2, {{ITEM_TINY_MUSHROOM, 1}}},
    {ITEM_SUPER_REPEL,  1, {{ITEM_REPEL, 2}, {ITEM_TINY_MUSHROOM, 1}}},
    {ITEM_HONEY,        1, {{ITEM_PECHA_BERRY, 3}}},
    // Evolution stones, like the Route 124 treasure hunter.
    {ITEM_FIRE_STONE,    1, {{ITEM_RED_SHARD, 4}}},
    {ITEM_WATER_STONE,   1, {{ITEM_BLUE_SHARD, 4}}},
    {ITEM_THUNDER_STONE, 1, {{ITEM_YELLOW_SHARD, 4}}},
    {ITEM_LEAF_STONE,    1, {{ITEM_GREEN_SHARD, 4}}},
    {ITEM_STAR_PIECE,    1, {{ITEM_STARDUST, 4}}},
};

#define RECIPE_COUNT ARRAY_COUNT(sRecipes)
#define ROWS         7
#define LINE_HEIGHT  16
#define LIST_TOP     16
#define PANEL_X      112 // the selected recipe, right of the list
#define TAG_ICON     0x4352 // 'CR'

static const struct WindowTemplate sListWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

struct Crafting
{
    u8 windowId;
    u8 iconSprite;
    u8 cursor;
    u8 scroll;
    u8 holdFrames;
    bool8 dirty;
    u8 message[64];
};

static EWRAM_DATA struct Crafting sCraft = {0};

static void Task_Crafting(u8 taskId);

static bool32 CanCraft(const struct Recipe *recipe)
{
    u32 i;

    for (i = 0; i < MAX_INGREDIENTS && recipe->needs[i].item != ITEM_NONE; i++)
    {
        if (!CheckBagHasItem(recipe->needs[i].item, recipe->needs[i].count))
            return FALSE;
    }
    return CheckBagHasSpace(recipe->item, recipe->count);
}

static void DrawIcon(void)
{
    if (sCraft.iconSprite != MAX_SPRITES)
    {
        DestroySpriteAndFreeResources(&gSprites[sCraft.iconSprite]);
        sCraft.iconSprite = MAX_SPRITES;
    }
    sCraft.iconSprite = AddItemIconSprite(TAG_ICON, TAG_ICON, sRecipes[sCraft.cursor].item);
    if (sCraft.iconSprite != MAX_SPRITES)
    {
        // Window tile (1, 1) is pixel (8, 8); the icon is 32x32, centred.
        gSprites[sCraft.iconSprite].x = 8 + PANEL_X + 16;
        gSprites[sCraft.iconSprite].y = 8 + 16 + 16;
        gSprites[sCraft.iconSprite].oam.priority = 0;
    }
}

static void Draw(void)
{
    const struct Recipe *recipe = &sRecipes[sCraft.cursor];
    u8 line[64];
    u32 i, y;

    FillWindowPixelBuffer(sCraft.windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(sCraft.windowId, FONT_NORMAL, COMPOUND_STRING("CRAFTING"), 4, 0, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < ROWS && sCraft.scroll + i < RECIPE_COUNT; i++)
    {
        const struct Recipe *row = &sRecipes[sCraft.scroll + i];
        y = LIST_TOP + i * LINE_HEIGHT;
        // Recipes missing an ingredient are greyed out.
        StringCopy(line, CanCraft(row) ? gText_EmptyString2 : COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
        CopyItemName(row->item, line + StringLength(line));
        if (row->count > 1)
        {
            StringAppend(line, COMPOUND_STRING(" x"));
            ConvertIntToDecimalStringN(line + StringLength(line), row->count, STR_CONV_MODE_LEFT_ALIGN, 1);
        }
        if (sCraft.scroll + i == sCraft.cursor)
            AddTextPrinterParameterized(sCraft.windowId, FONT_NORMAL, gText_SelectorArrow2, 0, y, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(sCraft.windowId, FONT_NORMAL, line, 10, y, TEXT_SKIP_DRAW, NULL);
    }

    // The selected recipe, on the right: what it needs and what the bag has.
    y = LIST_TOP + ROWS * LINE_HEIGHT + 2;
    StringCopy(line, COMPOUND_STRING("In BAG: "));
    ConvertIntToDecimalStringN(line + StringLength(line), min(CountTotalItemQuantityInBag(recipe->item), 999), STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized(sCraft.windowId, FONT_SMALL, line, PANEL_X + 36, 26, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sCraft.windowId, FONT_SMALL, COMPOUND_STRING("NEEDS"), PANEL_X, 54, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < MAX_INGREDIENTS && recipe->needs[i].item != ITEM_NONE; i++)
    {
        bool32 enough = CheckBagHasItem(recipe->needs[i].item, recipe->needs[i].count);
        StringCopy(line, enough ? gText_EmptyString2 : COMPOUND_STRING("{COLOR RED}{SHADOW LIGHT_RED}"));
        ConvertIntToDecimalStringN(line + StringLength(line), recipe->needs[i].count, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(line, COMPOUND_STRING(" "));
        CopyItemName(recipe->needs[i].item, line + StringLength(line));
        StringAppend(line, COMPOUND_STRING(" (have "));
        ConvertIntToDecimalStringN(line + StringLength(line), min(CountTotalItemQuantityInBag(recipe->needs[i].item), 999), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(line, COMPOUND_STRING(")"));
        AddTextPrinterParameterized(sCraft.windowId, FONT_SMALL, line, PANEL_X, 66 + i * 12, TEXT_SKIP_DRAW, NULL);
    }

    if (sCraft.message[0] != EOS)
        AddTextPrinterParameterized(sCraft.windowId, FONT_SMALL, sCraft.message, 4, y, TEXT_SKIP_DRAW, NULL);
    else
        AddTextPrinterParameterized(sCraft.windowId, FONT_SMALL, COMPOUND_STRING("A: craft (hold to keep going)   B: close"), 4, y, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sCraft.windowId, COPYWIN_FULL);
}

void Crafting_ShowList(void)
{
    sCraft.cursor = 0;
    sCraft.scroll = 0;
    sCraft.holdFrames = 0;
    sCraft.message[0] = EOS;
    sCraft.iconSprite = MAX_SPRITES;
    sCraft.windowId = AddWindow(&sListWindowTemplate);
    DrawStdWindowFrame(sCraft.windowId, FALSE);
    PutWindowTilemap(sCraft.windowId);
    Draw();
    DrawIcon();
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Crafting, 80);
}

static void Craft(void)
{
    const struct Recipe *recipe = &sRecipes[sCraft.cursor];
    u32 i;

    if (!CanCraft(recipe))
    {
        PlaySE(SE_BOO);
        StringCopy(sCraft.message, COMPOUND_STRING("{COLOR RED}{SHADOW LIGHT_RED}Not enough to make that."));
        return;
    }
    for (i = 0; i < MAX_INGREDIENTS && recipe->needs[i].item != ITEM_NONE; i++)
        RemoveBagItem(recipe->needs[i].item, recipe->needs[i].count);
    AddBagItem(recipe->item, recipe->count);
    gSaveBlock3Ptr->adventure.itemsCrafted++;
    TrainerLevel_AddXp(XP_CRAFT);
    PlaySE(SE_SHOP);
    StringCopy(sCraft.message, COMPOUND_STRING("Made "));
    if (recipe->count > 1)
    {
        ConvertIntToDecimalStringN(sCraft.message + StringLength(sCraft.message), recipe->count, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(sCraft.message, COMPOUND_STRING(" "));
        CopyItemNameHandlePlural(recipe->item, sCraft.message + StringLength(sCraft.message), recipe->count);
    }
    else
    {
        StringAppend(sCraft.message, COMPOUND_STRING("a "));
        CopyItemName(recipe->item, sCraft.message + StringLength(sCraft.message));
    }
    StringAppend(sCraft.message, COMPOUND_STRING("!"));
}

static void Close(u8 taskId)
{
    PlaySE(SE_SELECT);
    if (sCraft.iconSprite != MAX_SPRITES)
        DestroySpriteAndFreeResources(&gSprites[sCraft.iconSprite]);
    ClearStdWindowAndFrameToTransparent(sCraft.windowId, FALSE);
    ClearWindowTilemap(sCraft.windowId);
    RemoveWindow(sCraft.windowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Crafting(u8 taskId)
{
    u8 old = sCraft.cursor;

    if (JOY_NEW(B_BUTTON))
    {
        Close(taskId);
        return;
    }
    // A crafts once; held down, it crafts again every few frames.
    if (JOY_NEW(A_BUTTON) || (JOY_HELD(A_BUTTON) && ++sCraft.holdFrames > 30 && sCraft.holdFrames % 8 == 0))
    {
        Craft();
        sCraft.dirty = TRUE;
    }
    if (!JOY_HELD(A_BUTTON))
        sCraft.holdFrames = 0;
    if (JOY_REPEAT(DPAD_DOWN) && sCraft.cursor + 1 < RECIPE_COUNT)
        sCraft.cursor++;
    else if (JOY_REPEAT(DPAD_UP) && sCraft.cursor > 0)
        sCraft.cursor--;
    if (sCraft.cursor != old)
    {
        if (sCraft.cursor < sCraft.scroll)
            sCraft.scroll = sCraft.cursor;
        else if (sCraft.cursor >= sCraft.scroll + ROWS)
            sCraft.scroll = sCraft.cursor - ROWS + 1;
        PlaySE(SE_SELECT);
        sCraft.message[0] = EOS;
        DrawIcon();
        sCraft.dirty = TRUE;
    }
    if (sCraft.dirty)
    {
        sCraft.dirty = FALSE;
        Draw();
    }
}
