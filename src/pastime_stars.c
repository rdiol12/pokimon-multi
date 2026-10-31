#include "global.h"
#include "event_data.h"
#include "menu.h"
#include "overworld.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "random.h"
#include "rtc.h"
#include "sprite.h"
#include "string_util.h"
#include "window.h"
#include "wild_encounter.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/species.h"

// Star map and meteor shower nights. At night (20:00 to 4:59) a stargazer
// stands at each of STAR_CONSTELLATIONS lookouts around Hoenn; looking
// through their telescope shows a constellation, drawn star by star, and
// adds it to the STAR MAP. The astronomer at the Mossdeep Space Center
// gives prizes as the map fills and says when the next meteor shower is.
//
// Every STAR_METEOR_EVERY nights there is a meteor shower: shooting stars
// streak across the sky, small meteorites (one find each a night) land on
// the routes around Meteor Falls, and Pokémon from space turn up there.

extern const u8 Star_EventScript_Astronomer[];
extern const u8 Star_EventScript_Stargazer[];
extern const u8 Star_EventScript_Meteorite[];

struct ChartPoint
{
    u8 x, y;
};

struct Constellation
{
    const u8 *name;
    u16 species;
    const struct ChartPoint *stars;
    u8 starCount;
    bool8 closed; // the last star joins back to the first
    const u8 (*extraLines)[2]; // more pairs of star indexes to join
    u8 extraCount;
};

// Stars on a 64 x 64 chart, joined in order.
static const struct ChartPoint sWhale[] = {{8, 32}, {18, 20}, {36, 17}, {52, 23}, {57, 34}, {45, 45}, {22, 45}, {10, 40}, {30, 6}};
static const u8 sWhaleLines[][2] = {{2, 8}};
static const struct ChartPoint sDragon[] = {{5, 56}, {14, 47}, {18, 36}, {28, 30}, {40, 28}, {46, 18}, {56, 8}, {38, 40}};
static const u8 sDragonLines[][2] = {{3, 7}};
static const struct ChartPoint sWish[] = {{32, 5}, {39, 23}, {58, 25}, {44, 37}, {50, 57}, {32, 46}, {14, 57}, {20, 37}, {6, 25}, {25, 23}};
static const struct ChartPoint sCloudBird[] = {{5, 30}, {17, 20}, {30, 26}, {43, 19}, {57, 28}, {30, 40}, {21, 52}, {41, 52}};
static const u8 sCloudBirdLines[][2] = {{2, 5}, {5, 6}, {5, 7}};
static const struct ChartPoint sBlade[] = {{14, 6}, {27, 13}, {35, 26}, {37, 40}, {30, 55}, {51, 47}};
static const u8 sBladeLines[][2] = {{3, 5}};
static const struct ChartPoint sHeart[] = {{32, 18}, {22, 8}, {10, 12}, {7, 26}, {18, 41}, {32, 55}, {46, 41}, {57, 26}, {54, 12}, {42, 8}};
static const struct ChartPoint sMoon[] = {{41, 5}, {24, 11}, {13, 28}, {18, 46}, {35, 57}, {28, 41}, {26, 26}, {32, 15}};
static const struct ChartPoint sStarfish[] = {{32, 32}, {32, 5}, {57, 24}, {48, 55}, {16, 55}, {7, 24}};
static const u8 sStarfishLines[][2] = {{0, 2}, {0, 3}, {0, 4}, {0, 5}};

#define C(n, sp, s, c, l) {COMPOUND_STRING(n), sp, s, ARRAY_COUNT(s), c, l, ARRAY_COUNT(l)}
#define C0(n, sp, s, c) {COMPOUND_STRING(n), sp, s, ARRAY_COUNT(s), c, NULL, 0}

