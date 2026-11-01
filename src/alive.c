#include "global.h"
#include "alive.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "data.h"
#include "follower_helper.h"
#include "main.h"
#include "menu.h"
#include "metatile_behavior.h"
#include "netplay.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "rovers.h"
#include "rtc.h"
#include "save_ext.h"
#include "script.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text_window.h"
#include "window.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/map_types.h"
#include "constants/trainer_types.h"

// The living world: the hooks the overworld calls, and what the features
// in alive_*.c share (spawning objects, captions, the clock).

EWRAM_DATA struct AliveShowcase gAliveShowcase = {0};

// --- Clock and places ----------------------------------------------------

u32 Alive_Hour(void)
{
    s32 hours, minutes;
    if (DayNight_GetServerClock(&hours, &minutes))
        return hours;
    RtcCalcLocalTime();
    return gLocalTime.hours;
}

u32 Alive_Day(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

bool32 Alive_IsNight(void)
{
    u32 hour = Alive_Hour();
    return hour >= 21 || hour < 5;
}

bool32 Alive_IsTown(void)
{
    return gMapHeader.mapType == MAP_TYPE_TOWN || gMapHeader.mapType == MAP_TYPE_CITY;
}

bool32 Alive_IsRoute(void)
{
    return gMapHeader.mapType == MAP_TYPE_ROUTE;
}

bool32 Alive_IsOutdoors(void)
{
    return Alive_IsTown() || Alive_IsRoute() || gMapHeader.mapType == MAP_TYPE_OCEAN_ROUTE;
}

// The automated tests walk set routes: nothing turns up in their way unless
// a screenshot scene asked for it.
bool32 Alive_IsQuiet(void)
{
    return Netplay_IsTestBoot() && !gAliveShowcase.living;
}

u32 Alive_MapSeed(void)
{
    return gSaveBlock1Ptr->location.mapGroup * 131 + gSaveBlock1Ptr->location.mapNum * 17 + Alive_Day() * 7919;
}

u32 Alive_Badges(void)
{
    u32 i, n = 0;
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        n += FlagGet(i);
    return n;
}

u16 Alive_LeadSpecies(void)
{
    struct Pokemon *mon = GetFirstLiveMon();
    if (mon == NULL)
        return SPECIES_NONE;
    return GetMonData(mon, MON_DATA_SPECIES);
}

// --- Objects -------------------------------------------------------------

struct ObjectEvent *Alive_Obj(u8 localId)
{
    u8 id = GetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    return id < OBJECT_EVENTS_COUNT ? &gObjectEvents[id] : NULL;
}

void Alive_Remove(u8 localId)
{
    RemoveObjectEventByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

bool32 Alive_Busy(struct ObjectEvent *obj)
{
    return obj != NULL && (obj->singleMovementActive || (obj->heldMovementActive && !obj->heldMovementFinished));
}

void Alive_Do(struct ObjectEvent *obj, u8 action)
{
    if (obj != NULL)
        ObjectEventForceSetHeldMovement(obj, action);
}

u8 Alive_Spawn(u8 localId, u16 gfx, s16 x, s16 y, u32 facing)
{
    struct ObjectEventTemplate template = {0};
    static const u8 sFace[] =
    {
        [DIR_NONE] = MOVEMENT_TYPE_FACE_DOWN,
        [DIR_SOUTH] = MOVEMENT_TYPE_FACE_DOWN, [DIR_NORTH] = MOVEMENT_TYPE_FACE_UP,
        [DIR_WEST] = MOVEMENT_TYPE_FACE_LEFT, [DIR_EAST] = MOVEMENT_TYPE_FACE_RIGHT,
    };
    u8 objectId;

    if (facing > DIR_EAST)
        facing = DIR_SOUTH;
    Alive_Remove(localId);
    template.localId = localId;
    template.graphicsId = gfx;
    template.kind = OBJ_KIND_NORMAL;
    template.x = x - MAP_OFFSET;
    template.y = y - MAP_OFFSET;
    template.elevation = MapGridGetElevationAt(x, y);
    template.movementType = sFace[facing];
    template.trainerType = TRAINER_TYPE_NONE;
    objectId = SpawnSpecialObjectEvent(&template);
    if (objectId < OBJECT_EVENTS_COUNT)
    {
        // Held movements only from here on.
        gObjectEvents[objectId].movementType = MOVEMENT_TYPE_NONE;
        ObjectEventTurn(&gObjectEvents[objectId], facing == DIR_NONE ? DIR_SOUTH : facing);
    }
    return objectId;
}

u16 Alive_MonGfx(u16 species)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        species = SPECIES_ZIGZAGOON;
    return species + OBJ_EVENT_MON;
}

static u32 CountFreeObjects(void)
{
    u32 i, n = 0;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (!gObjectEvents[i].active)
            n++;
    }
    return n;
}

