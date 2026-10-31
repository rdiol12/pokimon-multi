#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "overworld.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "netplay.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_level.h"
#include "window.h"
#include "world.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/songs.h"

// Trainer levels: playing online earns XP, and every level up to 30 gives
// an item and sometimes an outfit (Brendan or May in new colours, which
// other players see too) or a perk. R > ADVENTURE > TRAINER LEVEL shows the
// level and what each one unlocks; OUTFITS changes clothes.
//
// XP only counts while connected:
//   - 2 XP for every minute in the overworld,
//   - 60 for winning a battle against another player, 25 for any other
//     result, 30 for a trade,
//   - 5 for a catch and 10 for beating a trainer,
//   - and whatever other online features give (TrainerLevel_AddXp).

#define XP_PER_MINUTE    2
#define XP_PVP_WIN       60
#define XP_PVP_OTHER     25
#define XP_TRADE         30
#define XP_CATCH         5
#define XP_TRAINER_WIN   10

enum RewardKind
{
    REWARD_ITEM,
    REWARD_OUTFIT,
    REWARD_PERK,
};

struct LevelReward
{
    u8 kind;
    u16 id; // item, OUTFIT_* or PERK_*
};

// What reaching each level gives (index = level).
static const struct LevelReward sRewards[TRAINER_LEVEL_MAX + 1] =
{
    [2]  = {REWARD_ITEM,   ITEM_GREAT_BALL},
    [3]  = {REWARD_OUTFIT, OUTFIT_AUTUMN},
    [4]  = {REWARD_ITEM,   ITEM_SUPER_POTION},
    [5]  = {REWARD_PERK,   PERK_PRIZE_MONEY},
    [6]  = {REWARD_ITEM,   ITEM_ULTRA_BALL},
    [7]  = {REWARD_ITEM,   ITEM_REVIVE},
    [8]  = {REWARD_OUTFIT, OUTFIT_STARLIGHT},
    [9]  = {REWARD_ITEM,   ITEM_RARE_CANDY},
    [10] = {REWARD_PERK,   PERK_DISCOUNT},
    [11] = {REWARD_ITEM,   ITEM_HYPER_POTION},
    [12] = {REWARD_OUTFIT, OUTFIT_ACE},
    [13] = {REWARD_ITEM,   ITEM_PP_UP},
    [14] = {REWARD_ITEM,   ITEM_NUGGET},
    [15] = {REWARD_PERK,   PERK_HATCHING},
    [16] = {REWARD_ITEM,   ITEM_ABILITY_CAPSULE},
    [17] = {REWARD_ITEM,   ITEM_RARE_CANDY},
    [18] = {REWARD_OUTFIT, OUTFIT_SHADOW},
    [19] = {REWARD_ITEM,   ITEM_FULL_RESTORE},
    [20] = {REWARD_PERK,   PERK_PRIZE_MONEY2},
    [21] = {REWARD_ITEM,   ITEM_BOTTLE_CAP},
    [22] = {REWARD_ITEM,   ITEM_MAX_REVIVE},
    [23] = {REWARD_ITEM,   ITEM_PP_MAX},
    [24] = {REWARD_ITEM,   ITEM_BIG_NUGGET},
    [25] = {REWARD_PERK,   PERK_SHINY},
    [26] = {REWARD_ITEM,   ITEM_ABILITY_PATCH},
    [27] = {REWARD_ITEM,   ITEM_GOLD_BOTTLE_CAP},
    [28] = {REWARD_ITEM,   ITEM_COMET_SHARD},
    [29] = {REWARD_ITEM,   ITEM_RARE_CANDY},
    [30] = {REWARD_OUTFIT, OUTFIT_CHAMPION},
};

static const u8 *const sOutfitNames[OUTFIT_COUNT] =
{
    [OUTFIT_NONE]      = COMPOUND_STRING("Everyday"),
    [OUTFIT_AUTUMN]    = COMPOUND_STRING("Autumn"),
    [OUTFIT_STARLIGHT] = COMPOUND_STRING("Starlight"),
    [OUTFIT_ACE]       = COMPOUND_STRING("Ace"),
    [OUTFIT_SHADOW]    = COMPOUND_STRING("Shadow"),
    [OUTFIT_CHAMPION]  = COMPOUND_STRING("Champion"),
};