static const struct Constellation sConstellations[STAR_CONSTELLATIONS] =
{
    C("THE SKY WHALE", SPECIES_WAILORD, sWhale, TRUE, sWhaleLines),
    C("THE SERPENT", SPECIES_RAYQUAZA, sDragon, FALSE, sDragonLines),
    C0("THE WISHING STAR", SPECIES_JIRACHI, sWish, TRUE),
    C("THE CLOUD BIRD", SPECIES_ALTARIA, sCloudBird, FALSE, sCloudBirdLines),
    C("THE BLADE", SPECIES_ABSOL, sBlade, FALSE, sBladeLines),
    C0("THE HEART FISH", SPECIES_LUVDISC, sHeart, TRUE),
    C0("THE STONE MOON", SPECIES_LUNATONE, sMoon, TRUE),
    // sStarfish's first star is the middle; its arms are all extra lines.
    {COMPOUND_STRING("THE SEA STAR"), SPECIES_STARYU, sStarfish, 1, FALSE, sStarfishLines, ARRAY_COUNT(sStarfishLines)},
};

static const u16 sMeteorMons[] =
{
    SPECIES_CLEFAIRY, SPECIES_MINIOR_METEOR_RED, SPECIES_BELDUM, SPECIES_ELGYEM, SPECIES_SOLROCK, SPECIES_LUNATONE,
};

static const u16 sMeteorMaps[] =
{
    MAP_ROUTE111, MAP_ROUTE113, MAP_ROUTE114, MAP_ROUTE115, MAP_ROUTE120,
};

// Each meteorite's find: Stardust, sometimes better.
static const u16 sMeteoriteItems[STAR_METEORITES] = {ITEM_STARDUST, ITEM_STARDUST, ITEM_STAR_PIECE, ITEM_STARDUST, ITEM_STARDUST};

// The astronomer's prizes, at 2, 4, 6 and 8 constellations.
static const u16 sMapPrizes[] = {ITEM_STARDUST, ITEM_STAR_PIECE, ITEM_MOON_STONE, ITEM_COMET_SHARD};

#define sSave (gSaveBlock3Ptr->pastime)

static bool32 IsStargazerShown(u32 arg);
static bool32 IsMeteoriteShown(u32 arg);

#define GAZER(map, x, y, gfx, n) {map, x, y, gfx, MOVEMENT_TYPE_FACE_UP, n, Star_EventScript_Stargazer, IsStargazerShown}
#define ROCK(map, x, y, n) {map, x, y, OBJ_EVENT_GFX_BREAKABLE_ROCK, MOVEMENT_TYPE_NONE, n, Star_EventScript_Meteorite, IsMeteoriteShown}

const struct PastimeSpot gStarSpots[1 + STAR_CONSTELLATIONS + STAR_METEORITES] =
{
    {MAP_MOSSDEEP_CITY_SPACE_CENTER_1F, 8, 7, OBJ_EVENT_GFX_SCIENTIST_2, MOVEMENT_TYPE_FACE_DOWN, 0, Star_EventScript_Astronomer},
    GAZER(MAP_LILYCOVE_CITY, 30, 21, OBJ_EVENT_GFX_MAN_3, 0),
    GAZER(MAP_ROUTE120, 32, 25, OBJ_EVENT_GFX_HIKER, 1),
    GAZER(MAP_FALLARBOR_TOWN, 13, 10, OBJ_EVENT_GFX_LASS, 2),
    GAZER(MAP_FORTREE_CITY, 7, 17, OBJ_EVENT_GFX_OLD_MAN, 3),
    GAZER(MAP_ROUTE113, 46, 8, OBJ_EVENT_GFX_PSYCHIC_M, 4),
    GAZER(MAP_MOSSDEEP_CITY, 33, 30, OBJ_EVENT_GFX_WOMAN_4, 5),
    GAZER(MAP_ROUTE114, 21, 27, OBJ_EVENT_GFX_EXPERT_F, 6),
    GAZER(MAP_DEWFORD_TOWN, 4, 12, OBJ_EVENT_GFX_FISHERMAN, 7),
    ROCK(MAP_ROUTE114, 24, 51, 0),
    ROCK(MAP_ROUTE115, 11, 6, 1),
    ROCK(MAP_ROUTE113, 63, 10, 2),
    ROCK(MAP_ROUTE111, 20, 6, 3),
    ROCK(MAP_ROUTE120, 29, 52, 4),
};

