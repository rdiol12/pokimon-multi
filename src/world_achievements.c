#include "global.h"
#include "achievements.h"
#include "decompress.h"
#include "event_data.h"
#include "field_weather.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "menu.h"
#include "overworld.h"
#include "pokedex.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "world.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/weather.h"

// Achievements: 28 goals across the game, each with an item reward. Most
// are checked every couple of seconds in the overworld from the game's own
// counters (badges, Pokédex, game stats); the rest are unlocked where they
// happen (Achievements_Unlock). Unlocking shows a banner with the trophy at
// the top of the screen and puts the reward in the bag. R > ACHIEVEMENTS
// lists them. Titles (netplay batch 2) read Achievements_Has.

struct AchievementInfo
{
    const u8 *name;
    const u8 *description;
    u16 reward;
};

#define ACH(id, n, d, r) [id] = {COMPOUND_STRING(n), COMPOUND_STRING(d), r}

static const struct AchievementInfo sAchievements[ACHIEVEMENT_COUNT] =
{
    ACH(ACH_FIRST_CATCH,     "First Catch",       "Catch a wild Pokémon.",                          ITEM_POKE_BALL),
    ACH(ACH_DEX_25,          "Collector",         "Own 25 kinds of Pokémon.",                       ITEM_GREAT_BALL),
    ACH(ACH_DEX_100,         "Researcher",        "Own 100 kinds of Pokémon.",                      ITEM_ULTRA_BALL),
    ACH(ACH_DEX_250,         "Living Pokédex",    "Own 250 kinds of Pokémon.",                      ITEM_SHINY_CHARM),
    ACH(ACH_BADGES_4,        "Halfway There",     "Earn 4 Gym Badges.",                             ITEM_RARE_CANDY),
    ACH(ACH_BADGES_8,        "Full Case",         "Earn all 8 Gym Badges.",                         ITEM_PP_UP),
    ACH(ACH_CHAMPION,        "Champion",          "Enter the Hall of Fame.",                        ITEM_MASTER_BALL),
    ACH(ACH_NIGHT_OWL,       "Night Owl",         "Catch a Pokémon at night.",                      ITEM_DUSK_BALL),
    ACH(ACH_STORM_CHASER,    "Storm Chaser",      "Catch a Pokémon in rain, sand or snow.",         ITEM_NET_BALL),
    ACH(ACH_RIDER,           "Saddle Up",         "Ride a Pokémon.",                                ITEM_NUGGET),
    ACH(ACH_SHINY,           "Glitter",           "Catch a shiny Pokémon.",                         ITEM_STAR_PIECE),
    ACH(ACH_CHAIN_20,        "Chain Reaction",    "Reach a chain of 20 in a row.",                  ITEM_SUPER_LURE),
    ACH(ACH_HATCH_10,        "Breeder",           "Hatch 10 Eggs.",                                 ITEM_OVAL_CHARM),
    ACH(ACH_EVOLVE_20,       "Evolution Expert",  "Evolve Pokémon 20 times.",                       ITEM_RARE_CANDY),
    ACH(ACH_TRADE,           "Trading Partner",   "Trade a Pokémon.",                               ITEM_LUXURY_BALL),
    ACH(ACH_PVP_WIN,         "Rival Down",        "Win a battle against another player.",           ITEM_BOTTLE_CAP),
    ACH(ACH_ANGLER,          "Angler",            "Land 25 Pokémon with a rod.",                    ITEM_DIVE_BALL),
    ACH(ACH_BIG_FISH,        "The Big One",       "Land a fish over 1.5 m.",                        ITEM_BIG_NUGGET),
    ACH(ACH_FARMER,          "Green Thumb",       "Harvest 50 Berries.",                            ITEM_HEART_SCALE),
    ACH(ACH_PHOTOGRAPHER,    "Say Cheese",        "Take a photo.",                                  ITEM_PREMIER_BALL),
    ACH(ACH_HUNTER,          "Safari Hunter",     "Enter a Safari hunt.",                           ITEM_SAFARI_BALL),
    ACH(ACH_HARDCORE_GYM,    "Brave",             "Win a badge in hardcore mode.",                  ITEM_MAX_REVIVE),
    ACH(ACH_HARDCORE_CHAMP,  "Unbreakable",       "Become Champion in hardcore mode.",              ITEM_GOLD_BOTTLE_CAP),
    ACH(ACH_LEGEND,          "Legend Seeker",     "Finish a legendary quest.",                      ITEM_COMET_SHARD),
    ACH(ACH_ALL_LEGENDS,     "Mythmaker",         "Finish every legendary quest.",                  ITEM_ABILITY_CAPSULE),
    ACH(ACH_HOMEOWNER,       "Homeowner",         "Put 5 things in your house.",                    ITEM_SOOTHE_BELL),
    ACH(ACH_RICH,            "High Roller",       "Have ¥100,000.",                                 ITEM_AMULET_COIN),
    ACH(ACH_STEPS,           "Wanderer",          "Walk 10,000 steps.",                             ITEM_EXP_CHARM),
};

