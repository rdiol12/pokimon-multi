#include "global.h"
#include "event_data.h"
#include "life.h"
#include "overworld.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "random.h"
#include "rtc.h"
#include "sprite.h"
#include "string_util.h"
#include "sound.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/life.h"
#include "constants/map_groups.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Seasonal festivals in Lilycove City. Each season (src/life_seasons.c) has
// its festival, held on the last two days of every week of the game clock:
// spring's BLOSSOM FESTIVAL, summer's FIREWORKS FESTIVAL, autumn's HARVEST
// FESTIVAL and winter's LIGHTS FESTIVAL. While one is on, a host and three
// stalls stand in Lilycove; a stamp from each stall earns the host's
// seasonal prize, once per festival. Petals drift over Lilycove in spring,
// leaves in autumn, and on summer festival nights fireworks burst in the
// sky.

extern const u8 Festival_EventScript_Host[];
extern const u8 Festival_EventScript_Stall[];

struct Festival
{
    const u8 *name;
    const u8 *stalls[FESTIVAL_STALLS];  // what each stall hands out
    u16 prize;
};

static const struct Festival sFestivals[SEASON_COUNT] =
{
    [SEASON_SPRING] = {COMPOUND_STRING("BLOSSOM FESTIVAL"),
        {COMPOUND_STRING("a flower crown"), COMPOUND_STRING("a cup of blossom tea"), COMPOUND_STRING("a paper kite")},
        ITEM_LEAF_STONE},
    [SEASON_SUMMER] = {COMPOUND_STRING("FIREWORKS FESTIVAL"),
        {COMPOUND_STRING("a sparkler"), COMPOUND_STRING("a shaved ice"), COMPOUND_STRING("a paper fan")},
        ITEM_FIRE_STONE},
    [SEASON_AUTUMN] = {COMPOUND_STRING("HARVEST FESTIVAL"),
        {COMPOUND_STRING("a baked sweet potato"), COMPOUND_STRING("a basket of apples"), COMPOUND_STRING("a straw hat")},
        ITEM_SITRUS_BERRY},
    [SEASON_WINTER] = {COMPOUND_STRING("LIGHTS FESTIVAL"),
        {COMPOUND_STRING("a paper lantern"), COMPOUND_STRING("a mug of cocoa"), COMPOUND_STRING("a knitted scarf")},
        ITEM_ICE_STONE},
};

#define sSave (gSaveBlock3Ptr->pastime)

static bool32 IsFestivalShown(u32 arg);

#define STALL(x, y, gfx, n) {MAP_LILYCOVE_CITY, x, y, gfx, MOVEMENT_TYPE_FACE_DOWN, n, Festival_EventScript_Stall, IsFestivalShown}

const struct PastimeSpot gFestivalSpots[1 + FESTIVAL_STALLS] =
{
    {MAP_LILYCOVE_CITY, 31, 15, OBJ_EVENT_GFX_MAN_4, MOVEMENT_TYPE_FACE_DOWN, 0, Festival_EventScript_Host, IsFestivalShown},
    STALL(37, 17, OBJ_EVENT_GFX_COOK, 0),
    STALL(17, 16, OBJ_EVENT_GFX_WOMAN_2, 1),
    STALL(50, 13, OBJ_EVENT_GFX_MART_EMPLOYEE, 2),
};

static u32 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

// The festival week, + 1 so 0 means never.
static u16 FestivalKey(void)
{
    return Today() / FESTIVAL_WEEK_DAYS + 1;
}

static bool32 IsFestivalDay(void)
{
    return Today() % FESTIVAL_WEEK_DAYS >= FESTIVAL_WEEK_DAYS - FESTIVAL_DAYS;
}

static bool32 IsFestivalShown(u32 arg)
{
    return IsFestivalDay();
}

// The stamps collected at this festival (older ones don't count).
static u32 Stamps(void)
{
    return sSave.festivalWeek == FestivalKey() ? sSave.festivalStamps : 0;
}

static bool32 InLilycove(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_LILYCOVE_CITY)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_LILYCOVE_CITY);
}

// Festival sky: petals, leaves and fireworks ----------------------------------

#define TAG_FIREWORK      0x5A71
#define TAG_FIREWORK_PAL2 0x5A72
#define TAG_FIREWORK_PAL3 0x5A73
#define TAG_PETAL         0x5A74
#define TAG_LEAF          0x5A75

static const u32 sFireworkGfx[] = INCBIN_U32("graphics/pastime/firework.4bpp");
static const u16 sFireworkPal[] = INCBIN_U16("graphics/pastime/firework.gbapal");
static const u32 sPetalGfx[] = INCBIN_U32("graphics/pastime/petal.4bpp");
static const u16 sPetalPal[] = INCBIN_U16("graphics/pastime/petal.gbapal");
static const u32 sLeafGfx[] = INCBIN_U32("graphics/pastime/leaf.4bpp");
static const u16 sLeafPal[] = INCBIN_U16("graphics/pastime/leaf.gbapal");

