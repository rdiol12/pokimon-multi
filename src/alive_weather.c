#include "global.h"
#include "alive.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "item.h"
#include "metatile_behavior.h"
#include "random.h"
#include "save_ext.h"
#include "sound.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"
#include "constants/items.h"
#include "constants/metatile_labels.h"
#include "constants/songs.h"
#include "constants/weather.h"

// Weather that changes the map (docs/alive.md):
//  - rain floods low ground: puddles open up on the grass;
//  - snow covers the ground, the player leaves a trail of footprints, and
//    drifts pile up that the player can push through;
//  - in a dry spell small ponds dry up, and sometimes something lost turns
//    up on the bottom.
// The changes last while the weather does; leave and come back after it
// clears and the map is as it was.

#define GRASS        METATILE_General_Grass
#define SNOWY_GRASS  0x1EA // General: grass under snow (MB_NORMAL)
#define PUDDLE_FIRST 0x0C8 // General: a 3x3 puddle, rows 0x0C8, 0x0D0, 0x0D8 (MB_PUDDLE)
#define DRY_SAND     0x1D9 // General: bare earth (the light path; MB_NORMAL)

#define MAX_DRIFTS   3
#define FINDS_PER_DAY 3

enum { CHANGE_NONE, CHANGE_FLOOD, CHANGE_SNOW, CHANGE_DRY };

static EWRAM_DATA struct
{
    u8 change;
    u8 driftsSpawned;
    u8 findSpawned;
    s16 findX, findY; // the dried pond's middle (0: none)
    s16 lastX, lastY; // the player's last tile (for footprints)
} sWeather = {0};

// Scratch for finding ponds: a bit per tile, and a queue.
#define MAX_MAP_TILES 0x4000
static EWRAM_EXT_DATA u8 sSeen[MAX_MAP_TILES / 8] = {0};
static EWRAM_EXT_DATA u16 sQueue[MAX_MAP_TILES] = {0};

static bool32 IsRainy(u32 weather)
{
    return weather == WEATHER_RAIN || weather == WEATHER_RAIN_THUNDERSTORM || weather == WEATHER_DOWNPOUR;
}

// One day in five is a dry spell on the routes (and any drought).
static bool32 IsDry(u32 weather)
{
    if (weather == WEATHER_DROUGHT)
        return TRUE;
    if (gAliveShowcase.scene == ALIVE_SHOW_DRY)
        return TRUE;
    return (weather == WEATHER_NONE || weather == WEATHER_SUNNY || weather == WEATHER_SUNNY_CLOUDS)
        && Alive_Day() % 5 == 3;
}

static bool32 OpenGrass(s32 x, s32 y)
{
    const struct MapEvents *events = gMapHeader.events;
    u32 i;
    if (MapGridGetMetatileIdAt(x, y) != GRASS || MapGridGetCollisionAt(x, y) != 0)
        return FALSE;
    for (i = 0; i < events->objectEventCount; i++)
    {
        if (events->objectEvents[i].x + MAP_OFFSET == x && events->objectEvents[i].y + MAP_OFFSET == y)
            return FALSE;
    }
    for (i = 0; i < events->coordEventCount; i++)
    {
        if (events->coordEvents[i].x + MAP_OFFSET == x && events->coordEvents[i].y + MAP_OFFSET == y)
            return FALSE;
    }
    return TRUE;
}

// --- Rain: puddles --------------------------------------------------------------

static void Flood(void)
{
    s32 w = gMapHeader.mapLayout->width, h = gMapHeader.mapLayout->height;
    u32 placed = 0, tries, seed = Alive_MapSeed();
    s32 px = gSaveBlock1Ptr->pos.x + MAP_OFFSET, py = gSaveBlock1Ptr->pos.y + MAP_OFFSET;

    // A few where the player can see them, then the rest anywhere; the same
    // puddles all day.
    for (tries = 0; tries < 600 && placed < 10; tries++)
    {
        s32 x, y, i, j;
        bool32 ok = TRUE;
        seed = seed * 1103515245 + 12345;
        if (tries < 200 && placed < 3)
        {
            x = px - 7 + (seed >> 16) % 13;
            seed = seed * 1103515245 + 12345;
            y = py - 5 + (seed >> 16) % 9;
        }
        else
        {
            x = MAP_OFFSET + (seed >> 16) % max(w - 3, 1);
            seed = seed * 1103515245 + 12345;
            y = MAP_OFFSET + (seed >> 16) % max(h - 3, 1);
        }
        for (j = 0; j < 3 && ok; j++)
        {
            for (i = 0; i < 3 && ok; i++)
                ok = OpenGrass(x + i, y + j);
        }
        if (!ok)
            continue;
        for (j = 0; j < 3; j++)
        {
            for (i = 0; i < 3; i++)
                MapGridSetMetatileIdAt(x + i, y + j, PUDDLE_FIRST + j * 8 + i);
        }
        placed++;
    }
}

