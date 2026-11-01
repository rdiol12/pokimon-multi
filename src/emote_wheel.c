// The emote wheel: L in the overworld while online. Eight emotes sit in a
// ring around the player; the D-pad points at one (diagonals too), L and R
// turn the page (FACES, MORE, PHRASES), A sends it and B closes the wheel.
// The emote pops up in a bubble above the player's head for everyone on the
// map; phrases also show in a line of text with the player's name.
#include "global.h"
#include "emote_wheel.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

#define TAG_EMOTE_ICONS 0x5E70

#define WHEEL_SLOTS 8
#define PAGE_COUNT  3
#define ICON_CHAT   16 // the speech-bubble icon shown for phrases

#define BALLOON_FRAMES 110

struct Emote
{
    const u8 *name;
    u8 icon; // in graphics/emote_wheel/icons.png
};

static const struct Emote sEmotes[EMOTE_COUNT] =
{
    // The first 14 are the ids of the old emote list.
    [0]  = {COMPOUND_STRING("HAPPY"), 0},
    [1]  = {COMPOUND_STRING("LOVE"), 1},
    [2]  = {COMPOUND_STRING("MUSIC"), 2},
    [3]  = {COMPOUND_STRING("WOW!"), 3},
    [4]  = {COMPOUND_STRING("HUH?"), 4},
    [5]  = {COMPOUND_STRING("ANGRY"), 5},
    [6]  = {COMPOUND_STRING("SAD"), 6},
    [7]  = {COMPOUND_STRING("HMM…"), 7},
    [8]  = {COMPOUND_STRING("HI!"), ICON_CHAT},
    [9]  = {COMPOUND_STRING("GG!"), ICON_CHAT},
    [10] = {COMPOUND_STRING("BATTLE?"), ICON_CHAT},
    [11] = {COMPOUND_STRING("TRADE?"), ICON_CHAT},
    [12] = {COMPOUND_STRING("THANKS!"), ICON_CHAT},
    [13] = {COMPOUND_STRING("BYE!"), ICON_CHAT},
    // New with the wheel.
    [14] = {COMPOUND_STRING("THUMBS UP"), 8},
    [15] = {COMPOUND_STRING("LOL"), 9},
    [16] = {COMPOUND_STRING("PHEW"), 10},
    [17] = {COMPOUND_STRING("SLEEPY"), 11},
    [18] = {COMPOUND_STRING("SPARKLE"), 12},
    [19] = {COMPOUND_STRING("FIRED UP"), 13},
    [20] = {COMPOUND_STRING("COOL"), 14},
    [21] = {COMPOUND_STRING("WAVE"), 15},
    [22] = {COMPOUND_STRING("FOLLOW ME!"), ICON_CHAT},
    [23] = {COMPOUND_STRING("NICE!"), ICON_CHAT},
};

// Each page, clockwise from the top.
static const u8 sPages[PAGE_COUNT][WHEEL_SLOTS] =
{
    {0, 1, 2, 3, 4, 5, 6, 7},
    {14, 15, 16, 17, 18, 19, 20, 21},
    {8, 9, 10, 11, 12, 13, 22, 23},
};

static const u8 *const sPageNames[PAGE_COUNT] =
{
    COMPOUND_STRING("FACES"),
    COMPOUND_STRING("MORE"),
    COMPOUND_STRING("PHRASES"),
};

static const s8 sSlotOffsets[WHEEL_SLOTS][2] =
{
    {0, -36}, {29, -26}, {42, 0}, {29, 26}, {0, 36}, {-29, 26}, {-42, 0}, {-29, -26},
};

static const u16 sIconsPal[] = INCGFX_U16("graphics/emote_wheel/icons.pal", ".gbapal");
static const u8 sIconsGfx[] = INCGFX_U8("graphics/emote_wheel/icons.png", ".4bpp", "-mwidth 2 -mheight 2");
static const u8 sCursorGfx[] = INCGFX_U8("graphics/emote_wheel/cursor.png", ".4bpp");

static const struct SpritePalette sIconsSpritePalette = {sIconsPal, TAG_EMOTE_ICONS};