static const u8 *const sPerkNames[PERK_COUNT] =
{
    [PERK_PRIZE_MONEY]  = COMPOUND_STRING("+10% prize money"),
    [PERK_DISCOUNT]     = COMPOUND_STRING("10% off in Poké Marts"),
    [PERK_HATCHING]     = COMPOUND_STRING("Eggs hatch faster"),
    [PERK_PRIZE_MONEY2] = COMPOUND_STRING("+25% prize money"),
    [PERK_SHINY]        = COMPOUND_STRING("Shiny Pokémon more often"),
};

static const u8 *const sPerkDetails[PERK_COUNT] =
{
    [PERK_PRIZE_MONEY]  = COMPOUND_STRING("Trainers you beat pay 10% more."),
    [PERK_DISCOUNT]     = COMPOUND_STRING("Everything in Poké Marts costs 10% less."),
    [PERK_HATCHING]     = COMPOUND_STRING("Eggs in the party hatch twice as fast."),
    [PERK_PRIZE_MONEY2] = COMPOUND_STRING("Trainers you beat pay 25% more."),
    [PERK_SHINY]        = COMPOUND_STRING("Wild Pokémon get one more shiny roll."),
};

// Outfit graphics, from tools/adventure/gen_outfits.py: per outfit and
// gender, OUTFIT_STATES graphics in the order below.
#define OUTFIT_GFX(outfit, gender, n) (OBJ_EVENT_GFX_OUTFIT_FIRST + (((outfit) - 1) * 2 + (gender)) * OUTFIT_STATES + (n))
#define OUTFIT_ROW(outfit, gender, underwater)                                     \
{                                                                                  \
    [PLAYER_AVATAR_STATE_NORMAL]     = OUTFIT_GFX(outfit, gender, 0),              \
    [PLAYER_AVATAR_STATE_MACH_BIKE]  = OUTFIT_GFX(outfit, gender, 1),              \
    [PLAYER_AVATAR_STATE_ACRO_BIKE]  = OUTFIT_GFX(outfit, gender, 2),              \
    [PLAYER_AVATAR_STATE_SURFING]    = OUTFIT_GFX(outfit, gender, 3),              \
    [PLAYER_AVATAR_STATE_UNDERWATER] = underwater,                                 \
    [PLAYER_AVATAR_STATE_FIELD_MOVE] = OUTFIT_GFX(outfit, gender, 4),              \
    [PLAYER_AVATAR_STATE_FISHING]    = OUTFIT_GFX(outfit, gender, 5),              \
    [PLAYER_AVATAR_STATE_WATERING]   = OUTFIT_GFX(outfit, gender, 6),              \
    [PLAYER_AVATAR_STATE_VSSEEKER]   = OUTFIT_GFX(outfit, gender, 4),              \
}
#define OUTFIT_ROWS(outfit) {OUTFIT_ROW(outfit, MALE, OBJ_EVENT_GFX_BRENDAN_UNDERWATER), OUTFIT_ROW(outfit, FEMALE, OBJ_EVENT_GFX_MAY_UNDERWATER)}

static const u16 sOutfitGfxIds[OUTFIT_COUNT][2][PLAYER_AVATAR_STATE_VSSEEKER + 1] =
{
    [OUTFIT_AUTUMN]    = OUTFIT_ROWS(OUTFIT_AUTUMN),
    [OUTFIT_STARLIGHT] = OUTFIT_ROWS(OUTFIT_STARLIGHT),
    [OUTFIT_ACE]       = OUTFIT_ROWS(OUTFIT_ACE),
    [OUTFIT_SHADOW]    = OUTFIT_ROWS(OUTFIT_SHADOW),
    [OUTFIT_CHAMPION]  = OUTFIT_ROWS(OUTFIT_CHAMPION),
};

static EWRAM_DATA u16 sOnlineFrames = 0;

#define sSave (gSaveBlock3Ptr->adventure)

// XP --------------------------------------------------------------------

// XP from one level to the next.
static u32 XpForLevel(u32 level)
{
    return 40 + 20 * level;
}

