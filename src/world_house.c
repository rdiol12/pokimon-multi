#include "global.h"
#include "achievements.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "main.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "world.h"
#include "netplay.h"
#include "constants/map_groups.h"
#include "constants/songs.h"

// The player's house on Starfall Isle. The room (data/layouts/
// StarfallIsle_PlayerHouse, tools/world_maps/make_house.py) is empty; the
// player picks furniture from the catalog on the wall and places it on the
// tile in front of them with A. A on a placed piece offers to put it away.
//
// Furniture is drawn as sprites anchored to the map (original art:
// graphics/world/furniture/, tools/world_shots/draw_furniture.py) and
// blocks its footprint tiles. Saved in houseDecor as (type, x | y << 4)
// pairs, up to HOUSE_MAX_PIECES pieces.

extern const u8 StarfallIsle_PlayerHouse_EventScript_PutAway[];
extern const u8 StarfallIsle_PlayerHouse_EventScript_CantPlace[];
extern const u8 StarfallIsle_PlayerHouse_EventScript_Placed[];

#define HOUSE_MAX_PIECES (WORLD_HOUSE_SLOTS / 2)
#define HOMEOWNER_PIECES 5
#define TAG_FURNITURE_PAL 0x5750
#define TAG_FURNITURE     0x5751 // + type

// Where furniture can stand (map coordinates without MAP_OFFSET): from the
// baseboard under the wall down to the row above the door.
#define ROOM_LEFT   0
#define ROOM_RIGHT  12
#define ROOM_TOP    2
#define ROOM_BOTTOM 8

struct Furniture
{
    const u8 *name;
    const u32 *gfx;
    u32 gfxSize;
    u8 width, height;       // sprite size in pixels
    u8 tilesWide, tilesDeep; // footprint, from the bottom-left tile
};

static const u32 sPlantGfx[] = INCBIN_U32("graphics/world/furniture/plant.4bpp");
static const u32 sBookcaseGfx[] = INCBIN_U32("graphics/world/furniture/bookcase.4bpp");
static const u32 sTableGfx[] = INCBIN_U32("graphics/world/furniture/table.4bpp");
static const u32 sStoolGfx[] = INCBIN_U32("graphics/world/furniture/stool.4bpp");
static const u32 sBedGfx[] = INCBIN_U32("graphics/world/furniture/bed.4bpp");
static const u32 sTvGfx[] = INCBIN_U32("graphics/world/furniture/tv.4bpp");
static const u32 sLampGfx[] = INCBIN_U32("graphics/world/furniture/lamp.4bpp");
static const u32 sAquariumGfx[] = INCBIN_U32("graphics/world/furniture/aquarium.4bpp");
static const u32 sCushionGfx[] = INCBIN_U32("graphics/world/furniture/cushion.4bpp");
static const u32 sTrophiesGfx[] = INCBIN_U32("graphics/world/furniture/trophies.4bpp");
static const u32 sTelescopeGfx[] = INCBIN_U32("graphics/world/furniture/telescope.4bpp");
static const u16 sFurniturePal[] = INCBIN_U16("graphics/world/furniture/plant.gbapal");

#define FURNITURE(gfxArray, nameStr, w, h, tw, td) \
    {.name = COMPOUND_STRING(nameStr), .gfx = gfxArray, .gfxSize = sizeof(gfxArray), \
     .width = w, .height = h, .tilesWide = tw, .tilesDeep = td}