// The sprites use tile sheets copied straight to VRAM, not per-frame image
// copies: those go through a queue of 64 that the field's objects can fill, and
// a dropped copy leaves a sprite showing stale tiles.
#define ICON_COUNT       17
#define ICON_BYTES       (4 * TILE_SIZE_4BPP) // one 16x16 frame
#define TAG_WHEEL_SHEET  0x5E71               // all the icons, while the wheel is open
#define TAG_WHEEL_CURSOR 0x5E72
#define TAG_BALLOON      0x5E80               // + icon: that icon's two frames

static const struct SpriteSheet sWheelSheet = {sIconsGfx, sizeof(sIconsGfx), TAG_WHEEL_SHEET};
static const struct SpriteSheet sCursorSheet = {sCursorGfx, sizeof(sCursorGfx), TAG_WHEEL_CURSOR};

// Frames of sheet sprites are counted in tiles.
#define ICON_ANIM(n) static const union AnimCmd sIconAnim##n[] = \
    {ANIMCMD_FRAME((n) * 8, 18), ANIMCMD_FRAME((n) * 8 + 4, 18), ANIMCMD_JUMP(0)}

ICON_ANIM(0); ICON_ANIM(1); ICON_ANIM(2); ICON_ANIM(3); ICON_ANIM(4); ICON_ANIM(5);
ICON_ANIM(6); ICON_ANIM(7); ICON_ANIM(8); ICON_ANIM(9); ICON_ANIM(10); ICON_ANIM(11);
ICON_ANIM(12); ICON_ANIM(13); ICON_ANIM(14); ICON_ANIM(15); ICON_ANIM(16);

static const union AnimCmd *const sIconAnims[ICON_COUNT] =
{
    sIconAnim0, sIconAnim1, sIconAnim2, sIconAnim3, sIconAnim4, sIconAnim5,
    sIconAnim6, sIconAnim7, sIconAnim8, sIconAnim9, sIconAnim10, sIconAnim11,
    sIconAnim12, sIconAnim13, sIconAnim14, sIconAnim15, sIconAnim16,
};

static const union AnimCmd sBalloonAnim[] = {ANIMCMD_FRAME(0, 18), ANIMCMD_FRAME(4, 18), ANIMCMD_JUMP(0)};
static const union AnimCmd *const sBalloonAnims[] = {sBalloonAnim};

static const struct OamData sOam16x16 =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 0,
};

static const struct OamData sOam32x32 =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 0,
};

static void SpriteCB_Balloon(struct Sprite *sprite);

static const struct SpriteTemplate sIconTemplate =
{
    .tileTag = TAG_WHEEL_SHEET,
    .paletteTag = TAG_EMOTE_ICONS,
    .oam = &sOam16x16,
    .anims = sIconAnims,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

#define BALLOON_TEMPLATE(n) [n] = { \
    .tileTag = TAG_BALLOON + (n), \
    .paletteTag = TAG_EMOTE_ICONS, \
    .oam = &sOam16x16, \
    .anims = sBalloonAnims, \
    .images = NULL, \
    .affineAnims = gDummySpriteAffineAnimTable, \
    .callback = SpriteCB_Balloon, \
}

static const struct SpriteTemplate sBalloonTemplates[ICON_COUNT] =
{
    BALLOON_TEMPLATE(0), BALLOON_TEMPLATE(1), BALLOON_TEMPLATE(2), BALLOON_TEMPLATE(3),
    BALLOON_TEMPLATE(4), BALLOON_TEMPLATE(5), BALLOON_TEMPLATE(6), BALLOON_TEMPLATE(7),
    BALLOON_TEMPLATE(8), BALLOON_TEMPLATE(9), BALLOON_TEMPLATE(10), BALLOON_TEMPLATE(11),
    BALLOON_TEMPLATE(12), BALLOON_TEMPLATE(13), BALLOON_TEMPLATE(14), BALLOON_TEMPLATE(15),
    BALLOON_TEMPLATE(16),
};

static const struct SpriteTemplate sCursorTemplate =
{
    .tileTag = TAG_WHEEL_CURSOR,
    .paletteTag = TAG_EMOTE_ICONS,
    .oam = &sOam32x32,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct WindowTemplate sLabelWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 15,
    .width = 28,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x139,
};

