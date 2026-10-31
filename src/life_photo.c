#include "global.h"
#include "battle_environment.h"
#include "bg.h"
#include "decompress.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "life.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "wild_encounter.h"
#include "window.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/map_types.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/trainer_types.h"
#include "constants/wild_encounter.h"
#include "constants/songs.h"

// Photo safari (docs/life.md): the photo ranger at the Safari Zone entrance
// lends a camera with 12 shots. In the Safari Zone, wild Pokémon from the
// area roam the grass (and don't attack); R takes a photo of whatever is in
// front of the player. A photo is scored by how close and centred the
// Pokémon is, its pose, how rare it is, and how many of its kind are in the
// shot, and the best photo of each species goes in the album (R > ALBUM).
// After the last shot the ranger pays out for the safari's total score.

extern const u8 Life_EventScript_PhotoSnap[];

#define FILM 12
#define PHOTO_RANGE 5

#define TAG_FINDER 0x5C00

static const u8 sTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sRedColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};

static const u8 *const sPoseNames[] =
{
    [LIFE_POSE_NONE] = COMPOUND_STRING("Scenery"),
    [LIFE_POSE_LOOKING] = COMPOUND_STRING("Facing you"),
    [LIFE_POSE_MOVING] = COMPOUND_STRING("On the move"),
    [LIFE_POSE_SIDE] = COMPOUND_STRING("Side view"),
    [LIFE_POSE_BEHIND] = COMPOUND_STRING("From behind"),
};

static const u16 sPoseScores[] = {0, 300, 200, 100, 0};
static const u16 sDistanceScores[PHOTO_RANGE + 1] = {0, 350, 400, 300, 200, 100};

// The photo the player just took.
struct Shot
{
    u16 species;
    u16 score;
    u8 distance;
    s8 lateral;
    u8 pose;
    u8 rarity; // multiplier
    u8 group;  // others of its kind in the shot
    bool8 shiny;
    bool8 best;
};

static EWRAM_DATA struct Shot sShot = {0};
static EWRAM_DATA u8 sSpawnTimer = 0;

// --- State ---------------------------------------------------------------------

bool32 LifePhoto_IsActive(void)
{
    return gSaveBlock3Ptr->life.photoFilm != 0;
}

static bool32 IsSafariZoneMap(void)
{
    static const u16 sMaps[] =
    {
        MAP_SAFARI_ZONE_SOUTH, MAP_SAFARI_ZONE_SOUTHWEST, MAP_SAFARI_ZONE_SOUTHEAST,
        MAP_SAFARI_ZONE_NORTH, MAP_SAFARI_ZONE_NORTHWEST, MAP_SAFARI_ZONE_NORTHEAST,
    };
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMaps); i++)
    {
        if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(sMaps[i]) && gSaveBlock1Ptr->location.mapNum == MAP_NUM(sMaps[i]))
            return TRUE;
    }
    return FALSE;
}

static bool32 IsSafariRestHouse(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_SAFARI_ZONE_REST_HOUSE)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_SAFARI_ZONE_REST_HOUSE);
}

// The roaming Pokémon aren't in the map's object list (talking to one goes
// through field_control_avatar.c).
bool32 LifePhoto_IsPhotoMon(struct ObjectEvent *objectEvent)
{
    return LifePhoto_IsActive() && IsSafariZoneMap()
        && objectEvent->localId >= LOCALID_LIFE_PHOTO_MON && objectEvent->localId < LOCALID_LIFE_PHOTO_MON + LIFE_PHOTO_MONS;
}

static void EndSafari(void)
{
    u32 i;
    gSaveBlock3Ptr->life.photoFilm = 0;
    for (i = 0; i < LIFE_PHOTO_MONS; i++)
        RemoveObjectEventByLocalIdAndMap(LOCALID_LIFE_PHOTO_MON + i, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

// --- Roaming Pokémon -----------------------------------------------------------

static u16 RandomAreaSpecies(void)
{
    u32 headerId = GetCurrentMapWildMonHeaderId();
    const struct WildPokemonInfo *info;

    if (headerId == HEADER_NONE)
        return SPECIES_NONE;
    info = gWildMonHeaders[headerId].encounterTypes[GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND)].landMonsInfo;
    if (info == NULL)
        return SPECIES_NONE;
    return info->wildPokemon[Random() % NUM_LAND_MONS_ENCOUNTER_SLOTS].species;
}

static bool32 CanStandAt(s16 x, s16 y)
{
    u32 behavior;
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    return !MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) && !MetatileBehavior_IsWarpDoor(behavior)
        && MapGridGetElevationAt(x, y) == gObjectEvents[gPlayerAvatar.objectEventId].currentElevation;
}

