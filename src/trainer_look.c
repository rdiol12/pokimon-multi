#include "global.h"
#include "bg.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fpmath.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "netplay.h"
#include "overworld.h"
#include "palette.h"
#include "random.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_look.h"
#include "window.h"
#include "constants/adventure.h"
#include "constants/event_objects.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/trainers.h"

// The trainer look (docs/trainer_look.md).
//
// Colours: Brendan's and May's sprites keep their art; a look only changes
// palette entries (skin, hat or hair, bandana, clothes, and a dark part).
// A recoloured trainer gets a palette from a small pool of tags; players
// with the same colours share one, and if the sprite palettes run out the
// trainer keeps the colours they were drawn with.
//
// Accessories: a second 32x32 sprite drawn over the trainer, made on the
// fly from pictures drawn once per facing. For every frame of every
// Brendan and May sprite sheet, tools/trainer_look/gen_trainer_look.py
// worked out where the head is and which pixels the trainer covers (for
// parts that sit behind them, like long hair). The layer uses the
// trainer's own palette, so it costs no palette slot.

enum
{
    BODY_BRENDAN,
    BODY_MAY,
    BODY_COUNT,
};

struct LookPart
{
    const u16 (*colours)[4];
    const u8 *const *names;
    u8 count; // with ORIGINAL
    const u8 *label[BODY_COUNT];
    u8 indices[BODY_COUNT][4];
    u8 numIndices[BODY_COUNT];
};

struct LookAccessoryGroup
{
    const u8 *label;
    const u8 *const *names;
    u8 count; // with NONE
    const u8 (*pics[BODY_COUNT])[3][256];
};

struct LookAnchor
{
    u8 facing; // 0: no head found (no accessories on this frame), 1 S, 2 N, 3 W
    s8 x, y;   // the picture's top left in the frame
    u16 mask[16]; // the trainer's pixels under the picture
};

struct LookSheet
{
    const struct LookAnchor *anchors;
    u8 count;
    u8 width;
};

#include "data/trainer_look.h"

#define TAG_LOOK_PAL_FIRST  0x5F70 // the palette pool
#define LOOK_PAL_POOL       10
#define TAG_LOOK_TILES      0x5F80 // + overlay slot
#define LOOK_OVERLAYS       (1 + NETPLAY_MAX_REMOTES) // the player, then the other players
#define OVERLAY_SIZE        (32 * 32 / 2)
#define LOOK_REFRESH_FRAMES 8

// Overlay sprite data.
#define sSlot    data[0] // overlay slot + 1
#define sKey     data[1] // the frame drawn: frame | sheet << 8 | body << 12
#define sWorn    data[2] // the accessories drawn (look >> 20)

static EWRAM_DATA u8 sOverlaySprites[LOOK_OVERLAYS] = {0};

static const u8 sZeroTiles[OVERLAY_SIZE] = {0};

static const struct OamData sOam_Overlay =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 2,
};

static void SpriteCB_LookOverlay(struct Sprite *sprite)
{
}

// The look ----------------------------------------------------------------

u32 TrainerLook_Get(void)
{
    const u8 *b = gSaveBlock2Ptr->trainerLook;

    return b[0] | (b[1] << 8) | (b[2] << 16) | ((u32)b[3] << 24);
}

void TrainerLook_Set(u32 look)
{
    u8 *b = gSaveBlock2Ptr->trainerLook;

    b[0] = look;
    b[1] = look >> 8;
    b[2] = look >> 16;
    b[3] = look >> 24;
}

static u32 SetField(u32 look, u32 field, u32 value)
{
    return (look & ~(0xFu << (field * 4))) | (value << (field * 4));
}

// Whose look an object event wears: the player's, another player's, or none.
static u32 LookOf(const struct ObjectEvent *objectEvent)
{
    if (objectEvent->isPlayer)
        return TrainerLook_Get();
    if (Netplay_IsNetplayObjectEvent(objectEvent))
        return WorldOnline_GetRemoteTrainerLook(objectEvent->localId - LOCALID_NETPLAY_PLAYER_START);
    return 0;
}

