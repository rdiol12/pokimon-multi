#include "global.h"
#include "event_data.h"
#include "field_effect.h"
#include "field_weather.h"
#include "menu.h"
#include "online.h"
#include "pokemon.h"
#include "script_menu.h"
#include "window.h"
#include "sprite.h"
#include "trainer_pokemon_sprites.h"
#include "constants/party_menu.h"

// Pokémon pageants (server/online_pageant.py): the entry's picture in a
// box, shiny colours and all (showmonpic can't do shiny).

#define PIC_X 19
#define PIC_Y 2

struct PageantPic
{
    bool8 shown;
    u8 spriteId;
    u8 windowId;
};

static EWRAM_DATA struct PageantPic sPic = {0};

// Its own tiles, clear of the menus shown next to it (showmonpic's window
// shares tiles with them).
static const struct WindowTemplate sPicWindow =
{
    .bg = 0, .tilemapLeft = PIC_X + 1, .tilemapTop = PIC_Y + 1, .width = 8, .height = 8,
    .paletteNum = 15, .baseBlock = 0x120,
};

void Online_HidePageantPic(void)
{
    if (!sPic.shown)
        return;
    FreeResourcesAndDestroySprite(&gSprites[sPic.spriteId], sPic.spriteId);
    ClearToTransparentAndRemoveWindow(sPic.windowId);
    sPic.shown = FALSE;
}

static void ShowPic(u16 species, bool32 shiny, u32 personality)
{
    u16 spriteId;

    Online_HidePageantPic();
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    spriteId = CreateMonPicSprite(species, shiny, personality, TRUE, PIC_X * 8 + 40, PIC_Y * 8 + 40, 0, species);
    if (spriteId == 0xFFFF)
        return;
    sPic.spriteId = spriteId;
    PreservePaletteInWeather(gSprites[spriteId].oam.paletteNum + 0x10);
    gSprites[sPic.spriteId].callback = SpriteCallbackDummy;
    gSprites[sPic.spriteId].oam.priority = 0;
    sPic.windowId = AddWindow(&sPicWindow);
    PutWindowTilemap(sPic.windowId);
    SetStandardWindowBorderStyle(sPic.windowId, TRUE);
    ScheduleBgCopyTilemapToVram(0);
    sPic.shown = TRUE;
}

// After PAGEANT SHOW (row: species, shiny, personality, entry, next index):
// shows the entry; VAR_0x8006 is its id, VAR_0x8007 the next index.
void Online_ShowPageantEntry(void)
{
    ShowPic(Online_DataNum(0, 0), Online_DataNum(0, 1), Online_DataNum(0, 2));
    gSpecialVar_0x8006 = Online_DataNum(0, 3);
    gSpecialVar_0x8007 = Online_DataNum(0, 4);
}

// Party slot VAR_0x8005 (as the entry is sent: the low half of its
// personality).
void Online_ShowPartyMonPic(void)
{
    struct Pokemon *mon;

    if (gSpecialVar_0x8005 >= PARTY_SIZE)
        return;
    mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005];
    ShowPic(GetMonData(mon, MON_DATA_SPECIES), GetMonData(mon, MON_DATA_IS_SHINY),
            GetMonData(mon, MON_DATA_PERSONALITY) & 0xFFFF);
}