static const struct Furniture sFurniture[FURNITURE_COUNT] =
{
    [FURNITURE_PLANT]     = FURNITURE(sPlantGfx,     "POTTED PLANT", 16, 32, 1, 1),
    [FURNITURE_BOOKCASE]  = FURNITURE(sBookcaseGfx,  "BOOKCASE",     16, 32, 1, 1),
    [FURNITURE_TABLE]     = FURNITURE(sTableGfx,     "TABLE",        32, 32, 2, 2),
    [FURNITURE_STOOL]     = FURNITURE(sStoolGfx,     "STOOL",        16, 16, 1, 1),
    [FURNITURE_BED]       = FURNITURE(sBedGfx,       "BED",          16, 32, 1, 2),
    [FURNITURE_TV]        = FURNITURE(sTvGfx,        "TV",           16, 32, 1, 1),
    [FURNITURE_LAMP]      = FURNITURE(sLampGfx,      "LAMP",         16, 32, 1, 1),
    [FURNITURE_AQUARIUM]  = FURNITURE(sAquariumGfx,  "AQUARIUM",     32, 32, 2, 2),
    [FURNITURE_CUSHION]   = FURNITURE(sCushionGfx,   "BALL CUSHION", 16, 16, 1, 1),
    [FURNITURE_TROPHIES]  = FURNITURE(sTrophiesGfx,  "TROPHY CASE",  16, 32, 1, 1),
    [FURNITURE_TELESCOPE] = FURNITURE(sTelescopeGfx, "TELESCOPE",    16, 32, 1, 1),
};

static const struct OamData sOam16x16 = {.shape = SPRITE_SHAPE(16x16), .size = SPRITE_SIZE(16x16), .priority = 2};
static const struct OamData sOam16x32 = {.shape = SPRITE_SHAPE(16x32), .size = SPRITE_SIZE(16x32), .priority = 2};
static const struct OamData sOam32x32 = {.shape = SPRITE_SHAPE(32x32), .size = SPRITE_SIZE(32x32), .priority = 2};

static void SpriteCB_Furniture(struct Sprite *sprite) {}

struct HouseState
{
    u16 mapKey;            // the map the sprites were made on
    u16 hostId;            // visiting another player's open house, or 0
    u8 hostName[NETPLAY_NAME_LENGTH];
    u8 shown[WORLD_HOUSE_SLOTS]; // the furniture the sprites show
    u8 spriteIds[HOUSE_MAX_PIECES];
    u8 holding;            // FURNITURE_* being placed, or NONE
    u8 ghostSprite;
    u8 ghostTimer;
};

static EWRAM_DATA struct HouseState sHouse = {0};

static u8 *Decor(void)
{
    return gSaveBlock3Ptr->world.houseDecor;
}

// Whose furniture to show: ours, unless another player in here opened
// their house to visitors (and ours isn't open).
static const u8 *ShownDecor(void)
{
    const u8 *decor;

    if (!FlagGet(FLAG_WORLD_HOUSE_OPEN))
    {
        decor = WorldOnline_GetOpenHouse(&sHouse.hostId, sHouse.hostName);
        if (decor == NULL && sHouse.hostId != 0)
        {
            sHouse.hostId = 0; // they left; look for anyone else next time
            decor = WorldOnline_GetOpenHouse(&sHouse.hostId, sHouse.hostName);
        }
        if (decor != NULL)
            return decor;
    }
    sHouse.hostId = 0;
    return Decor();
}

static bool32 IsInHouse(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_STARFALL_ISLE_PLAYER_HOUSE)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_STARFALL_ISLE_PLAYER_HOUSE);
}

static u32 CountPieces(void)
{
    u32 i, n = 0;

    for (i = 0; i < HOUSE_MAX_PIECES; i++)
        n += Decor()[i * 2] != FURNITURE_NONE;
    return n;
}

