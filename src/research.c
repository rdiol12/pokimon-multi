#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "field_effect.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "online.h"
#include "pokedex.h"
#include "pokemon.h"
#include "research.h"
#include "rtc.h"
#include "save_ext.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "world.h"
#include "constants/items.h"
#include "constants/songs.h"

// Research Pokédex (docs/research.md). Every species has up to five research
// tasks, each with tiers. Every tier reached is one research level and pays
// RESEARCH_LEVEL_MONEY times the new level; level 10 completes the species
// (more money, Ultra Balls, and wild ones of it get extra shiny rolls).
// Completing species adds up to research ranks with their own rewards.

#define RESEARCH_LEVEL_MONEY    50
#define RESEARCH_COMPLETE_MONEY 1000
#define RESEARCH_COMPLETE_BALLS 3
#define RESEARCH_SHINY_ROLLS    2 // for completed species; +1 when perfect
#define BIG_PERCENT             115

enum
{
    TASK_CATCH,
    TASK_DEFEAT,
    TASK_EVOLVE,
    TASK_BIG,
    TASK_DAYNIGHT,
    TASK_COUNT,
};

static const u8 sCatchTiers[] = {1, 2, 4, 7, 10};
static const u8 sDefeatTiers[] = {1, 3, 6, 10, 15};
static const u8 sEvolveTiers[] = {1, 3};
static const u8 sOneTier[] = {1};
static const u8 sTwoParts[] = {2};

struct ResearchTask
{
    const u8 *name;
    const u8 *done; // for the toast: "<done> <count>" or just <done>
    const u8 *tiers;
    u8 tierCount;
    bool8 counted;  // the toast shows the count
};

static const struct ResearchTask sTasks[TASK_COUNT] =
{
    [TASK_CATCH]    = {COMPOUND_STRING("Catch it"), COMPOUND_STRING("Caught"), sCatchTiers, ARRAY_COUNT(sCatchTiers), TRUE},
    [TASK_DEFEAT]   = {COMPOUND_STRING("Defeat it"), COMPOUND_STRING("Defeated"), sDefeatTiers, ARRAY_COUNT(sDefeatTiers), TRUE},
    [TASK_EVOLVE]   = {COMPOUND_STRING("Evolve it"), COMPOUND_STRING("Evolved"), sEvolveTiers, ARRAY_COUNT(sEvolveTiers), TRUE},
    [TASK_BIG]      = {COMPOUND_STRING("Big one"), COMPOUND_STRING("Caught a big one"), sOneTier, 1, FALSE},
    [TASK_DAYNIGHT] = {COMPOUND_STRING("Day & night"), COMPOUND_STRING("Caught by day and night"), sTwoParts, 1, FALSE},
};

struct ResearchRank
{
    u16 completed;
    u16 item;
    u8 count;
};

static const struct ResearchRank sRanks[] =
{
    {1, ITEM_GREAT_BALL, 5},
    {5, ITEM_ULTRA_BALL, 5},
    {10, ITEM_RARE_CANDY, 3},
    {25, ITEM_LUCKY_EGG, 1},
    {50, ITEM_OVAL_CHARM, 1},
    {100, ITEM_SHINY_CHARM, 1},
    {200, ITEM_MASTER_BALL, 1},
};

#define RANK_COUNT ARRAY_COUNT(sRanks)

#define TOAST_QUEUE 4
struct ResearchToast
{
    u8 title[32];
    u8 text[48];
};

struct ResearchState
{
    u16 wildSpecies;     // the wild Pokémon of the battle that started last
    u16 nextWildSpecies; // the wild Pokémon about to be made (shiny rolls)
    u32 wildPersonality;
    u8 toastCount;
    struct ResearchToast toasts[TOAST_QUEUE];
};

static EWRAM_EXT_DATA struct ResearchState sResearch;

static struct ResearchSave *Save(void)
{
    return &gSaveBlockExt.research;
}

