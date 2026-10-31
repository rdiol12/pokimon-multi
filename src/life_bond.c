#include "global.h"
#include "bg.h"
#include "battle_environment.h"
#include "berry.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "item.h"
#include "life.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "rtc.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trig.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Bonding (docs/life.md): pet a party Pokémon by holding A and rubbing it
// with the hand, or feed it a berry. Both raise its friendship, a little per
// day, and friendship is what the battle engine's affection effects read
// (B_AFFECTION_MECHANICS): with more hearts a Pokémon gets more EXP, can
// shrug off a status, hang on to a hit with 1 HP and land more critical
// hits. The scene is set on the grassy battle field, the Pokémon on the
// far platform.

#define TAG_BOND_HAND   0x5A00
#define TAG_BOND_HEART  0x5A01
#define TAG_BOND_HEARTS 0x5A02
#define TAG_BOND_PAL    0x5A00

#define MON_X 176
#define MON_Y 52
#define MON_HALF 28 // the hand pets inside this box around the mon

#define RUB_PER_PET 40 // pixels of rubbing per point of friendship

enum
{
    WIN_INFO,
    WIN_MSG,
};

static const struct BgTemplate sBgTemplates[] =
{
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .priority = 0},
    {.bg = 1, .charBaseIndex = 2, .mapBaseIndex = 26, .screenSize = 1, .priority = 3},
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_INFO] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 13, .height = 10, .paletteNum = 15, .baseBlock = 0x00A},
    [WIN_MSG] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 15, .width = 28, .height = 4, .paletteNum = 15, .baseBlock = 0x0A0},
    DUMMY_WIN_TEMPLATE
};

static const u32 sHandGfx[] = INCBIN_U32("graphics/life/bond_hand.4bpp");
static const u32 sHeartGfx[] = INCBIN_U32("graphics/life/bond_heart.4bpp");
static const u32 sHeartsSmallGfx[] = INCBIN_U32("graphics/life/bond_hearts_small.4bpp");
static const u16 sBondPal[] = INCBIN_U16("graphics/life/bond_hand.gbapal");

static const struct SpriteSheet sSpriteSheets[] =
{
    {sHandGfx, sizeof(sHandGfx), TAG_BOND_HAND},
    {sHeartGfx, sizeof(sHeartGfx), TAG_BOND_HEART},
    {sHeartsSmallGfx, sizeof(sHeartsSmallGfx), TAG_BOND_HEARTS},
    {0},
};

static const struct SpritePalette sSpritePalette = {sBondPal, TAG_BOND_PAL};

static const struct OamData sOam16 = {.size = SPRITE_SIZE(16x16), .shape = SPRITE_SHAPE(16x16), .priority = 0};
static const struct OamData sOam8 = {.size = SPRITE_SIZE(8x8), .shape = SPRITE_SHAPE(8x8), .priority = 0};

static void SpriteCB_FloatingHeart(struct Sprite *sprite);