// --- Snow: a white blanket, footprints and drifts --------------------------------

static void SnowOver(void)
{
    s32 w = gMapHeader.mapLayout->width, h = gMapHeader.mapLayout->height, x, y;
    for (y = MAP_OFFSET; y < h + MAP_OFFSET; y++)
    {
        for (x = MAP_OFFSET; x < w + MAP_OFFSET; x++)
        {
            if (MapGridGetMetatileIdAt(x, y) == GRASS)
                MapGridSetMetatileIdAt(x, y, SNOWY_GRASS);
        }
    }
}

static void UpdateFootprints(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x = player->currentCoords.x, y = player->currentCoords.y;
    s16 px = sWeather.lastX, py = sWeather.lastY;
    u16 left;

    if (x == px && y == py)
        return;
    sWeather.lastX = x;
    sWeather.lastY = y;
    if (px == 0 && py == 0)
        return;
    left = MapGridGetMetatileIdAt(px, py);
    if (left != SNOWY_GRASS && left != GRASS)
        return;
    // Prints in the snow behind the player, and the green grass shows
    // through where the snow was kicked up.
    gFieldEffectArguments[0] = px;
    gFieldEffectArguments[1] = py;
    gFieldEffectArguments[2] = 149;
    gFieldEffectArguments[3] = 2;
    gFieldEffectArguments[4] = player->facingDirection;
    FieldEffectStart(FLDEFF_SAND_FOOTPRINTS);
    if (left == SNOWY_GRASS)
    {
        MapGridSetMetatileIdAt(px, py, GRASS);
        CurrentMapDrawMetatileAt(px, py);
    }
}

static void SpawnDrifts(void)
{
    u32 i, n = 0;
    sWeather.driftsSpawned = TRUE;
    for (i = 0; i < MAX_DRIFTS * 3 && n < MAX_DRIFTS; i++)
    {
        s16 x, y;
        if (!Alive_RoomForObjects(1) || !Alive_FindSpotOnScreen(3, &x, &y))
            break;
        // Only on open snow, never in a doorway or a narrow path.
        if (MapGridGetMetatileIdAt(x, y) != SNOWY_GRASS)
            continue;
        if (Alive_Spawn(LOCALID_ALIVE_CRITTER + n, OBJ_EVENT_GFX_SNOW_DRIFT, x, y, DIR_SOUTH) < OBJECT_EVENTS_COUNT)
            n++;
    }
}

bool32 AliveWeather_IsWeatherObject(struct ObjectEvent *obj)
{
    return obj->graphicsId == OBJ_EVENT_GFX_SNOW_DRIFT || obj->localId == LOCALID_ALIVE_WEATHER;
}

// The player walked into a drift: it scatters in a puff.
bool32 AliveWeather_OnBump(struct ObjectEvent *obj)
{
    if (obj->graphicsId != OBJ_EVENT_GFX_SNOW_DRIFT)
        return FALSE;
    gFieldEffectArguments[0] = obj->currentCoords.x;
    gFieldEffectArguments[1] = obj->currentCoords.y;
    gFieldEffectArguments[2] = 0x48;
    gFieldEffectArguments[3] = 1;
    FieldEffectStart(FLDEFF_ASH_PUFF);
    PlaySE(SE_M_GUST2);
    RemoveObjectEvent(obj);
    return TRUE;
}

// --- Dry spells: ponds dry up ---------------------------------------------------

static bool32 IsPondWater(s32 x, s32 y)
{
    u32 behavior = MapGridGetMetatileBehaviorAt(x, y);
    return MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) && !MetatileBehavior_IsWaterfall(behavior);
}

#define SEEN(i)    (sSeen[(i) / 8] & (1 << ((i) % 8)))
#define SET_SEEN(i) (sSeen[(i) / 8] |= (1 << ((i) % 8)))