static EWRAM_DATA u32 sPendingBanners = 0;
static EWRAM_DATA u16 sLastCaughtCount = 0;

bool32 Achievements_Has(u8 id)
{
    return id < ACHIEVEMENT_COUNT && (gSaveBlock3Ptr->world.achievements & (1u << id));
}

u32 Achievements_Count(void)
{
    u32 i, n = 0;

    for (i = 0; i < ACHIEVEMENT_COUNT; i++)
        n += Achievements_Has(i);
    return n;
}

void Achievements_Unlock(u8 id)
{
    if (id >= ACHIEVEMENT_COUNT || Achievements_Has(id))
        return;
    gSaveBlock3Ptr->world.achievements |= 1u << id;
    AddBagItem(sAchievements[id].reward, 1);
    sPendingBanners |= 1u << id;
}

// Banner ------------------------------------------------------------------

#define BANNER_FRAMES (3 * 60)
#define TAG_TROPHY    0x5741 // 'WA'

static const u32 sTrophyGfx[] = INCBIN_U32("graphics/world/trophy.4bpp");
static const u16 sTrophyPal[] = INCBIN_U16("graphics/world/trophy.gbapal");

static const struct SpriteSheet sTrophySheet = {sTrophyGfx, sizeof(sTrophyGfx), TAG_TROPHY};
static const struct SpritePalette sTrophyPalette = {sTrophyPal, TAG_TROPHY};

static const struct OamData sTrophyOam =
{
    .size = SPRITE_SIZE(16x16),
    .shape = SPRITE_SHAPE(16x16),
    .priority = 0,
};

static const struct SpriteTemplate sTrophyTemplate =
{
    .tileTag = TAG_TROPHY,
    .paletteTag = TAG_TROPHY,
    .oam = &sTrophyOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct WindowTemplate sBannerWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 4,
    .tilemapTop = 1,
    .width = 24,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x1A0,
};

#define tTimer    data[0]
#define tWindowId data[1]
#define tSpriteId data[2]