static void Spawn(u16 graphicsId, s16 x, s16 y, u32 localId, u32 movementType)
{
    struct ObjectEventTemplate template = {0};
    template.localId = localId;
    template.graphicsId = graphicsId;
    template.kind = OBJ_KIND_NORMAL;
    template.x = x - MAP_OFFSET;
    template.y = y - MAP_OFFSET;
    template.elevation = MapGridGetElevationAt(x, y);
    template.movementType = movementType;
    template.movementRangeX = 3;
    template.movementRangeY = 3;
    template.trainerType = TRAINER_TYPE_NONE;
    SpawnSpecialObjectEvent(&template);
}

// For screenshots (tools/life_shots/photo.lua): a posed subject in front of
// the player.
static void SpawnShowcaseSubject(void)
{
    static const u8 sFaceMovements[] =
    {
        [DIR_SOUTH] = MOVEMENT_TYPE_FACE_DOWN, [DIR_NORTH] = MOVEMENT_TYPE_FACE_UP,
        [DIR_WEST] = MOVEMENT_TYPE_FACE_LEFT, [DIR_EAST] = MOVEMENT_TYPE_FACE_RIGHT,
    };
    static const u8 sOpposite[] = {[DIR_SOUTH] = DIR_NORTH, [DIR_NORTH] = DIR_SOUTH, [DIR_WEST] = DIR_EAST, [DIR_EAST] = DIR_WEST};
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 dir = player->facingDirection, facing = gLifeShowcase.photoFacing ? gLifeShowcase.photoFacing : sOpposite[dir];
    s16 fx = dir == DIR_WEST ? -1 : dir == DIR_EAST ? 1 : 0, fy = dir == DIR_NORTH ? -1 : dir == DIR_SOUTH ? 1 : 0;
    u32 i;

    for (i = 0; i < LIFE_PHOTO_MONS; i++)
        RemoveObjectEventByLocalIdAndMap(LOCALID_LIFE_PHOTO_MON + i, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    for (i = 0; i < max(gLifeShowcase.photoCount, 1) && i < LIFE_PHOTO_MONS; i++)
    {
        s16 side = (i + 1) / 2 * ((i & 1) ? 1 : -1);
        Spawn(OBJ_EVENT_MON + gLifeShowcase.photoSpecies,
              player->currentCoords.x + fx * gLifeShowcase.photoDistance + fy * side,
              player->currentCoords.y + fy * gLifeShowcase.photoDistance + fx * side,
              LOCALID_LIFE_PHOTO_MON + i, sFaceMovements[facing]);
    }
    gLifeShowcase.photoSpecies = SPECIES_NONE;
}

static void TrySpawn(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u16 species;
    u32 i, tries;
    s16 x, y;

    for (i = 0; i < LIFE_PHOTO_MONS; i++)
    {
        if (GetObjectEventIdByLocalIdAndMap(LOCALID_LIFE_PHOTO_MON + i, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup) == OBJECT_EVENTS_COUNT)
            break;
    }
    if (i == LIFE_PHOTO_MONS || (species = RandomAreaSpecies()) == SPECIES_NONE)
        return;
    for (tries = 0; tries < 16; tries++)
    {
        x = player->currentCoords.x - 7 + Random() % 15;
        y = player->currentCoords.y - 5 + Random() % 11;
        if (abs(x - player->currentCoords.x) + abs(y - player->currentCoords.y) >= 3 && CanStandAt(x, y))
            break;
    }
    if (tries == 16)
        return;
    Spawn(OBJ_EVENT_MON + species + ((Random() % 64) == 0 ? OBJ_EVENT_MON_SHINY : 0), x, y,
          LOCALID_LIFE_PHOTO_MON + i, MOVEMENT_TYPE_WANDER_AROUND);
}

// Called every overworld frame: keeps the Safari Zone stocked while the
// camera has film.
void LifePhoto_Update(void)
{
    if (!LifePhoto_IsActive() || !IsSafariZoneMap() || ArePlayerFieldControlsLocked())
        return;
    if (gLifeShowcase.photoSpecies != SPECIES_NONE)
    {
        SpawnShowcaseSubject();
        sSpawnTimer = 0;
        return;
    }
    if (++sSpawnTimer < 45)
        return;
    sSpawnTimer = 0;
    TrySpawn();
}

void LifePhoto_OnMapTransition(void)
{
    sSpawnTimer = 40;
    // Leaving the Safari Zone ends the safari; the ranger pays for what was taken.
    if (LifePhoto_IsActive() && !IsSafariZoneMap() && !IsSafariRestHouse())
        gSaveBlock3Ptr->life.photoFilm = 0;
}

// --- Taking a photo ------------------------------------------------------------

static u32 RarityOf(u16 species)
{
    u32 rate = gSpeciesInfo[species].catchRate;
    if (rate <= 45)
        return 3;
    if (rate <= 120)
        return 2;
    return 1;
}

static u32 PoseOf(struct ObjectEvent *mon, u32 playerDir)
{
    static const u8 sOpposite[] = {[DIR_SOUTH] = DIR_NORTH, [DIR_NORTH] = DIR_SOUTH, [DIR_WEST] = DIR_EAST, [DIR_EAST] = DIR_WEST};
    if (mon->singleMovementActive || mon->heldMovementActive)
        return LIFE_POSE_MOVING;
    if (mon->facingDirection == sOpposite[playerDir])
        return LIFE_POSE_LOOKING;
    if (mon->facingDirection == playerDir)
        return LIFE_POSE_BEHIND;
    return LIFE_POSE_SIDE;
}

// Fills sShot with the best subject in front of the player.
static void FrameShot(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 dir = player->facingDirection, i, best = OBJECT_EVENTS_COUNT, bestScore = 0;
    s16 fx = 0, fy = 0;

    memset(&sShot, 0, sizeof(sShot));
    switch (dir)
    {
    case DIR_NORTH: fy = -1; break;
    case DIR_SOUTH: fy = 1; break;
    case DIR_WEST: fx = -1; break;
    default: fx = 1; break;
    }
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *mon = &gObjectEvents[i];
        s32 dx, dy, forward, lateral, score;
        if (!mon->active || mon->localId < LOCALID_LIFE_PHOTO_MON || mon->localId >= LOCALID_LIFE_PHOTO_MON + LIFE_PHOTO_MONS)
            continue;
        dx = mon->currentCoords.x - player->currentCoords.x;
        dy = mon->currentCoords.y - player->currentCoords.y;
        forward = dx * fx + dy * fy;
        lateral = fx != 0 ? dy : dx;
        if (forward < 1 || forward > PHOTO_RANGE || abs(lateral) > 1 + forward / 2)
            continue;
        score = sDistanceScores[forward] + (lateral == 0 ? 200 : abs(lateral) == 1 ? 100 : 0) + sPoseScores[PoseOf(mon, dir)];
        if (score > bestScore)
        {
            bestScore = score;
            best = i;
            sShot.distance = forward;
            sShot.lateral = lateral;
        }
    }
    if (best == OBJECT_EVENTS_COUNT)
        return;

    sShot.species = gObjectEvents[best].graphicsId & (OBJ_EVENT_MON - 1) & ~OBJ_EVENT_MON_SHINY;
    sShot.shiny = (gObjectEvents[best].graphicsId & OBJ_EVENT_MON_SHINY) != 0;
    sShot.pose = PoseOf(&gObjectEvents[best], dir);
    sShot.rarity = RarityOf(sShot.species);
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *mon = &gObjectEvents[i];
        s32 dx, dy, forward;
        if (i == best || !mon->active || mon->localId < LOCALID_LIFE_PHOTO_MON || mon->localId >= LOCALID_LIFE_PHOTO_MON + LIFE_PHOTO_MONS
         || (mon->graphicsId & (OBJ_EVENT_MON - 1) & ~OBJ_EVENT_MON_SHINY) != sShot.species)
            continue;
        dx = mon->currentCoords.x - player->currentCoords.x;
        dy = mon->currentCoords.y - player->currentCoords.y;
        forward = dx * fx + dy * fy;
        if (forward >= 1 && forward <= PHOTO_RANGE)
            sShot.group++;
    }
    sShot.score = (bestScore + sShot.group * 150) * sShot.rarity * (sShot.shiny ? 2 : 1);
}