static struct ResearchEntry *Entry(u32 nat)
{
    if (nat == 0 || nat >= RESEARCH_SPECIES)
        return NULL;
    return &Save()->entries[nat];
}

// Tasks ----------------------------------------------------------------------

static bool32 HasTask(u32 nat, u32 task)
{
    u16 species = NationalPokedexNumToSpecies(nat);

    switch (task)
    {
    case TASK_EVOLVE:
    {
        const struct Evolution *evos = GetSpeciesEvolutions(species);
        return evos != NULL && evos[0].method != EVOLUTIONS_END;
    }
    case TASK_DAYNIGHT:
        return DayNight_GetSpeciesTag(species) == DAYNIGHT_BOTH;
    }
    return TRUE;
}

static u32 TaskCount(const struct ResearchEntry *e, u32 task)
{
    switch (task)
    {
    case TASK_CATCH:
        return e->caught;
    case TASK_DEFEAT:
        return e->defeated;
    case TASK_EVOLVE:
        return e->evolved;
    case TASK_BIG:
        return (e->flags & RESEARCH_FLAG_BIG) != 0;
    case TASK_DAYNIGHT:
        return ((e->flags & RESEARCH_FLAG_DAY) != 0) + ((e->flags & RESEARCH_FLAG_NIGHT) != 0);
    }
    return 0;
}

static u32 TaskLevel(const struct ResearchEntry *e, u32 task)
{
    u32 i, count = TaskCount(e, task);

    for (i = 0; i < sTasks[task].tierCount && count >= sTasks[task].tiers[i]; i++)
        ;
    return i;
}

// The count for the next tier (or the last one).
static u32 TaskGoal(const struct ResearchEntry *e, u32 task)
{
    u32 level = TaskLevel(e, task);

    if (level >= sTasks[task].tierCount)
        level = sTasks[task].tierCount - 1;
    return sTasks[task].tiers[level];
}

static u32 LevelOf(u32 nat)
{
    const struct ResearchEntry *e = Entry(nat);
    u32 task, level = 0;

    if (e == NULL)
        return 0;
    for (task = 0; task < TASK_COUNT; task++)
    {
        if (HasTask(nat, task))
            level += TaskLevel(e, task);
    }
    return level;
}

static u32 MaxLevelOf(u32 nat)
{
    u32 task, level = 0;

    for (task = 0; task < TASK_COUNT; task++)
    {
        if (HasTask(nat, task))
            level += sTasks[task].tierCount;
    }
    return level;
}

u32 Research_GetLevel(u16 species)
{
    return LevelOf(SpeciesToNationalPokedexNum(species));
}

// Rewards --------------------------------------------------------------------

static void QueueToast(const u8 *title, const u8 *text)
{
    struct ResearchToast *t;

    if (sResearch.toastCount >= TOAST_QUEUE)
    {
        memmove(&sResearch.toasts[0], &sResearch.toasts[1], sizeof(sResearch.toasts[0]) * (TOAST_QUEUE - 1));
        sResearch.toastCount--;
    }
    t = &sResearch.toasts[sResearch.toastCount++];
    StringCopyN(t->title, title, sizeof(t->title) - 1);
    t->title[sizeof(t->title) - 1] = EOS;
    StringCopyN(t->text, text, sizeof(t->text) - 1);
    t->text[sizeof(t->text) - 1] = EOS;
}

static u8 *AppendNumber(u8 *dest, u32 n)
{
    ConvertIntToDecimalStringN(dest, n, STR_CONV_MODE_LEFT_ALIGN, 6);
    return dest + StringLength(dest);
}