// Busy maps (crowds online, the Haven Reserve) are left alone.
bool32 Alive_RoomForObjects(u32 count)
{
    return CountFreeObjects() >= count + 6 && CountFreePaletteSlots() >= 2;
}

u32 Alive_PlayerDistance(s16 x, s16 y)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    return abs(x - player->currentCoords.x) + abs(y - player->currentCoords.y);
}

// A tile someone can walk onto: no wall, nobody there, not water or a door,
// on the player's level.
bool32 Alive_CanStandAt(s16 x, s16 y)
{
    u32 behavior;
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (!AreCoordsInsidePlayerMap(x, y))
        return FALSE;
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    if (x == player->currentCoords.x && y == player->currentCoords.y)
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) || MetatileBehavior_IsWarpDoor(behavior)
     || MetatileBehavior_IsDoor(behavior) || MetatileBehavior_IsNonAnimDoor(behavior)
     || MetatileBehavior_IsLongGrass(behavior) || MetatileBehavior_IsJumpSouth(behavior)
     || MetatileBehavior_IsJumpNorth(behavior) || MetatileBehavior_IsJumpEast(behavior) || MetatileBehavior_IsJumpWest(behavior))
        return FALSE;
    return MapGridGetElevationAt(x, y) == player->currentElevation || MapGridGetElevationAt(x, y) == 0
        || player->currentElevation == 0;
}

bool32 Alive_FindSpotNear(s16 cx, s16 cy, u32 radius, s16 *outX, s16 *outY)
{
    u32 tries;

    if (Alive_CanStandAt(cx, cy))
    {
        *outX = cx;
        *outY = cy;
        return TRUE;
    }
    for (tries = 0; tries < 40; tries++)
    {
        s16 x = cx - radius + Random() % (radius * 2 + 1);
        s16 y = cy - radius + Random() % (radius * 2 + 1);
        if (!Alive_CanStandAt(x, y))
            continue;
        *outX = x;
        *outY = y;
        return TRUE;
    }
    return FALSE;
}

// A free tile somewhere on screen, at least minDist steps from the player.
bool32 Alive_FindSpotOnScreen(u32 minDist, s16 *outX, s16 *outY)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 tries;

    for (tries = 0; tries < 50; tries++)
    {
        s16 x = player->currentCoords.x - 6 + Random() % 13;
        s16 y = player->currentCoords.y - 4 + Random() % 9;
        if (Alive_PlayerDistance(x, y) < minDist || !Alive_CanStandAt(x, y))
            continue;
        *outX = x;
        *outY = y;
        return TRUE;
    }
    return FALSE;
}

// One step toward (tx, ty), around whatever is in the way. FALSE if there
// is nowhere to go (or it's there).
bool32 Alive_StepToward(struct ObjectEvent *obj, s16 tx, s16 ty, bool32 fast)
{
    s16 dx = tx - obj->currentCoords.x, dy = ty - obj->currentCoords.y;
    u32 dirs[4], n = 0, i;

    if (dx == 0 && dy == 0)
        return FALSE;
    if (abs(dx) >= abs(dy))
    {
        if (dx != 0) dirs[n++] = dx > 0 ? DIR_EAST : DIR_WEST;
        if (dy != 0) dirs[n++] = dy > 0 ? DIR_SOUTH : DIR_NORTH;
    }
    else
    {
        if (dy != 0) dirs[n++] = dy > 0 ? DIR_SOUTH : DIR_NORTH;
        if (dx != 0) dirs[n++] = dx > 0 ? DIR_EAST : DIR_WEST;
    }
    // Sidestep when blocked.
    if (n < 4)
    {
        u32 side = (dirs[0] == DIR_EAST || dirs[0] == DIR_WEST) ? ((Random() & 1) ? DIR_NORTH : DIR_SOUTH)
                                                               : ((Random() & 1) ? DIR_EAST : DIR_WEST);
        dirs[n++] = side;
    }
    for (i = 0; i < n; i++)
    {
        s16 x = obj->currentCoords.x, y = obj->currentCoords.y;
        MoveCoords(dirs[i], &x, &y);
        // The goal itself may be a door.
        if ((x == tx && y == ty && MapGridGetCollisionAt(x, y) == 0 && GetObjectEventIdByXY(x, y) == OBJECT_EVENTS_COUNT)
         || Alive_CanStandAt(x, y))
        {
            Alive_Do(obj, fast ? GetWalkFastMovementAction(dirs[i]) : GetWalkNormalMovementAction(dirs[i]));
            return TRUE;
        }
    }
    return FALSE;
}