static void CloseBanner(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
    ClearWindowTilemap(task->tWindowId);
    RemoveWindow(task->tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    if (task->tSpriteId != MAX_SPRITES)
    {
        DestroySprite(&gSprites[task->tSpriteId]);
        FreeSpriteTilesByTag(TAG_TROPHY);
        FreeSpritePaletteByTag(TAG_TROPHY);
    }
    DestroyTask(taskId);
}

static void Task_Banner(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    // Anything that takes over the screen (a script, a menu, a warp) closes it.
    if (++task->tTimer > BANNER_FRAMES || ArePlayerFieldControlsLocked() || gMain.callback2 != CB2_Overworld)
        CloseBanner(taskId);
}

// Two lines at the top of the screen; with the trophy for achievements.
static void ShowBanner(const u8 *title, const u8 *subtitle, bool32 trophy)
{
    u8 taskId = CreateTask(Task_Banner, 90);
    struct Task *task = &gTasks[taskId];
    u32 x = trophy ? 26 : 4;

    task->tWindowId = AddWindow(&sBannerWindowTemplate);
    DrawStdWindowFrame(task->tWindowId, FALSE);
    FillWindowPixelBuffer(task->tWindowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, title, x, 1, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(task->tWindowId, FONT_SMALL, subtitle, x, 17, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(task->tWindowId);
    CopyWindowToVram(task->tWindowId, COPYWIN_FULL);

    task->tSpriteId = MAX_SPRITES;
    if (trophy)
    {
        LoadSpriteSheet(&sTrophySheet);
        LoadSpritePalette(&sTrophyPalette);
        task->tSpriteId = CreateSprite(&sTrophyTemplate, 4 * 8 + 12, 8 + 16, 0);
        PlayFanfare(MUS_OBTAIN_TMHM);
    }
}

static void ShowAchievementBanner(u8 id)
{
    u8 *title = gStringVar3;

    StringCopy(StringCopy(title, COMPOUND_STRING("ACHIEVEMENT! ")), sAchievements[id].name);
    CopyItemName(sAchievements[id].reward, gStringVar1);
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Reward: {STR_VAR_1}"));
    ShowBanner(title, gStringVar4, TRUE);
}

// A short notice in the same place (no trophy), shown once the screen is
// free. Only the latest one is kept.
static EWRAM_DATA u8 sToastTitle[32] = {0};
static EWRAM_DATA u8 sToastText[48] = {0};

void World_ShowToast(const u8 *title, const u8 *text)
{
    StringCopyN(sToastTitle, title, sizeof(sToastTitle) - 1);
    sToastTitle[sizeof(sToastTitle) - 1] = EOS;
    StringCopyN(sToastText, text, sizeof(sToastText) - 1);
    sToastText[sizeof(sToastText) - 1] = EOS;
}

#undef tTimer
#undef tWindowId
#undef tSpriteId

// Checks ------------------------------------------------------------------

static u32 CountBadges(void)
{
    u32 i, n = 0;

    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        n += FlagGet(i);
    return n;
}

static void CheckCounters(void)
{
    u32 owned = GetNationalPokedexCount(FLAG_GET_CAUGHT);
    u32 badges = CountBadges();

    if (owned >= 25)
        Achievements_Unlock(ACH_DEX_25);
    if (owned >= 100)
        Achievements_Unlock(ACH_DEX_100);
    if (owned >= 250)
        Achievements_Unlock(ACH_DEX_250);
    if (badges >= 4)
        Achievements_Unlock(ACH_BADGES_4);
    if (badges >= 8)
        Achievements_Unlock(ACH_BADGES_8);
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        Achievements_Unlock(ACH_CHAMPION);
    if (GetGameStat(GAME_STAT_HATCHED_EGGS) >= 10)
        Achievements_Unlock(ACH_HATCH_10);
    if (GetGameStat(GAME_STAT_EVOLVED_POKEMON) >= 20)
        Achievements_Unlock(ACH_EVOLVE_20);
    if (GetGameStat(GAME_STAT_POKEMON_TRADES) >= 1)
        Achievements_Unlock(ACH_TRADE);
    if (GetGameStat(GAME_STAT_LINK_BATTLE_WINS) >= 1)
        Achievements_Unlock(ACH_PVP_WIN);
    if (GetGameStat(GAME_STAT_STEPS) >= 10000)
        Achievements_Unlock(ACH_STEPS);
    if (GetMoney(&gSaveBlock1Ptr->money) >= 100000)
        Achievements_Unlock(ACH_RICH);
    if (Hardcore_IsOn() && badges >= 1)
        Achievements_Unlock(ACH_HARDCORE_GYM);
    if (Hardcore_IsOn() && FlagGet(FLAG_SYS_GAME_CLEAR))
        Achievements_Unlock(ACH_HARDCORE_CHAMP);
    if (gSaveBlock3Ptr->world.fishLanded >= 25)
        Achievements_Unlock(ACH_ANGLER);
    if (gSaveBlock3Ptr->world.berriesHarvested >= 50)
        Achievements_Unlock(ACH_FARMER);
}

// Called by World_OnWildBattleEnd when the player caught a Pokémon.
void Achievements_OnCatch(u16 species, bool32 shiny)
{
    u32 weather = GetSavedWeather();

    Achievements_Unlock(ACH_FIRST_CATCH);
    if (GetTimeOfDay() == TIME_NIGHT)
        Achievements_Unlock(ACH_NIGHT_OWL);
    if (weather == WEATHER_RAIN || weather == WEATHER_RAIN_THUNDERSTORM || weather == WEATHER_DOWNPOUR
     || weather == WEATHER_SANDSTORM || weather == WEATHER_SNOW)
        Achievements_Unlock(ACH_STORM_CHASER);
    if (shiny)
        Achievements_Unlock(ACH_SHINY);
}

// Called every overworld frame.
void Achievements_Update(void)
{
    u32 i;

    if ((gMain.vblankCounter1 & 0x7F) == 0)
        CheckCounters();
    if ((sPendingBanners == 0 && sToastTitle[0] == EOS) || ArePlayerFieldControlsLocked() || FuncIsActiveTask(Task_Banner))
        return;
    if (sPendingBanners == 0)
    {
        ShowBanner(sToastTitle, sToastText, FALSE);
        sToastTitle[0] = EOS;
        return;
    }
    for (i = 0; i < ACHIEVEMENT_COUNT; i++)
    {
        if (sPendingBanners & (1u << i))
        {
            sPendingBanners &= ~(1u << i);
            ShowAchievementBanner(i);
            return;
        }
    }
}

// R > ACHIEVEMENTS --------------------------------------------------------

static void GetAchievementLine(u16 i, u8 *dest)
{
    // Unlocked ones in green, the rest grayed out.
    dest = StringCopy(dest, Achievements_Has(i) ? COMPOUND_STRING("{COLOR GREEN}") : COMPOUND_STRING("{COLOR LIGHT_GRAY}"));
    StringCopy(dest, sAchievements[i].name);
}

static void GetAchievementDetail(u16 i, u8 *dest)
{
    dest = StringCopy(dest, sAchievements[i].description);
    dest = StringCopy(dest, COMPOUND_STRING("  Reward: "));
    CopyItemName(sAchievements[i].reward, dest);
}

void Achievements_ShowList(void)
{
    u8 *title = gStringVar3;

    ConvertIntToDecimalStringN(gStringVar1, Achievements_Count(), STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, ACHIEVEMENT_COUNT, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringExpandPlaceholders(title, COMPOUND_STRING("ACHIEVEMENTS  {STR_VAR_1}/{STR_VAR_2}"));
    WorldBoard_Show(title, ACHIEVEMENT_COUNT, GetAchievementLine, GetAchievementDetail);
}