static EWRAM_DATA struct
{
    void (*onChosen)(u8 emote);
    u8 icons[WHEEL_SLOTS];
    u8 cursor;
    u8 windowId;
    u8 page;
    u8 slot;
    u8 lastDpad;
    bool8 open;
    bool8 ready; // the task skips the frame it was opened on (L is still new then)
    s16 centerX;
    s16 centerY;
} sWheel = {0};

static EWRAM_DATA u8 sLastPage = 0;
static EWRAM_DATA u8 sLastSlot = 0;

bool32 EmoteWheel_IsPhrase(u8 emote)
{
    return emote < EMOTE_COUNT && sEmotes[emote].icon == ICON_CHAT;
}

const u8 *EmoteWheel_GetText(u8 emote)
{
    return emote < EMOTE_COUNT ? sEmotes[emote].name : gText_EmptyString2;
}

bool32 EmoteWheel_IsOpen(void)
{
    return sWheel.open;
}

static bool32 IsBalloon(const struct Sprite *sprite)
{
    return sprite->inUse && sprite->template >= &sBalloonTemplates[0] && sprite->template < &sBalloonTemplates[ICON_COUNT];
}

static bool32 AnyEmoteSprites(void)
{
    u32 i;

    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (IsBalloon(&gSprites[i])
         || (gSprites[i].inUse && (gSprites[i].template == &sIconTemplate || gSprites[i].template == &sCursorTemplate)))
            return TRUE;
    }
    return FALSE;
}

static void FreePaletteIfUnused(void)
{
    if (!AnyEmoteSprites())
        FreeSpritePaletteByTag(TAG_EMOTE_ICONS);
}

// Frees a balloon's sheet once no other balloon shows that icon.
static void DestroyBalloon(struct Sprite *sprite)
{
    u32 i;
    const struct SpriteTemplate *template = sprite->template;

    DestroySprite(sprite);
    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].template == template)
            return;
    }
    FreeSpriteTilesByTag(template->tileTag);
}

// Balloons ---------------------------------------------------------------------

#define sLocalId  data[0]
#define sMapNum   data[1]
#define sMapGroup data[2]
#define sYVel     data[3]
#define sYOffset  data[4]
#define sTimer    data[5]

static void SpriteCB_Balloon(struct Sprite *sprite)
{
    u8 objEventId;

    if (TryGetObjectEventIdByLocalIdAndMap(sprite->sLocalId, sprite->sMapNum, sprite->sMapGroup, &objEventId)
     || ++sprite->sTimer > BALLOON_FRAMES)
    {
        DestroyBalloon(sprite);
        FreePaletteIfUnused();
        return;
    }
    else
    {
        struct Sprite *objSprite = &gSprites[gObjectEvents[objEventId].spriteId];

        // Hop up, then settle (like the "!" above trainers).
        sprite->sYOffset += sprite->sYVel;
        if (sprite->sYOffset < 0)
            sprite->sYVel++;
        else
            sprite->sYOffset = sprite->sYVel = 0;
        sprite->x = objSprite->x;
        sprite->y = objSprite->y - 27; // above the head and anything worn on it
        sprite->x2 = objSprite->x2;
        sprite->y2 = objSprite->y2 + sprite->sYOffset;
        sprite->invisible = sprite->sTimer > BALLOON_FRAMES - 16 && (sprite->sTimer & 2);
    }
}

