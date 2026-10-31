#include "global.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "pokemon_icon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "window.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "constants/species.h"

// Pokémon orchestra: the CONDUCTOR in Verdanturf leads a quartet of
// Pokémon musicians, one on each direction of the + Control Pad. They play
// a tune; the player plays it back. Each tune right adds a note, up to
// ORCHESTRA_MAX_NOTES. The longest tune played back is the record, and
// reaching 5, 8 and 12 notes earns a prize each.

extern const u8 Orchestra_EventScript_Conductor[];

enum { PAD_UP, PAD_LEFT, PAD_RIGHT, PAD_DOWN, PAD_COUNT };

static const struct
{
    u16 species;
    u16 note;
    u8 x, y;   // the pad in the window, 28 x 20 pixels
    u8 color;  // lit
} sPads[PAD_COUNT] =
{
    [PAD_UP]    = {SPECIES_CHIMECHO, SE_NOTE_C_HIGH, 26, 0, TEXT_COLOR_LIGHT_BLUE},
    [PAD_LEFT]  = {SPECIES_JIGGLYPUFF, SE_NOTE_E, 0, 22, TEXT_COLOR_LIGHT_RED},
    [PAD_RIGHT] = {SPECIES_KRICKETUNE, SE_NOTE_G, 52, 22, TEXT_COLOR_LIGHT_GREEN},
    [PAD_DOWN]  = {SPECIES_LOUDRED, SE_NOTE_C, 26, 44, TEXT_COLOR_BLUE},
};

#define PAD_W 28
#define PAD_H 20

#define STAGE_X 10 // tiles
#define STAGE_Y 1
#define STAGE_W 10
#define STAGE_H 8
#define STAGE_BASE_BLOCK 0xD0

#define LIT_FRAMES   22
#define GAP_FRAMES   10
#define INPUT_FRAMES (60 * 6) // to press the next note

static const u16 sPrizes[] = {ITEM_METRONOME, ITEM_SOOTHE_BELL, ITEM_THROAT_SPRAY};
static const u8 sPrizeNotes[] = {5, 8, ORCHESTRA_MAX_NOTES};

enum
{
    STATE_START,
    STATE_PLAY_NOTE,
    STATE_PLAY_GAP,
    STATE_INPUT,
    STATE_INPUT_LIT,
    STATE_TUNE_DONE,
    STATE_END,
};

struct Orchestra
{
    u8 tune[ORCHESTRA_MAX_NOTES];
    u8 length;   // notes in the tune now
    u8 index;    // the note being played or expected
    u8 state;
    u8 timer;
    u16 inputTimer;
    u8 best;     // longest tune played back
    u8 lit;      // pad, or PAD_COUNT
    u8 windowId;
    u8 icons[PAD_COUNT];
    bool8 wrong;
};

// Not static: the screenshot driver reads the tune to play it back.
EWRAM_DATA struct Orchestra gOrchestra = {0};
#define sGame gOrchestra

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gOrchestraSpots[1] =
{
    {MAP_VERDANTURF_TOWN, 17, 10, OBJ_EVENT_GFX_GENTLEMAN, MOVEMENT_TYPE_FACE_DOWN, 0, Orchestra_EventScript_Conductor},
};

// The stage --------------------------------------------------------------------

static void DrawStage(void)
{
    static const u8 sLabelColors[] = {TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY};
    bool32 yourTurn = sGame.state == STATE_INPUT || sGame.state == STATE_INPUT_LIT;
    u32 i;

    FillWindowPixelBuffer(sGame.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY));
    for (i = 0; i < PAD_COUNT; i++)
    {
        u32 color = (sGame.lit == i) ? sPads[i].color : TEXT_COLOR_LIGHT_GRAY;
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), sPads[i].x, sPads[i].y, PAD_W, PAD_H);
        FillWindowPixelRect(sGame.windowId, PIXEL_FILL(color), sPads[i].x + 1, sPads[i].y + 1, PAD_W - 2, PAD_H - 2);
    }
    // In the middle: whose turn it is.
    AddTextPrinterParameterized3(sGame.windowId, FONT_SMALL, 31, 25, sLabelColors, TEXT_SKIP_DRAW,
                                 yourTurn ? COMPOUND_STRING("YOU!") : COMPOUND_STRING("WAIT"));
    CopyWindowToVram(sGame.windowId, COPYWIN_GFX);
}