static void SaveToAlbum(void)
{
    struct LifePhoto *album = gSaveBlock3Ptr->life.album;
    u32 i, slot = LIFE_ALBUM_SIZE, lowest = 0;

    for (i = 0; i < LIFE_ALBUM_SIZE; i++)
    {
        if (album[i].species == sShot.species)
        {
            slot = i;
            break;
        }
    }
    if (slot == LIFE_ALBUM_SIZE)
    {
        // A new species: an empty page, or the page with the lowest score.
        for (i = 0; i < LIFE_ALBUM_SIZE; i++)
        {
            if (album[i].species == SPECIES_NONE || album[i].score < album[lowest].score)
                lowest = i;
            if (album[i].species == SPECIES_NONE)
                break;
        }
        slot = lowest;
        album[slot].species = SPECIES_NONE;
    }
    if (album[slot].species == SPECIES_NONE || sShot.score > album[slot].score)
    {
        album[slot].species = sShot.species;
        album[slot].score = sShot.score;
        album[slot].pose = sShot.pose;
        album[slot].shiny = sShot.shiny;
        sShot.best = TRUE;
    }
}

// R in the overworld: takes a photo if the camera has film.
bool32 LifePhoto_TrySnap(void)
{
    if (!LifePhoto_IsActive() || !IsSafariZoneMap())
        return FALSE;
    FrameShot();
    gSaveBlock3Ptr->life.photoFilm--;
    if (sShot.species != SPECIES_NONE)
    {
        gSaveBlock3Ptr->life.photoTotal += sShot.score;
        SaveToAlbum();
    }
    ScriptContext_SetupScript(Life_EventScript_PhotoSnap);
    return TRUE;
}