// Night and meteor nights ---------------------------------------------------

static bool32 IsNight(void)
{
    RtcCalcLocalTime();
    return gLocalTime.hours >= STAR_NIGHT_START || gLocalTime.hours < STAR_NIGHT_END;
}

// The day the current night began on (the small hours belong to the night
// before).
static u32 NightDay(void)
{
    RtcCalcLocalTime();
    if (gLocalTime.hours < STAR_NIGHT_END && gLocalTime.days > 0)
        return gLocalTime.days - 1;
    return gLocalTime.days;
}

static bool32 IsMeteorDay(u32 day)
{
    return day % STAR_METEOR_EVERY == STAR_METEOR_EVERY - 1;
}

static bool32 IsMeteorNight(void)
{
    return IsNight() && IsMeteorDay(NightDay());
}

// 0: tonight (or in the day before tonight).
static u32 NightsToShower(void)
{
    u32 day = NightDay(), wait = 0;

    while (!IsMeteorDay(day + wait))
        wait++;
    return wait;
}

// The meteorites found tonight (the bits are from an older shower otherwise).
static u32 PickedTonight(void)
{
    return sSave.meteorNight == NightDay() + 1 ? sSave.meteorPicked : 0;
}

static bool32 IsStargazerShown(u32 arg)
{
    return IsNight();
}

static bool32 IsMeteoriteShown(u32 arg)
{
    return IsMeteorNight() && !(PickedTonight() & (1 << arg));
}

static bool32 OnMeteorMap(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMeteorMaps); i++)
    {
        if (sMeteorMaps[i] == map)
            return TRUE;
    }
    return FALSE;
}

// Wild encounters: on a meteor night, part of the land encounters around
// Meteor Falls are Pokémon from space.
u16 Star_ModifyWildSpecies(u16 species)
{
    if (gIsFishingEncounter || gIsSurfingEncounter || !OnMeteorMap() || !IsMeteorNight())
        return species;
    if (Random() % 100 >= STAR_METEOR_MON_PERCENT)
        return species;
    return sMeteorMons[Random() % ARRAY_COUNT(sMeteorMons)];
}

// Shooting stars ------------------------------------------------------------

#define TAG_METEOR 0x5A70

static const u32 sMeteorGfx[] = INCBIN_U32("graphics/pastime/meteor.4bpp");
static const u16 sMeteorPal[] = INCBIN_U16("graphics/pastime/meteor.gbapal");

static const struct SpriteSheet sMeteorSheet = {sMeteorGfx, sizeof(sMeteorGfx), TAG_METEOR};
static const struct SpritePalette sMeteorPalette = {sMeteorPal, TAG_METEOR};

static const struct OamData sMeteorOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static void SpriteCB_Meteor(struct Sprite *sprite);

static const struct SpriteTemplate sMeteorTemplate =
{
    .tileTag = TAG_METEOR,
    .paletteTag = TAG_METEOR,
    .oam = &sMeteorOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Meteor,
};

#define sTimer data[0]

static void SpriteCB_Meteor(struct Sprite *sprite)
{
    sprite->x -= 3;
    sprite->y += 2;
    if (++sprite->sTimer > 36)
        DestroySprite(sprite);
}

static void LaunchMeteor(void)
{
    if (GetSpriteTileStartByTag(TAG_METEOR) == 0xFFFF)
        LoadSpriteSheet(&sMeteorSheet);
    if (IndexOfSpritePaletteTag(TAG_METEOR) == 0xFF)
        LoadSpritePalette(&sMeteorPalette);
    CreateSprite(&sMeteorTemplate, 80 + Random() % 160, Random() % 48, 0);
}

