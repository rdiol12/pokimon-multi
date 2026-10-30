#include "global.h"
#include "decompress.h"
#include "main.h"
#include "random.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "world.h"
#include "constants/songs.h"

// The timing meter used by the fishing and berry-picking minigames: a
// needle sweeps across a bar, and A stops it. Stopping it on the green zone
// is a hit. Runs as its own task over the overworld; the caller polls
// TimingMeter_GetResult (-1 while running, else the number of hits).
// Art: graphics/world/meter_*.png (tools/world_shots/draw_icons.py).

#define TAG_METER_BAR    0x5742
#define TAG_METER_ZONE   0x5743
#define TAG_METER_NEEDLE 0x5744
#define TAG_METER_PAL    0x5742

#define BAR_X        (120 - 32) // left edge of the bar on screen
#define BAR_Y        52
#define GROOVE_START 7          // first and last pixel the needle can point at
#define GROOVE_END   56
#define MAX_ZONE_TILES 4
#define PASSES_PER_ROUND 4      // the needle sweeps this many times before it's a miss
#define FLASH_FRAMES 24

static const u32 sBarGfx[] = INCBIN_U32("graphics/world/meter_bar.4bpp");
static const u32 sZoneGfx[] = INCBIN_U32("graphics/world/meter_zone.4bpp");
static const u32 sNeedleGfx[] = INCBIN_U32("graphics/world/meter_needle.4bpp");
static const u16 sMeterPal[] = INCBIN_U16("graphics/world/meter_bar.gbapal");

static const struct SpriteSheet sMeterSheets[] =
{
    {sBarGfx, sizeof(sBarGfx), TAG_METER_BAR},
    {sZoneGfx, sizeof(sZoneGfx), TAG_METER_ZONE},
    {sNeedleGfx, sizeof(sNeedleGfx), TAG_METER_NEEDLE},
    {0},
};
static const struct SpritePalette sMeterPalette = {sMeterPal, TAG_METER_PAL};

static const struct OamData sOam32x8 = {.shape = SPRITE_SHAPE(32x8), .size = SPRITE_SIZE(32x8), .priority = 0};
static const struct OamData sOam8x8 = {.shape = SPRITE_SHAPE(8x8), .size = SPRITE_SIZE(8x8), .priority = 0};
static const struct OamData sOam8x16 = {.shape = SPRITE_SHAPE(8x16), .size = SPRITE_SIZE(8x16), .priority = 0};

#define TEMPLATE(tag, oamData) {                         \
    .tileTag = tag, .paletteTag = TAG_METER_PAL,         \
    .oam = oamData, .anims = gDummySpriteAnimTable,      \
    .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, \
    .callback = SpriteCallbackDummy }

static const struct SpriteTemplate sBarTemplate = TEMPLATE(TAG_METER_BAR, &sOam32x8);
static const struct SpriteTemplate sZoneTemplate = TEMPLATE(TAG_METER_ZONE, &sOam8x8);
static const struct SpriteTemplate sNeedleTemplate = TEMPLATE(TAG_METER_NEEDLE, &sOam8x16);

struct TimingMeter
{
    u8 barSprites[2];
    u8 zoneSprites[MAX_ZONE_TILES];
    u8 needleSprite;
    u8 zoneTiles;
    u8 speed;
    u8 rounds;
    u8 round;
    u8 hits;
    u8 passes;
    u8 flash;
    s8 dir;
    s16 needle; // groove pixel the needle points at
    s16 zoneX;  // groove pixel where the zone starts
    s8 result;  // -1 while running
};

// Not static: the screenshot driver reads it to stop the needle in time.
EWRAM_DATA struct TimingMeter gWorldTimingMeter = {0};
#define sMeter gWorldTimingMeter

static void Task_TimingMeter(u8 taskId);

static void PlaceZone(void)
{
    u32 i, width = sMeter.zoneTiles * 8;

    sMeter.zoneX = GROOVE_START + Random() % (GROOVE_END - GROOVE_START - width + 2);
    for (i = 0; i < sMeter.zoneTiles; i++)
    {
        struct Sprite *zone = &gSprites[sMeter.zoneSprites[i]];
        zone->x = BAR_X + sMeter.zoneX + i * 8 + 4;
        zone->y = BAR_Y + 4;
        zone->invisible = FALSE;
    }
    sMeter.needle = GROOVE_START;
    gSprites[sMeter.needleSprite].x = BAR_X + GROOVE_START;
    sMeter.dir = 1;
    sMeter.passes = 0;
}