static const struct SpriteTemplate sHandTemplate =
{
    .tileTag = TAG_BOND_HAND, .paletteTag = TAG_BOND_PAL, .oam = &sOam16,
    .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sHeartTemplate =
{
    .tileTag = TAG_BOND_HEART, .paletteTag = TAG_BOND_PAL, .oam = &sOam16,
    .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_FloatingHeart,
};

static const struct SpriteTemplate sMeterTemplate =
{
    .tileTag = TAG_BOND_HEARTS, .paletteTag = TAG_BOND_PAL, .oam = &sOam8,
    .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const u8 sTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sRedColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};

// What the hearts give in battle (see the comment at the top).
static const u8 *const sPerkText[] =
{
    COMPOUND_STRING("Pet it to\nmake friends!"),
    COMPOUND_STRING("Bond growing.\nKeep at it!"),
    COMPOUND_STRING("Next heart: may\nhang on with 1 HP."),
    COMPOUND_STRING("May hang on\nwith 1 HP."),
    COMPOUND_STRING("More EXP, may\nshake off status."),
    COMPOUND_STRING("Best friends!\nMore crits too."),
};

struct BondScene
{
    u8 slot;
    u8 state;
    u8 monSpriteId;
    u8 handSpriteId;
    u8 meterSpriteIds[5];
    s16 lastX;
    s16 lastY;
    u16 rub;
    u16 hop;
    u16 berryIndex; // into the berries pocket while choosing a treat
    bool8 choosing;
    bool8 cried;
};

static EWRAM_DATA struct BondScene *sBond = NULL;

static void CB2_Bond(void);
static void Task_Bond(u8 taskId);
static void Task_BondExit(u8 taskId);

static struct Pokemon *BondMon(void)
{
    return &gParties[B_TRAINER_PLAYER][sBond->slot];
}

// Today's counters for the Pokémon in the scene, reset when the day changes
// and when another Pokémon moves into its party slot.
static u32 BondIndex(void)
{
    struct LifeSaveData *life = &gSaveBlock3Ptr->life;
    u32 personality = GetMonData(BondMon(), MON_DATA_PERSONALITY);
    u32 i;

    RtcCalcLocalTime();
    if (life->bondDay != gLocalTime.days)
    {
        life->bondDay = gLocalTime.days;
        memset(life->bondPets, 0, sizeof(life->bondPets));
        memset(life->bondFeeds, 0, sizeof(life->bondFeeds));
    }
    for (i = 0; i < LIFE_BOND_SLOTS; i++)
    {
        if (life->bondPersonality[i] == personality)
            return i;
    }
    i = sBond->slot;
    life->bondPersonality[i] = personality;
    life->bondPets[i] = 0;
    life->bondFeeds[i] = 0;
    return i;
}

static void AddFriendship(u32 amount)
{
    u32 friendship = GetMonData(BondMon(), MON_DATA_FRIENDSHIP) + amount;
    if (friendship > MAX_FRIENDSHIP)
        friendship = MAX_FRIENDSHIP;
    SetMonData(BondMon(), MON_DATA_FRIENDSHIP, &friendship);
}

static void PrintMessage(const u8 *text)
{
    FillWindowPixelBuffer(WIN_MSG, PIXEL_FILL(1));
    StringExpandPlaceholders(gStringVar4, text);
    AddTextPrinterParameterized3(WIN_MSG, FONT_NORMAL, 2, 1, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
    CopyWindowToVram(WIN_MSG, COPYWIN_FULL);
}

static void PrintHelp(void)
{
    PrintMessage(COMPOUND_STRING("Hold A and rub to pet. SELECT: treat.\nL/R: switch Pokémon. B: done."));
}

static void UpdateMeter(void)
{
    u32 hearts = GetMonAffectionHearts(BondMon());
    u32 i;
    for (i = 0; i < 5; i++)
        gSprites[sBond->meterSpriteIds[i]].oam.tileNum = GetSpriteTileStartByTag(TAG_BOND_HEARTS) + (i < hearts ? 0 : 1);
}

static void PrintInfo(void)
{
    struct Pokemon *mon = BondMon();
    u32 index = BondIndex();
    u32 hearts = GetMonAffectionHearts(mon);

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 2, 1, sTextColors, TEXT_SKIP_DRAW, gStringVar1);
    AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 2, 27, hearts >= AFFECTION_THREE_HEARTS ? sRedColors : sTextColors, TEXT_SKIP_DRAW, sPerkText[hearts]);
    ConvertIntToDecimalStringN(gStringVar2, LIFE_BOND_PET_LIMIT - gSaveBlock3Ptr->life.bondPets[index], STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar3, LIFE_BOND_FEED_LIMIT - gSaveBlock3Ptr->life.bondFeeds[index], STR_CONV_MODE_LEFT_ALIGN, 1);
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Pets left: {STR_VAR_2}\nTreats left: {STR_VAR_3}"));
    AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 2, 53, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
    UpdateMeter();
}

