#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "malloc.h"
#include "money.h"
#include "netplay.h"
#include "party_menu.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "pokemon.h"
#include "rtc.h"
#include "script_menu.h"
#include "sprite.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Pokémon fashion: the stylist at the Pokémon Fan Club in Slateport sells
// FASHION_ACCESSORIES accessories and dresses the lead Pokémon in one; it
// wears it while following the player. Each day she has a theme (cute,
// cool, natural, royal, fun): a lead Pokémon wearing an accessory of the
// day's theme wins her daily prize. R > PASTIMES > FASHION lists the
// accessories.

extern const u8 Fashion_EventScript_Stylist[];

enum
{
    THEME_CUTE,
    THEME_COOL,
    THEME_NATURAL,
    THEME_ROYAL,
    THEME_FUN,
    THEME_COUNT,
};

static const u8 *const sThemeNames[THEME_COUNT] =
{
    COMPOUND_STRING("CUTE"), COMPOUND_STRING("COOL"), COMPOUND_STRING("NATURAL"), COMPOUND_STRING("ROYAL"), COMPOUND_STRING("FUN"),
};

struct Accessory
{
    const u8 *name;
    u16 price;
    u8 theme;
    s8 y;           // from the follower's middle, in pixels
    bool8 front;    // only seen from the front (glasses, a collar)
};

static const struct Accessory sAccessories[FASHION_ACCESSORIES] =
{
    {COMPOUND_STRING("RED BOW"),      1000, THEME_CUTE,     -3, FALSE},
    {COMPOUND_STRING("STRAW HAT"),    1500, THEME_NATURAL,  -4, FALSE},
    {COMPOUND_STRING("BLUE SCARF"),   1500, THEME_COOL,     11, TRUE},
    {COMPOUND_STRING("FLOWER CROWN"), 2000, THEME_NATURAL,  -3, FALSE},
    {COMPOUND_STRING("SUNGLASSES"),   2500, THEME_COOL,      4, TRUE},
    {COMPOUND_STRING("GOLD CROWN"),   8000, THEME_ROYAL,    -4, FALSE},
    {COMPOUND_STRING("BELL COLLAR"),  1200, THEME_CUTE,     11, TRUE},
    {COMPOUND_STRING("PARTY HAT"),     800, THEME_FUN,      -7, FALSE},
};

// The daily prize for a look that fits the theme.
static const u16 sPrizes[] = {ITEM_HEART_SCALE, ITEM_PP_UP, ITEM_STAR_PIECE, ITEM_BIG_PEARL, ITEM_RARE_CANDY};

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gFashionSpots[1] =
{
    {MAP_SLATEPORT_CITY_POKEMON_FAN_CLUB, 4, 3, OBJ_EVENT_GFX_BEAUTY, MOVEMENT_TYPE_FACE_DOWN, 0, Fashion_EventScript_Stylist},
};

static u32 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 TodaysTheme(void)
{
    return Today() % THEME_COUNT;
}

// The accessory the following Pokémon wears + 1, or 0.
u32 Fashion_WornNow(void)
{
    struct Pokemon *lead = GetFirstLiveMon();

    if (sSave.fashionWorn == 0 || lead == NULL || GetMonData(lead, MON_DATA_PERSONALITY) != sSave.fashionPersonality)
        return 0;
    return sSave.fashionWorn;
}

// The accessory on the follower ------------------------------------------------------------

#define TAG_ACCESSORY 0x5A77

static const u32 sAccessoryGfx[] = INCBIN_U32("graphics/pastime/accessories.4bpp");
static const u16 sAccessoryPal[] = INCBIN_U16("graphics/pastime/accessories.gbapal");
static const struct SpriteSheet sAccessorySheet = {sAccessoryGfx, sizeof(sAccessoryGfx), TAG_ACCESSORY};
static const struct SpritePalette sAccessoryPalette = {sAccessoryPal, TAG_ACCESSORY};

static const struct OamData sAccessoryOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 2,
};

static void SpriteCB_Accessory(struct Sprite *sprite);

static const struct SpriteTemplate sAccessoryTemplate =
{
    .tileTag = TAG_ACCESSORY,
    .paletteTag = TAG_ACCESSORY,
    .oam = &sAccessoryOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Accessory,
};

static EWRAM_DATA u8 sAccessorySpriteId = 0;
// Other players' followers' accessories (src/world_online.c), by netplay slot.
static EWRAM_DATA u8 sRemoteSpriteIds[NETPLAY_MAX_REMOTES] = {0};

#define TILES_PER_ACCESSORY 4

