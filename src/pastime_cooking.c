#include "global.h"
#include "event_data.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "task.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Cooking contest: the COOK-OFF judge next to CAFÉ SOLEIL in Lilycove runs
// one contest a day. The player cooks one of the cafe's dishes (using its
// ingredients) in three timing meter steps (chop, stir, plate) against three
// rival chefs. Winning moves the player up a rank (NORMAL, GREAT, ULTRA,
// MASTER), where the meter is harder and the rivals better, and wins the
// rank's prize. The best score goes to the server's COOK-OFF CHAMPS board.

extern const u8 Cook_EventScript_Judge[];

static const u8 *const sRankNames[COOK_RANK_COUNT] =
{
    [COOK_RANK_NORMAL] = COMPOUND_STRING("NORMAL"),
    [COOK_RANK_GREAT]  = COMPOUND_STRING("GREAT"),
    [COOK_RANK_ULTRA]  = COMPOUND_STRING("ULTRA"),
    [COOK_RANK_MASTER] = COMPOUND_STRING("MASTER"),
};

static const struct
{
    u8 zoneTiles;
    u8 speed;
    u8 rivalBase; // rivals score this plus 0 to 15
    u16 prize;
} sRanks[COOK_RANK_COUNT] =
{
    [COOK_RANK_NORMAL] = {4, 2, 45, ITEM_RARE_CANDY},
    [COOK_RANK_GREAT]  = {3, 3, 57, ITEM_PP_MAX},
    [COOK_RANK_ULTRA]  = {3, 4, 69, ITEM_LEFTOVERS},
    [COOK_RANK_MASTER] = {2, 4, 80, ITEM_ABILITY_CAPSULE},
};