static void CreateBondMon(void)
{
    struct Pokemon *mon = BondMon();
    enum Species species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);

    sBond->monSpriteId = CreateMonPicSprite(species, GetMonData(mon, MON_DATA_IS_SHINY), GetMonData(mon, MON_DATA_PERSONALITY),
                                            TRUE, MON_X, MON_Y, 15, TAG_NONE);
    gSprites[sBond->monSpriteId].oam.priority = 1;
    sBond->cried = FALSE;
    sBond->rub = 0;
    sBond->hop = 0;
}

static void SpawnHeart(void)
{
    u8 spriteId = CreateSprite(&sHeartTemplate, MON_X - 16 + (Random() % 32), MON_Y - 24, 0);
    if (spriteId != MAX_SPRITES)
        gSprites[spriteId].data[0] = (Random() & 1) ? 1 : -1;
}

static void SpriteCB_FloatingHeart(struct Sprite *sprite)
{
    sprite->data[1]++;
    sprite->y2 = -sprite->data[1] / 2;
    sprite->x2 = Sin(sprite->data[1] * 8, 4) * sprite->data[0];
    if (sprite->data[1] > 48)
        DestroySprite(sprite);
}

static bool32 HandOverMon(void)
{
    struct Sprite *hand = &gSprites[sBond->handSpriteId];
    return hand->x >= MON_X - MON_HALF && hand->x <= MON_X + MON_HALF
        && hand->y >= MON_Y - MON_HALF && hand->y <= MON_Y + MON_HALF;
}

static void Pet(void)
{
    u32 index = BondIndex();

    if (!sBond->cried)
    {
        PlayCry_Normal(GetMonData(BondMon(), MON_DATA_SPECIES), 0);
        sBond->cried = TRUE;
    }
    sBond->hop = 16;
    SpawnHeart();
    if (gSaveBlock3Ptr->life.bondPets[index] >= LIFE_BOND_PET_LIMIT)
    {
        PrintMessage(COMPOUND_STRING("{STR_VAR_1} is happy, but it has had\nenough petting for today."));
        return;
    }
    gSaveBlock3Ptr->life.bondPets[index]++;
    AddFriendship(1);
    PlaySE(SE_BALL_TRAY_ENTER);
    PrintMessage(COMPOUND_STRING("{STR_VAR_1} leans into your hand!"));
    PrintInfo();
}

// The berries pocket, skipping empty slots.
static u16 BerryAt(u32 n)
{
    struct BagPocket *pocket = &gBagPockets[POCKET_BERRIES];
    u32 i;
    for (i = 0; i < pocket->capacity; i++)
    {
        enum Item item = GetBagItemId(POCKET_BERRIES, i);
        if (item == ITEM_NONE || GetBagItemQuantity(POCKET_BERRIES, i) == 0)
            continue;
        if (n-- == 0)
            return item;
    }
    return ITEM_NONE;
}