// The firework in green and in blue (the same tiles).
static const u16 sFireworkPal2[] =
{
    RGB(31, 0, 31), RGB_WHITE, RGB(27, 31, 22), RGB(14, 29, 12), RGB(6, 24, 10), RGB(4, 15, 8),
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
static const u16 sFireworkPal3[] =
{
    RGB(31, 0, 31), RGB_WHITE, RGB(22, 29, 31), RGB(12, 22, 31), RGB(8, 13, 29), RGB(10, 7, 22),
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const struct SpriteSheet sFireworkSheet = {sFireworkGfx, sizeof(sFireworkGfx), TAG_FIREWORK};
static const struct SpriteSheet sPetalSheet = {sPetalGfx, sizeof(sPetalGfx), TAG_PETAL};
static const struct SpriteSheet sLeafSheet = {sLeafGfx, sizeof(sLeafGfx), TAG_LEAF};
static const struct SpritePalette sSkyPalettes[] =
{
    {sFireworkPal, TAG_FIREWORK},
    {sFireworkPal2, TAG_FIREWORK_PAL2},
    {sFireworkPal3, TAG_FIREWORK_PAL3},
    {sPetalPal, TAG_PETAL},
    {sLeafPal, TAG_LEAF},
};

static const struct OamData sFireworkOam =
{
    .affineMode = ST_OAM_AFFINE_DOUBLE,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
};

static const struct OamData sFlakeOam =
{
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 1,
};

// The burst grows from a spark to full size, then fades by blinking.
static const union AffineAnimCmd sAffineAnim_Burst[] =
{
    AFFINEANIMCMD_FRAME(32, 32, 0, 0),
    AFFINEANIMCMD_FRAME(14, 14, 0, 16),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd *const sAffineAnims_Burst[] = {sAffineAnim_Burst};

static void SpriteCB_Firework(struct Sprite *sprite);
static void SpriteCB_Flake(struct Sprite *sprite);

static const struct SpriteTemplate sFireworkTemplate =
{
    .tileTag = TAG_FIREWORK,
    .paletteTag = TAG_FIREWORK,
    .oam = &sFireworkOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = sAffineAnims_Burst,
    .callback = SpriteCB_Firework,
};

static const struct SpriteTemplate sPetalTemplate =
{
    .tileTag = TAG_PETAL,
    .paletteTag = TAG_PETAL,
    .oam = &sFlakeOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Flake,
};

static const struct SpriteTemplate sLeafTemplate =
{
    .tileTag = TAG_LEAF,
    .paletteTag = TAG_LEAF,
    .oam = &sFlakeOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Flake,
};

#define sTimer data[0]

static void SpriteCB_Firework(struct Sprite *sprite)
{
    sprite->sTimer++;
    if (sprite->sTimer > 40)
        sprite->invisible = (sprite->sTimer / 2) & 1;
    if (sprite->sTimer > 60)
        DestroySpriteAndFreeResources(sprite);
}

// Drifts down and sideways, swaying.
static void SpriteCB_Flake(struct Sprite *sprite)
{
    sprite->sTimer++;
    if (sprite->sTimer % 2 == 0)
        sprite->y++;
    sprite->x2 = ((sprite->sTimer / 12) % 4 < 2) ? (sprite->sTimer / 3) % 4 : 3 - (sprite->sTimer / 3) % 4;
    if (sprite->sTimer % 5 == 0)
        sprite->x--;
    if (sprite->y > DISPLAY_HEIGHT + 8)
        DestroySprite(sprite);
}

static void LoadSkyGraphics(const struct SpriteSheet *sheet, u32 paletteTag)
{
    u32 i;

    if (GetSpriteTileStartByTag(sheet->tag) == 0xFFFF)
        LoadSpriteSheet(sheet);
    for (i = 0; i < ARRAY_COUNT(sSkyPalettes); i++)
    {
        if (sSkyPalettes[i].tag == paletteTag && IndexOfSpritePaletteTag(paletteTag) == 0xFF)
            LoadSpritePalette(&sSkyPalettes[i]);
    }
}

static void LaunchFirework(void)
{
    static const u16 sPaletteTags[] = {TAG_FIREWORK, TAG_FIREWORK_PAL2, TAG_FIREWORK_PAL3};
    struct SpriteTemplate template = sFireworkTemplate;
    u8 spriteId;

    template.paletteTag = sPaletteTags[Random() % ARRAY_COUNT(sPaletteTags)];
    LoadSkyGraphics(&sFireworkSheet, template.paletteTag);
    spriteId = CreateSprite(&template, 40 + Random() % 160, 16 + Random() % 40, 0);
    if (spriteId != MAX_SPRITES)
        PlaySE(SE_M_EXPLOSION);
}

static void DropFlake(const struct SpriteTemplate *template, const struct SpriteSheet *sheet)
{
    LoadSkyGraphics(sheet, template->paletteTag);
    CreateSprite(template, 16 + Random() % 240, -4, 0);
}

// Pastime_Update: the festival sky over Lilycove.
void Festival_Update(void)
{
    if (!InLilycove() || !IsFestivalDay())
        return;
    switch (Seasons_Get())
    {
    case SEASON_SPRING:
        if (Random() % FESTIVAL_FLAKE_RATE == 0)
            DropFlake(&sPetalTemplate, &sPetalSheet);
        break;
    case SEASON_AUTUMN:
        if (Random() % FESTIVAL_FLAKE_RATE == 0)
            DropFlake(&sLeafTemplate, &sLeafSheet);
        break;
    case SEASON_SUMMER:
        RtcCalcLocalTime();
        if ((gLocalTime.hours >= 19 || gLocalTime.hours < 2) && Random() % FESTIVAL_FIREWORK_RATE == 0)
            LaunchFirework();
        break;
    }
}

// Scripts ----------------------------------------------------------------------------

// Script: the host. gStringVar1 = the festival; gStringVar2 = stamps so far.
// VAR_RESULT = FESTIVAL_NEED_STAMPS, FESTIVAL_PRIZE (VAR_0x8005 = the prize,
// now given) or FESTIVAL_DONE (already had it this festival).
void Festival_TalkToHost(void)
{
    const struct Festival *festival = &sFestivals[Seasons_Get()];
    u32 stamps = Stamps();

    StringCopy(gStringVar1, festival->name);
    ConvertIntToDecimalStringN(gStringVar2, (stamps & 1) + ((stamps >> 1) & 1) + ((stamps >> 2) & 1), STR_CONV_MODE_LEFT_ALIGN, 1);
    if (sSave.festivalWeek == FestivalKey() && sSave.festivalPrize)
    {
        gSpecialVar_Result = FESTIVAL_DONE;
        return;
    }
    if (stamps != (1 << FESTIVAL_STALLS) - 1)
    {
        gSpecialVar_Result = FESTIVAL_NEED_STAMPS;
        return;
    }
    sSave.festivalPrize = TRUE;
    if (sSave.festivalsDone < 0xFFFF)
        sSave.festivalsDone++;
    gSpecialVar_0x8005 = festival->prize;
    gSpecialVar_Result = FESTIVAL_PRIZE;
}

// Script: stall VAR_0x8004 stamps the card. gStringVar1 = what it hands
// out; VAR_RESULT = TRUE if the stamp is new.
void Festival_VisitStall(void)
{
    const struct Festival *festival = &sFestivals[Seasons_Get()];

    if (sSave.festivalWeek != FestivalKey())
    {
        sSave.festivalWeek = FestivalKey();
        sSave.festivalStamps = 0;
        sSave.festivalPrize = FALSE;
    }
    StringCopy(gStringVar1, festival->stalls[gSpecialVar_0x8004]);
    StringCopy(gStringVar2, festival->name);
    gSpecialVar_Result = !(sSave.festivalStamps & (1 << gSpecialVar_0x8004));
    sSave.festivalStamps |= 1 << gSpecialVar_0x8004;
}

// R > PASTIMES > FESTIVALS ---------------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < SEASON_COUNT)
    {
        static const u8 *const sSeasonNames[SEASON_COUNT] =
        {
            COMPOUND_STRING("Spring"), COMPOUND_STRING("Summer"), COMPOUND_STRING("Autumn"), COMPOUND_STRING("Winter"),
        };
        dest = StringCopy(dest, sSeasonNames[index]);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 48}"));
        dest = StringCopy(dest, sFestivals[index].name);
        if (index == Seasons_Get())
            StringCopy(dest, COMPOUND_STRING(" {COLOR GREEN}{SHADOW LIGHT_GREEN}NOW"));
        return;
    }
    if (index == SEASON_COUNT)
    {
        u32 wait = (FESTIVAL_WEEK_DAYS - FESTIVAL_DAYS) - Today() % FESTIVAL_WEEK_DAYS;
        dest = StringCopy(dest, COMPOUND_STRING("Next festival{CLEAR_TO 112}"));
        if (IsFestivalDay())
        {
            StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}TODAY!"));
            return;
        }
        dest = StringCopy(dest, COMPOUND_STRING("in "));
        dest = ConvertIntToDecimalStringN(dest, wait, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringCopy(dest, wait == 1 ? COMPOUND_STRING(" day") : COMPOUND_STRING(" days"));
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Festivals enjoyed{CLEAR_TO 112}"));
    ConvertIntToDecimalStringN(dest, sSave.festivalsDone, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void GetDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("Festivals are held in LILYCOVE CITY"));
}

void Festival_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("FESTIVALS"), SEASON_COUNT + 2, GetLine, GetDetail);
}