static const u8 *const sRivalNames[] =
{
    COMPOUND_STRING("ARNAUD"), COMPOUND_STRING("MIREILLE"), COMPOUND_STRING("GUS"),
    COMPOUND_STRING("PAPRIKA"), COMPOUND_STRING("BASIL"), COMPOUND_STRING("ODETTE"),
};

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gCookingSpots[1] =
{
    {MAP_LILYCOVE_CITY, 40, 26, OBJ_EVENT_GFX_GENTLEMAN, MOVEMENT_TYPE_FACE_DOWN, 0, Cook_EventScript_Judge},
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

// Script: VAR_RESULT = TRUE if today's contest is still open.
// gStringVar1 = the player's rank.
void Cook_CheckEntry(void)
{
    gSpecialVar_Result = sSave.cookDay != Today() + 1; // + 1, so 0 means never
    StringCopy(gStringVar1, sRankNames[sSave.cookRank]);
}

// Script: VAR_0x8005 = the dish picked. VAR_RESULT = TRUE if the player has
// its ingredients, which are then used up. gStringVar2 = the dish.
void Cook_CheckDish(void)
{
    u32 dish = gSpecialVar_0x8005;

    StringCopy(gStringVar2, Cafe_GetDishName(dish));
    gSpecialVar_Result = Cafe_HasIngredients(dish);
    if (!gSpecialVar_Result)
        return;
    Cafe_UseIngredients(dish);
    sSave.cookDay = Today() + 1;
    if (sSave.cookEntries < 0xFFFF)
        sSave.cookEntries++;
}

static void Task_Cook(u8 taskId)
{
    s32 hits = TimingMeter_GetResult();

    if (hits < 0)
        return;
    gSpecialVar_0x8006 = hits;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Script: `callnative Cook_Play, waitstate=1`. VAR_0x8006 = steps done well.
void Cook_Play(void)
{
    CreateTask(Task_Cook, 80);
    TimingMeter_Start(COOK_STEPS, sRanks[sSave.cookRank].zoneTiles, sRanks[sSave.cookRank].speed);
}

static u8 *AppendScore(u8 *p, const u8 *name, u32 score)
{
    p = StringCopy(p, name);
    *p++ = CHAR_SPACE;
    return ConvertIntToDecimalStringN(p, score, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Script: judges the dish (VAR_0x8005) cooked with VAR_0x8006 good steps.
// gPastimeText = the verdict. VAR_RESULT = TRUE if the player won, and then
// VAR_0x8005 = the prize.
void Cook_Judge(void)
{
    u32 dish = gSpecialVar_0x8005, rank = sSave.cookRank;
    u32 score, rivals[COOK_RIVALS], first, place = 1, i;
    u8 *p = gPastimeText;

    // 30 for the dish (40 for the feast), 20 a good step, 0 to 10 for taste.
    score = 30 + gSpecialVar_0x8006 * 20 + Random() % 11;
    if (dish == CAFE_DISH_COUNT - 1)
        score += 10;
    score = min(score, 100);
    first = Random() % ARRAY_COUNT(sRivalNames);
    for (i = 0; i < COOK_RIVALS; i++)
    {
        rivals[i] = sRanks[rank].rivalBase + Random() % 16;
        if (rivals[i] >= score)
            place++;
    }

    p = StringCopy(p, COMPOUND_STRING("The judges taste your "));
    p = StringCopy(p, Cafe_GetDishName(dish));
    p = StringCopy(p, COMPOUND_STRING("…\nScore: "));
    p = ConvertIntToDecimalStringN(p, score, STR_CONV_MODE_LEFT_ALIGN, 3);
    p = StringCopy(p, COMPOUND_STRING(" points!\p"));
    for (i = 0; i < COOK_RIVALS; i++)
    {
        p = AppendScore(p, sRivalNames[(first + i) % ARRAY_COUNT(sRivalNames)], rivals[i]);
        p = StringCopy(p, i == 0 ? COMPOUND_STRING(", ") : i == 1 ? COMPOUND_STRING(",\n") : COMPOUND_STRING("."));
    }
    if (place == 1)
    {
        p = StringCopy(p, COMPOUND_STRING("\pYou win the "));
        p = StringCopy(p, sRankNames[rank]);
        StringCopy(p, COMPOUND_STRING(" RANK\nCOOK-OFF!"));
    }
    else
    {
        p = StringCopy(p, COMPOUND_STRING("\pYou placed "));
        StringCopy(p, place == 2 ? COMPOUND_STRING("2nd.") : place == 3 ? COMPOUND_STRING("3rd.") : COMPOUND_STRING("4th."));
    }

    if (score > sSave.cookBest)
        sSave.cookBest = score;
    gSpecialVar_Result = (place == 1);
    if (place == 1)
    {
        gSpecialVar_0x8005 = sRanks[rank].prize;
        if (rank < COOK_RANK_MASTER)
            sSave.cookRank++;
        else
            gSpecialVar_0x8005 = ITEM_RARE_CANDY;
        if (sSave.cookWins < 0xFFFF)
            sSave.cookWins++;
    }
    StringCopy(gStringVar1, sRankNames[sSave.cookRank]);
    World_SubmitScore(WORLD_BOARD_COOKING, sSave.cookBest, sSave.cookWins);
}

// R > PASTIMES > COOK-OFF -------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    static const u8 *const labels[] =
    {
        COMPOUND_STRING("Rank"),
        COMPOUND_STRING("Best score"),
        COMPOUND_STRING("Contests won"),
        COMPOUND_STRING("Contests entered"),
        COMPOUND_STRING("Today's contest"),
    };

    dest = StringCopy(dest, labels[index]);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 128}"));
    switch (index)
    {
    case 0:
        StringCopy(dest, sRankNames[sSave.cookRank]);
        break;
    case 1:
        ConvertIntToDecimalStringN(dest, sSave.cookBest, STR_CONV_MODE_LEFT_ALIGN, 3);
        break;
    case 2:
        ConvertIntToDecimalStringN(dest, sSave.cookWins, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    case 3:
        ConvertIntToDecimalStringN(dest, sSave.cookEntries, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    default:
        StringCopy(dest, sSave.cookDay == Today() + 1 ? COMPOUND_STRING("Entered") : COMPOUND_STRING("Open"));
        break;
    }
}

static void GetDetail(u16 index, u8 *dest)
{
    if (index == 0 && sSave.cookRank < COOK_RANK_MASTER)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Win to reach "));
        dest = StringCopy(dest, sRankNames[sSave.cookRank + 1]);
        StringCopy(dest, COMPOUND_STRING(" rank"));
    }
    else
    {
        StringCopy(dest, COMPOUND_STRING("The judge is next to CAFÉ SOLEIL"));
    }
}

void Cook_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("LILYCOVE COOK-OFF"), 5, GetLine, GetDetail);
}