static u8 CreateFurnitureSprite(u32 type, s16 x, s16 y)
{
    const struct Furniture *piece = &sFurniture[type];
    struct SpriteSheet sheet = {piece->gfx, piece->gfxSize, TAG_FURNITURE + type};
    struct SpritePalette palette = {sFurniturePal, TAG_FURNITURE_PAL};
    struct SpriteTemplate template =
    {
        .tileTag = TAG_FURNITURE + type,
        .paletteTag = TAG_FURNITURE_PAL,
        .oam = piece->width == 32 ? &sOam32x32 : piece->height == 32 ? &sOam16x32 : &sOam16x16,
        .anims = gDummySpriteAnimTable,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCB_Furniture,
    };
    struct Sprite *player = &gSprites[gPlayerAvatar.spriteId];
    s16 sx, sy;
    u8 spriteId;

    if (GetSpriteTileStartByTag(sheet.tag) == TAG_NONE)
        LoadSpriteSheet(&sheet);
    LoadSpritePalette(&palette);
    // (x, y) is the footprint's bottom-left tile; the art stands on it.
    SetSpritePosToMapCoords(x + MAP_OFFSET, y + MAP_OFFSET, &sx, &sy);
    spriteId = CreateSprite(&template, sx + piece->width / 2, sy + 16 - piece->height / 2, 0);
    if (spriteId == MAX_SPRITES)
        return MAX_SPRITES;
    gSprites[spriteId].coordOffsetEnabled = TRUE;
    // Sorted with the player by y, so the player can stand behind tall pieces.
    gSprites[spriteId].oam.priority = player->oam.priority;
    gSprites[spriteId].subpriority = player->subpriority;
    return spriteId;
}

static void SetFootprint(u32 type, s32 x, s32 y, bool32 blocked)
{
    s32 dx, dy;

    for (dy = 0; dy < sFurniture[type].tilesDeep; dy++)
        for (dx = 0; dx < sFurniture[type].tilesWide; dx++)
            MapGridSetMetatileImpassabilityAt(x + dx + MAP_OFFSET, y - dy + MAP_OFFSET, blocked);
}