static void CheckRanks(void)
{
    struct ResearchSave *save = Save();
    u8 title[32], text[48], *p;

    while (save->rank < RANK_COUNT && save->completed >= sRanks[save->rank].completed)
    {
        const struct ResearchRank *r = &sRanks[save->rank++];
        AddBagItem(r->item, r->count);
        p = StringCopy(title, COMPOUND_STRING("RESEARCH RANK "));
        AppendNumber(p, save->rank);
        p = StringCopy(text, COMPOUND_STRING("Reward: "));
        if (r->count > 1)
        {
            p = AppendNumber(p, r->count);
            *p++ = CHAR_SPACE;
            *p = EOS;
        }
        CopyItemName(r->item, p);
        QueueToast(title, text);
    }
}

static void OnLevelsGained(u32 nat, u32 before, u32 task)
{
    struct ResearchEntry *e = Entry(nat);
    u32 after = LevelOf(nat), money = 0, l;
    u16 species = NationalPokedexNumToSpecies(nat);
    u8 title[32], text[48], *p;

    if (after <= before)
        return;
    for (l = before + 1; l <= after; l++)
        money += RESEARCH_LEVEL_MONEY * min(l, RESEARCH_COMPLETE);
    AddMoney(&gSaveBlock1Ptr->money, money);

    if (after >= RESEARCH_COMPLETE && !(e->flags & RESEARCH_FLAG_COMPLETE))
    {
        s32 args[2];

        e->flags |= RESEARCH_FLAG_COMPLETE;
        Save()->completed++;
        AddMoney(&gSaveBlock1Ptr->money, RESEARCH_COMPLETE_MONEY);
        AddBagItem(ITEM_ULTRA_BALL, RESEARCH_COMPLETE_BALLS);
        p = StringCopy(title, COMPOUND_STRING("RESEARCH DONE: "));
        StringCopyN(p, GetSpeciesName(species), 12);
        p[12] = EOS;
        p = StringCopy(text, COMPOUND_STRING("¥"));
        p = AppendNumber(p, money + RESEARCH_COMPLETE_MONEY);
        StringCopy(p, COMPOUND_STRING(", 3 ULTRA BALLS, shiny odds up!"));
        QueueToast(title, text);
        args[0] = Save()->completed;
        args[1] = species;
        Online_Report("RESEARCH", 2, args);
        CheckRanks();
    }
    else
    {
        p = StringCopy(title, after > RESEARCH_COMPLETE ? COMPOUND_STRING("PERFECT ") : COMPOUND_STRING("RESEARCH LV "));
        if (after > RESEARCH_COMPLETE)
        {
            p = AppendNumber(p, after);
            p = StringCopy(p, COMPOUND_STRING("/"));
            p = AppendNumber(p, MaxLevelOf(nat));
        }
        else
        {
            p = AppendNumber(p, after);
        }
        p = StringCopy(p, COMPOUND_STRING(": "));
        StringCopyN(p, GetSpeciesName(species), 12);
        p[12] = EOS;
        p = StringCopy(text, sTasks[task].done);
        if (sTasks[task].counted)
        {
            *p++ = CHAR_SPACE;
            p = AppendNumber(p, TaskCount(e, task));
        }
        p = StringCopy(p, COMPOUND_STRING(". Got ¥"));
        p = AppendNumber(p, money);
        StringCopy(p, COMPOUND_STRING("!"));
        QueueToast(title, text);
    }
    if (after >= MaxLevelOf(nat) && !(e->flags & RESEARCH_FLAG_PERFECT))
    {
        e->flags |= RESEARCH_FLAG_PERFECT;
        AddBagItem(ITEM_RARE_CANDY, 1);
        p = StringCopy(title, COMPOUND_STRING("PERFECT: "));
        StringCopyN(p, GetSpeciesName(species), 12);
        p[12] = EOS;
        QueueToast(title, COMPOUND_STRING("Every task done! Got a RARE CANDY."));
    }
}

static void Bump(u8 *counter)
{
    if (*counter < 255)
        (*counter)++;
}

// Hooks ------------------------------------------------------------------------

void Research_OnWildBattleStart(void)
{
    sResearch.wildSpecies = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES);
    sResearch.wildPersonality = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_PERSONALITY);
}