// Which body and sheet a graphics id draws. Outfits (src/trainer_level.c)
// bring their own clothes, so only skin and hair colours go on them.
static bool32 GetTarget(u16 graphicsId, u32 *body, u32 *sheet, bool32 *outfit)
{
    static const u16 sGfx[BODY_COUNT][LOOK_SHEET_COUNT] =
    {
        [BODY_BRENDAN] = {OBJ_EVENT_GFX_BRENDAN_NORMAL, OBJ_EVENT_GFX_BRENDAN_MACH_BIKE, OBJ_EVENT_GFX_BRENDAN_ACRO_BIKE,
                          OBJ_EVENT_GFX_BRENDAN_SURFING, OBJ_EVENT_GFX_BRENDAN_FIELD_MOVE, OBJ_EVENT_GFX_BRENDAN_FISHING,
                          OBJ_EVENT_GFX_BRENDAN_WATERING},
        [BODY_MAY] = {OBJ_EVENT_GFX_MAY_NORMAL, OBJ_EVENT_GFX_MAY_MACH_BIKE, OBJ_EVENT_GFX_MAY_ACRO_BIKE,
                      OBJ_EVENT_GFX_MAY_SURFING, OBJ_EVENT_GFX_MAY_FIELD_MOVE, OBJ_EVENT_GFX_MAY_FISHING,
                      OBJ_EVENT_GFX_MAY_WATERING},
    };
    u32 b, s;

    if (graphicsId >= OBJ_EVENT_GFX_OUTFIT_FIRST && graphicsId <= OBJ_EVENT_GFX_OUTFIT_LAST)
    {
        u32 n = graphicsId - OBJ_EVENT_GFX_OUTFIT_FIRST;
        *sheet = n % OUTFIT_STATES; // same order as the look's sheets
        *body = (n / OUTFIT_STATES) % 2 ? BODY_MAY : BODY_BRENDAN;
        *outfit = TRUE;
        return TRUE;
    }
    for (b = 0; b < BODY_COUNT; b++)
    {
        for (s = 0; s < LOOK_SHEET_COUNT; s++)
        {
            if (sGfx[b][s] == graphicsId)
            {
                *body = b;
                *sheet = s;
                *outfit = FALSE;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// Colours -----------------------------------------------------------------

static void BuildPalette(u16 *dest, const u16 *base, u32 body, u32 look, bool32 outfit)
{
    u32 part, i;

    CpuCopy16(base, dest, PLTT_SIZE_4BPP);
    for (part = 0; part < LOOK_COLOUR_PARTS; part++)
    {
        const struct LookPart *p = &sLookParts[part];
        u32 choice = LOOK_FIELD(look, part);

        if (choice == 0 || choice >= p->count)
            continue;
        if (outfit && part != LOOK_SKIN && part != LOOK_HAT)
            continue;
        if (outfit && part == LOOK_HAT && body == BODY_BRENDAN)
        {
            dest[9] = p->colours[choice - 1][0]; // index 5 is the outfit's
            continue;
        }
        for (i = 0; i < p->numIndices[body]; i++)
            dest[p->indices[body][i]] = p->colours[choice - 1][i];
    }
}

static bool32 IsPoolTag(u16 tag)
{
    return tag >= TAG_LOOK_PAL_FIRST && tag < TAG_LOOK_PAL_FIRST + LOOK_PAL_POOL;
}

// A sprite palette slot holding these colours: one already loaded if
// another trainer looks the same, otherwise a new one. 0xFF if none is free.
static u8 LoadLookPalette(const u16 *colours, bool32 inField)
{
    u32 i, slot;

    for (i = 0; i < LOOK_PAL_POOL; i++)
    {
        slot = IndexOfSpritePaletteTag(TAG_LOOK_PAL_FIRST + i);
        if (slot != 0xFF && memcmp(&gPlttBufferUnfaded[OBJ_PLTT_ID(slot)], colours, PLTT_SIZE_4BPP) == 0)
            return slot;
    }
    for (i = 0; i < LOOK_PAL_POOL; i++)
    {
        if (IndexOfSpritePaletteTag(TAG_LOOK_PAL_FIRST + i) == 0xFF)
        {
            struct SpritePalette palette = {colours, TAG_LOOK_PAL_FIRST + i};

            slot = LoadSpritePalette(&palette);
            if (slot != 0xFF && inField)
                UpdateSpritePaletteWithWeather(slot, FALSE);
            return slot;
        }
    }
    return 0xFF;
}


// Moves a trainer (and their accessory layer) to palette slot `slot`, and
// frees the pool palette they leave if nobody else uses it.
static void SwitchPalette(struct Sprite *sprite, u8 slot)
{
    u8 old = sprite->oam.paletteNum;
    u32 i;

    if (slot == 0xFF || slot == old)
        return;
    sprite->oam.paletteNum = slot;
    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].callback == SpriteCB_LookOverlay && gSprites[i].oam.paletteNum == old)
            gSprites[i].oam.paletteNum = slot;
    }
    if (IsPoolTag(GetSpritePaletteTagByPaletteNum(old)))
        FieldEffectFreePaletteIfUnused(old);
}

static void ApplyLookPalette(struct Sprite *sprite, u16 graphicsId, u32 look, bool32 inField)
{
    const struct ObjectEventGraphicsInfo *info = GetObjectEventGraphicsInfo(graphicsId);
    const u16 *base;
    u32 body, sheet;
    bool32 outfit;
    u16 colours[16];

    if (!GetTarget(graphicsId, &body, &sheet, &outfit))
        return;
    base = GetObjectEventPaletteDataByTag(info->paletteTag);
    if (base == NULL)
        return;
    if ((look & LOOK_COLOUR_MASK) == 0)
    {
        // Back to the colours it was drawn with.
        if (IsPoolTag(GetSpritePaletteTagByPaletteNum(sprite->oam.paletteNum)))
            SwitchPalette(sprite, LoadObjectEventPalette(info->paletteTag));
        return;
    }
    BuildPalette(colours, base, body, look, outfit);
    if (memcmp(&gPlttBufferUnfaded[OBJ_PLTT_ID(sprite->oam.paletteNum)], colours, sizeof(colours)) == 0)
        return;
    SwitchPalette(sprite, LoadLookPalette(colours, inField));
}

void TrainerLook_ApplyToObjectEvent(struct ObjectEvent *objectEvent)
{
    u32 look;

    if (!objectEvent->active || objectEvent->spriteId >= MAX_SPRITES)
        return;
    look = LookOf(objectEvent);
    if (look == 0 && !IsPoolTag(GetSpritePaletteTagByPaletteNum(gSprites[objectEvent->spriteId].oam.paletteNum)))
        return;
    ApplyLookPalette(&gSprites[objectEvent->spriteId], objectEvent->graphicsId, look, TRUE);
}

// Big trainer pictures -------------------------------------------------------
//
// Brendan's and May's front and back pictures share one palette each, laid
// out like their overworld sprites but with their own ramps.

static const u8 sPicIndices[LOOK_COLOUR_PARTS][BODY_COUNT][4] =
{
    [LOOK_SKIN]    = {{1, 2, 3, 4}, {1, 2, 3, 4}},
    [LOOK_HAT]     = {{9},          {7, 8}},
    [LOOK_BAND]    = {{10, 11},     {10, 11}},
    [LOOK_CLOTHES] = {{12, 13},     {12, 13}},
    [LOOK_DARK]    = {{5, 6, 7, 8}, {5, 6}},
};
static const u8 sPicIndexCount[LOOK_COLOUR_PARTS][BODY_COUNT] =
{
    [LOOK_SKIN]    = {4, 4},
    [LOOK_HAT]     = {1, 2},
    [LOOK_BAND]    = {2, 2},
    [LOOK_CLOTHES] = {2, 2},
    [LOOK_DARK]    = {4, 2},
};

static u16 MixColour(u16 a, u16 b, u32 weightB) // weightB of 4
{
    u32 r = (GET_R(a) * (4 - weightB) + GET_R(b) * weightB) / 4;
    u32 g = (GET_G(a) * (4 - weightB) + GET_G(b) * weightB) / 4;
    u32 bl = (GET_B(a) * (4 - weightB) + GET_B(b) * weightB) / 4;

    return RGB(r, g, bl);
}

// Recolours the 16 colours at palette offset `offset` (0-511) drawn as
// trainer picture `trainerPic`, if that is Brendan or May.
void TrainerLook_RecolorTrainerPic(u16 offset, u16 trainerPic, u32 look)
{
    u32 body, part, i;
    u16 *pal = &gPlttBufferUnfaded[offset];

    if (trainerPic == TRAINER_PIC_BRENDAN)
        body = BODY_BRENDAN;
    else if (trainerPic == TRAINER_PIC_MAY)
        body = BODY_MAY;
    else
        return;
    if ((look & LOOK_COLOUR_MASK) == 0 || offset > PLTT_BUFFER_SIZE - 16)
        return;
    for (part = 0; part < LOOK_COLOUR_PARTS; part++)
    {
        const struct LookPart *p = &sLookParts[part];
        u32 choice = LOOK_FIELD(look, part);
        u32 n = sPicIndexCount[part][body];
        const u16 *c;
        u16 ramp[4];

        if (choice == 0 || choice >= p->count)
            continue;
        c = p->colours[choice - 1];
        if (part == LOOK_SKIN)
        {
            for (i = 0; i < 4; i++)
                ramp[i] = c[i];
        }
        else if (n == 4)
        {
            // Light, main, between, shade.
            ramp[0] = MixColour(c[0], RGB_WHITE, 1);
            ramp[1] = c[0];
            ramp[2] = MixColour(c[0], c[1], 2);
            ramp[3] = c[1];
        }
        else
        {
            ramp[0] = c[0];
            ramp[1] = c[1];
        }
        for (i = 0; i < n; i++)
            pal[sPicIndices[part][body][i]] = ramp[i];
    }
    if (!gPaletteFade.active)
        CpuCopy16(pal, &gPlttBufferFaded[offset], PLTT_SIZE_4BPP);
}

// Accessories -------------------------------------------------------------

// The picture index the sprite shows now, or -1.
static s32 CurrentImage(const struct Sprite *sprite)
{
    const union AnimCmd *cmd;

    if (sprite->anims == NULL)
        return -1;
    cmd = &sprite->anims[sprite->animNum][sprite->animCmdIndex];
    if (cmd->type < 0)
        return -1;
    return cmd->frame.imageValue;
}

static void SetPixel(u8 *buf, u32 x, u32 y, u32 value)
{
    u8 *b = &buf[((y / 8) * 4 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];

    if (x & 1)
        *b = (*b & 0x0F) | (value << 4);
    else
        *b = (*b & 0xF0) | value;
}

// Draws the accessories in look onto a 32x32 picture for frame `anchor`
// of a sheet `width` pixels wide.
static void ComposeOverlay(u8 *buf, u32 body, u32 width, const struct LookAnchor *anchor, u32 look)
{
    // Back to front: hair, headwear, then glasses.
    static const u8 sOrder[] = {LOOK_HAIRSTYLE, LOOK_HEADWEAR, LOOK_FACE};
    u32 n, tx, ty;
    s32 left = (32 - width) / 2;

    CpuFill32(0, buf, OVERLAY_SIZE);
    if (anchor->facing == 0)
        return;
    for (n = 0; n < ARRAY_COUNT(sOrder); n++)
    {
        const struct LookAccessoryGroup *group = &sLookAccessories[sOrder[n] - LOOK_COLOUR_PARTS];
        u32 choice = LOOK_FIELD(look, sOrder[n]);
        const u8 *pic;

        if (choice == 0 || choice >= group->count)
            continue;
        pic = group->pics[body][choice - 1][anchor->facing - 1];
        for (ty = 0; ty < 16; ty++)
        {
            s32 y = anchor->y + ty;

            if (y < 0 || y >= 32)
                continue;
            for (tx = 0; tx < 16; tx++)
            {
                u32 v = pic[ty * 16 + tx];
                s32 x = left + anchor->x + tx;

                if (v == 0 || x < 0 || x >= 32)
                    continue;
                if ((v & 0x80) && (anchor->mask[ty] >> tx) & 1)
                    continue; // behind the trainer
                SetPixel(buf, x, y, v & 0xF);
            }
        }
    }
}

// Keeps overlay `ov` on `wearer` (drawn with graphicsId) wearing look.
// Returns FALSE if nothing should show.
static bool32 SyncOverlay(struct Sprite *ov, const struct Sprite *wearer, u16 graphicsId, u32 look)
{
    u32 body, sheet;
    bool32 outfit;
    s32 frame;
    const struct LookSheet *s;
    u32 key;

    if (!GetTarget(graphicsId, &body, &sheet, &outfit))
        return FALSE;
    s = &sLookSheets[body][sheet];
    frame = CurrentImage(wearer);
    if (frame < 0 || frame >= s->count || s->anchors[frame].facing == 0)
        return FALSE;
    key = frame | (sheet << 8) | (body << 12);
    if (ov->sKey != (s16)key || ov->sWorn != (s16)(look >> 20))
    {
        u8 buf[OVERLAY_SIZE] ALIGNED(4);

        ComposeOverlay(buf, body, s->width, &s->anchors[frame], look);
        CpuCopy32(buf, (void *)(OBJ_VRAM0 + ov->oam.tileNum * TILE_SIZE_4BPP), OVERLAY_SIZE);
        ov->sKey = key;
        ov->sWorn = look >> 20;
    }
    ov->x = wearer->x;
    ov->y = wearer->y;
    ov->x2 = wearer->x2;
    ov->y2 = wearer->y2;
    ov->coordOffsetEnabled = wearer->coordOffsetEnabled;
    ov->oam.priority = wearer->oam.priority;
    ov->subpriority = wearer->subpriority > 0 ? wearer->subpriority - 1 : 0;
    ov->oam.paletteNum = wearer->oam.paletteNum;
    ov->oam.objMode = wearer->oam.objMode;
    if (wearer->oam.affineMode == ST_OAM_AFFINE_OFF)
    {
        ov->oam.affineMode = ST_OAM_AFFINE_OFF;
        ov->oam.matrixNum = wearer->oam.matrixNum; // the flip bits
    }
    return TRUE;
}

static u8 CreateOverlay(u32 slot)
{
    struct SpriteSheet sheet = {sZeroTiles, OVERLAY_SIZE, TAG_LOOK_TILES + slot};
    struct SpriteTemplate template =
    {
        .tileTag = TAG_LOOK_TILES + slot,
        .paletteTag = TAG_NONE,
        .oam = &sOam_Overlay,
        .anims = gDummySpriteAnimTable,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCB_LookOverlay,
    };
    u8 spriteId;

    if (GetSpriteTileStartByTag(TAG_LOOK_TILES + slot) == 0xFFFF)
        LoadSpriteSheet(&sheet);
    if (GetSpriteTileStartByTag(TAG_LOOK_TILES + slot) == 0xFFFF)
        return MAX_SPRITES;
    spriteId = CreateSprite(&template, 0, 0, 0);
    if (spriteId == MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_LOOK_TILES + slot);
        return MAX_SPRITES;
    }
    gSprites[spriteId].sSlot = slot + 1;
    gSprites[spriteId].sKey = -1;
    gSprites[spriteId].invisible = TRUE;
    return spriteId;
}

static struct Sprite *GetOverlay(u32 slot)
{
    struct Sprite *sprite = &gSprites[sOverlaySprites[slot]];

    if (sOverlaySprites[slot] < MAX_SPRITES && sprite->inUse && sprite->callback == SpriteCB_LookOverlay
     && sprite->sSlot == (s16)(slot + 1))
        return sprite;
    return NULL;
}

static void DestroyOverlay(u32 slot)
{
    struct Sprite *sprite = GetOverlay(slot);

    if (sprite != NULL)
    {
        DestroySprite(sprite);
        FreeSpriteTilesByTag(TAG_LOOK_TILES + slot);
    }
    sOverlaySprites[slot] = MAX_SPRITES;
}

static struct ObjectEvent *OverlayWearer(u32 slot)
{
    u8 objId;

    if (slot == 0)
        return &gObjectEvents[gPlayerAvatar.objectEventId];
    objId = GetObjectEventIdByLocalIdAndMap(LOCALID_NETPLAY_PLAYER_START + slot - 1,
                                            gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    return objId < OBJECT_EVENTS_COUNT ? &gObjectEvents[objId] : NULL;
}

void TrainerLook_UpdateOverlays(void)
{
    u32 slot;
    bool32 refresh = (gMain.vblankCounter1 % LOOK_REFRESH_FRAMES) == 0;

    for (slot = 0; slot < LOOK_OVERLAYS; slot++)
    {
        struct ObjectEvent *obj = OverlayWearer(slot);
        struct Sprite *ov;
        u32 look;

        if (obj == NULL || !obj->active || obj->spriteId >= MAX_SPRITES || (slot == 0 && !obj->isPlayer))
        {
            DestroyOverlay(slot);
            continue;
        }
        look = LookOf(obj);
        if (refresh)
            TrainerLook_ApplyToObjectEvent(obj);
        if ((look & LOOK_ACCESSORY_MASK) == 0)
        {
            DestroyOverlay(slot);
            continue;
        }
        ov = GetOverlay(slot);
        if (ov == NULL)
        {
            sOverlaySprites[slot] = CreateOverlay(slot);
            ov = GetOverlay(slot);
            if (ov == NULL)
                continue;
        }
        if (SyncOverlay(ov, &gSprites[obj->spriteId], obj->graphicsId, look))
            ov->invisible = gSprites[obj->spriteId].invisible || obj->invisible;
        else
            ov->invisible = TRUE;
    }
}

// The Look screen ---------------------------------------------------------

enum
{
    ROW_BODY,
    ROW_PARTS, // LOOK_FIELDS rows
    ROW_RANDOM = ROW_PARTS + LOOK_FIELDS,
    ROW_DONE,
    ROW_COUNT,
};

enum
{
    WIN_LIST,
    WIN_NAME,
    WIN_HELP,
};

#define ROW_HEIGHT 13
#define PREVIEW_X  200
#define PREVIEW_Y  60
#define TURN_FRAMES 70

struct LookScreen
{
    void (*returnCallback)(void);
    u32 look;
    u32 oldLook;
    u8 oldStyle;
    u8 newGame;
    u8 cursor;
    u8 facing; // 0 S, 1 W, 2 N, 3 E
    u16 timer;
    u8 wearer;
    u8 overlay;
    u8 matrix[2]; // 2x, and 2x flipped
    u16 gfx;
};

static EWRAM_DATA struct LookScreen *sLook = NULL;

static const struct BgTemplate sLookBgTemplates[] =
{
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .priority = 0},
};

static const struct WindowTemplate sLookWindows[] =
{
    [WIN_LIST] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 18, .height = 18, .paletteNum = 15, .baseBlock = 0x020},
    [WIN_NAME] = {.bg = 0, .tilemapLeft = 21, .tilemapTop = 1, .width = 8, .height = 2, .paletteNum = 15, .baseBlock = 0x170},
    [WIN_HELP] = {.bg = 0, .tilemapLeft = 21, .tilemapTop = 13, .width = 8, .height = 6, .paletteNum = 15, .baseBlock = 0x180},
    DUMMY_WIN_TEMPLATE,
};