// --- The photo -----------------------------------------------------------------

static const u32 sFinderGfx[] = INCBIN_U32("graphics/life/photo_finder.4bpp");
static const u16 sFinderPal[] = INCBIN_U16("graphics/life/photo_finder.gbapal");

static const struct BgTemplate sBgTemplates[] =
{
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .priority = 0},
    {.bg = 1, .charBaseIndex = 2, .mapBaseIndex = 26, .screenSize = 1, .priority = 3},
};

static const struct WindowTemplate sWindowTemplates[] =
{
    {.bg = 0, .tilemapLeft = 18, .tilemapTop = 1, .width = 11, .height = 14, .paletteNum = 15, .baseBlock = 0x001},
    {.bg = 0, .tilemapLeft = 1, .tilemapTop = 17, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 0x0A0},
    DUMMY_WIN_TEMPLATE
};

static const struct OamData sFinderOam = {.size = SPRITE_SIZE(16x16), .shape = SPRITE_SHAPE(16x16), .priority = 0};
static const struct SpriteTemplate sFinderTemplate =
{
    .tileTag = TAG_FINDER, .paletteTag = TAG_FINDER, .oam = &sFinderOam,
    .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

// By distance (1-5), facing left then turned right (side views).
#define PHOTO_SCALE(name, scale) \
    static const union AffineAnimCmd name[] = {AFFINEANIMCMD_FRAME(scale, scale, 0, 0), AFFINEANIMCMD_END}; \
    static const union AffineAnimCmd name##Flip[] = {AFFINEANIMCMD_FRAME(-(scale), scale, 0, 0), AFFINEANIMCMD_END};
PHOTO_SCALE(sAffineD1, 0x200)
PHOTO_SCALE(sAffineD2, 0x1A0)
PHOTO_SCALE(sAffineD3, 0x140)
PHOTO_SCALE(sAffineD4, 0x100)
PHOTO_SCALE(sAffineD5, 0xC0)
static const union AffineAnimCmd *const sDistanceAffineAnims[] =
{
    sAffineD1, sAffineD1, sAffineD2, sAffineD3, sAffineD4, sAffineD5,
    sAffineD1Flip, sAffineD1Flip, sAffineD2Flip, sAffineD3Flip, sAffineD4Flip, sAffineD5Flip,
};

#define PHOTO_PICS 3

struct PhotoScene
{
    u8 monSpriteIds[PHOTO_PICS]; // the subject, then others of its kind behind it
};

static EWRAM_DATA struct PhotoScene *sScene = NULL;

static void Task_Photo(u8 taskId);

static void PrintPhotoInfo(void)
{
    u8 *end;
    u32 y = 1;

    FillWindowPixelBuffer(0, PIXEL_FILL(1));
    FillWindowPixelBuffer(1, PIXEL_FILL(1));
    if (sShot.species == SPECIES_NONE)
    {
        AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sTextColors, TEXT_SKIP_DRAW, COMPOUND_STRING("No Pokémon\nin this shot."));
    }
    else
    {
        AddTextPrinterParameterized3(0, FONT_NORMAL, 2, y, sTextColors, TEXT_SKIP_DRAW, GetSpeciesName(sShot.species));
        y += 16;
        ConvertIntToDecimalStringN(gStringVar1, sDistanceScores[sShot.distance], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, sShot.distance <= 2 ? COMPOUND_STRING("Close-up +{STR_VAR_1}") : COMPOUND_STRING("Distance +{STR_VAR_1}"));
        AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
        y += 11;
        if (sShot.lateral == 0)
            AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sTextColors, TEXT_SKIP_DRAW, COMPOUND_STRING("Centered +200"));
        else if (abs(sShot.lateral) == 1)
            AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sTextColors, TEXT_SKIP_DRAW, COMPOUND_STRING("Off-center +100"));
        else
            AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sTextColors, TEXT_SKIP_DRAW, COMPOUND_STRING("Edge of shot"));
        y += 11;
        end = StringCopy(gStringVar4, sPoseNames[sShot.pose]);
        *end++ = CHAR_SPACE;
        *end++ = CHAR_PLUS;
        ConvertIntToDecimalStringN(end, sPoseScores[sShot.pose], STR_CONV_MODE_LEFT_ALIGN, 3);
        AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
        y += 11;
        if (sShot.group != 0)
        {
            ConvertIntToDecimalStringN(gStringVar1, sShot.group * 150, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Group +{STR_VAR_1}"));
            AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
            y += 11;
        }
        if (sShot.rarity > 1)
        {
            AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sRedColors, TEXT_SKIP_DRAW,
                                         sShot.rarity == 3 ? COMPOUND_STRING("Rare! x3") : COMPOUND_STRING("Uncommon x2"));
            y += 11;
        }
        if (sShot.shiny)
        {
            AddTextPrinterParameterized3(0, FONT_SMALL, 2, y, sRedColors, TEXT_SKIP_DRAW, COMPOUND_STRING("Shiny! x2"));
            y += 11;
        }
        ConvertIntToDecimalStringN(gStringVar1, sShot.score, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("SCORE {STR_VAR_1}"));
        AddTextPrinterParameterized3(0, FONT_NORMAL, 2, 92, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
        if (sShot.best)
            AddTextPrinterParameterized3(0, FONT_SMALL, 2, 79, sRedColors, TEXT_SKIP_DRAW, COMPOUND_STRING("New best photo!"));
    }
    ConvertIntToDecimalStringN(gStringVar1, gSaveBlock3Ptr->life.photoFilm, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Shots left: {STR_VAR_1}"));
    AddTextPrinterParameterized3(1, FONT_NORMAL, 2, 0, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
    CopyWindowToVram(0, COPYWIN_FULL);
    CopyWindowToVram(1, COPYWIN_FULL);
}

static void CreatePhotoSprites(void)
{
    static const s8 sCorners[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
    struct SpriteSheet sheet = {sFinderGfx, sizeof(sFinderGfx), TAG_FINDER};
    struct SpritePalette pal = {sFinderPal, TAG_FINDER};
    u32 i;

    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&pal);
    // The viewfinder over the photo area (the left 17 tiles).
    for (i = 0; i < 4; i++)
    {
        u8 spriteId = CreateSprite(&sFinderTemplate, 68 + sCorners[i][0] * 56, 64 + sCorners[i][1] * 48, 0);
        gSprites[spriteId].oam.matrixNum = (sCorners[i][0] > 0 ? ST_OAM_HFLIP : 0) | (sCorners[i][1] > 0 ? ST_OAM_VFLIP : 0);
    }
    for (i = 0; i < PHOTO_PICS; i++)
        sScene->monSpriteIds[i] = MAX_SPRITES;
    if (sShot.species == SPECIES_NONE)
        return;
    // Close shots are big; the Pokémon sits where it stood in the frame,
    // with the rest of its group a step behind on either side. Seen from
    // behind it's the back picture; side on, turned.
    for (i = 0; i <= min(sShot.group, PHOTO_PICS - 1); i++)
    {
        static const s8 sGroupX[PHOTO_PICS] = {0, -40, 40};
        struct Sprite *mon;
        u32 distance = min(sShot.distance + (i != 0), PHOTO_RANGE);
        s16 x = 68 + sShot.lateral * 40 / max(sShot.distance, 1) + sGroupX[i];
        u8 spriteId = CreateMonPicSprite_Affine(sShot.species, sShot.shiny, 0, sShot.pose == LIFE_POSE_BEHIND ? MON_PIC_AFFINE_BACK : MON_PIC_AFFINE_FRONT,
                                                x, i == 0 ? 64 : 56, 15, TAG_NONE);
        sScene->monSpriteIds[i] = spriteId;
        mon = &gSprites[spriteId];
        mon->oam.priority = i == 0 ? 1 : 2;
        mon->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
        CalcCenterToCornerVec(mon, mon->oam.shape, mon->oam.size, mon->oam.affineMode);
        mon->affineAnims = sDistanceAffineAnims;
        InitSpriteAffineAnim(mon);
        StartSpriteAffineAnim(mon, distance + ((sShot.pose == LIFE_POSE_SIDE || sShot.pose == LIFE_POSE_MOVING) ? 6 : 0));
    }
}

static void VBlankCB_Photo(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Photo(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void CB2_InitPhoto(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        ScanlineEffect_Stop();
        ResetPaletteFade();
        ResetSpriteData();
        ResetAllPicSprites();
        FreeAllSpritePalettes();
        ResetTasks();
        gMain.state++;
        break;
    case 1:
        ResetVramOamAndBgCntRegs();
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ResetAllBgsCoordinates();
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        LoadUserWindowBorderGfx(0, 0x200, BG_PLTT_ID(14));
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 2:
        DecompressDataWithHeaderVram(gBattleEnvironmentInfo[BATTLE_ENVIRONMENT_GRASS].background.tileset, (void *)BG_CHAR_ADDR(2));
        DecompressDataWithHeaderVram(gBattleEnvironmentInfo[BATTLE_ENVIRONMENT_GRASS].background.tilemap, (void *)BG_SCREEN_ADDR(26));
        LoadPalette(gBattleEnvironmentInfo[BATTLE_ENVIRONMENT_GRASS].palette, BG_PLTT_ID(2), 3 * PLTT_SIZE_4BPP);
        // The far platform sits under the subject.
        SetGpuReg(REG_OFFSET_BG1HOFS, 108);
        SetGpuReg(REG_OFFSET_BG1VOFS, 0);
        gMain.state++;
        break;
    case 3:
        CreatePhotoSprites();
        DrawStdFrameWithCustomTileAndPalette(0, FALSE, 0x200, 14);
        DrawStdFrameWithCustomTileAndPalette(1, FALSE, 0x200, 14);
        PutWindowTilemap(0);
        PutWindowTilemap(1);
        PrintPhotoInfo();
        ScheduleBgCopyTilemapToVram(0);
        gMain.state++;
        break;
    case 4:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        ShowBg(0);
        ShowBg(1);
        // The shutter: in from white.
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_WHITE);
        SetVBlankCallback(VBlankCB_Photo);
        CreateTask(Task_Photo, 0);
        SetMainCallback2(CB2_Photo);
        break;
    }
}

static void Task_PhotoExit(u8 taskId)
{
    u32 i;

    if (gPaletteFade.active)
        return;
    for (i = 0; i < PHOTO_PICS; i++)
    {
        if (sScene->monSpriteIds[i] != MAX_SPRITES)
            FreeAndDestroyMonPicSprite(sScene->monSpriteIds[i]);
    }
    FreeAllWindowBuffers();
    Free(sScene);
    sScene = NULL;
    DestroyTask(taskId);
    // Out of film: the script tells the player to go back to the ranger.
    gSpecialVar_Result = gSaveBlock3Ptr->life.photoFilm;
    if (gSaveBlock3Ptr->life.photoFilm == 0)
        EndSafari();
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_Photo(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_PhotoExit;
    }
}

// Script: after the shutter sound.
void LifePhoto_ShowPhoto(void)
{
    sScene = AllocZeroed(sizeof(*sScene));
    gMain.state = 0;
    SetMainCallback2(CB2_InitPhoto);
}

// --- The ranger and the album -------------------------------------------------

// The photo ranger. VAR_RESULT: LIFE_PHOTO_RANGER_OFFER, _STILL_OUT (film
// left in STR_VAR_1) or _PAYOUT (score in STR_VAR_1, prize in VAR_0x8005).
void LifePhoto_RangerState(void)
{
    struct LifeSaveData *life = &gSaveBlock3Ptr->life;

    if (life->photoFilm != 0)
    {
        ConvertIntToDecimalStringN(gStringVar1, life->photoFilm, STR_CONV_MODE_LEFT_ALIGN, 2);
        gSpecialVar_Result = LIFE_PHOTO_RANGER_STILL_OUT;
    }
    else if (life->photoTotal != 0)
    {
        ConvertIntToDecimalStringN(gStringVar1, life->photoTotal, STR_CONV_MODE_LEFT_ALIGN, 5);
        if (life->photoTotal >= 12000)
            gSpecialVar_0x8005 = ITEM_NUGGET;
        else if (life->photoTotal >= 7000)
            gSpecialVar_0x8005 = ITEM_ULTRA_BALL;
        else if (life->photoTotal >= 3500)
            gSpecialVar_0x8005 = ITEM_GREAT_BALL;
        else
            gSpecialVar_0x8005 = ITEM_POKE_BALL;
        life->photoTotal = 0;
        gSpecialVar_Result = LIFE_PHOTO_RANGER_PAYOUT;
    }
    else
    {
        gSpecialVar_Result = LIFE_PHOTO_RANGER_OFFER;
    }
}

void LifePhoto_Start(void)
{
    gSaveBlock3Ptr->life.photoFilm = FILM;
    gSaveBlock3Ptr->life.photoTotal = 0;
}

static u32 AlbumCount(void)
{
    u32 i, n = 0;
    for (i = 0; i < LIFE_ALBUM_SIZE; i++)
        n += gSaveBlock3Ptr->life.album[i].species != SPECIES_NONE;
    return n;
}

static const struct LifePhoto *AlbumEntry(u32 index)
{
    u32 i;
    for (i = 0; i < LIFE_ALBUM_SIZE; i++)
    {
        if (gSaveBlock3Ptr->life.album[i].species != SPECIES_NONE && index-- == 0)
            return &gSaveBlock3Ptr->life.album[i];
    }
    return NULL;
}

static void AlbumLine(u16 index, u8 *dest)
{
    const struct LifePhoto *photo = AlbumEntry(index);
    u8 *end = StringCopy(dest, GetSpeciesName(photo->species));
    end = StringAppend(end, COMPOUND_STRING("  "));
    ConvertIntToDecimalStringN(end + StringLength(end), photo->score, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void AlbumDetail(u16 index, u8 *dest)
{
    const struct LifePhoto *photo = AlbumEntry(index);
    u8 *end = StringCopy(dest, sPoseNames[photo->pose]);
    if (photo->shiny)
        StringAppend(end, COMPOUND_STRING(", shiny!"));
}

// R > ALBUM.
void LifePhoto_ShowAlbum(void)
{
    WorldBoard_Show(COMPOUND_STRING("PHOTO ALBUM: best shots"), AlbumCount(), AlbumLine, AlbumDetail);
}
