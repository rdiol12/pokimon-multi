#include "global.h"
#include "alive.h"
#include "overworld.h"
#include "trig.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "main.h"
#include "metatile_behavior.h"
#include "random.h"
#include "sound.h"
#include "sprite.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"
#include "constants/songs.h"
#include "constants/weather.h"

// Ambient life (docs/alive.md): birds pecking about that take off when
// you come close, fish jumping in the water and Pokémon flying over. None
// of them battle; they're just there.

#define FISH_SLOT (ALIVE_CRITTERS - 1) // the last critter slot is a jumping fish

enum
{
    CRITTER_NONE,
    CRITTER_IDLE,
    CRITTER_FLEE,   // hopping away from the player
    CRITTER_FLYING, // up and away
    CRITTER_FISH,
};

static EWRAM_DATA struct
{
    u8 state[ALIVE_CRITTERS];
    u8 timer[ALIVE_CRITTERS];
    s8 flyDx[ALIVE_CRITTERS];
    u8 spawned;
    u16 fishTimer;
    u16 flyoverTimer;
} sAmbient = {0};

static bool32 Raining(void)
{
    switch (GetCurrentWeather())
    {
    case WEATHER_RAIN:
    case WEATHER_RAIN_THUNDERSTORM:
    case WEATHER_DOWNPOUR:
    case WEATHER_SANDSTORM:
    case WEATHER_SNOW:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 WaterOnScreen(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 x, y;
    for (y = player->currentCoords.y - 4; y <= player->currentCoords.y + 4; y += 2)
    {
        for (x = player->currentCoords.x - 7; x <= player->currentCoords.x + 7; x += 2)
        {
            if (MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y)))
                return TRUE;
        }
    }
    return FALSE;
}

static u16 CritterSpecies(bool32 coast)
{
    static const u16 sLand[] = {SPECIES_TAILLOW, SPECIES_TAILLOW, SPECIES_PIDGEY, SPECIES_STARLY, SPECIES_PIDOVE};
    static const u16 sMountain[] = {SPECIES_SWABLU, SPECIES_TAILLOW};
    if (Alive_IsNight())
        return SPECIES_HOOTHOOT;
    if (coast)
        return SPECIES_WINGULL;
    if (gMapHeader.weather == WEATHER_VOLCANIC_ASH)
        return sMountain[Random() % ARRAY_COUNT(sMountain)];
    return sLand[Random() % ARRAY_COUNT(sLand)];
}

static void SpawnCritters(void)
{
    u32 i, count;
    bool32 coast;

    sAmbient.spawned = TRUE;
    if (!Alive_IsOutdoors() || Raining() || Alive_IsQuiet() || TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING))
        return;
    coast = WaterOnScreen();
    count = (gAliveShowcase.scene == ALIVE_SHOW_CRITTERS) ? 3 : Random() % 4;
    if (Alive_IsNight())
        count = min(count, 1);
    for (i = 0; i < count && i < FISH_SLOT; i++)
    {
        s16 x, y;
        if (!Alive_RoomForObjects(1) || !Alive_FindSpotOnScreen(4, &x, &y))
            break;
        if (Alive_Spawn(LOCALID_ALIVE_CRITTER + i, Alive_MonGfx(CritterSpecies(coast)), x, y, Random() % 4 + 1) >= OBJECT_EVENTS_COUNT)
            break;
        sAmbient.state[i] = CRITTER_IDLE;
        sAmbient.timer[i] = Random() % 60;
    }
}

bool32 AliveAmbient_IsCritter(struct ObjectEvent *obj)
{
    return obj->localId >= LOCALID_ALIVE_CRITTER && obj->localId < LOCALID_ALIVE_CRITTER + ALIVE_CRITTERS;
}