void EmoteWheel_ShowBalloon(u8 localId, u8 emote)
{
    u32 i;
    u8 spriteId, icon;

    if (emote >= EMOTE_COUNT)
        return;
    icon = sEmotes[emote].icon;
    // One balloon per person: a new emote replaces the last one.
    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (IsBalloon(&gSprites[i]) && gSprites[i].sLocalId == localId)
            DestroyBalloon(&gSprites[i]);
    }
    if (GetSpriteTileStartByTag(TAG_BALLOON + icon) == TAG_NONE)
    {
        struct SpriteSheet sheet = {sIconsGfx + icon * 2 * ICON_BYTES, 2 * ICON_BYTES, TAG_BALLOON + icon};

        if (LoadSpriteSheet(&sheet) == 0)
            return; // no room for the tiles
    }
    LoadSpritePalette(&sIconsSpritePalette);
    spriteId = CreateSpriteAtEnd(&sBalloonTemplates[icon], 0, 0, 0);
    if (spriteId == MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_BALLOON + icon);
        FreePaletteIfUnused();
        return;
    }
    gSprites[spriteId].coordOffsetEnabled = TRUE;
    gSprites[spriteId].oam.priority = 1;
    gSprites[spriteId].sLocalId = localId;
    gSprites[spriteId].sMapNum = gSaveBlock1Ptr->location.mapNum;
    gSprites[spriteId].sMapGroup = gSaveBlock1Ptr->location.mapGroup;
    gSprites[spriteId].sYVel = -5;
    gSprites[spriteId].sYOffset = -1;
    SpriteCB_Balloon(&gSprites[spriteId]);
}

#undef sLocalId
#undef sMapNum
#undef sMapGroup
#undef sYVel
#undef sYOffset
#undef sTimer

// The wheel --------------------------------------------------------------------

static void Task_EmoteWheel(u8 taskId);

#define tChosen data[0]

static void Task_FinishWheel(u8 taskId)
{
    u8 emote = gTasks[taskId].tChosen;

    DestroyTask(taskId);
    sWheel.onChosen(emote);
}