// rounds: how many stops; zoneTiles: zone width in 8-pixel tiles (1-4);
// speed: needle pixels per frame.
void TimingMeter_Start(u8 rounds, u8 zoneTiles, u8 speed)
{
    u32 i;

    for (i = 0; sMeterSheets[i].data != NULL; i++)
        LoadSpriteSheet(&sMeterSheets[i]);
    LoadSpritePalette(&sMeterPalette);

    sMeter.zoneTiles = min(max(zoneTiles, 1), MAX_ZONE_TILES);
    sMeter.speed = speed;
    sMeter.rounds = rounds;
    sMeter.round = 0;
    sMeter.hits = 0;
    sMeter.flash = 0;
    sMeter.result = -1;
    for (i = 0; i < 2; i++)
    {
        sMeter.barSprites[i] = CreateSprite(&sBarTemplate, BAR_X + 16 + i * 32, BAR_Y + 4, 2);
        gSprites[sMeter.barSprites[i]].oam.tileNum += i * 4;
    }
    for (i = 0; i < MAX_ZONE_TILES; i++)
    {
        sMeter.zoneSprites[i] = CreateSprite(&sZoneTemplate, 0, 0, 1);
        gSprites[sMeter.zoneSprites[i]].invisible = TRUE;
    }
    sMeter.needleSprite = CreateSprite(&sNeedleTemplate, 0, BAR_Y - 2, 0);
    PlaceZone();
    CreateTask(Task_TimingMeter, 50);
}

s32 TimingMeter_GetResult(void)
{
    return sMeter.result;
}

static void DestroyMeter(void)
{
    u32 i;

    for (i = 0; i < 2; i++)
        DestroySprite(&gSprites[sMeter.barSprites[i]]);
    for (i = 0; i < MAX_ZONE_TILES; i++)
        DestroySprite(&gSprites[sMeter.zoneSprites[i]]);
    DestroySprite(&gSprites[sMeter.needleSprite]);
    FreeSpriteTilesByTag(TAG_METER_BAR);
    FreeSpriteTilesByTag(TAG_METER_ZONE);
    FreeSpriteTilesByTag(TAG_METER_NEEDLE);
    FreeSpritePaletteByTag(TAG_METER_PAL);
}

static void EndRound(bool32 hit)
{
    if (hit)
    {
        sMeter.hits++;
        PlaySE(SE_SUCCESS);
    }
    else
    {
        PlaySE(SE_FAILURE);
    }
    sMeter.flash = FLASH_FRAMES;
}

static void Task_TimingMeter(u8 taskId)
{
    struct Sprite *needle = &gSprites[sMeter.needleSprite];
    u32 i;

    if (sMeter.flash != 0)
    {
        // Blink the zone to show the stop, then start the next round.
        for (i = 0; i < sMeter.zoneTiles; i++)
            gSprites[sMeter.zoneSprites[i]].invisible = (sMeter.flash / 4) & 1;
        if (--sMeter.flash != 0)
            return;
        if (++sMeter.round >= sMeter.rounds)
        {
            DestroyMeter();
            sMeter.result = sMeter.hits;
            DestroyTask(taskId);
            return;
        }
        PlaceZone();
    }

    if (JOY_NEW(A_BUTTON))
    {
        EndRound(sMeter.needle >= sMeter.zoneX && sMeter.needle < sMeter.zoneX + sMeter.zoneTiles * 8);
        return;
    }
    sMeter.needle += sMeter.dir * sMeter.speed;
    if (sMeter.needle >= GROOVE_END || sMeter.needle <= GROOVE_START)
    {
        sMeter.needle = sMeter.needle >= GROOVE_END ? GROOVE_END : GROOVE_START;
        sMeter.dir = -sMeter.dir;
        if (++sMeter.passes >= PASSES_PER_ROUND)
            EndRound(FALSE);
    }
    needle->x = BAR_X + sMeter.needle;
}