static void UpdateCritter(u32 i)
{
    struct ObjectEvent *obj = Alive_Obj(LOCALID_ALIVE_CRITTER + i);
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Sprite *sprite;
    u32 d;

    if (obj == NULL)
    {
        sAmbient.state[i] = CRITTER_NONE;
        return;
    }
    sprite = &gSprites[obj->spriteId];
    switch (sAmbient.state[i])
    {
    case CRITTER_IDLE:
        d = Alive_PlayerDistance(obj->currentCoords.x, obj->currentCoords.y);
        if (d <= 2 && !Alive_Busy(obj))
        {
            // Startled: a hop away, then off it flies.
            s16 dx = obj->currentCoords.x - player->currentCoords.x;
            s16 dy = obj->currentCoords.y - player->currentCoords.y;
            u32 dir = abs(dx) >= abs(dy) ? (dx >= 0 ? DIR_EAST : DIR_WEST) : (dy >= 0 ? DIR_SOUTH : DIR_NORTH);
            Alive_Do(obj, GetJumpInPlaceMovementAction(dir));
            sAmbient.flyDx[i] = (dir == DIR_WEST) ? -2 : (dir == DIR_EAST ? 2 : ((Random() & 1) ? 1 : -1));
            sAmbient.state[i] = CRITTER_FLEE;
            sAmbient.timer[i] = 0;
            if (obj->graphicsId - OBJ_EVENT_MON == SPECIES_WINGULL || Random() % 3 == 0)
                PlayCry_ByMode(obj->graphicsId - OBJ_EVENT_MON, 0, CRY_MODE_ENCOUNTER);
            return;
        }
        if (Alive_Busy(obj))
            return;
        if (sAmbient.timer[i] != 0)
        {
            sAmbient.timer[i]--;
            return;
        }
        // Pecking about.
        if (Random() % 3 == 0)
            Alive_Do(obj, GetJumpInPlaceMovementAction(obj->facingDirection));
        else
            Alive_Do(obj, GetFaceDirectionMovementAction(Random() % 4 + 1));
        sAmbient.timer[i] = 30 + Random() % 90;
        break;
    case CRITTER_FLEE:
        if (Alive_Busy(obj))
            return;
        sAmbient.state[i] = CRITTER_FLYING;
        sAmbient.timer[i] = 0;
        obj->disableCoveringGroundEffects = TRUE;
        break;
    case CRITTER_FLYING:
        // Up and away over the rooftops.
        sAmbient.timer[i]++;
        sprite->y2 -= 3;
        sprite->x2 += sAmbient.flyDx[i];
        sprite->subpriority = 0;
        sprite->oam.priority = 0;
        if (sAmbient.timer[i] > 50)
        {
            Alive_Remove(obj->localId);
            sAmbient.state[i] = CRITTER_NONE;
        }
        break;
    }
}

// --- Fish jumping -----------------------------------------------------------

static void UpdateFish(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *fish;
    u32 tries;
    static const u16 sFish[] = {SPECIES_MAGIKARP, SPECIES_MAGIKARP, SPECIES_LUVDISC, SPECIES_GOLDEEN, SPECIES_FINIZEN};

    if (sAmbient.state[FISH_SLOT] == CRITTER_FISH)
    {
        fish = Alive_Obj(LOCALID_ALIVE_CRITTER + FISH_SLOT);
        if (fish == NULL || (++sAmbient.timer[FISH_SLOT] > 36 && !Alive_Busy(fish)))
        {
            if (fish != NULL)
            {
                gFieldEffectArguments[0] = fish->currentCoords.x;
                gFieldEffectArguments[1] = fish->currentCoords.y;
                gFieldEffectArguments[2] = fish->previousElevation;
                gFieldEffectArguments[3] = 1;
                FieldEffectStart(FLDEFF_JUMP_SMALL_SPLASH);
                PlaySE(SE_PUDDLE);
            }
            Alive_Remove(LOCALID_ALIVE_CRITTER + FISH_SLOT);
            sAmbient.state[FISH_SLOT] = CRITTER_NONE;
        }
        return;
    }
    if (sAmbient.fishTimer != 0)
    {
        sAmbient.fishTimer--;
        return;
    }
    sAmbient.fishTimer = 180 + Random() % 420;
    if (!Alive_IsOutdoors() || !Alive_RoomForObjects(1))
        return;
    for (tries = 0; tries < 30; tries++)
    {
        s16 x = player->currentCoords.x - 7 + Random() % 15;
        s16 y = player->currentCoords.y - 4 + Random() % 9;
        u32 behavior = MapGridGetMetatileBehaviorAt(x, y);
        if (!MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) || MetatileBehavior_IsWaterfall(behavior)
         || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT || Alive_PlayerDistance(x, y) < 3)
            continue;
        // Up out of the water with a splash, and back in.
        gFieldEffectArguments[0] = x;
        gFieldEffectArguments[1] = y;
        gFieldEffectArguments[2] = 0;
        gFieldEffectArguments[3] = 1;
        FieldEffectStart(FLDEFF_JUMP_SMALL_SPLASH);
        if (Alive_Spawn(LOCALID_ALIVE_CRITTER + FISH_SLOT, Alive_MonGfx(sFish[Random() % ARRAY_COUNT(sFish)]), x, y,
                        (Random() & 1) ? DIR_EAST : DIR_WEST) >= OBJECT_EVENTS_COUNT)
            return;
        fish = Alive_Obj(LOCALID_ALIVE_CRITTER + FISH_SLOT);
        fish->disableCoveringGroundEffects = TRUE;
        Alive_Do(fish, GetJumpInPlaceMovementAction(fish->facingDirection));
        sAmbient.state[FISH_SLOT] = CRITTER_FISH;
        sAmbient.timer[FISH_SLOT] = 0;
        return;
    }
}