// Pastime_Update: shooting stars over outdoor maps on a meteor night.
void Star_Update(void)
{
    if (!IsMapTypeOutdoors(gMapHeader.mapType) || Random() % STAR_METEOR_RATE != 0 || !IsMeteorNight())
        return;
    LaunchMeteor();
}

// The telescope's star chart ------------------------------------------------

#define CHART_X 11 // tiles
#define CHART_Y 2
#define CHART_SIZE 8
#define CHART_BASE_BLOCK 0xD0 // as Pastime_ShowMonPic's box, which it replaces

static EWRAM_DATA u8 sChartWindowId = 0;
static EWRAM_DATA bool8 sChartShown = FALSE;

static void DrawLine(u32 windowId, const struct ChartPoint *a, const struct ChartPoint *b)
{
    s32 dx = b->x - a->x, dy = b->y - a->y;
    s32 steps = max(abs(dx), abs(dy)), i;

    for (i = 1; i < steps; i++)
        FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_LIGHT_BLUE), a->x + dx * i / steps, a->y + dy * i / steps, 1, 1);
}

static void DrawChart(u32 windowId, const struct Constellation *c)
{
    u32 i, count = c->starCount;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_BLUE));
    // A few faint background stars.
    for (i = 0; i < 12; i++)
        FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_LIGHT_GRAY), (i * 37 + 11) % 62, (i * 23 + 5) % 62, 1, 1);
    for (i = 0; i + 1 < count; i++)
        DrawLine(windowId, &c->stars[i], &c->stars[i + 1]);
    if (c->closed)
        DrawLine(windowId, &c->stars[count - 1], &c->stars[0]);
    for (i = 0; i < c->extraCount; i++)
        DrawLine(windowId, &c->stars[c->extraLines[i][0]], &c->stars[c->extraLines[i][1]]);
    for (i = 0; i < count; i++)
        FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_WHITE), c->stars[i].x - 1, c->stars[i].y - 1, 2, 2);
    // Stars only reached by the extra lines.
    for (i = 0; i < c->extraCount; i++)
    {
        const struct ChartPoint *p = &c->stars[c->extraLines[i][1]];
        FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_WHITE), p->x - 1, p->y - 1, 2, 2);
    }
}

// Script: shows the constellation of stargazer VAR_0x8004.
void Star_ShowChart(void)
{
    struct WindowTemplate template = CreateWindowTemplate(0, CHART_X, CHART_Y, CHART_SIZE, CHART_SIZE, 15, CHART_BASE_BLOCK);

    Star_HideChart();
    sChartWindowId = AddWindow(&template);
    PutWindowTilemap(sChartWindowId);
    SetStandardWindowBorderStyle(sChartWindowId, TRUE);
    DrawChart(sChartWindowId, &sConstellations[gSpecialVar_0x8004]);
    CopyWindowToVram(sChartWindowId, COPYWIN_FULL);
    sChartShown = TRUE;
    ScheduleBgCopyTilemapToVram(0);
}

void Star_HideChart(void)
{
    if (!sChartShown)
        return;
    ClearStdWindowAndFrameToTransparent(sChartWindowId, TRUE);
    RemoveWindow(sChartWindowId);
    sChartShown = FALSE;
}

// Scripts -------------------------------------------------------------------

static u32 CountSeen(void)
{
    u32 i, count = 0;

    for (i = 0; i < STAR_CONSTELLATIONS; i++)
    {
        if (sSave.starSeen & (1 << i))
            count++;
    }
    return count;
}