void Research_OnCatch(void)
{
    u16 species = gBattleResults.caughtMonSpecies;
    u32 nat = SpeciesToNationalPokedexNum(species), before;
    struct ResearchEntry *e = Entry(nat);

    if (e == NULL)
        return;
    before = LevelOf(nat);
    Bump(&e->caught);
    if (species == sResearch.wildSpecies && gSpeciesInfo[species].height != 0
     && Records_SizeOf(species, sResearch.wildPersonality) * 100 >= gSpeciesInfo[species].height * 100 * BIG_PERCENT)
        e->flags |= RESEARCH_FLAG_BIG;
    e->flags |= GetTimeOfDay() == TIME_NIGHT ? RESEARCH_FLAG_NIGHT : RESEARCH_FLAG_DAY;
    OnLevelsGained(nat, before, TASK_CATCH);
}

void Research_OnDefeat(u16 species)
{
    u32 nat = SpeciesToNationalPokedexNum(species), before;
    struct ResearchEntry *e = Entry(nat);

    if (e == NULL)
        return;
    before = LevelOf(nat);
    Bump(&e->defeated);
    OnLevelsGained(nat, before, TASK_DEFEAT);
}

void Research_OnEvolve(u16 preEvoSpecies)
{
    u32 nat = SpeciesToNationalPokedexNum(preEvoSpecies), before;
    struct ResearchEntry *e = Entry(nat);

    if (e == NULL)
        return;
    before = LevelOf(nat);
    Bump(&e->evolved);
    OnLevelsGained(nat, before, TASK_EVOLVE);
}

void Research_OnWildSpecies(u16 species)
{
    sResearch.nextWildSpecies = species;
}

// Extra shiny rolls for the wild Pokémon being made (ComputePlayerShinyOdds).
u32 Research_ShinyRolls(void)
{
    u32 nat = SpeciesToNationalPokedexNum(sResearch.nextWildSpecies);
    const struct ResearchEntry *e = Entry(nat);

    sResearch.nextWildSpecies = SPECIES_NONE;
    if (e == NULL || !(e->flags & RESEARCH_FLAG_COMPLETE))
        return 0;
    return RESEARCH_SHINY_ROLLS + ((e->flags & RESEARCH_FLAG_PERFECT) != 0);
}

// Every overworld frame: the next toast, once the screen is free.
void Research_Update(void)
{
    if (sResearch.toastCount == 0 || World_IsToastBusy() || ArePlayerFieldControlsLocked())
        return;
    World_ShowToast(sResearch.toasts[0].title, sResearch.toasts[0].text);
    memmove(&sResearch.toasts[0], &sResearch.toasts[1], sizeof(sResearch.toasts[0]) * (TOAST_QUEUE - 1));
    sResearch.toastCount--;
}

void Research_GetCompletedCount(void)
{
    gSpecialVar_0x8005 = Save()->completed;
}

// The research screen (R menu > RESEARCH) --------------------------------------

#define LIST_ROWS        8
#define LIST_LINE_HEIGHT 14 // 8 rows end at 127; the hint line fits below
#define LIST_TOP         15
#define PIC_X            200 // the Pokémon's picture on a species page (centre, screen pixels)
#define PIC_Y            112

static const struct WindowTemplate sWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static const u8 sColorsNormal[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sColorsDone[3]   = {TEXT_COLOR_WHITE, TEXT_COLOR_GREEN, TEXT_COLOR_LIGHT_GREEN};
static const u8 sColorsHeader[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};
static const u8 sColorsRed[3]    = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};

struct ResearchScreen
{
    u16 count;
    u16 cursor;
    u16 scroll;
    u8 windowId;
    u8 spriteId;
    bool8 page; // a species page is open
    u16 list[RESEARCH_SPECIES];
};

static EWRAM_EXT_DATA struct ResearchScreen sScreen;

static void Task_Screen(u8 taskId);