static bool32 SpritesAreUp(void)
{
    u32 i;

    if (sHouse.mapKey != ((gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum))
        return FALSE;
    for (i = 0; i < HOUSE_MAX_PIECES; i++)
    {
        u8 id = sHouse.spriteIds[i];
        if (sHouse.shown[i * 2] != FURNITURE_NONE
         && (id == MAX_SPRITES || !gSprites[id].inUse || gSprites[id].callback != SpriteCB_Furniture))
            return FALSE;
    }
    return TRUE;
}

static void ClearAll(void)
{
    u32 i;

    for (i = 0; i < HOUSE_MAX_PIECES; i++)
    {
        u8 type = sHouse.shown[i * 2], pos = sHouse.shown[i * 2 + 1], id = sHouse.spriteIds[i];
        if (type == FURNITURE_NONE || type >= FURNITURE_COUNT)
            continue;
        SetFootprint(type, pos & 0xF, pos >> 4, FALSE);
        if (id != MAX_SPRITES && gSprites[id].inUse && gSprites[id].callback == SpriteCB_Furniture)
            DestroySprite(&gSprites[id]);
        sHouse.spriteIds[i] = MAX_SPRITES;
    }
    memset(sHouse.shown, 0, sizeof(sHouse.shown));
}

static void PlaceAll(const u8 *decor)
{
    u32 i;

    sHouse.mapKey = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    memcpy(sHouse.shown, decor, sizeof(sHouse.shown));
    for (i = 0; i < HOUSE_MAX_PIECES; i++)
    {
        u8 type = decor[i * 2], pos = decor[i * 2 + 1];
        sHouse.spriteIds[i] = MAX_SPRITES;
        if (type == FURNITURE_NONE || type >= FURNITURE_COUNT)
            continue;
        sHouse.spriteIds[i] = CreateFurnitureSprite(type, pos & 0xF, pos >> 4);
        SetFootprint(type, pos & 0xF, pos >> 4, TRUE);
    }
}

static void DropGhost(void)
{
    if (sHouse.holding != FURNITURE_NONE && sHouse.ghostSprite != MAX_SPRITES
     && gSprites[sHouse.ghostSprite].inUse && gSprites[sHouse.ghostSprite].callback == SpriteCB_Furniture)
        DestroySprite(&gSprites[sHouse.ghostSprite]);
    sHouse.ghostSprite = MAX_SPRITES;
}

static void FrontTile(s16 *x, s16 *y)
{
    PlayerGetDestCoords(x, y);
    MoveCoords(GetPlayerFacingDirection(), x, y);
    *x -= MAP_OFFSET;
    *y -= MAP_OFFSET;
}

// The piece being placed hovers, blinking, on the tile in front of the player.
static void UpdateGhost(void)
{
    s16 x, y, sx, sy;
    const struct Furniture *piece = &sFurniture[sHouse.holding];
    struct Sprite *ghost;

    if (sHouse.ghostSprite == MAX_SPRITES || !gSprites[sHouse.ghostSprite].inUse
     || gSprites[sHouse.ghostSprite].callback != SpriteCB_Furniture)
    {
        sHouse.ghostSprite = CreateFurnitureSprite(sHouse.holding, 0, 0);
        if (sHouse.ghostSprite == MAX_SPRITES)
            return;
    }
    ghost = &gSprites[sHouse.ghostSprite];
    FrontTile(&x, &y);
    SetSpritePosToMapCoords(x + MAP_OFFSET, y + MAP_OFFSET, &sx, &sy);
    ghost->x = sx + piece->width / 2;
    ghost->y = sy + 16 - piece->height / 2;
    ghost->x2 = ghost->y2 = 0;
    ghost->subpriority = 0;
    ghost->invisible = (++sHouse.ghostTimer & 0x10) != 0;
}

void House_Update(void)
{
    const u8 *decor;

    if (!IsInHouse())
    {
        sHouse.mapKey = 0;
        sHouse.hostId = 0;
        sHouse.holding = FURNITURE_NONE;
        return;
    }
    decor = ShownDecor();
    if (!SpritesAreUp())
    {
        // Back from a menu or a new visit: everything was reset.
        sHouse.ghostSprite = MAX_SPRITES;
        memset(sHouse.spriteIds, MAX_SPRITES, sizeof(sHouse.spriteIds));
        PlaceAll(decor);
    }
    else if (memcmp(decor, sHouse.shown, sizeof(sHouse.shown)) != 0)
    {
        ClearAll(); // the host moved something, or we came or went
        PlaceAll(decor);
    }
    if (sHouse.holding != FURNITURE_NONE)
        UpdateGhost();
}

static bool32 IsTileFree(s32 x, s32 y)
{
    u32 i;

    if (x < ROOM_LEFT || x > ROOM_RIGHT || y < ROOM_TOP || y > ROOM_BOTTOM)
        return FALSE;
    if (MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET))
        return FALSE;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *obj = &gObjectEvents[i];
        if (obj->active && ((obj->currentCoords.x == x + MAP_OFFSET && obj->currentCoords.y == y + MAP_OFFSET)
                         || (obj->previousCoords.x == x + MAP_OFFSET && obj->previousCoords.y == y + MAP_OFFSET)))
            return FALSE;
    }
    return TRUE;
}

static bool32 CanPlace(u32 type, s32 x, s32 y)
{
    s32 dx, dy;

    for (dy = 0; dy < sFurniture[type].tilesDeep; dy++)
        for (dx = 0; dx < sFurniture[type].tilesWide; dx++)
            if (!IsTileFree(x + dx, y - dy))
                return FALSE;
    return TRUE;
}

static void Place(s16 x, s16 y)
{
    u32 i;

    for (i = 0; i < HOUSE_MAX_PIECES; i++)
    {
        if (Decor()[i * 2] == FURNITURE_NONE)
            break;
    }
    if (i == HOUSE_MAX_PIECES)
        return;
    Decor()[i * 2] = sHouse.shown[i * 2] = sHouse.holding;
    Decor()[i * 2 + 1] = sHouse.shown[i * 2 + 1] = x | (y << 4);
    DropGhost();
    sHouse.spriteIds[i] = CreateFurnitureSprite(sHouse.holding, x, y);
    SetFootprint(sHouse.holding, x, y, TRUE);
    sHouse.holding = FURNITURE_NONE;
    if (CountPieces() >= HOMEOWNER_PIECES)
        Achievements_Unlock(ACH_HOMEOWNER);
}