static void ShowBerryChoice(void)
{
    u16 berry = BerryAt(sBond->berryIndex);
    CopyItemName(berry, gStringVar2);
    ConvertIntToDecimalStringN(gStringVar3, CountTotalItemQuantityInBag(berry), STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintMessage(COMPOUND_STRING("Treat: {LEFT_ARROW}{STR_VAR_2} x{STR_VAR_3}{RIGHT_ARROW}\nA: give it  B: cancel"));
}

static void Feed(void)
{
    u16 berry = BerryAt(sBond->berryIndex);
    const struct BerryInfo *info = GetBerryInfo(ItemIdToBerryType(berry));
    const u8 flavors[FLAVOR_COUNT] = {info->spicy, info->dry, info->sweet, info->bitter, info->sour};
    u32 index = BondIndex(), best = 0, i;
    s32 relation;

    sBond->choosing = FALSE;
    if (gSaveBlock3Ptr->life.bondFeeds[index] >= LIFE_BOND_FEED_LIMIT)
    {
        PrintMessage(COMPOUND_STRING("{STR_VAR_1} is full. Try again\ntomorrow!"));
        return;
    }
    // The berry's strongest flavor decides how much the Pokémon likes it.
    for (i = 1; i < FLAVOR_COUNT; i++)
    {
        if (flavors[i] > flavors[best])
            best = i;
    }
    relation = GetMonFlavorRelation(BondMon(), best);
    RemoveBagItem(berry, 1);
    gSaveBlock3Ptr->life.bondFeeds[index]++;
    CopyItemName(berry, gStringVar2);
    sBond->hop = 24;
    if (relation > 0)
    {
        AddFriendship(8);
        SpawnHeart();
        SpawnHeart();
        PlaySE(SE_SUCCESS);
        PrintMessage(COMPOUND_STRING("{STR_VAR_1} loved the {STR_VAR_2}!\nIt's its favorite flavor!"));
    }
    else if (relation < 0)
    {
        AddFriendship(1);
        PlaySE(SE_BOO);
        PrintMessage(COMPOUND_STRING("{STR_VAR_1} ate the {STR_VAR_2},\nbut it made a face."));
    }
    else
    {
        AddFriendship(4);
        SpawnHeart();
        PlaySE(SE_SELECT);
        PrintMessage(COMPOUND_STRING("{STR_VAR_1} happily ate\nthe {STR_VAR_2}."));
    }
    PrintInfo();
}

static void SwitchMon(s32 delta)
{
    u32 slot = sBond->slot;
    do
    {
        slot = (slot + PARTY_SIZE + delta) % PARTY_SIZE;
    } while (GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_SPECIES) == SPECIES_NONE
          || GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_IS_EGG));
    if (slot == sBond->slot)
        return;
    PlaySE(SE_SELECT);
    FreeAndDestroyMonPicSprite(sBond->monSpriteId);
    sBond->slot = slot;
    CreateBondMon();
    GetMonData(BondMon(), MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    PrintInfo();
    PrintHelp();
}

static void Task_Bond(u8 taskId)
{
    struct Sprite *hand = &gSprites[sBond->handSpriteId];
    struct Sprite *mon = &gSprites[sBond->monSpriteId];

    if (sBond->hop != 0)
    {
        sBond->hop--;
        mon->y2 = -Sin(sBond->hop * 8, 6);
    }
    if (gPaletteFade.active)
        return;

    if (sBond->choosing)
    {
        if (JOY_NEW(A_BUTTON))
        {
            Feed();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            sBond->choosing = FALSE;
            PrintHelp();
        }
        else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
        {
            u32 count = 0;
            while (BerryAt(count) != ITEM_NONE)
                count++;
            sBond->berryIndex = (sBond->berryIndex + (JOY_NEW(DPAD_RIGHT) ? 1 : count - 1)) % count;
            PlaySE(SE_SELECT);
            ShowBerryChoice();
        }
        return;
    }

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_BondExit;
        return;
    }
    if (JOY_NEW(SELECT_BUTTON))
    {
        if (BerryAt(0) == ITEM_NONE)
        {
            PlaySE(SE_FAILURE);
            PrintMessage(COMPOUND_STRING("You don't have any BERRIES to\ngive as a treat."));
            return;
        }
        sBond->choosing = TRUE;
        sBond->berryIndex = 0;
        PlaySE(SE_SELECT);
        ShowBerryChoice();
        return;
    }
    if (JOY_NEW(L_BUTTON))
    {
        SwitchMon(-1);
        return;
    }
    if (JOY_NEW(R_BUTTON))
    {
        SwitchMon(1);
        return;
    }

    if (JOY_HELD(DPAD_LEFT) && hand->x > 8)
        hand->x -= 2;
    if (JOY_HELD(DPAD_RIGHT) && hand->x < DISPLAY_WIDTH - 8)
        hand->x += 2;
    if (JOY_HELD(DPAD_UP) && hand->y > 8)
        hand->y -= 2;
    if (JOY_HELD(DPAD_DOWN) && hand->y < 112)
        hand->y += 2;

    // Rubbing: held A and the hand moving over the Pokémon.
    if (JOY_HELD(A_BUTTON) && HandOverMon())
    {
        sBond->rub += abs(hand->x - sBond->lastX) + abs(hand->y - sBond->lastY);
        hand->y2 = (gMain.vblankCounter1 / 4) & 1; // a little stroke
        if (sBond->rub >= RUB_PER_PET)
        {
            sBond->rub -= RUB_PER_PET;
            Pet();
        }
    }
    else
    {
        hand->y2 = 0;
    }
    sBond->lastX = hand->x;
    sBond->lastY = hand->y;
}