static bool32 IsListed(u32 nat)
{
    const struct ResearchEntry *e = Entry(nat);

    return GetSetPokedexFlag(nat, FLAG_GET_SEEN)
        || (e != NULL && (e->caught || e->defeated || e->evolved || e->flags));
}

static void BuildList(void)
{
    u32 nat;

    sScreen.count = 0;
    for (nat = 1; nat < RESEARCH_SPECIES; nat++)
    {
        if (IsListed(nat))
            sScreen.list[sScreen.count++] = nat;
    }
}

static void DrawBar(u32 x, u32 y, u32 width, u32 value, u32 max, bool32 done)
{
    u32 filled = max == 0 ? 0 : (width - 2) * min(value, max) / max;

    FillWindowPixelRect(sScreen.windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), x, y, width, 6);
    FillWindowPixelRect(sScreen.windowId, PIXEL_FILL(TEXT_COLOR_WHITE), x + 1, y + 1, width - 2, 4);
    if (filled != 0)
        FillWindowPixelRect(sScreen.windowId, PIXEL_FILL(done ? TEXT_COLOR_GREEN : TEXT_COLOR_BLUE), x + 1, y + 1, filled, 4);
}

static void Print(const u8 *text, u32 font, u32 x, u32 y, const u8 *colors)
{
    AddTextPrinterParameterized3(sScreen.windowId, font, x, y, colors, TEXT_SKIP_DRAW, text);
}

static void PrintRight(const u8 *text, u32 font, u32 right, u32 y, const u8 *colors)
{
    Print(text, font, right - GetStringWidth(font, text, 0), y, colors);
}

static u8 *SpeciesLabel(u8 *dest, u32 nat)
{
    u8 *p = StringCopy(dest, COMPOUND_STRING("No."));
    ConvertIntToDecimalStringN(p, nat, STR_CONV_MODE_LEADING_ZEROS, nat >= 1000 ? 4 : 3);
    p += StringLength(p);
    *p++ = CHAR_SPACE;
    return StringCopy(p, GetSpeciesName(NationalPokedexNumToSpecies(nat)));
}

static void DrawList(void)
{
    u8 line[64], *p;
    u32 i;
    struct ResearchSave *save = Save();

    FillWindowPixelBuffer(sScreen.windowId, PIXEL_FILL(TEXT_COLOR_WHITE));
    Print(COMPOUND_STRING("RESEARCH POKéDEX"), FONT_NORMAL, 4, 0, sColorsNormal);
    p = StringCopy(line, COMPOUND_STRING("Done "));
    p = AppendNumber(p, save->completed);
    p = StringCopy(p, COMPOUND_STRING("   Rank "));
    AppendNumber(p, save->rank);
    PrintRight(line, FONT_SMALL, 220, 3, sColorsHeader);

    for (i = 0; i < LIST_ROWS && sScreen.scroll + i < sScreen.count; i++)
    {
        u32 index = sScreen.scroll + i, nat = sScreen.list[index];
        u32 y = LIST_TOP + i * LIST_LINE_HEIGHT, level = LevelOf(nat);
        bool32 done = (Entry(nat)->flags & RESEARCH_FLAG_COMPLETE) != 0;

        if (index == sScreen.cursor)
            Print(gText_SelectorArrow2, FONT_NORMAL, 0, y, sColorsNormal);
        SpeciesLabel(line, nat);
        Print(line, FONT_NORMAL, 10, y, done ? sColorsDone : sColorsNormal);
        p = StringCopy(line, COMPOUND_STRING("Lv "));
        AppendNumber(p, min(level, RESEARCH_COMPLETE));
        Print(line, FONT_SMALL, 140, y + 2, done ? sColorsDone : sColorsNormal);
        if (done)
            Print(COMPOUND_STRING("DONE"), FONT_SMALL, 182, y + 2, sColorsDone);
        else
            DrawBar(170, y + 5, 50, level, RESEARCH_COMPLETE, FALSE);
    }
    if (sScreen.count == 0)
        Print(COMPOUND_STRING("Catch or battle Pokémon to start."), FONT_NORMAL, 10, LIST_TOP, sColorsNormal);

    if (save->rank < RANK_COUNT)
    {
        p = StringCopy(line, COMPOUND_STRING("Rank "));
        p = AppendNumber(p, save->rank + 1);
        p = StringCopy(p, COMPOUND_STRING(" at "));
        p = AppendNumber(p, sRanks[save->rank].completed);
        p = StringCopy(p, COMPOUND_STRING(": "));
        CopyItemName(sRanks[save->rank].item, p);
    }
    else
    {
        StringCopy(line, COMPOUND_STRING("Top research rank!"));
    }
    Print(line, FONT_SMALL, 4, LIST_TOP + LIST_ROWS * LIST_LINE_HEIGHT, sColorsHeader);
    PrintRight(COMPOUND_STRING("A: tasks  B: close"), FONT_SMALL, 220, LIST_TOP + LIST_ROWS * LIST_LINE_HEIGHT, sColorsNormal);
    CopyWindowToVram(sScreen.windowId, COPYWIN_FULL);
}

