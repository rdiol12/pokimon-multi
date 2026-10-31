#include "global.h"
#include "community.h"
#include "easy_chat.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "region_map.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/easy_chat.h"
#include "constants/songs.h"

// Chat and lobby rooms (server/community_chat.py): START > ONLINE >
// COMMUNITY > CHAT lists the rooms, then shows the one you're in. Messages
// are Easy Chat phrases, picked on the game's own word screen.

extern const u8 Community_EventScript_Chat[];

#define CHAT_WORDS   6
#define CHAT_HISTORY 4
#define CHAT_MESSAGE_SIZE (8 + CHAT_WORDS * 2)

// gCommunityMailbox.events[COMMUNITY_EVENT_CHAT]: a counter, how many
// messages, then each one's name (8 bytes) and words (little-endian u16s).
// Read byte by byte: the build pads structs, so no struct overlays it.
#define CHAT_EVENT gCommunityMailbox.events[COMMUNITY_EVENT_CHAT]

static EWRAM_DATA u16 sWords[CHAT_WORDS] = {0};
static EWRAM_DATA u8 sRoom = 0;

void Community_OpenChat(void)
{
    ScriptContext_SetupScript(Community_EventScript_Chat);
}

static u8 *BufferAreaName(u8 *dest)
{
    return GetMapNameGeneric(dest, gMapHeader.regionMapSectionId);
}

// Script, waitstate: the room list. Sends this area's name for its room.
void CommunityChat_RequestRooms(void)
{
    u8 area[24];

    BufferAreaName(area);
    Community_RequestAndWait(COMMUNITY_REQ_CHAT_ROOMS, 0, 0, 0, area, StringLength(area) + 1);
}

// Script, waitstate: VAR_RESULT = the list row picked; joins that room.
void CommunityChat_JoinRoom(void)
{
    u8 area[24];

    sRoom = gCommunityMailbox.respData[gSpecialVar_Result];
    BufferAreaName(area);
    Community_RequestAndWait(COMMUNITY_REQ_CHAT_JOIN, sRoom, 0, 0, area, StringLength(area) + 1);
}

// Script, waitstate: the Easy Chat screen for a new phrase. Fade out first.
void CommunityChat_PickPhrase(void)
{
    u32 i;

    for (i = 0; i < CHAT_WORDS; i++)
        sWords[i] = EC_EMPTY_WORD;
    DoEasyChatScreen(EASY_CHAT_TYPE_COMMUNITY_CHAT, sWords, CB2_ReturnToFieldContinueScript, EASY_CHAT_PERSON_DISPLAY_NONE);
}

// Script, waitstate: sends the phrase just picked (if any).
void CommunityChat_Say(void)
{
    Community_RequestAndWait(COMMUNITY_REQ_CHAT_SAY, sRoom, 0, 0, sWords, sizeof(sWords));
}

void CommunityChat_Leave(void)
{
    Community_Request(COMMUNITY_REQ_CHAT_LEAVE, 0, 0, 0, 0, NULL, 0);
}

// --- The room window ------------------------------------------------------

static const struct WindowTemplate sRoomWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static const u8 sNameColor[] = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};

#define tWindowId data[0]
#define tSeq      data[1]

static void DrawRoom(struct Task *task)
{
    u8 text[96];
    u32 i, count = min(CHAT_EVENT[1], CHAT_HISTORY);

    FillWindowPixelBuffer(task->tWindowId, PIXEL_FILL(1));
    StringCopyN(text, gCommunityMailbox.rows[0], COMMUNITY_ROW_LENGTH);
    text[COMMUNITY_ROW_LENGTH] = EOS;
    AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, text, 4, 0, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < count; i++)
    {
        const u8 *message = &CHAT_EVENT[2 + i * CHAT_MESSAGE_SIZE];
        u16 words[CHAT_WORDS];
        u32 w, y = 16 + i * 28;

        for (w = 0; w < CHAT_WORDS; w++)
            words[w] = message[8 + w * 2] | (message[9 + w * 2] << 8);
        StringCopyN(text, message, 7);
        text[7] = EOS;
        AddTextPrinterParameterized3(task->tWindowId, FONT_SMALL, 4, y + 2, sNameColor, TEXT_SKIP_DRAW, text);
        ConvertEasyChatWordsToString(text, words, 3, 2);
        AddTextPrinterParameterized(task->tWindowId, FONT_SMALL, text, 52, y, TEXT_SKIP_DRAW, NULL);
    }
    if (count == 0)
        AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, COMPOUND_STRING("It's quiet here. Say hello!"), 4, 32, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(task->tWindowId, FONT_SMALL, COMPOUND_STRING("A: SAY SOMETHING   B: LEAVE"), 4, 130, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
}

static void Task_Room(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (JOY_NEW(A_BUTTON | B_BUTTON) || !Community_IsOnline())
    {
        PlaySE(SE_SELECT);
        gSpecialVar_Result = JOY_NEW(A_BUTTON) && Community_IsOnline();
        ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
        ClearWindowTilemap(task->tWindowId);
        RemoveWindow(task->tWindowId);
        ScheduleBgCopyTilemapToVram(0);
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    if (CHAT_EVENT[0] != (u8)task->tSeq)
    {
        task->tSeq = CHAT_EVENT[0];
        PlaySE(SE_BALL_TRAY_EXIT);
        DrawRoom(task);
    }
}

// Script, waitstate: the room. VAR_RESULT: TRUE to say something, FALSE to leave.
void CommunityChat_ShowRoom(void)
{
    u8 taskId = CreateTask(Task_Room, 80);
    struct Task *task = &gTasks[taskId];

    task->tSeq = CHAT_EVENT[0];
    task->tWindowId = AddWindow(&sRoomWindowTemplate);
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PutWindowTilemap(task->tWindowId);
    DrawRoom(task);
    ScheduleBgCopyTilemapToVram(0);
}

#undef tWindowId
#undef tSeq