static void OpenStage(void)
{
    struct WindowTemplate template = CreateWindowTemplate(0, STAGE_X, STAGE_Y, STAGE_W, STAGE_H, 15, STAGE_BASE_BLOCK);
    u32 i;

    sGame.windowId = AddWindow(&template);
    PutWindowTilemap(sGame.windowId);
    SetStandardWindowBorderStyle(sGame.windowId, TRUE);
    sGame.lit = PAD_COUNT;
    DrawStage();
    CopyWindowToVram(sGame.windowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
    for (i = 0; i < PAD_COUNT; i++)
    {
        s16 x = STAGE_X * 8 + sPads[i].x + PAD_W / 2;
        s16 y = STAGE_Y * 8 + sPads[i].y + PAD_H / 2 - 2;

        LoadMonIconPalette(sPads[i].species);
        sGame.icons[i] = CreateMonIconNoPersonality(sPads[i].species, SpriteCallbackDummy, x, y, 0);
        gSprites[sGame.icons[i]].oam.priority = 0;
    }
}

static void CloseStage(void)
{
    u32 i;

    for (i = 0; i < PAD_COUNT; i++)
    {
        FreeAndDestroyMonIconSprite(&gSprites[sGame.icons[i]]);
        FreeMonIconPalette(sPads[i].species);
    }
    ClearStdWindowAndFrameToTransparent(sGame.windowId, TRUE);
    RemoveWindow(sGame.windowId);
}

static void LightPad(u32 pad)
{
    sGame.lit = pad;
    if (pad < PAD_COUNT)
    {
        gSprites[sGame.icons[pad]].y2 = -4;
        PlaySE(sPads[pad].note);
    }
    else
    {
        u32 i;
        for (i = 0; i < PAD_COUNT; i++)
            gSprites[sGame.icons[i]].y2 = 0;
    }
    DrawStage();
}

// The game -----------------------------------------------------------------------

static void AddNote(void)
{
    sGame.tune[sGame.length++] = Random() % PAD_COUNT;
}

static s32 PressedPad(void)
{
    if (JOY_NEW(DPAD_UP))
        return PAD_UP;
    if (JOY_NEW(DPAD_LEFT))
        return PAD_LEFT;
    if (JOY_NEW(DPAD_RIGHT))
        return PAD_RIGHT;
    if (JOY_NEW(DPAD_DOWN))
        return PAD_DOWN;
    return -1;
}

static void Task_Orchestra(u8 taskId)
{
    s32 pad;

    switch (sGame.state)
    {
    case STATE_START:
        if (++sGame.timer < 40)
            break;
        sGame.timer = 0;
        sGame.index = 0;
        sGame.state = STATE_PLAY_NOTE;
        LightPad(sGame.tune[0]);
        break;
    case STATE_PLAY_NOTE:
        if (++sGame.timer < LIT_FRAMES)
            break;
        sGame.timer = 0;
        LightPad(PAD_COUNT);
        sGame.state = STATE_PLAY_GAP;
        break;
    case STATE_PLAY_GAP:
        if (++sGame.timer < GAP_FRAMES)
            break;
        sGame.timer = 0;
        if (++sGame.index < sGame.length)
        {
            LightPad(sGame.tune[sGame.index]);
            sGame.state = STATE_PLAY_NOTE;
        }
        else
        {
            sGame.index = 0;
            sGame.inputTimer = 0;
            sGame.state = STATE_INPUT;
            DrawStage();
        }
        break;
    case STATE_INPUT:
        pad = PressedPad();
        if (pad < 0)
        {
            if (++sGame.inputTimer >= INPUT_FRAMES)
            {
                sGame.wrong = TRUE;
                sGame.state = STATE_END;
            }
            break;
        }
        LightPad(pad);
        sGame.timer = 0;
        if (pad != sGame.tune[sGame.index])
        {
            PlaySE(SE_FAILURE);
            sGame.wrong = TRUE;
        }
        sGame.state = STATE_INPUT_LIT;
        break;
    case STATE_INPUT_LIT:
        if (++sGame.timer < LIT_FRAMES / 2)
            break;
        sGame.timer = 0;
        LightPad(PAD_COUNT);
        sGame.inputTimer = 0;
        if (sGame.wrong)
            sGame.state = STATE_END;
        else if (++sGame.index >= sGame.length)
            sGame.state = STATE_TUNE_DONE;
        else
            sGame.state = STATE_INPUT;
        break;
    case STATE_TUNE_DONE:
        if (sGame.timer++ == 0)
        {
            sGame.best = sGame.length;
            PlaySE(SE_SUCCESS);
        }
        if (sGame.timer < 40)
            break;
        sGame.timer = 0;
        if (sGame.length >= ORCHESTRA_MAX_NOTES)
        {
            sGame.state = STATE_END;
            break;
        }
        AddNote();
        sGame.index = 0;
        sGame.state = STATE_PLAY_NOTE;
        LightPad(sGame.tune[0]);
        break;
    case STATE_END:
        if (++sGame.timer < 30)
            break;
        CloseStage();
        DestroyTask(taskId);
        ScriptContext_Enable();
        break;
    }
}

// Script: `callnative Orchestra_Play, waitstate=1`. Afterwards gStringVar1
// = the longest tune played back, gStringVar2 = the record; VAR_RESULT =
// TRUE if it's a new record; VAR_0x8005 = a prize (else ITEM_NONE).
void Orchestra_Play(void)
{
    u32 i;

    memset(&sGame, 0, sizeof(sGame));
    for (i = 0; i < ORCHESTRA_FIRST_NOTES; i++)
        AddNote();
    OpenStage();
    CreateTask(Task_Orchestra, 80);
}

void Orchestra_Finish(void)
{
    u32 i;

    if (sSave.orchestraPlays < 0xFFFF)
        sSave.orchestraPlays++;
    gSpecialVar_Result = sGame.best > sSave.orchestraBest;
    if (gSpecialVar_Result)
        sSave.orchestraBest = sGame.best;
    ConvertIntToDecimalStringN(gStringVar1, sGame.best, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, sSave.orchestraBest, STR_CONV_MODE_LEFT_ALIGN, 2);
    gSpecialVar_0x8005 = ITEM_NONE;
    for (i = 0; i < ARRAY_COUNT(sPrizes); i++)
    {
        if (sGame.best >= sPrizeNotes[i] && !(sSave.orchestraPrizes & (1 << i)))
        {
            sSave.orchestraPrizes |= 1 << i;
            gSpecialVar_0x8005 = sPrizes[i];
            break;
        }
    }
}

// R > PASTIMES > ORCHESTRA -------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < PAD_COUNT)
    {
        static const u8 *const sDirections[PAD_COUNT] =
        {
            COMPOUND_STRING("UP"), COMPOUND_STRING("LEFT"), COMPOUND_STRING("RIGHT"), COMPOUND_STRING("DOWN"),
        };
        dest = StringCopy(dest, sDirections[index]);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 48}"));
        StringCopy(dest, GetSpeciesName(sPads[index].species));
        return;
    }
    if (index == PAD_COUNT)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Longest tune{CLEAR_TO 120}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.orchestraBest, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(dest, COMPOUND_STRING(" notes"));
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Concerts played{CLEAR_TO 120}"));
    ConvertIntToDecimalStringN(dest, sSave.orchestraPlays, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void GetDetail(u16 index, u8 *dest)
{
    StringCopy(dest, COMPOUND_STRING("The CONDUCTOR is in VERDANTURF TOWN"));
}

void Orchestra_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("POKéMON ORCHESTRA"), PAD_COUNT + 2, GetLine, GetDetail);
}