static void ShowPic(u32 nat)
{
    sScreen.spriteId = CreateMonSprite_PicBox(NationalPokedexNumToSpecies(nat), PIC_X, PIC_Y, 0);
    gSprites[sScreen.spriteId].callback = SpriteCallbackDummy;
    gSprites[sScreen.spriteId].oam.priority = 0;
}

static void HidePic(void)
{
    if (sScreen.spriteId != MAX_SPRITES)
        FreeResourcesAndDestroySprite(&gSprites[sScreen.spriteId], sScreen.spriteId);
    sScreen.spriteId = MAX_SPRITES;
}

static void DrawPage(void)
{
    u32 nat = sScreen.list[sScreen.cursor], task, y = 32;
    const struct ResearchEntry *e = Entry(nat);
    u32 level = LevelOf(nat), max = MaxLevelOf(nat);
    bool32 done = (e->flags & RESEARCH_FLAG_COMPLETE) != 0;
    u8 line[64], *p;

    FillWindowPixelBuffer(sScreen.windowId, PIXEL_FILL(TEXT_COLOR_WHITE));
    SpeciesLabel(line, nat);
    Print(line, FONT_NORMAL, 4, 0, done ? sColorsDone : sColorsNormal);
    p = StringCopy(line, COMPOUND_STRING("Research level "));
    p = AppendNumber(p, min(level, RESEARCH_COMPLETE));
    p = StringCopy(p, COMPOUND_STRING(" / "));
    AppendNumber(p, RESEARCH_COMPLETE);
    Print(line, FONT_SMALL, 4, 14, sColorsHeader);
    DrawBar(110, 18, 110, level, RESEARCH_COMPLETE, done);

    for (task = 0; task < TASK_COUNT; task++)
    {
        u32 count, goal, tierLevel;
        bool32 taskDone;

        if (!HasTask(nat, task))
            continue;
        count = TaskCount(e, task);
        goal = TaskGoal(e, task);
        tierLevel = TaskLevel(e, task);
        taskDone = tierLevel >= sTasks[task].tierCount;
        Print(sTasks[task].name, FONT_SMALL, 4, y, taskDone ? sColorsDone : sColorsNormal);
        DrawBar(64, y + 4, 56, tierLevel, sTasks[task].tierCount, taskDone);
        p = AppendNumber(line, min(count, goal));
        p = StringCopy(p, COMPOUND_STRING("/"));
        AppendNumber(p, goal);
        PrintRight(line, FONT_SMALL, 152, y, taskDone ? sColorsDone : sColorsNormal);
        y += 16;
    }

    if (done)
    {
        Print(COMPOUND_STRING("Research complete!"), FONT_SMALL, 4, 114, sColorsDone);
        Print((e->flags & RESEARCH_FLAG_PERFECT) ? COMPOUND_STRING("Shiny odds x4 in the wild.") : COMPOUND_STRING("Shiny odds x3 in the wild."),
            FONT_SMALL, 4, 126, sColorsDone);
    }
    else
    {
        p = StringCopy(line, COMPOUND_STRING("Next level: ¥"));
        AppendNumber(p, RESEARCH_LEVEL_MONEY * min(level + 1, RESEARCH_COMPLETE));
        Print(line, FONT_SMALL, 4, 114, sColorsRed);
        Print(COMPOUND_STRING("Lv 10: ¥1000, shiny odds x3"), FONT_SMALL, 4, 126, sColorsRed);
    }
    if (max > RESEARCH_COMPLETE)
    {
        p = StringCopy(line, COMPOUND_STRING("Perfect: "));
        p = AppendNumber(p, level);
        p = StringCopy(p, COMPOUND_STRING("/"));
        AppendNumber(p, max);
        PrintRight(line, FONT_SMALL, 220, 3, (e->flags & RESEARCH_FLAG_PERFECT) ? sColorsDone : sColorsHeader);
    }
    CopyWindowToVram(sScreen.windowId, COPYWIN_FULL);
}