#define FRAME_TILE 0x001
#define FRAME_PAL  14

static const u8 sTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sValueColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};
static const u8 sGreyColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_WHITE};

static const u8 *const sStyleNames[PLAYER_STYLE_COUNT] =
{
    [PLAYER_STYLE_BRENDAN] = COMPOUND_STRING("BRENDAN"),
    [PLAYER_STYLE_MAY]     = COMPOUND_STRING("MAY"),
    [PLAYER_STYLE_RED]     = COMPOUND_STRING("RED"),
    [PLAYER_STYLE_LEAF]    = COMPOUND_STRING("LEAF"),
};

static bool32 StyleHasLook(enum PlayerStyle style)
{
    return style == PLAYER_STYLE_BRENDAN || style == PLAYER_STYLE_MAY;
}

static u32 StyleBody(enum PlayerStyle style)
{
    return style == PLAYER_STYLE_MAY ? BODY_MAY : BODY_BRENDAN;
}

static u32 FieldCount(u32 field)
{
    if (field < LOOK_COLOUR_PARTS)
        return sLookParts[field].count;
    return sLookAccessories[field - LOOK_COLOUR_PARTS].count;
}

static const u8 *FieldLabel(u32 field, u32 body)
{
    if (field < LOOK_COLOUR_PARTS)
        return sLookParts[field].label[body];
    return sLookAccessories[field - LOOK_COLOUR_PARTS].label;
}