// --- Townsfolk -----------------------------------------------------------

bool32 Alive_IsHumanGfx(u16 gfx)
{
    if (gfx >= OBJ_EVENT_MON)
        return FALSE;
    switch (gfx)
    {
    case OBJ_EVENT_GFX_NINJA_BOY ... OBJ_EVENT_GFX_NURSE:
    case OBJ_EVENT_GFX_MAN_4:
    case OBJ_EVENT_GFX_MAN_5:
    case OBJ_EVENT_GFX_REPORTER_M:
    case OBJ_EVENT_GFX_REPORTER_F:
    case OBJ_EVENT_GFX_BARD:
    case OBJ_EVENT_GFX_MART_EMPLOYEE:
    case OBJ_EVENT_GFX_ARTIST:
    case OBJ_EVENT_GFX_SCIENTIST_2:
    case OBJ_EVENT_GFX_DEVON_EMPLOYEE:
    case OBJ_EVENT_GFX_LITTLE_BOY_3:
    case OBJ_EVENT_GFX_GAMEBOY_KID:
    case OBJ_EVENT_GFX_BOY ... OBJ_EVENT_GFX_OLD_MAN_2:
    case OBJ_EVENT_GFX_COOLTRAINER_M:
    case OBJ_EVENT_GFX_COOLTRAINER_F:
        return gfx != OBJ_EVENT_GFX_RAYQUAZA_STILL && gfx != OBJ_EVENT_GFX_LINK_RECEPTIONIST;
    default:
        return FALSE;
    }
}

// An ordinary person on the map: one of the map's own objects, not a
// trainer, not someone the story moves around.
bool32 Alive_IsTownsperson(struct ObjectEvent *obj)
{
    if (obj == NULL || !obj->active || obj->isPlayer || obj->invisible)
        return FALSE;
    if (obj->localId == 0 || obj->localId >= LOCALID_ALIVE_FIRST)
        return FALSE;
    if (obj->trainerType != TRAINER_TYPE_NONE)
        return FALSE;
    if (Netplay_IsNetplayObjectEvent(obj))
        return FALSE;
    return Alive_IsHumanGfx(obj->graphicsId);
}