// Rides on a follower's sprite, a step in front of it.
static void PlaceAccessory(struct Sprite *sprite, struct ObjectEvent *follower, u32 worn)
{
    const struct Accessory *accessory;
    struct Sprite *mon;

    if (follower == NULL || worn == 0 || worn > FASHION_ACCESSORIES)
    {
        sprite->invisible = TRUE;
        return;
    }
    accessory = &sAccessories[worn - 1];
    mon = &gSprites[follower->spriteId];
    sprite->x = mon->x;
    sprite->y = mon->y + accessory->y;
    sprite->x2 = mon->x2;
    sprite->y2 = mon->y2;
    sprite->oam.priority = mon->oam.priority;
    sprite->subpriority = mon->subpriority > 0 ? mon->subpriority - 1 : 0;
    sprite->oam.tileNum = GetSpriteTileStartByTag(TAG_ACCESSORY) + (worn - 1) * TILES_PER_ACCESSORY;
    sprite->invisible = mon->invisible || follower->invisible
                     || (accessory->front && follower->facingDirection == DIR_NORTH);
}

static void SpriteCB_Accessory(struct Sprite *sprite)
{
    PlaceAccessory(sprite, GetFollowerObject(), Fashion_WornNow());
}

#define sSlot     data[0]
#define sWorn     data[1]
#define sObjectId data[2]

static void SpriteCB_RemoteAccessory(struct Sprite *sprite)
{
    struct ObjectEvent *follower = &gObjectEvents[sprite->sObjectId];

    PlaceAccessory(sprite, follower->active ? follower : NULL, sprite->sWorn);
}

static bool32 LoadAccessoryGfx(void)
{
    if (GetSpriteTileStartByTag(TAG_ACCESSORY) == 0xFFFF)
        LoadSpriteSheet(&sAccessorySheet);
    if (IndexOfSpritePaletteTag(TAG_ACCESSORY) == 0xFF)
        LoadSpritePalette(&sAccessoryPalette);
    return GetSpriteTileStartByTag(TAG_ACCESSORY) != 0xFFFF && IndexOfSpritePaletteTag(TAG_ACCESSORY) != 0xFF;
}

static bool32 IsRemoteSpriteUp(u32 slot)
{
    struct Sprite *sprite = &gSprites[sRemoteSpriteIds[slot]];

    return sprite->inUse && sprite->callback == SpriteCB_RemoteAccessory && sprite->sSlot == slot;
}

// Called every overworld frame for each other player online: puts what
// their follower (object followerObjectId, or OBJECT_EVENTS_COUNT for none)
// wears on it, worn being the accessory + 1 (0: nothing).
void Fashion_ShowRemote(u32 slot, u8 followerObjectId, u32 worn)
{
    struct Sprite *sprite;

    if (slot >= NETPLAY_MAX_REMOTES)
        return;
    if (followerObjectId == OBJECT_EVENTS_COUNT || worn == 0 || worn > FASHION_ACCESSORIES)
    {
        if (IsRemoteSpriteUp(slot))
            DestroySprite(&gSprites[sRemoteSpriteIds[slot]]);
        return;
    }
    if (!IsRemoteSpriteUp(slot))
    {
        u8 spriteId;

        if (!LoadAccessoryGfx())
            return;
        spriteId = CreateSprite(&sAccessoryTemplate, 0, 0, 0);
        if (spriteId == MAX_SPRITES)
            return;
        sRemoteSpriteIds[slot] = spriteId;
        gSprites[spriteId].callback = SpriteCB_RemoteAccessory;
        gSprites[spriteId].coordOffsetEnabled = TRUE;
        gSprites[spriteId].sSlot = slot;
    }
    sprite = &gSprites[sRemoteSpriteIds[slot]];
    sprite->sWorn = worn;
    sprite->sObjectId = followerObjectId;
    SpriteCB_RemoteAccessory(sprite);
}

#undef sSlot
#undef sWorn
#undef sObjectId

static bool32 IsAccessorySpriteUp(void)
{
    return gSprites[sAccessorySpriteId].inUse && gSprites[sAccessorySpriteId].callback == SpriteCB_Accessory;
}

// Pastime_Update: makes the accessory sprite when the follower wears one.
void Fashion_Update(void)
{
    if (Fashion_WornNow() == 0 || GetFollowerObject() == NULL || IsAccessorySpriteUp())
        return;
    if (!LoadAccessoryGfx())
        return;
    sAccessorySpriteId = CreateSprite(&sAccessoryTemplate, 0, 0, 0);
    if (sAccessorySpriteId == MAX_SPRITES)
        return;
    gSprites[sAccessorySpriteId].coordOffsetEnabled = TRUE;
    SpriteCB_Accessory(&gSprites[sAccessorySpriteId]);
}

// Scripts ------------------------------------------------------------------------------

