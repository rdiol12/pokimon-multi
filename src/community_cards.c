#include "global.h"
#include "community.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/songs.h"
#include "constants/trainers.h"

// Trainer cards (server/community_cards.py): START > ONLINE > COMMUNITY >
// CARDS sends your own card to the server, then lists everyone's; picking
// one shows their team, badges, Pokédex, play time and ladder standing.

extern const u8 Community_EventScript_Cards[];

// The card as it travels (byte offsets; no struct, the build pads structs).
#define CARD_NAME      0  // 8 bytes, game charset
#define CARD_PIC       8  // PLAYER_STYLE_*: Brendan, May, Red, Leaf
#define CARD_BADGES    9
#define CARD_DEX       10 // u16
#define CARD_HOURS     12 // u16
#define CARD_MINUTES   14
#define CARD_COUNT     15
#define CARD_SPECIES   16 // u16 x 6
#define CARD_LEVELS    28 // u8 x 6
#define CARD_TID       34 // u16
#define CARD_SHINY     36 // a bit per Pokémon
#define CARD_SIZE      40

static void Put16(u8 *p, u16 v)
{
    p[0] = v;
    p[1] = v >> 8;
}

static u16 Get16(const u8 *p)
{
    return p[0] | (p[1] << 8);
}

void Community_OpenCards(void)
{
    ScriptContext_SetupScript(Community_EventScript_Cards);
}

// Script, waitstate: sends this player's card.
void CommunityCards_Publish(void)
{
    u8 card[CARD_SIZE] = {0};
    u32 i, count = 0, badges = 0;

    StringCopyN(card + CARD_NAME, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    card[CARD_NAME + PLAYER_NAME_LENGTH] = EOS;
    card[CARD_PIC] = gSaveBlock2Ptr->playerStyle != 0 ? gSaveBlock2Ptr->playerStyle - 1 : gSaveBlock2Ptr->playerGender;
    for (i = 0; i < 8; i++)
        badges += FlagGet(FLAG_BADGE01_GET + i);
    card[CARD_BADGES] = badges;
    Put16(card + CARD_DEX, GetNationalPokedexCount(FLAG_GET_CAUGHT));
    Put16(card + CARD_HOURS, gSaveBlock2Ptr->playTimeHours);
    card[CARD_MINUTES] = gSaveBlock2Ptr->playTimeMinutes;
    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER] && i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);

        if (species == SPECIES_NONE || species == SPECIES_EGG)
            continue;
        Put16(card + CARD_SPECIES + count * 2, species);
        card[CARD_LEVELS + count] = GetMonData(mon, MON_DATA_LEVEL);
        if (GetMonData(mon, MON_DATA_IS_SHINY))
            card[CARD_SHINY] |= 1 << count;
        count++;
    }
    card[CARD_COUNT] = count;
    Put16(card + CARD_TID, gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8));
    Community_RequestAndWait(COMMUNITY_REQ_CARD_PUBLISH, 0, 0, 0, card, sizeof(card));
}

// Script, waitstate: VAR_RESULT = the list row picked; asks for that card.
void CommunityCards_View(void)
{
    Community_RequestAndWait(COMMUNITY_REQ_CARD_VIEW, gSpecialVar_Result, 0, 0, NULL, 0);
}

// --- The card ---------------------------------------------------------------

static const struct WindowTemplate sCardWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static const u8 sLabelColor[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sValueColor[] = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};

#define ICON_X 28
#define ICON_Y 120
#define ICON_GAP 36

struct CardView
{
    u8 windowId;
    u8 picSprite;
    u8 iconSprites[PARTY_SIZE];
};

static EWRAM_DATA struct CardView sCard = {0};

static void PrintLine(const u8 *label, const u8 *value, u32 y)
{
    AddTextPrinterParameterized3(sCard.windowId, FONT_NORMAL, 4, y, sLabelColor, TEXT_SKIP_DRAW, label);
    AddTextPrinterParameterized3(sCard.windowId, FONT_NORMAL, 72, y, sValueColor, TEXT_SKIP_DRAW, value);
}