static const u8 *FieldValueName(u32 field, u32 value)
{
    if (field < LOOK_COLOUR_PARTS)
        return sLookParts[field].names[value];
    return sLookAccessories[field - LOOK_COLOUR_PARTS].names[value];
}

static void DrawList(void)
{
    enum PlayerStyle style = GetPlayerStyle();
    bool32 hasLook = StyleHasLook(style);
    u32 body = StyleBody(style);
    u32 row;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    for (row = 0; row < ROW_COUNT; row++)
    {
        u32 y = 2 + row * ROW_HEIGHT;
        const u8 *label, *value = NULL;
        bool32 enabled = TRUE;

        if (row == sLook->cursor)
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 2, y, sTextColors, TEXT_SKIP_DRAW, COMPOUND_STRING("▶"));
        if (row == ROW_BODY)
        {
            label = COMPOUND_STRING("BODY");
            value = sStyleNames[style];
        }
        else if (row == ROW_RANDOM)
        {
            label = COMPOUND_STRING("RANDOM LOOK");
            enabled = hasLook;
        }
        else if (row == ROW_DONE)
        {
            label = COMPOUND_STRING("DONE");
        }
        else
        {
            u32 field = row - ROW_PARTS;

            label = FieldLabel(field, body);
            enabled = hasLook;
            value = enabled ? FieldValueName(field, LOOK_FIELD(sLook->look, field)) : COMPOUND_STRING("-");
        }
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 10, y, enabled ? sTextColors : sGreyColors, TEXT_SKIP_DRAW, label);
        if (value != NULL)
        {
            u8 *str = gStringVar4;

            if (enabled)
            {
                str = StringCopy(str, COMPOUND_STRING("{LEFT_ARROW}"));
                str = StringCopy(str, value);
                StringCopy(str, COMPOUND_STRING("{RIGHT_ARROW}"));
            }
            else
            {
                StringCopy(str, value);
            }
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 72, y, enabled ? sValueColors : sGreyColors, TEXT_SKIP_DRAW, gStringVar4);
        }
    }
    CopyWindowToVram(WIN_LIST, COPYWIN_GFX);
}