static u32 LevelFromXp(u32 xp, u32 *intoLevel)
{
    u32 level = 1;

    while (level < TRAINER_LEVEL_MAX && xp >= XpForLevel(level))
        xp -= XpForLevel(level++);
    if (intoLevel != NULL)
        *intoLevel = xp;
    return level;
}

u32 TrainerLevel_GetLevel(void)
{
    return LevelFromXp(sSave.trainerXp, NULL);
}

static void AppendRewardName(u8 *dest, const struct LevelReward *reward)
{
    switch (reward->kind)
    {
    case REWARD_ITEM:
        CopyItemName(reward->id, dest + StringLength(dest));
        break;
    case REWARD_OUTFIT:
        StringAppend(dest, sOutfitNames[reward->id]);
        StringAppend(dest, COMPOUND_STRING(" outfit"));
        break;
    case REWARD_PERK:
        StringAppend(dest, sPerkNames[reward->id]);
        break;
    }
}

// Gives the rewards of levels the player reached since the last check.
static void CheckLevelUp(void)
{
    u32 level = TrainerLevel_GetLevel();
    u8 title[32], text[48];

    if (sSave.trainerLevel >= level)
        return;
    while (sSave.trainerLevel < level)
    {
        const struct LevelReward *reward = &sRewards[++sSave.trainerLevel];
        if (reward->kind == REWARD_ITEM && reward->id != ITEM_NONE)
            AddBagItem(reward->id, 1);
    }
    StringCopy(title, COMPOUND_STRING("Trainer Lv. "));
    ConvertIntToDecimalStringN(title + StringLength(title), level, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringAppend(title, COMPOUND_STRING("!"));
    text[0] = EOS;
    AppendRewardName(text, &sRewards[level]);
    World_ShowToast(title, text);
}

void TrainerLevel_AddXp(u32 xp)
{
    if (!Netplay_IsOnline())
        return;
    sSave.trainerXp += xp;
    CheckLevelUp();
}

void TrainerLevel_SetXp(u32 xp)
{
    sSave.trainerXp = xp;
    if (sSave.trainerLevel > TrainerLevel_GetLevel())
        sSave.trainerLevel = TrainerLevel_GetLevel();
    CheckLevelUp();
}

// Battles against other players and trades show up in the game stats.
static void CheckStats(void)
{
    u16 wins = GetGameStat(GAME_STAT_LINK_BATTLE_WINS);
    u16 battles = wins + GetGameStat(GAME_STAT_LINK_BATTLE_LOSSES) + GetGameStat(GAME_STAT_LINK_BATTLE_DRAWS);
    u16 trades = GetGameStat(GAME_STAT_POKEMON_TRADES);
    u16 newWins = (u16)(wins - sSave.lastPvpWins);
    u16 newBattles = (u16)(battles - sSave.lastPvpBattles);
    u16 newTrades = (u16)(trades - sSave.lastTrades);

    sSave.lastPvpWins = wins;
    sSave.lastPvpBattles = battles;
    sSave.lastTrades = trades;
    if (newBattles >= newWins && newBattles < 100 && newTrades < 100)
        TrainerLevel_AddXp(newWins * XP_PVP_WIN + (newBattles - newWins) * XP_PVP_OTHER + newTrades * XP_TRADE);
}

void TrainerLevel_Update(void)
{
    if (sSave.trainerLevel == 0)
    {
        // A save from before trainer levels: what's already done doesn't count.
        sSave.trainerLevel = 1;
        sSave.lastPvpWins = sSave.lastPvpBattles = sSave.lastTrades = 0;
        CheckStats();
        sSave.trainerXp = 0;
    }
    if ((gMain.vblankCounter1 & 0x3F) == 0)
        CheckStats();
    if (!Netplay_IsOnline())
        return;
    if (++sOnlineFrames >= 60 * 60)
    {
        sOnlineFrames = 0;
        TrainerLevel_AddXp(XP_PER_MINUTE);
    }
}

void TrainerLevel_OnCatch(void)
{
    TrainerLevel_AddXp(XP_CATCH);
}

void TrainerLevel_OnTrainerWin(void)
{
    TrainerLevel_AddXp(XP_TRAINER_WIN);
}

// Perks -------------------------------------------------------------------

static u32 LevelOf(u32 kind, u32 id)
{
    u32 level;

    for (level = 0; level <= TRAINER_LEVEL_MAX; level++)
    {
        if (sRewards[level].kind == kind && sRewards[level].id == id)
            return level;
    }
    return TRAINER_LEVEL_MAX + 1;
}

bool32 TrainerLevel_HasPerk(u32 perk)
{
    return TrainerLevel_GetLevel() >= LevelOf(REWARD_PERK, perk);
}

u32 TrainerLevel_BoostPrizeMoney(u32 money)
{
    if (TrainerLevel_HasPerk(PERK_PRIZE_MONEY2))
        return money + money / 4;
    if (TrainerLevel_HasPerk(PERK_PRIZE_MONEY))
        return money + money / 10;
    return money;
}

u32 TrainerLevel_DiscountPrice(u32 price)
{
    if (TrainerLevel_HasPerk(PERK_DISCOUNT))
        return price - price / 10;
    return price;
}

u32 TrainerLevel_ShinyRolls(void)
{
    return TrainerLevel_HasPerk(PERK_SHINY) ? 1 : 0;
}

// Outfits -----------------------------------------------------------------

static bool32 HasOutfit(u32 outfit)
{
    return outfit == OUTFIT_NONE || TrainerLevel_GetLevel() >= LevelOf(REWARD_OUTFIT, outfit);
}

const u16 *TrainerLevel_GetOutfitGfxIds(enum PlayerStyle style)
{
    u32 outfit = sSave.outfit;

    if (outfit == OUTFIT_NONE || outfit >= OUTFIT_COUNT || !HasOutfit(outfit))
        return NULL;
    if (style == PLAYER_STYLE_BRENDAN)
        return sOutfitGfxIds[outfit][MALE];
    if (style == PLAYER_STYLE_MAY)
        return sOutfitGfxIds[outfit][FEMALE];
    return NULL;
}

bool32 TrainerLevel_IsOutfitSurfingGfx(u16 graphicsId)
{
    u32 n = graphicsId - OBJ_EVENT_GFX_OUTFIT_FIRST;

    return graphicsId >= OBJ_EVENT_GFX_OUTFIT_FIRST && graphicsId <= OBJ_EVENT_GFX_OUTFIT_LAST
        && n % OUTFIT_STATES == 3;
}

u8 TrainerLevel_GetOutfitGfxGender(u16 graphicsId)
{
    return ((graphicsId - OBJ_EVENT_GFX_OUTFIT_FIRST) / OUTFIT_STATES) % 2 ? FEMALE : MALE;
}

void TrainerLevel_SetOutfit(u32 outfit)
{
    sSave.outfit = outfit;
    ApplyPlayerStyleToAvatar();
}

// Trainer level card ----------------------------------------------------

static void GetRewardLine(u16 index, u8 *dest)
{
    u32 level = index + 2;

    // Levels not reached yet are greyed out.
    StringCopy(dest, TrainerLevel_GetLevel() >= level ? COMPOUND_STRING("Lv.") : COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}Lv."));
    ConvertIntToDecimalStringN(dest + StringLength(dest), level, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringAppend(dest, COMPOUND_STRING("  "));
    AppendRewardName(dest, &sRewards[level]);
}

static void GetRewardDetail(u16 index, u8 *dest)
{
    const struct LevelReward *reward = &sRewards[index + 2];

    switch (reward->kind)
    {
    case REWARD_OUTFIT:
        StringCopy(dest, COMPOUND_STRING("New clothes for BRENDAN and MAY. Pick them in OUTFITS."));
        break;
    case REWARD_PERK:
        StringCopy(dest, sPerkDetails[reward->id]);
        break;
    default:
        StringCopy(dest, COMPOUND_STRING("An item, put in the BAG."));
        break;
    }
}

static EWRAM_DATA u8 sCardTitle[64] = {0};

void TrainerLevel_ShowCard(void)
{
    u32 into, level = LevelFromXp(sSave.trainerXp, &into);
    u8 *p;

    p = StringCopy(sCardTitle, COMPOUND_STRING("TRAINER LV. "));
    p = ConvertIntToDecimalStringN(p, level, STR_CONV_MODE_LEFT_ALIGN, 2);
    if (level < TRAINER_LEVEL_MAX)
    {
        p = StringCopy(p, COMPOUND_STRING("   XP "));
        p = ConvertIntToDecimalStringN(p, into, STR_CONV_MODE_LEFT_ALIGN, 4);
        p = StringCopy(p, COMPOUND_STRING("/"));
        p = ConvertIntToDecimalStringN(p, XpForLevel(level), STR_CONV_MODE_LEFT_ALIGN, 4);
    }
    else
    {
        p = StringCopy(p, COMPOUND_STRING("   MAX"));
    }
    if (!Netplay_IsOnline())
        StringCopy(p, COMPOUND_STRING("  {COLOR RED}OFFLINE"));
    WorldBoard_Show(sCardTitle, TRAINER_LEVEL_MAX - 1, GetRewardLine, GetRewardDetail);
}

// Outfit menu -----------------------------------------------------------

// Like START > STYLE: the player changes clothes as the cursor moves, A
// keeps them and B puts the old ones back.

#define tWindowId  data[0]
#define tOld       data[1]
#define tCount     data[2]
#define tOutfits   data[3] // up to OUTFIT_COUNT entries: data[3..8]

static const struct WindowTemplate sOutfitWindowTemplate = // the height fits the list
{
    .bg = 0,
    .tilemapLeft = 19,
    .tilemapTop = 1,
    .width = 10,
    .height = OUTFIT_COUNT * 2,
    .paletteNum = 15,
    .baseBlock = 0x139,
};

static void Task_OutfitMenu(u8 taskId);

static void ShowOutfit(u32 outfit)
{
    if (sSave.outfit == outfit)
        return;
    sSave.outfit = outfit;
    ApplyPlayerStyleToAvatar();
}

void TrainerLevel_ShowOutfitMenu(void)
{
    u8 taskId;
    struct Task *task;
    struct WindowTemplate template;
    u32 i, cursor = 0;

    if (GetPlayerStyle() != PLAYER_STYLE_BRENDAN && GetPlayerStyle() != PLAYER_STYLE_MAY)
    {
        gSpecialVar_Result = FALSE;
        ScriptContext_Enable();
        return;
    }
    gSpecialVar_Result = TRUE;
    taskId = CreateTask(Task_OutfitMenu, 80);
    task = &gTasks[taskId];
    task->tOld = sSave.outfit;
    task->tCount = 0;
    for (i = 0; i < OUTFIT_COUNT; i++)
    {
        if (!HasOutfit(i))
            continue;
        if (i == sSave.outfit)
            cursor = task->tCount;
        task->data[3 + task->tCount++] = i;
    }
    template = sOutfitWindowTemplate;
    template.height = task->tCount * 2;
    task->tWindowId = AddWindow(&template);
    DrawStdWindowFrame(task->tWindowId, FALSE);
    FillWindowPixelBuffer(task->tWindowId, PIXEL_FILL(1));
    for (i = 0; i < task->tCount; i++)
        AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, sOutfitNames[task->data[3 + i]], 8, i * 16 + 1, TEXT_SKIP_DRAW, NULL);
    InitMenuInUpperLeftCornerNormal(task->tWindowId, task->tCount, cursor);
    CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void CloseOutfitMenu(u8 taskId)
{
    ClearStdWindowAndFrame(gTasks[taskId].tWindowId, TRUE);
    RemoveWindow(gTasks[taskId].tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_OutfitMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s8 input = Menu_ProcessInputNoWrap();

    switch (input)
    {
    case MENU_NOTHING_CHOSEN:
        ShowOutfit(task->data[3 + Menu_GetCursorPos()]);
        break;
    case MENU_B_PRESSED:
        PlaySE(SE_SELECT);
        ShowOutfit(task->tOld);
        CloseOutfitMenu(taskId);
        break;
    default:
        ShowOutfit(task->data[3 + input]);
        CloseOutfitMenu(taskId);
        break;
    }
}