// Dries every pond of at most maxTiles tiles that doesn't reach the map's
// edge (rivers and the sea go on). The middle of the biggest one is where
// something may turn up.
static void DryPonds(u32 maxTiles)
{
    s32 w = gMapHeader.mapLayout->width, h = gMapHeader.mapLayout->height, x, y;
    u32 best = 0;

    if (w * h > MAX_MAP_TILES)
        return;
    memset(sSeen, 0, (w * h + 7) / 8);
    sWeather.findX = sWeather.findY = 0;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            u32 head = 0, tail = 0, i;
            bool32 edge = FALSE;
            s32 sumX = 0, sumY = 0;

            if (SEEN(y * w + x) || !IsPondWater(x + MAP_OFFSET, y + MAP_OFFSET))
                continue;
            SET_SEEN(y * w + x);
            sQueue[tail++] = y * w + x;
            while (head < tail)
            {
                u32 t = sQueue[head++];
                s32 tx = t % w, ty = t / w, d;
                static const s8 sDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                sumX += tx;
                sumY += ty;
                if (tx == 0 || ty == 0 || tx == w - 1 || ty == h - 1)
                    edge = TRUE;
                for (d = 0; d < 4; d++)
                {
                    s32 nx = tx + sDirs[d][0], ny = ty + sDirs[d][1];
                    if (nx < 0 || ny < 0 || nx >= w || ny >= h || SEEN(ny * w + nx))
                        continue;
                    if (!IsPondWater(nx + MAP_OFFSET, ny + MAP_OFFSET))
                        continue;
                    SET_SEEN(ny * w + nx);
                    if (tail < MAX_MAP_TILES)
                        sQueue[tail++] = ny * w + nx;
                }
            }
            if (edge || tail > maxTiles || tail < 2)
                continue;
            for (i = 0; i < tail; i++)
                MapGridSetMetatileIdAt(sQueue[i] % w + MAP_OFFSET, sQueue[i] / w + MAP_OFFSET, DRY_SAND);
            if (tail > best)
            {
                best = tail;
                sWeather.findX = sumX / tail + MAP_OFFSET;
                sWeather.findY = sumY / tail + MAP_OFFSET;
            }
        }
    }
}

static void SpawnFind(void)
{
    struct AliveSave *save = &gSaveBlockExt.alive;
    u16 day = Alive_Day() + 1;

    sWeather.findSpawned = TRUE;
    if (sWeather.findX == 0)
        return;
    if (save->findDay != day)
    {
        save->findDay = day;
        save->findsToday = 0;
    }
    if (save->findsToday >= FINDS_PER_DAY || (Alive_MapSeed() % 3) == 0)
        return;
    Alive_Spawn(LOCALID_ALIVE_WEATHER, OBJ_EVENT_GFX_ITEM_BALL, sWeather.findX, sWeather.findY, DIR_SOUTH);
}

const u8 *AliveWeather_GetTalkScript(struct ObjectEvent *obj)
{
    extern const u8 Alive_EventScript_PondFind[];
    if (obj->localId != LOCALID_ALIVE_WEATHER)
        return NULL;
    return Alive_EventScript_PondFind;
}

// Script: VAR_0x8004 what was lying in the dry pond.
void AliveWeather_TakeFind(void)
{
    static const u16 sFinds[] = {ITEM_BIG_PEARL, ITEM_PEARL, ITEM_NUGGET, ITEM_STAR_PIECE, ITEM_DAMP_ROCK, ITEM_HEART_SCALE,
                                 ITEM_MYSTIC_WATER, ITEM_RARE_CANDY, ITEM_WATER_STONE, ITEM_SHOAL_SHELL};
    gSpecialVar_0x8004 = sFinds[Random() % ARRAY_COUNT(sFinds)];
    gSaveBlockExt.alive.findsToday++;
    Alive_Remove(LOCALID_ALIVE_WEATHER);
}

// --- Hooks ------------------------------------------------------------------------

void AliveWeather_OnMapTransition(void)
{
    memset(&sWeather, 0, sizeof(sWeather));
    if (gAliveShowcase.weather != 0)
        SetSavedWeather(gAliveShowcase.weather);
}

void AliveWeather_OnMapLoad(void)
{
    u32 weather = GetSavedWeather();

    sWeather.change = CHANGE_NONE;
    if (!Alive_IsOutdoors() || Alive_IsQuiet() || TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING))
        return;
    if (gAliveShowcase.scene == ALIVE_SHOW_DRY && Alive_IsRoute())
    {
        sWeather.change = CHANGE_DRY;
        DryPonds(40);
    }
    else if (IsRainy(weather) && Alive_IsRoute())
    {
        sWeather.change = CHANGE_FLOOD;
        Flood();
    }
    else if (weather == WEATHER_SNOW)
    {
        sWeather.change = CHANGE_SNOW;
        SnowOver();
    }
    else if (IsDry(weather) && Alive_IsRoute())
    {
        sWeather.change = CHANGE_DRY;
        DryPonds(40);
    }
}

void AliveWeather_Update(void)
{
    switch (sWeather.change)
    {
    case CHANGE_SNOW:
        UpdateFootprints();
        if (!sWeather.driftsSpawned)
            SpawnDrifts();
        break;
    case CHANGE_DRY:
        if (!sWeather.findSpawned)
            SpawnFind();
        break;
    }
}