// The placed piece whose footprint covers (x, y), or -1.
static s32 PieceAt(s32 x, s32 y)
{
    u32 i;

    for (i = 0; i < HOUSE_MAX_PIECES; i++)
    {
        u8 type = Decor()[i * 2], pos = Decor()[i * 2 + 1];
        s32 px = pos & 0xF, py = pos >> 4;
        if (type == FURNITURE_NONE || type >= FURNITURE_COUNT)
            continue;
        if (x >= px && x < px + sFurniture[type].tilesWide && y <= py && y > py - sFurniture[type].tilesDeep)
            return i;
    }
    return -1;
}

bool32 House_TryInteract(s16 x, s16 y)
{
    s32 piece;

    if (!IsInHouse() || sHouse.hostId != 0)
        return FALSE;
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;
    // Outside the room (the walls, the catalog) A works as usual.
    if (x < ROOM_LEFT || x > ROOM_RIGHT || y < ROOM_TOP || y > ROOM_BOTTOM + 1)
        return FALSE;
    if (sHouse.holding != FURNITURE_NONE)
    {
        if (CanPlace(sHouse.holding, x, y))
        {
            Place(x, y);
            // Field input stays locked until a script ends.
            ScriptContext_SetupScript(StarfallIsle_PlayerHouse_EventScript_Placed);
            return TRUE;
        }
        StringCopy(gStringVar1, sFurniture[sHouse.holding].name);
        ScriptContext_SetupScript(StarfallIsle_PlayerHouse_EventScript_CantPlace);
        return TRUE;
    }
    piece = PieceAt(x, y);
    if (piece < 0)
        return FALSE;
    gSpecialVar_0x8004 = piece;
    StringCopy(gStringVar1, sFurniture[Decor()[piece * 2]].name);
    ScriptContext_SetupScript(StarfallIsle_PlayerHouse_EventScript_PutAway);
    return TRUE;
}

// Script: VAR_0x8004 = FURNITURE_*. VAR_RESULT = HOUSE_PLACING or HOUSE_FULL.
void House_StartPlacing(void)
{
    if (CountPieces() >= HOUSE_MAX_PIECES)
    {
        gSpecialVar_Result = HOUSE_FULL;
        return;
    }
    DropGhost();
    sHouse.holding = gSpecialVar_0x8004;
    StringCopy(gStringVar1, sFurniture[sHouse.holding].name);
    gSpecialVar_Result = HOUSE_PLACING;
}

// Script: the catalog. VAR_RESULT = 0 our house, 1 our house, open to
// visitors, 2 visiting (STR_VAR_1 = whose).
void House_GetCatalogState(void)
{
    if (sHouse.hostId != 0)
    {
        StringCopy(gStringVar1, sHouse.hostName);
        gSpecialVar_Result = 2;
        return;
    }
    gSpecialVar_Result = FlagGet(FLAG_WORLD_HOUSE_OPEN) ? 1 : 0;
}

// Script: opens or closes the house to visitors.
void House_ToggleOpen(void)
{
    if (FlagGet(FLAG_WORLD_HOUSE_OPEN))
        FlagClear(FLAG_WORLD_HOUSE_OPEN);
    else
        FlagSet(FLAG_WORLD_HOUSE_OPEN);
}

// Script: puts piece VAR_0x8004 away.
void House_PutAway(void)
{
    u32 i = gSpecialVar_0x8004;
    u8 type = Decor()[i * 2], pos = Decor()[i * 2 + 1];

    if (type == FURNITURE_NONE)
        return;
    SetFootprint(type, pos & 0xF, pos >> 4, FALSE);
    if (sHouse.spriteIds[i] != MAX_SPRITES)
        DestroySprite(&gSprites[sHouse.spriteIds[i]]);
    sHouse.spriteIds[i] = MAX_SPRITES;
    Decor()[i * 2] = sHouse.shown[i * 2] = FURNITURE_NONE;
    Decor()[i * 2 + 1] = sHouse.shown[i * 2 + 1] = 0;
}