// --- Flyovers ---------------------------------------------------------------

#define sDx    data[0]
#define sTimer data[1]

static void SpriteCB_Flyover(struct Sprite *sprite)
{
    sprite->x += sprite->sDx;
    sprite->sTimer++;
    sprite->y2 = Sin((sprite->sTimer * 6) & 0xFF, 6);
    if (sprite->x < -40 || sprite->x > DISPLAY_WIDTH + 40 || gMain.callback2 != CB2_Overworld)
        DestroySprite(sprite);
}

static void StartFlyover(void)
{
    static const u16 sDay[] = {SPECIES_WINGULL, SPECIES_PELIPPER, SPECIES_SWELLOW, SPECIES_TAILLOW, SPECIES_SKARMORY,
                               SPECIES_ALTARIA, SPECIES_TROPIUS, SPECIES_BEAUTIFLY, SPECIES_MASQUERAIN};
    static const u16 sNight[] = {SPECIES_ZUBAT, SPECIES_GOLBAT, SPECIES_HOOTHOOT, SPECIES_NOCTOWL, SPECIES_MURKROW};
    u16 species = Alive_IsNight() ? sNight[Random() % ARRAY_COUNT(sNight)] : sDay[Random() % ARRAY_COUNT(sDay)];
    bool32 east = Random() & 1;
    s16 y = 24 + Random() % 40;
    u8 spriteId = CreateObjectGraphicsSprite(Alive_MonGfx(species), SpriteCB_Flyover, east ? -32 : DISPLAY_WIDTH + 32, y, 0);

    if (spriteId == MAX_SPRITES)
        return;
    gSprites[spriteId].sDx = east ? 2 : -2;
    gSprites[spriteId].oam.priority = 0;
    StartSpriteAnim(&gSprites[spriteId], east ? ANIM_STD_GO_EAST : ANIM_STD_GO_WEST); // flapping the way it flies
    if (east)
        gSprites[spriteId].hFlip = FALSE;
    if (Random() % 2)
        PlayCry_ByMode(species, 0, CRY_MODE_ENCOUNTER);
}

static void UpdateFlyover(void)
{
    if (sAmbient.flyoverTimer != 0)
    {
        sAmbient.flyoverTimer--;
        return;
    }
    sAmbient.flyoverTimer = 1200 + Random() % 1800;
    if (Alive_IsOutdoors() && !Raining() && CountFreePaletteSlots() >= 3)
        StartFlyover();
}

#undef sDx
#undef sTimer

void AliveAmbient_OnMapTransition(void)
{
    memset(&sAmbient, 0, sizeof(sAmbient));
    sAmbient.fishTimer = 120 + Random() % 300;
    sAmbient.flyoverTimer = 600 + Random() % 1200;
}

void AliveAmbient_Update(void)
{
    u32 i;

    if (Alive_IsQuiet())
        return;
    if (gAliveShowcase.scene == ALIVE_SHOW_CRITTERS)
    {
        sAmbient.spawned = FALSE;
        sAmbient.flyoverTimer = 0;
        sAmbient.fishTimer = 0;
    }
    if (!sAmbient.spawned)
    {
        SpawnCritters();
        if (gAliveShowcase.scene == ALIVE_SHOW_CRITTERS)
        {
            StartFlyover();
            sAmbient.flyoverTimer = 300;
            gAliveShowcase.scene = ALIVE_SHOW_NONE;
        }
    }
    for (i = 0; i < FISH_SLOT; i++)
    {
        if (sAmbient.state[i] != CRITTER_NONE)
            UpdateCritter(i);
    }
    UpdateFish();
    UpdateFlyover();
}