static void DrawLabel(void)
{
    u8 emote = sPages[sWheel.page][sWheel.slot];
    u8 *str;

    FillWindowPixelBuffer(sWheel.windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(sWheel.windowId, FONT_NORMAL, sEmotes[emote].name, 4, 1, TEXT_SKIP_DRAW, NULL);
    str = StringCopy(gStringVar4, COMPOUND_STRING("{L_BUTTON}{R_BUTTON} "));
    str = StringCopy(str, sPageNames[sWheel.page]);
    str = StringCopy(str, COMPOUND_STRING(" "));
    str = ConvertIntToDecimalStringN(str, sWheel.page + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    str = StringCopy(str, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(str, PAGE_COUNT, STR_CONV_MODE_LEFT_ALIGN, 1);
    AddTextPrinterParameterized(sWheel.windowId, FONT_SMALL, gStringVar4, 128, 1, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sWheel.windowId, FONT_SMALL,
                                COMPOUND_STRING("{DPAD_NONE} Point  {A_BUTTON} Send  {B_BUTTON} Close"), 4, 17, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sWheel.windowId, COPYWIN_GFX);
}

static void ShowPage(void)
{
    u32 i;

    for (i = 0; i < WHEEL_SLOTS; i++)
    {
        struct Sprite *icon = &gSprites[sWheel.icons[i]];

        StartSpriteAnim(icon, sEmotes[sPages[sWheel.page][i]].icon);
        icon->animPaused = (i != sWheel.slot);
    }
    gSprites[sWheel.cursor].x = sWheel.centerX + sSlotOffsets[sWheel.slot][0];
    gSprites[sWheel.cursor].y = sWheel.centerY + sSlotOffsets[sWheel.slot][1];
    DrawLabel();
}

void EmoteWheel_Open(void (*onChosen)(u8 emote))
{
    u32 i;

    sWheel.onChosen = onChosen;
    sWheel.page = sLastPage % PAGE_COUNT;
    sWheel.slot = sLastSlot % WHEEL_SLOTS;
    sWheel.lastDpad = 0;
    sWheel.open = TRUE;
    sWheel.ready = FALSE;
    // The middle of the player on screen.
    // The camera keeps the player in the middle of the screen.
    sWheel.centerX = DISPLAY_WIDTH / 2;
    sWheel.centerY = 70;

    LoadSpritePalette(&sIconsSpritePalette);
    LoadSpriteSheet(&sWheelSheet);
    LoadSpriteSheet(&sCursorSheet);
    for (i = 0; i < WHEEL_SLOTS; i++)
    {
        sWheel.icons[i] = CreateSprite(&sIconTemplate, sWheel.centerX + sSlotOffsets[i][0],
                                       sWheel.centerY + sSlotOffsets[i][1], 1);
        if (sWheel.icons[i] != MAX_SPRITES)
            gSprites[sWheel.icons[i]].coordOffsetEnabled = FALSE;
    }
    sWheel.cursor = CreateSprite(&sCursorTemplate, sWheel.centerX, sWheel.centerY, 0);
    if (sWheel.cursor != MAX_SPRITES)
        gSprites[sWheel.cursor].coordOffsetEnabled = FALSE;

    sWheel.windowId = AddWindow(&sLabelWindowTemplate);
    DrawStdWindowFrame(sWheel.windowId, FALSE);
    PutWindowTilemap(sWheel.windowId);
    PlaySE(SE_WIN_OPEN);
    if (sWheel.cursor == MAX_SPRITES)
    {
        // Out of sprites: still usable through the label.
        sWheel.cursor = sWheel.icons[0];
    }
    ShowPage();
    CopyWindowToVram(sWheel.windowId, COPYWIN_FULL);
    CreateTask(Task_EmoteWheel, 80);
}

static void CloseWheel(u8 taskId, u8 emote)
{
    u32 i;

    for (i = 0; i < WHEEL_SLOTS; i++)
    {
        if (sWheel.icons[i] != MAX_SPRITES && sWheel.icons[i] != sWheel.cursor)
            DestroySprite(&gSprites[sWheel.icons[i]]);
    }
    if (sWheel.cursor != MAX_SPRITES)
        DestroySprite(&gSprites[sWheel.cursor]);
    FreeSpriteTilesByTag(TAG_WHEEL_SHEET);
    FreeSpriteTilesByTag(TAG_WHEEL_CURSOR);
    ClearStdWindowAndFrameToTransparent(sWheel.windowId, FALSE);
    ClearWindowTilemap(sWheel.windowId);
    RemoveWindow(sWheel.windowId);
    ScheduleBgCopyTilemapToVram(0);
    sWheel.open = FALSE;
    sLastPage = sWheel.page;
    sLastSlot = sWheel.slot;
    FreePaletteIfUnused();
    // Answer next frame: the freed sprite tiles may still have image copies
    // queued this frame, which would land on a new balloon's tiles.
    gTasks[taskId].tChosen = emote;
    gTasks[taskId].func = Task_FinishWheel;
}

static s32 SlotFromDpad(u32 dpad)
{
    switch (dpad)
    {
    case DPAD_UP:                return 0;
    case DPAD_UP | DPAD_RIGHT:   return 1;
    case DPAD_RIGHT:             return 2;
    case DPAD_DOWN | DPAD_RIGHT: return 3;
    case DPAD_DOWN:              return 4;
    case DPAD_DOWN | DPAD_LEFT:  return 5;
    case DPAD_LEFT:              return 6;
    case DPAD_UP | DPAD_LEFT:    return 7;
    default:                     return -1;
    }
}

static void Task_EmoteWheel(u8 taskId)
{
    u32 dpad = gMain.heldKeys & DPAD_ANY;

    if (!sWheel.ready)
    {
        sWheel.ready = TRUE;
        sWheel.lastDpad = dpad;
        return;
    }
    // Point with the D-pad. Only a newly pressed direction moves the cursor,
    // so letting go of a diagonal one key at a time keeps it there.
    if (dpad & ~sWheel.lastDpad)
    {
        s32 slot = SlotFromDpad(dpad);

        if (slot >= 0 && slot != sWheel.slot)
        {
            sWheel.slot = slot;
            PlaySE(SE_SELECT);
            ShowPage();
        }
    }
    sWheel.lastDpad = dpad;

    if (JOY_NEW(A_BUTTON))
    {
        CloseWheel(taskId, sPages[sWheel.page][sWheel.slot]);
    }
    else if (JOY_NEW(B_BUTTON | START_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseWheel(taskId, EMOTE_NONE);
    }
    else if (JOY_NEW(L_BUTTON | R_BUTTON))
    {
        sWheel.page = (sWheel.page + (JOY_NEW(R_BUTTON) ? 1 : PAGE_COUNT - 1)) % PAGE_COUNT;
        PlaySE(SE_SELECT);
        ShowPage();
    }
}