static void DrawCard(void)
{
    const u8 *card = gCommunityMailbox.respData;
    u8 text[40];
    u8 *end;
    u32 i, rank = gCommunityMailbox.respArgs[0];

    FillWindowPixelBuffer(sCard.windowId, PIXEL_FILL(1));
    StringCopyN(text, gCommunityMailbox.rows[0], COMMUNITY_ROW_LENGTH);
    text[COMMUNITY_ROW_LENGTH] = EOS;
    AddTextPrinterParameterized(sCard.windowId, FONT_NORMAL, text, 4, 0, TEXT_SKIP_DRAW, NULL);
    FillWindowPixelRect(sCard.windowId, PIXEL_FILL(2), 4, 14, 216, 1);

    StringCopyN(text, card + CARD_NAME, 7);
    text[7] = EOS;
    PrintLine(COMPOUND_STRING("NAME"), text, 18);
    end = StringCopy(text, COMPOUND_STRING("No."));
    ConvertIntToDecimalStringN(end, Get16(card + CARD_TID), STR_CONV_MODE_LEADING_ZEROS, 5);
    PrintLine(COMPOUND_STRING("ID"), text, 32);
    ConvertIntToDecimalStringN(text, card[CARD_BADGES], STR_CONV_MODE_LEFT_ALIGN, 1);
    PrintLine(COMPOUND_STRING("BADGES"), text, 46);
    ConvertIntToDecimalStringN(text, Get16(card + CARD_DEX), STR_CONV_MODE_LEFT_ALIGN, 4);
    PrintLine(COMPOUND_STRING("POKéDEX"), text, 60);
    end = ConvertIntToDecimalStringN(text, Get16(card + CARD_HOURS), STR_CONV_MODE_LEFT_ALIGN, 3);
    *end++ = CHAR_COLON;
    ConvertIntToDecimalStringN(end, card[CARD_MINUTES], STR_CONV_MODE_LEADING_ZEROS, 2);
    PrintLine(COMPOUND_STRING("TIME"), text, 74);
    if (rank != 0)
    {
        end = StringCopy(text, COMPOUND_STRING("No."));
        end = ConvertIntToDecimalStringN(end, rank, STR_CONV_MODE_LEFT_ALIGN, 4);
        end = StringCopy(end, COMPOUND_STRING("  "));
        end = ConvertIntToDecimalStringN(end, gCommunityMailbox.respArgs[2], STR_CONV_MODE_LEFT_ALIGN, 4);
        end = StringCopy(end, COMPOUND_STRING("-"));
        ConvertIntToDecimalStringN(end, gCommunityMailbox.respArgs[3], STR_CONV_MODE_LEFT_ALIGN, 4);
    }
    else
    {
        StringCopy(text, COMPOUND_STRING("Unranked"));
    }
    PrintLine(COMPOUND_STRING("LADDER"), text, 88);
    // Rows 1 and 2: their tier and anything else the server knows (cups won).
    for (i = 1; i < 3 && i < gCommunityMailbox.rowCount; i++)
    {
        StringCopyN(text, gCommunityMailbox.rows[i], COMMUNITY_ROW_LENGTH);
        text[COMMUNITY_ROW_LENGTH] = EOS;
        AddTextPrinterParameterized3(sCard.windowId, FONT_SMALL, 150, 72 + i * 10, sLabelColor, TEXT_SKIP_DRAW, text);
    }
    for (i = 0; i < card[CARD_COUNT] && i < PARTY_SIZE; i++)
    {
        end = StringCopy(text, COMPOUND_STRING("{LV}"));
        ConvertIntToDecimalStringN(end, card[CARD_LEVELS + i], STR_CONV_MODE_LEFT_ALIGN, 3);
        AddTextPrinterParameterized3(sCard.windowId, FONT_SMALL, ICON_X - 8 + i * ICON_GAP, 130, sLabelColor, TEXT_SKIP_DRAW, text);
    }
    CopyWindowToVram(sCard.windowId, COPYWIN_FULL);
}

static void DestroyCardSprites(void)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (sCard.iconSprites[i] != SPRITE_NONE)
            FreeAndDestroyMonIconSprite(&gSprites[sCard.iconSprites[i]]);
        sCard.iconSprites[i] = SPRITE_NONE;
    }
    FreeMonIconPalettes();
    if (sCard.picSprite != SPRITE_NONE)
        FreeAndDestroyTrainerPicSprite(sCard.picSprite);
    sCard.picSprite = SPRITE_NONE;
}

static void Task_Card(u8 taskId)
{
    if (!JOY_NEW(A_BUTTON | B_BUTTON))
        return;
    PlaySE(SE_SELECT);
    DestroyCardSprites();
    ClearStdWindowAndFrameToTransparent(sCard.windowId, FALSE);
    ClearWindowTilemap(sCard.windowId);
    RemoveWindow(sCard.windowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Script, waitstate: the card in the last answer.
void CommunityCards_Show(void)
{
    const u8 *card = gCommunityMailbox.respData;
    u32 i, pic = TRAINER_PIC_BRENDAN + min(card[CARD_PIC], TRAINER_PIC_LEAF - TRAINER_PIC_BRENDAN);

    sCard.windowId = AddWindow(&sCardWindowTemplate);
    DrawStdWindowFrame(sCard.windowId, FALSE);
    PutWindowTilemap(sCard.windowId);
    DrawCard();
    sCard.picSprite = CreateTrainerPicSprite(pic, TRUE, 196, 60, 15, GetTrainerPicTag(pic, TRUE));
    if (sCard.picSprite != SPRITE_NONE && sCard.picSprite != 0xFFFF)
        gSprites[sCard.picSprite].oam.priority = 0;
    else
        sCard.picSprite = SPRITE_NONE;
    LoadMonIconPalettes();
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = Get16(card + CARD_SPECIES + i * 2);

        sCard.iconSprites[i] = SPRITE_NONE;
        if (i >= card[CARD_COUNT] || species == SPECIES_NONE || species >= NUM_SPECIES)
            continue;
        sCard.iconSprites[i] = CreateMonIcon(species, SpriteCB_MonIcon, ICON_X + 8 + i * ICON_GAP, ICON_Y, 0, 0);
        gSprites[sCard.iconSprites[i]].oam.priority = 0;
    }
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Card, 80);
}