static void DrawSide(void)
{
    FillWindowPixelBuffer(WIN_NAME, PIXEL_FILL(1));
    AddTextPrinterParameterized3(WIN_NAME, FONT_NORMAL, GetStringCenterAlignXOffset(FONT_NORMAL, gSaveBlock2Ptr->playerName, 64),
                                 1, sTextColors, TEXT_SKIP_DRAW, gSaveBlock2Ptr->playerName);
    CopyWindowToVram(WIN_NAME, COPYWIN_GFX);
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    AddTextPrinterParameterized3(WIN_HELP, FONT_SMALL, 2, 1, sTextColors, TEXT_SKIP_DRAW,
                                 COMPOUND_STRING("{DPAD_LEFTRIGHT} CHANGE\n{SELECT_BUTTON} RANDOM\n{START_BUTTON} DONE"));
    if (!sLook->newGame)
        AddTextPrinterParameterized3(WIN_HELP, FONT_SMALL, 2, 37, sTextColors, TEXT_SKIP_DRAW, COMPOUND_STRING("{B_BUTTON} CANCEL"));
    CopyWindowToVram(WIN_HELP, COPYWIN_GFX);
}

static u16 PreviewGfx(void)
{
    // On foot, in the trainer-level outfit if one is worn.
    return GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_NORMAL);
}