const u8 *Alive_SpeakerName(struct ObjectEvent *obj)
{
    switch (obj->graphicsId)
    {
    case OBJ_EVENT_GFX_OLD_MAN:
    case OBJ_EVENT_GFX_OLD_MAN_1:
    case OBJ_EVENT_GFX_OLD_MAN_2:
        return COMPOUND_STRING("OLD MAN");
    case OBJ_EVENT_GFX_OLD_WOMAN:
        return COMPOUND_STRING("OLD LADY");
    case OBJ_EVENT_GFX_LITTLE_BOY:
    case OBJ_EVENT_GFX_LITTLE_GIRL:
    case OBJ_EVENT_GFX_LITTLE_BOY_3:
    case OBJ_EVENT_GFX_NINJA_BOY:
    case OBJ_EVENT_GFX_TWIN:
    case OBJ_EVENT_GFX_GAMEBOY_KID:
    case OBJ_EVENT_GFX_TUBER_F:
    case OBJ_EVENT_GFX_TUBER_M:
        return COMPOUND_STRING("KID");
    case OBJ_EVENT_GFX_BOY_1:
    case OBJ_EVENT_GFX_BOY_2:
    case OBJ_EVENT_GFX_BOY_3:
    case OBJ_EVENT_GFX_YOUNGSTER:
    case OBJ_EVENT_GFX_SCHOOL_KID_M:
    case OBJ_EVENT_GFX_BOY:
    case OBJ_EVENT_GFX_RICH_BOY:
        return COMPOUND_STRING("BOY");
    case OBJ_EVENT_GFX_GIRL_1:
    case OBJ_EVENT_GFX_GIRL_2:
    case OBJ_EVENT_GFX_GIRL_3:
    case OBJ_EVENT_GFX_LASS:
    case OBJ_EVENT_GFX_CRUSH_GIRL:
        return COMPOUND_STRING("GIRL");
    case OBJ_EVENT_GFX_SAILOR:
        return COMPOUND_STRING("SAILOR");
    case OBJ_EVENT_GFX_FISHERMAN:
        return COMPOUND_STRING("FISHERMAN");
    case OBJ_EVENT_GFX_HIKER:
        return COMPOUND_STRING("HIKER");
    case OBJ_EVENT_GFX_SCIENTIST_1:
    case OBJ_EVENT_GFX_SCIENTIST_2:
        return COMPOUND_STRING("SCIENTIST");
    case OBJ_EVENT_GFX_COOK:
        return COMPOUND_STRING("COOK");
    case OBJ_EVENT_GFX_NURSE:
        return COMPOUND_STRING("NURSE");
    case OBJ_EVENT_GFX_BEAUTY:
        return COMPOUND_STRING("BEAUTY");
    case OBJ_EVENT_GFX_GENTLEMAN:
        return COMPOUND_STRING("GENTLEMAN");
    case OBJ_EVENT_GFX_EXPERT_M:
    case OBJ_EVENT_GFX_EXPERT_F:
        return COMPOUND_STRING("EXPERT");
    case OBJ_EVENT_GFX_POKEFAN_M:
    case OBJ_EVENT_GFX_POKEFAN_F:
        return COMPOUND_STRING("POKéFAN");
    case OBJ_EVENT_GFX_ARTIST:
        return COMPOUND_STRING("ARTIST");
    case OBJ_EVENT_GFX_BARD:
        return COMPOUND_STRING("BARD");
    case OBJ_EVENT_GFX_MART_EMPLOYEE:
        return COMPOUND_STRING("CLERK");
    case OBJ_EVENT_GFX_WOMAN_1:
    case OBJ_EVENT_GFX_WOMAN_2:
    case OBJ_EVENT_GFX_WOMAN_3:
    case OBJ_EVENT_GFX_WOMAN_4:
    case OBJ_EVENT_GFX_WOMAN_5:
    case OBJ_EVENT_GFX_PICNICKER:
    case OBJ_EVENT_GFX_COOLTRAINER_F:
    case OBJ_EVENT_GFX_SWIMMER_F:
    case OBJ_EVENT_GFX_HEX_MANIAC:
    case OBJ_EVENT_GFX_REPORTER_F:
        return COMPOUND_STRING("WOMAN");
    default:
        return COMPOUND_STRING("MAN");
    }
}

// --- Captions: a line said nearby, at the bottom of the screen ------------

#define CAPTION_FRAMES 150
#define tTimer    data[0]
#define tWindowId data[1]

static const struct WindowTemplate sCaptionWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 2,
    .tilemapTop = 15,
    .width = 26,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static void Task_Caption(u8 taskId);