void Research_ShowScreen(void)
{
    BuildList();
    sScreen.cursor = 0;
    sScreen.scroll = 0;
    sScreen.page = FALSE;
    sScreen.spriteId = MAX_SPRITES;
    sScreen.windowId = AddWindow(&sWindowTemplate);
    DrawStdWindowFrame(sScreen.windowId, FALSE);
    PutWindowTilemap(sScreen.windowId);
    DrawList();
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Screen, 80);
}

static void Close(u8 taskId)
{
    HidePic();
    ClearStdWindowAndFrameToTransparent(sScreen.windowId, FALSE);
    ClearWindowTilemap(sScreen.windowId);
    RemoveWindow(sScreen.windowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Screen(u8 taskId)
{
    u16 old = sScreen.cursor;

    if (sScreen.page)
    {
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            HidePic();
            sScreen.page = FALSE;
            DrawList();
        }
        else if ((JOY_REPEAT(DPAD_DOWN) && sScreen.cursor + 1 < sScreen.count)
              || (JOY_REPEAT(DPAD_UP) && sScreen.cursor > 0))
        {
            sScreen.cursor += JOY_REPEAT(DPAD_DOWN) ? 1 : -1;
            PlaySE(SE_SELECT);
            HidePic();
            ShowPic(sScreen.list[sScreen.cursor]);
            DrawPage();
        }
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Close(taskId);
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sScreen.count == 0)
        {
            Close(taskId);
            return;
        }
        sScreen.page = TRUE;
        ShowPic(sScreen.list[sScreen.cursor]);
        DrawPage();
        return;
    }
    if (sScreen.count != 0)
    {
        if (JOY_REPEAT(DPAD_DOWN) && sScreen.cursor + 1 < sScreen.count)
            sScreen.cursor++;
        else if (JOY_REPEAT(DPAD_UP) && sScreen.cursor > 0)
            sScreen.cursor--;
        else if (JOY_REPEAT(DPAD_RIGHT))
            sScreen.cursor = min(sScreen.cursor + LIST_ROWS, sScreen.count - 1);
        else if (JOY_REPEAT(DPAD_LEFT))
            sScreen.cursor = sScreen.cursor >= LIST_ROWS ? sScreen.cursor - LIST_ROWS : 0;
    }
    if (sScreen.cursor != old)
    {
        if (sScreen.cursor < sScreen.scroll)
            sScreen.scroll = sScreen.cursor;
        else if (sScreen.cursor >= sScreen.scroll + LIST_ROWS)
            sScreen.scroll = sScreen.cursor - LIST_ROWS + 1;
        PlaySE(SE_SELECT);
        DrawList();
    }
}