static void SetPreviewMatrix(struct Sprite *sprite)
{
    sprite->subspriteTables = NULL; // affine sprites draw as one OAM entry
    sprite->subspriteMode = SUBSPRITES_OFF;
    sprite->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
    sprite->oam.matrixNum = sLook->matrix[sLook->facing == 3];
    CalcCenterToCornerVec(sprite, sprite->oam.shape, sprite->oam.size, ST_OAM_AFFINE_DOUBLE);
}

static void RemovePreview(void)
{
    if (sLook->wearer < MAX_SPRITES)
    {
        u8 pal = gSprites[sLook->wearer].oam.paletteNum;

        // The 2x matrices are shared with the accessory layer: keep them.
        gSprites[sLook->wearer].oam.affineMode = ST_OAM_AFFINE_OFF;
        DestroySpriteAndFreeResources(&gSprites[sLook->wearer]);
        if (IsPoolTag(GetSpritePaletteTagByPaletteNum(pal)))
            FieldEffectFreePaletteIfUnused(pal);
    }
    sLook->wearer = MAX_SPRITES;
}

// (Re)makes the big trainer and applies the look to them.
static void UpdatePreview(bool32 remake)
{
    static const u8 sDirs[] = {DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST};
    struct Sprite *wearer, *ov;
    u16 gfx = PreviewGfx();

    if (remake || gfx != sLook->gfx || sLook->wearer >= MAX_SPRITES)
    {
        RemovePreview();
        sLook->gfx = gfx;
        sLook->wearer = CreateObjectGraphicsSprite(gfx, SpriteCallbackDummy, PREVIEW_X, PREVIEW_Y, 2);
        if (sLook->wearer >= MAX_SPRITES)
            return;
        gSprites[sLook->wearer].oam.priority = 1;
        StartSpriteAnim(&gSprites[sLook->wearer], GetMoveDirectionAnimNum(sDirs[sLook->facing]));
    }
    wearer = &gSprites[sLook->wearer];
    SetPreviewMatrix(wearer);
    ApplyLookPalette(wearer, gfx, StyleHasLook(GetPlayerStyle()) ? sLook->look : 0, FALSE);

    ov = &gSprites[sLook->overlay];
    if (StyleHasLook(GetPlayerStyle()) && (sLook->look & LOOK_ACCESSORY_MASK)
     && SyncOverlay(ov, wearer, gfx, sLook->look))
    {
        ov->invisible = FALSE;
        ov->subpriority = 1;
        SetPreviewMatrix(ov);
    }
    else
    {
        ov->invisible = TRUE;
    }
}