// Script: logs stargazer VAR_0x8004's constellation. gStringVar1 = its
// name, gStringVar2 = the Pokémon; VAR_0x8005 = the species (for the
// picture box); VAR_RESULT = TRUE if it's new. VAR_0x8006 = TRUE on a
// meteor night.
void Star_Look(void)
{
    const struct Constellation *c = &sConstellations[gSpecialVar_0x8004];

    StringCopy(gStringVar1, c->name);
    StringCopy(gStringVar2, GetSpeciesName(c->species));
    gSpecialVar_0x8005 = c->species;
    gSpecialVar_Result = !(sSave.starSeen & (1 << gSpecialVar_0x8004));
    sSave.starSeen |= 1 << gSpecialVar_0x8004;
    gSpecialVar_0x8006 = IsMeteorNight();
}

// Script: picks up meteorite VAR_0x8004; VAR_0x8005 = its find.
void Star_PickMeteorite(void)
{
    if (sSave.meteorNight != NightDay() + 1)
    {
        sSave.meteorNight = NightDay() + 1;
        sSave.meteorPicked = 0;
    }
    sSave.meteorPicked |= 1 << gSpecialVar_0x8004;
    if (sSave.meteorFound < 0xFFFF)
        sSave.meteorFound++;
    gSpecialVar_0x8005 = sMeteoriteItems[gSpecialVar_0x8004];
}

// Script: the astronomer. gStringVar1 = constellations found,
// gStringVar2 = nights to the next meteor shower ("TONIGHT" is
// VAR_0x8006 = TRUE). VAR_0x8005 = the next prize earned (else ITEM_NONE).
void Star_TalkToAstronomer(void)
{
    u32 wait = NightsToShower(), seen = CountSeen();

    ConvertIntToDecimalStringN(gStringVar1, seen, STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertIntToDecimalStringN(gStringVar2, wait, STR_CONV_MODE_LEFT_ALIGN, 1);
    gSpecialVar_0x8006 = wait == 0;
    gSpecialVar_0x8005 = ITEM_NONE;
    if (sSave.starRewards < ARRAY_COUNT(sMapPrizes) && seen >= (sSave.starRewards + 1) * 2)
        gSpecialVar_0x8005 = sMapPrizes[sSave.starRewards++];
}

// R > PASTIMES > STAR MAP --------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < STAR_CONSTELLATIONS)
    {
        const struct Constellation *c = &sConstellations[index];
        if (sSave.starSeen & (1 << index))
        {
            dest = StringCopy(dest, c->name);
            dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 120}"));
            StringCopy(dest, GetSpeciesName(c->species));
        }
        else
        {
            StringCopy(dest, COMPOUND_STRING("? ? ?"));
        }
        return;
    }
    if (index == STAR_CONSTELLATIONS)
    {
        u32 wait = NightsToShower();
        dest = StringCopy(dest, COMPOUND_STRING("Meteor shower{CLEAR_TO 120}"));
        if (wait == 0)
        {
            StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}TONIGHT!"));
        }
        else
        {
            dest = StringCopy(dest, COMPOUND_STRING("in "));
            dest = ConvertIntToDecimalStringN(dest, wait, STR_CONV_MODE_LEFT_ALIGN, 1);
            StringCopy(dest, wait == 1 ? COMPOUND_STRING(" day") : COMPOUND_STRING(" days"));
        }
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Meteorites found{CLEAR_TO 120}"));
    ConvertIntToDecimalStringN(dest, sSave.meteorFound, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void GetDetail(u16 index, u8 *dest)
{
    if (index < STAR_CONSTELLATIONS && !(sSave.starSeen & (1 << index)))
        StringCopy(dest, COMPOUND_STRING("Stargazers come out at night"));
    else if (index == STAR_CONSTELLATIONS)
        StringCopy(dest, COMPOUND_STRING("Meteorites fall near METEOR FALLS"));
    else
        StringCopy(dest, COMPOUND_STRING("The astronomer is at the SPACE CENTER"));
}

void Star_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("STAR MAP"), STAR_CONSTELLATIONS + 2, GetLine, GetDetail);
}