static void PushMenuItem(const u8 *name, u32 id)
{
    struct ListMenuItem item;
    u8 *buffer = Alloc(StringLength(name) + 1);

    StringCopy(buffer, name);
    item.name = buffer;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

// Script: pushes the accessories not bought yet, with their prices.
void Fashion_PushBuyMenu(void)
{
    u8 text[32];
    u32 i;

    for (i = 0; i < FASHION_ACCESSORIES; i++)
    {
        u8 *p;
        if (sSave.fashionOwned & (1 << i))
            continue;
        p = StringCopy(text, sAccessories[i].name);
        p = StringCopy(p, COMPOUND_STRING("{CLEAR_TO 80}¥"));
        ConvertIntToDecimalStringN(p, sAccessories[i].price, STR_CONV_MODE_LEFT_ALIGN, 5);
        PushMenuItem(text, i);
    }
    PushMenuItem(COMPOUND_STRING("CANCEL"), FASHION_CANCEL);
}

// Script: buys accessory VAR_0x8005. gStringVar1 = its name; VAR_RESULT =
// FASHION_BUY_OK or FASHION_BUY_NO_MONEY.
void Fashion_Buy(void)
{
    const struct Accessory *accessory = &sAccessories[gSpecialVar_0x8005];

    StringCopy(gStringVar1, accessory->name);
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, accessory->price))
    {
        gSpecialVar_Result = FASHION_BUY_NO_MONEY;
        return;
    }
    RemoveMoney(&gSaveBlock1Ptr->money, accessory->price);
    sSave.fashionOwned |= 1 << gSpecialVar_0x8005;
    gSpecialVar_Result = FASHION_BUY_OK;
}

// Script: pushes the accessories bought, TAKE OFF and CANCEL.
void Fashion_PushDressMenu(void)
{
    u32 i;

    for (i = 0; i < FASHION_ACCESSORIES; i++)
    {
        if (sSave.fashionOwned & (1 << i))
            PushMenuItem(sAccessories[i].name, i);
    }
    PushMenuItem(COMPOUND_STRING("TAKE OFF"), FASHION_TAKE_OFF);
    PushMenuItem(COMPOUND_STRING("CANCEL"), FASHION_CANCEL);
}

// Script: dresses the lead Pokémon in accessory VAR_0x8005 (or takes it
// off, FASHION_TAKE_OFF). gStringVar1 = the Pokémon, gStringVar2 = the
// accessory.
void Fashion_Dress(void)
{
    struct Pokemon *lead = GetFirstLiveMon();

    if (lead == NULL)
        return;
    GetMonNickname(lead, gStringVar1);
    if (gSpecialVar_0x8005 == FASHION_TAKE_OFF)
    {
        sSave.fashionWorn = 0;
        return;
    }
    StringCopy(gStringVar2, sAccessories[gSpecialVar_0x8005].name);
    sSave.fashionWorn = gSpecialVar_0x8005 + 1;
    sSave.fashionPersonality = GetMonData(lead, MON_DATA_PERSONALITY);
}

// Script: the stylist looks at the lead Pokémon. gStringVar1 = today's
// theme, gStringVar2 = the Pokémon; VAR_RESULT = FASHION_JUDGE_*; for
// FASHION_JUDGE_MATCH VAR_0x8005 = the prize.
void Fashion_Judge(void)
{
    struct Pokemon *lead = GetFirstLiveMon();
    u32 worn = Fashion_WornNow();

    StringCopy(gStringVar1, sThemeNames[TodaysTheme()]);
    if (lead != NULL)
        GetMonNickname(lead, gStringVar2);
    if (worn == 0)
        gSpecialVar_Result = FASHION_JUDGE_NO_MON;
    else if (sSave.fashionJudgeDay == Today() + 1)
        gSpecialVar_Result = FASHION_JUDGE_DONE;
    else if (sAccessories[worn - 1].theme != TodaysTheme())
        gSpecialVar_Result = FASHION_JUDGE_MISS;
    else
    {
        gSpecialVar_Result = FASHION_JUDGE_MATCH;
        sSave.fashionJudgeDay = Today() + 1;
        gSpecialVar_0x8005 = sPrizes[Today() % ARRAY_COUNT(sPrizes)];
    }
}

// R > PASTIMES > FASHION ----------------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index == FASHION_ACCESSORIES)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Today's theme{CLEAR_TO 96}"));
        StringCopy(dest, sThemeNames[TodaysTheme()]);
        return;
    }
    dest = StringCopy(dest, sAccessories[index].name);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 96}"));
    dest = StringCopy(dest, sThemeNames[sAccessories[index].theme]);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 152}"));
    if (Fashion_WornNow() == index + 1)
        StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}WORN"));
    else if (sSave.fashionOwned & (1 << index))
        StringCopy(dest, COMPOUND_STRING("OWNED"));
    else
        StringCopy(dest, COMPOUND_STRING("-"));
}

static void GetDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("The stylist is at SLATEPORT's FAN CLUB"));
}

void Fashion_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("FASHION"), FASHION_ACCESSORIES + 1, GetLine, GetDetail);
}