static void VBlankCB_Look(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Look(void)
{
    RunTasks();
    AnimateSprites();
    if (sLook != NULL && sLook->wearer < MAX_SPRITES)
        UpdatePreview(FALSE);
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void Task_Look(u8 taskId);

static void CB2_InitLook(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        CpuFill32(0, (void *)VRAM, VRAM_SIZE);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sLookBgTemplates, ARRAY_COUNT(sLookBgTemplates));
        InitWindows(sLookWindows);
        DeactivateAllTextPrinters();
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ResetPaletteFade();
        ResetSpriteData();
        ResetAffineAnimData();
        FreeAllSpritePalettes();
        ScanlineEffect_Stop();
        ResetTasks();
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        gMain.state++;
        break;
    case 1:
    {
        // A soft teal backdrop, like Professor Birch's room.
        static const u16 sBackdrop[] = {RGB(14, 24, 20), RGB(12, 21, 18)};

        LoadUserWindowBorderGfx(WIN_LIST, FRAME_TILE, BG_PLTT_ID(FRAME_PAL));
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        LoadPalette(sBackdrop, BG_PLTT_ID(0), sizeof(sBackdrop));
        DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, FRAME_TILE, FRAME_PAL);
        DrawStdFrameWithCustomTileAndPalette(WIN_NAME, FALSE, FRAME_TILE, FRAME_PAL);
        DrawStdFrameWithCustomTileAndPalette(WIN_HELP, FALSE, FRAME_TILE, FRAME_PAL);
        PutWindowTilemap(WIN_LIST);
        PutWindowTilemap(WIN_NAME);
        PutWindowTilemap(WIN_HELP);
        gMain.state++;
        break;
    }
    case 2:
        DrawList();
        DrawSide();
        CopyBgTilemapBufferToVram(0);
        gMain.state++;
        break;
    case 3:
        sLook->matrix[0] = AllocOamMatrix();
        sLook->matrix[1] = AllocOamMatrix();
        SetOamMatrix(sLook->matrix[0], Q_8_8(0.5), 0, 0, Q_8_8(0.5));
        SetOamMatrix(sLook->matrix[1], -Q_8_8(0.5), 0, 0, Q_8_8(0.5));
        sLook->wearer = MAX_SPRITES;
        sLook->overlay = CreateOverlay(0);
        if (sLook->overlay >= MAX_SPRITES)
            sLook->overlay = CreateSprite(&gDummySpriteTemplate, 0, 0, 0); // drawn invisible
        UpdatePreview(TRUE);
        gMain.state++;
        break;
    case 4:
        ShowBg(0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_BG0_ON | DISPCNT_OBJ_1D_MAP);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Look);
        CreateTask(Task_Look, 0);
        SetMainCallback2(CB2_Look);
        gMain.state = 0;
        break;
    }
}