static void Task_BondExit(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    FreeAndDestroyMonPicSprite(sBond->monSpriteId);
    FreeAllWindowBuffers();
    Free(sBond);
    sBond = NULL;
    DestroyTask(taskId);
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void VBlankCB_Bond(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Bond(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void CB2_InitBond(void)
{
    u32 i;

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
        LoadUserWindowBorderGfx(WIN_INFO, 0x200, BG_PLTT_ID(14));
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 2:
        // A wild battle's grassy field, as in battle_bg.c.
        DecompressDataWithHeaderVram(gBattleEnvironmentInfo[BATTLE_ENVIRONMENT_GRASS].background.tileset, (void *)BG_CHAR_ADDR(2));
        DecompressDataWithHeaderVram(gBattleEnvironmentInfo[BATTLE_ENVIRONMENT_GRASS].background.tilemap, (void *)BG_SCREEN_ADDR(26));
        LoadPalette(gBattleEnvironmentInfo[BATTLE_ENVIRONMENT_GRASS].palette, BG_PLTT_ID(2), 3 * PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 3:
        for (i = 0; sSpriteSheets[i].data != NULL; i++)
            LoadSpriteSheet(&sSpriteSheets[i]);
        LoadSpritePalette(&sSpritePalette);
        CreateBondMon();
        sBond->handSpriteId = CreateSprite(&sHandTemplate, MON_X - 40, MON_Y + 24, 0);
        sBond->lastX = gSprites[sBond->handSpriteId].x;
        sBond->lastY = gSprites[sBond->handSpriteId].y;
        for (i = 0; i < 5; i++)
            sBond->meterSpriteIds[i] = CreateSprite(&sMeterTemplate, 8 + 12 + i * 10, 29, 0);
        gMain.state++;
        break;
    case 4:
        DrawStdFrameWithCustomTileAndPalette(WIN_INFO, FALSE, 0x200, 14);
        DrawStdFrameWithCustomTileAndPalette(WIN_MSG, FALSE, 0x200, 14);
        PutWindowTilemap(WIN_INFO);
        PutWindowTilemap(WIN_MSG);
        GetMonData(BondMon(), MON_DATA_NICKNAME, gStringVar1);
        StringGet_Nickname(gStringVar1);
        PrintInfo();
        PrintHelp();
        ScheduleBgCopyTilemapToVram(0);
        gMain.state++;
        break;
    case 5:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        ShowBg(0);
        ShowBg(1);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Bond);
        CreateTask(Task_Bond, 0);
        SetMainCallback2(CB2_Bond);
        break;
    }
}

void LifeBond_Open(void)
{
    u32 slot = gSpecialVar_0x8004;

    if (slot >= PARTY_SIZE || GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_IS_EGG))
    {
        ScriptContext_Enable();
        return;
    }
    sBond = AllocZeroed(sizeof(*sBond));
    sBond->slot = slot;
    gMain.state = 0;
    SetMainCallback2(CB2_InitBond);
}