static void CloseCaption(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
    ClearWindowTilemap(task->tWindowId);
    RemoveWindow(task->tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
}

static void Task_Caption(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    // A script, a menu, a battle or a warp closes it at once.
    if (++task->tTimer > CAPTION_FRAMES || ArePlayerFieldControlsLocked() || gMain.callback2 != CB2_Overworld)
        CloseCaption(taskId);
}

bool32 Alive_IsCaptionShown(void)
{
    return FindTaskIdByFunc(Task_Caption) != TASK_NONE;
}

void Alive_CloseCaption(void)
{
    u8 taskId = FindTaskIdByFunc(Task_Caption);
    if (taskId != TASK_NONE)
        CloseCaption(taskId);
}

// Breaks a text into lines of at most width characters, at spaces. In a
// message box (msgbox) the third and later lines scroll ("\l"); in a
// caption there's room for two lines and the rest is cut.
void Alive_Wrap(u8 *s, u32 width, bool32 msgbox)
{
    u32 i, col = 0, lines = 1;
    s32 lastSpace = -1;

    for (i = 0; s[i] != EOS; i++)
    {
        if (s[i] == CHAR_NEWLINE || s[i] == CHAR_PROMPT_SCROLL || s[i] == CHAR_PROMPT_CLEAR)
        {
            col = 0;
            lastSpace = -1;
            lines = (s[i] == CHAR_PROMPT_CLEAR) ? 1 : lines + 1;
            continue;
        }
        if (s[i] == CHAR_SPACE)
            lastSpace = i;
        if (++col > width && lastSpace >= 0)
        {
            lines++;
            if (!msgbox && lines > 2)
            {
                s[lastSpace] = EOS;
                return;
            }
            s[lastSpace] = (msgbox && lines > 2) ? CHAR_PROMPT_SCROLL : CHAR_NEWLINE;
            col = i - lastSpace;
            lastSpace = -1;
        }
    }
}

// speaker may be NULL. Long lines are broken (two lines fit).
void Alive_ShowCaption(const u8 *speaker, const u8 *text)
{
    u8 taskId;
    struct Task *task;
    u8 line[160];

    if (Rovers_IsCaptionShown())
        return;
    taskId = FindTaskIdByFunc(Task_Caption);
    if (taskId == TASK_NONE)
    {
        taskId = CreateTask(Task_Caption, 91);
        task = &gTasks[taskId];
        task->tWindowId = AddWindow(&sCaptionWindowTemplate);
        DrawStdWindowFrame(task->tWindowId, FALSE);
    }
    task = &gTasks[taskId];
    task->tTimer = 0;
    FillWindowPixelBuffer(task->tWindowId, PIXEL_FILL(1));
    if (speaker != NULL)
    {
        u8 *end = StringCopy(line, speaker);
        end = StringCopy(end, COMPOUND_STRING(": "));
        StringCopyN(end, text, sizeof(line) - (end - line) - 1);
        line[sizeof(line) - 1] = EOS;
    }
    else
    {
        StringCopyN(line, text, sizeof(line) - 1);
        line[sizeof(line) - 1] = EOS;
    }
    Alive_Wrap(line, 34, FALSE);
    text = line;
    AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, text, 2, 1, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(task->tWindowId);
    CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
}

#undef tTimer
#undef tWindowId

// --- Emotes and the sleeping "Z" ----------------------------------------

void Alive_Emote(struct ObjectEvent *obj, u8 emotion)
{
    if (obj != NULL && obj->active && !obj->invisible)
        ObjectEventEmote(obj, emotion);
}

// A 16x16 "Zz" drawn in code (tools/alive_shots/sprites.py), 4bpp: 1 = outline, 2 = white.
static const u32 sZzzGfx[] =
{
    0x00000000, 0x00000000, 0x10000000, 0x10000000, 0x10000000, 0x00000000, 0x00000000, 0x10000000,
    0x00000000, 0x00000000, 0x01111111, 0x01222222, 0x01122111, 0x00112210, 0x00011221, 0x01111122,
    0x10000000, 0x10000000, 0x00111110, 0x00122210, 0x00112110, 0x00111210, 0x00122210, 0x00111110,
    0x01222222, 0x01111111, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

static const u16 sZzzPal[16] = {RGB(31, 0, 31), RGB(6, 7, 12), RGB(31, 31, 31), RGB(20, 22, 30)};

#define TAG_ALIVE_ZZZ 0x5A11

static const struct OamData sZzzOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static const struct SpriteSheet sZzzSheet = {sZzzGfx, sizeof(sZzzGfx), TAG_ALIVE_ZZZ};
static const struct SpritePalette sZzzPalette = {sZzzPal, TAG_ALIVE_ZZZ};

#define sLocalId data[0]
#define sMapNum  data[1]
#define sMapGrp  data[2]
#define sTimer   data[3]

static void SpriteCB_Zzz(struct Sprite *sprite)
{
    u8 id = GetObjectEventIdByLocalIdAndMap(sprite->sLocalId, sprite->sMapNum, sprite->sMapGrp);
    struct Sprite *owner;

    if (id >= OBJECT_EVENTS_COUNT || gObjectEvents[id].invisible || gMain.callback2 != CB2_Overworld)
    {
        DestroySprite(sprite);
        return;
    }
    owner = &gSprites[gObjectEvents[id].spriteId];
    sprite->sTimer = (sprite->sTimer + 1) % 90;
    sprite->x = owner->x + owner->x2 + 6 + sprite->sTimer / 30;
    sprite->y = owner->y + owner->y2 - 16 - sprite->sTimer / 6;
    sprite->invisible = sprite->sTimer > 70;
}

static void SpriteCB_ZzzDummy(struct Sprite *sprite)
{
}

static const struct SpriteTemplate sZzzTemplate =
{
    .tileTag = TAG_ALIVE_ZZZ,
    .paletteTag = TAG_ALIVE_ZZZ,
    .oam = &sZzzOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_ZzzDummy,
};

void Alive_ShowZzz(struct ObjectEvent *obj)
{
    u8 spriteId;
    u32 i;

    if (obj == NULL || !obj->active)
        return;
    // Already sleeping?
    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].callback == SpriteCB_Zzz && gSprites[i].sLocalId == obj->localId)
            return;
    }
    if (GetSpriteTileStartByTag(TAG_ALIVE_ZZZ) == 0xFFFF)
        LoadSpriteSheet(&sZzzSheet);
    if (IndexOfSpritePaletteTag(TAG_ALIVE_ZZZ) == 0xFF)
        LoadSpritePalette(&sZzzPalette);
    spriteId = CreateSprite(&sZzzTemplate, 0, 0, 0);
    if (spriteId == MAX_SPRITES)
        return;
    gSprites[spriteId].sLocalId = obj->localId;
    gSprites[spriteId].sMapNum = obj->mapNum;
    gSprites[spriteId].sMapGrp = obj->mapGroup;
    gSprites[spriteId].sTimer = Random() % 90;
    gSprites[spriteId].callback = SpriteCB_Zzz;
    SpriteCB_Zzz(&gSprites[spriteId]);
}