void TrainerLook_OpenScreen(void (*returnCallback)(void), bool32 newGame)
{
    sLook = AllocZeroed(sizeof(*sLook));
    sLook->returnCallback = returnCallback;
    sLook->newGame = newGame;
    sLook->look = sLook->oldLook = TrainerLook_Get();
    sLook->oldStyle = gSaveBlock2Ptr->playerStyle;
    gMain.state = 0;
    SetMainCallback2(CB2_InitLook);
}

static u32 RandomLook(void)
{
    u32 look = 0, field;

    for (field = 0; field < LOOK_FIELDS; field++)
    {
        u32 count = FieldCount(field);
        u32 value = Random() % count;

        // Accessories are rarer than colours, so not everyone wears a hat.
        if (field >= LOOK_COLOUR_PARTS && (Random() % 3) == 0)
            value = 0;
        look = SetField(look, field, value);
    }
    return look;
}

#define tState data[0]

static void Task_Look(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s32 cursor = sLook->cursor;
    bool32 redraw = FALSE, remake = FALSE;

    if (task->tState == 1)
    {
        if (!gPaletteFade.active)
        {
            void (*callback)(void) = sLook->returnCallback;

            RemovePreview();
            FreeOamMatrix(sLook->matrix[0]);
            FreeOamMatrix(sLook->matrix[1]);
            FreeAllWindowBuffers();
            FREE_AND_SET_NULL(sLook);
            DestroyTask(taskId);
            SetMainCallback2(callback);
        }
        return;
    }
    if (gPaletteFade.active)
        return;

    // The trainer turns around now and then, walking on the spot.
    if (++sLook->timer >= TURN_FRAMES)
    {
        static const u8 sDirs[] = {DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST};

        sLook->timer = 0;
        sLook->facing = (sLook->facing + 1) % 4;
        if (sLook->wearer < MAX_SPRITES)
            StartSpriteAnim(&gSprites[sLook->wearer], GetMoveDirectionAnimNum(sDirs[sLook->facing]));
    }

    if (JOY_NEW(START_BUTTON) || (JOY_NEW(A_BUTTON) && cursor == ROW_DONE))
    {
        PlaySE(SE_SELECT);
        TrainerLook_Set(sLook->look);
        task->tState = 1;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        return;
    }
    if (JOY_NEW(B_BUTTON) && !sLook->newGame)
    {
        PlaySE(SE_SELECT);
        TrainerLook_Set(sLook->oldLook);
        gSaveBlock2Ptr->playerStyle = sLook->oldStyle;
        task->tState = 1;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        return;
    }
    if (JOY_NEW(SELECT_BUTTON) || (JOY_NEW(A_BUTTON) && cursor == ROW_RANDOM))
    {
        if (StyleHasLook(GetPlayerStyle()))
        {
            PlaySE(SE_BALL_TRAY_ENTER);
            sLook->look = RandomLook();
            redraw = TRUE;
        }
    }
    else if (JOY_REPEAT(DPAD_UP))
    {
        cursor = cursor == 0 ? ROW_COUNT - 1 : cursor - 1;
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        cursor = cursor == ROW_COUNT - 1 ? 0 : cursor + 1;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        cursor = cursor + 1; // to the next row
    }
    else if (JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
    {
        s32 delta = JOY_REPEAT(DPAD_LEFT) ? -1 : 1;

        if (cursor == ROW_BODY)
        {
            s32 style = (GetPlayerStyle() + delta + PLAYER_STYLE_COUNT) % PLAYER_STYLE_COUNT;

            SetPlayerStyle(style);
            PlaySE(SE_SELECT);
            redraw = remake = TRUE;
        }
        else if (cursor >= ROW_PARTS && cursor < ROW_PARTS + LOOK_FIELDS && StyleHasLook(GetPlayerStyle()))
        {
            u32 field = cursor - ROW_PARTS;
            s32 count = FieldCount(field);
            s32 value = (s32)LOOK_FIELD(sLook->look, field) + delta;

            value = (value + count) % count;
            sLook->look = SetField(sLook->look, field, value);
            PlaySE(SE_SELECT);
            redraw = TRUE;
        }
    }

    if (cursor != sLook->cursor)
    {
        PlaySE(SE_SELECT);
        sLook->cursor = cursor;
        redraw = TRUE;
    }
    if (redraw)
    {
        DrawList();
        UpdatePreview(remake);
    }
}

#undef tState