// Frees the "Z"'s graphics once nothing sleeps on screen.
void Alive_UpdateZzz(void)
{
    u32 i;

    if (GetSpriteTileStartByTag(TAG_ALIVE_ZZZ) == 0xFFFF)
        return;
    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].callback == SpriteCB_Zzz)
            return;
    }
    FreeSpriteTilesByTag(TAG_ALIVE_ZZZ);
    FreeSpritePaletteByTag(TAG_ALIVE_ZZZ);
}

#undef sLocalId
#undef sMapNum
#undef sMapGrp
#undef sTimer

// --- Hooks ---------------------------------------------------------------

void Alive_OnMapTransition(void)
{
    AliveWild_OnMapTransition();
    AliveTown_OnMapTransition();
    AliveTalk_OnMapTransition();
    AliveNews_OnMapTransition();
    AliveAmbient_OnMapTransition();
    AliveProjects_OnMapTransition();
    AliveEvents_OnMapTransition();
    AliveWeather_OnMapTransition();
}

void Alive_OnMapLoad(void)
{
    AliveProjects_OnMapLoad();
    AliveWeather_OnMapLoad();
}

bool32 Alive_OverworldUpdate(void)
{
    if (gMain.callback2 != CB2_Overworld)
        return FALSE;
    AliveWild_Update();
    Alive_UpdateZzz();
    if (ArePlayerFieldControlsLocked())
        return FALSE;
    AliveTown_Update();
    AliveNews_Update();
    AliveAmbient_Update();
    AliveProjects_Update();
    AliveEvents_Update();
    AliveWeather_Update();
    AliveTalk_UpdateChatter();
    return FALSE;
}

const u8 *Alive_GetTalkScript(struct ObjectEvent *obj, const u8 *script)
{
    const u8 *ours;

    if (obj->localId >= LOCALID_ALIVE_FIRST && obj->localId <= LOCALID_ALIVE_LAST)
    {
        if ((ours = AliveTown_GetTalkScript(obj)) != NULL)
            return ours;
        if ((ours = AliveEvents_GetTalkScript(obj)) != NULL)
            return ours;
        if ((ours = AliveWeather_GetTalkScript(obj)) != NULL)
            return ours;
        if (AliveNews_IsCrier(obj))
        {
            extern const u8 Alive_EventScript_Crier[];
            return Alive_EventScript_Crier;
        }
        if (AliveProjects_IsForeman(obj))
        {
            extern const u8 Alive_EventScript_Foreman[];
            return Alive_EventScript_Foreman;
        }
        return script;
    }
    return AliveTalk_GetRemarkScript(obj, script);
}

bool32 Alive_OnBump(struct ObjectEvent *obj)
{
    return AliveWeather_OnBump(obj);
}
