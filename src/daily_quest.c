#include "global.h"
#include "battle_main.h"
#include "daily_quest.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "overworld.h"
#include "pokemon.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "trainer_level.h"
#include "world.h"
#include "constants/items.h"

// Daily quests: three people ask for a small favour each day (the Pokémon
// Center side of Petalburg, Slateport and Fortree). A quest stays until it
// is finished; the next one is offered the day after it was turned in.
// R > ADVENTURE > DAILY QUESTS lists the quests taken and how far along they
// are, and a notice shows when one is ready to turn in.
//
// Most quests count something from the moment they are taken: catches,
// trainers beaten, steps, berries, fish, hatched Eggs, battles and trades
// with other players. "Show me" quests are checked when talking to the
// person: a Pokémon of the right type and level has to be in the party.

enum QuestKind
{
    QUEST_CATCH,      // catch `goal` Pokémon
    QUEST_CATCH_TYPE, // catch `goal` Pokémon of type `param`
    QUEST_TRAINERS,   // beat `goal` trainers
    QUEST_STEPS,      // walk `goal` steps
    QUEST_BERRIES,    // pick `goal` berries
    QUEST_FISH,       // land `goal` Pokémon with a rod
    QUEST_HATCH,      // hatch `goal` Eggs
    QUEST_PVP,        // battle another player `goal` times
    QUEST_TRADE,      // trade with another player `goal` times
    QUEST_SHOW_TYPE,  // show a Pokémon of type `param`, level `goal` or more
};

struct DailyQuest
{
    u8 kind;
    u8 param;
    u16 goal;
    u16 item;
    u16 money;
};

enum SlotState
{
    SLOT_FREE,   // a new quest is offered (from slot.day on)
    SLOT_ACTIVE,
    SLOT_READY,  // the goal is reached; the notice was shown
    SLOT_DONE,   // turned in on slot.day
};

#define XP_QUEST 40

// Petalburg: everyday training.
static const struct DailyQuest sPetalburgQuests[] =
{
    {QUEST_CATCH,      0,             3,    ITEM_GREAT_BALL,   1000},
    {QUEST_TRAINERS,   0,             3,    ITEM_SUPER_POTION, 1500},
    {QUEST_STEPS,      0,             1000, ITEM_RARE_CANDY,   1000},
    {QUEST_SHOW_TYPE,  TYPE_NORMAL,   20,   ITEM_SILK_SCARF,   1000},
    {QUEST_CATCH_TYPE, TYPE_BUG,      1,    ITEM_NET_BALL,     800},
    {QUEST_PVP,        0,             1,    ITEM_PP_UP,        2000},
    {QUEST_HATCH,      0,             1,    ITEM_OVAL_STONE,   1500},
};

// Slateport: the sea.
static const struct DailyQuest sSlateportQuests[] =
{
    {QUEST_FISH,       0,             3,    ITEM_DIVE_BALL,    1500},
    {QUEST_CATCH_TYPE, TYPE_WATER,    2,    ITEM_MYSTIC_WATER, 1200},
    {QUEST_SHOW_TYPE,  TYPE_WATER,    30,   ITEM_PEARL,        1500},
    {QUEST_TRADE,      0,             1,    ITEM_STAR_PIECE,   2000},
    {QUEST_CATCH_TYPE, TYPE_ELECTRIC, 1,    ITEM_MAGNET,       1200},
    {QUEST_TRAINERS,   0,             5,    ITEM_NUGGET,       2500},
    {QUEST_STEPS,      0,             2000, ITEM_SHELL_BELL,   1500},
};

// Fortree: nature and the sky.
static const struct DailyQuest sFortreeQuests[] =
{
    {QUEST_BERRIES,    0,             10,   ITEM_GROWTH_MULCH, 1200},
    {QUEST_CATCH_TYPE, TYPE_FLYING,   2,    ITEM_SHARP_BEAK,   1200},
    {QUEST_CATCH_TYPE, TYPE_GRASS,    2,    ITEM_MIRACLE_SEED, 1200},
    {QUEST_SHOW_TYPE,  TYPE_FLYING,   35,   ITEM_PRETTY_FEATHER, 2000},
    {QUEST_CATCH,      0,             5,    ITEM_ULTRA_BALL,   2000},
    {QUEST_SHOW_TYPE,  TYPE_GRASS,    30,   ITEM_BIG_ROOT,     1500},
    {QUEST_PVP,        0,             2,    ITEM_BOTTLE_CAP,   3000},
};

static const struct
{
    const struct DailyQuest *quests;
    u8 count;
    const u8 *place;
} sGivers[DAILY_QUEST_GIVERS] =
{
    [DAILY_QUEST_PETALBURG] = {sPetalburgQuests, ARRAY_COUNT(sPetalburgQuests), COMPOUND_STRING("PETALBURG")},
    [DAILY_QUEST_SLATEPORT] = {sSlateportQuests, ARRAY_COUNT(sSlateportQuests), COMPOUND_STRING("SLATEPORT")},
    [DAILY_QUEST_FORTREE]   = {sFortreeQuests,   ARRAY_COUNT(sFortreeQuests),   COMPOUND_STRING("FORTREE")},
};

#define sSlots (gSaveBlock3Ptr->adventure.dailyQuests)

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

// The quest offered on a given day: each giver walks its list in its own order.
static const struct DailyQuest *QuestFor(u32 giver, u32 day)
{
    return &sGivers[giver].quests[(day * 5 + giver * 3) % sGivers[giver].count];
}

static const struct DailyQuest *SlotQuest(u32 giver)
{
    return &sGivers[giver].quests[sSlots[giver].quest % sGivers[giver].count];
}

// The counter a quest measures: game stats and world records for most,
// the slot's own count for the ones counted as they happen.
static u16 Counter(const struct DailyQuest *quest, u32 giver)
{
    switch (quest->kind)
    {
    case QUEST_STEPS:
        return GetGameStat(GAME_STAT_STEPS);
    case QUEST_BERRIES:
        return gSaveBlock3Ptr->world.berriesHarvested;
    case QUEST_FISH:
        return gSaveBlock3Ptr->world.fishLanded;
    case QUEST_HATCH:
        return GetGameStat(GAME_STAT_HATCHED_EGGS);
    case QUEST_PVP:
        return GetGameStat(GAME_STAT_LINK_BATTLE_WINS) + GetGameStat(GAME_STAT_LINK_BATTLE_LOSSES)
             + GetGameStat(GAME_STAT_LINK_BATTLE_DRAWS);
    case QUEST_TRADE:
        return GetGameStat(GAME_STAT_POKEMON_TRADES);
    default:
        return sSlots[giver].count;
    }
}

static u32 Progress(u32 giver)
{
    const struct DailyQuest *quest = SlotQuest(giver);
    u32 done;

    if (quest->kind == QUEST_SHOW_TYPE)
        return 0;
    done = (u16)(Counter(quest, giver) - sSlots[giver].base);
    return min(done, quest->goal);
}

static bool32 PartyHasTypeAtLevel(u32 type, u32 level)
{
    u32 i;

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        u32 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        if (species == SPECIES_NONE || species == SPECIES_EGG)
            continue;
        if (GetMonData(&gPlayerParty[i], MON_DATA_LEVEL) >= level
         && (GetSpeciesType(species, 0) == type || GetSpeciesType(species, 1) == type))
            return TRUE;
    }
    return FALSE;
}

static void Describe(const struct DailyQuest *quest, u8 *dest)
{
    u8 num[8];

    ConvertIntToDecimalStringN(num, quest->goal, STR_CONV_MODE_LEFT_ALIGN, 5);
    switch (quest->kind)
    {
    case QUEST_CATCH:
        StringCopy(dest, COMPOUND_STRING("Catch "));
        StringAppend(dest, num);
        StringAppend(dest, COMPOUND_STRING(" Pokémon"));
        break;
    case QUEST_CATCH_TYPE:
        StringCopy(dest, COMPOUND_STRING("Catch "));
        StringAppend(dest, num);
        StringAppend(dest, COMPOUND_STRING(" "));
        StringAppend(dest, gTypesInfo[quest->param].name);
        StringAppend(dest, COMPOUND_STRING("-type"));
        break;
    case QUEST_TRAINERS:
        StringCopy(dest, COMPOUND_STRING("Beat "));
        StringAppend(dest, num);
        StringAppend(dest, COMPOUND_STRING(" Trainers"));
        break;
    case QUEST_STEPS:
        StringCopy(dest, COMPOUND_STRING("Walk "));
        StringAppend(dest, num);
        StringAppend(dest, COMPOUND_STRING(" steps"));
        break;
    case QUEST_BERRIES:
        StringCopy(dest, COMPOUND_STRING("Pick "));
        StringAppend(dest, num);
        StringAppend(dest, COMPOUND_STRING(" Berries"));
        break;
    case QUEST_FISH:
        StringCopy(dest, COMPOUND_STRING("Fish up "));
        StringAppend(dest, num);
        StringAppend(dest, COMPOUND_STRING(" Pokémon"));
        break;
    case QUEST_HATCH:
        StringCopy(dest, COMPOUND_STRING("Hatch an Egg"));
        break;
    case QUEST_PVP:
        StringCopy(dest, COMPOUND_STRING("Battle a player online"));
        if (quest->goal > 1)
        {
            StringAppend(dest, COMPOUND_STRING(" x"));
            StringAppend(dest, num);
        }
        break;
    case QUEST_TRADE:
        StringCopy(dest, COMPOUND_STRING("Trade with a player online"));
        break;
    case QUEST_SHOW_TYPE:
        StringCopy(dest, COMPOUND_STRING("Show a Lv."));
        StringAppend(dest, num);
        StringAppend(dest, COMPOUND_STRING("+ "));
        StringAppend(dest, gTypesInfo[quest->param].name);
        StringAppend(dest, COMPOUND_STRING(" type"));
        break;
    }
}

static void AppendProgress(u32 giver, u8 *dest)
{
    const struct DailyQuest *quest = SlotQuest(giver);

    if (quest->kind == QUEST_SHOW_TYPE)
        return;
    StringAppend(dest, COMPOUND_STRING(" "));
    ConvertIntToDecimalStringN(dest + StringLength(dest), Progress(giver), STR_CONV_MODE_LEFT_ALIGN, 5);
    StringAppend(dest, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(dest + StringLength(dest), quest->goal, STR_CONV_MODE_LEFT_ALIGN, 5);
}

// Talking to a quest giver (VAR_0x8004 = DAILY_QUEST_*). Sets VAR_RESULT to
// DAILY_QUEST_TALK_* and STR_VAR_1 to the quest, STR_VAR_2 to the reward.
void DailyQuest_Talk(void)
{
    u32 giver = gSpecialVar_0x8004;
    u16 today = Today();
    const struct DailyQuest *quest;

    if (giver >= DAILY_QUEST_GIVERS)
        return;
    if (sSlots[giver].state == SLOT_DONE && sSlots[giver].day != today)
        sSlots[giver].state = SLOT_FREE;
    switch (sSlots[giver].state)
    {
    case SLOT_FREE:
        quest = QuestFor(giver, today);
        Describe(quest, gStringVar1);
        CopyItemName(quest->item, gStringVar2);
        gSpecialVar_Result = DAILY_QUEST_TALK_OFFER;
        break;
    case SLOT_DONE:
        gSpecialVar_Result = DAILY_QUEST_TALK_COME_BACK;
        break;
    default:
        quest = SlotQuest(giver);
        Describe(quest, gStringVar1);
        CopyItemName(quest->item, gStringVar2);
        if ((quest->kind == QUEST_SHOW_TYPE && PartyHasTypeAtLevel(quest->param, quest->goal))
         || (quest->kind != QUEST_SHOW_TYPE && Progress(giver) >= quest->goal))
        {
            gSpecialVar_Result = DAILY_QUEST_TALK_COMPLETE;
        }
        else
        {
            AppendProgress(giver, gStringVar1);
            gSpecialVar_Result = DAILY_QUEST_TALK_PROGRESS;
        }
        break;
    }
}

void DailyQuest_Accept(void)
{
    u32 giver = gSpecialVar_0x8004;
    u16 today = Today();
    const struct DailyQuest *quest = QuestFor(giver, today);

    sSlots[giver].quest = quest - sGivers[giver].quests;
    sSlots[giver].day = today;
    sSlots[giver].count = 0;
    sSlots[giver].state = SLOT_ACTIVE;
    sSlots[giver].base = Counter(quest, giver);
}

// Gives the reward. VAR_RESULT: FALSE if the bag had no room for the item
// (the quest stays open).
void DailyQuest_Claim(void)
{
    u32 giver = gSpecialVar_0x8004;
    const struct DailyQuest *quest = SlotQuest(giver);

    if (!AddBagItem(quest->item, 1))
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    AddMoney(&gSaveBlock1Ptr->money, quest->money);
    ConvertIntToDecimalStringN(gStringVar3, quest->money, STR_CONV_MODE_LEFT_ALIGN, 6);
    sSlots[giver].state = SLOT_DONE;
    sSlots[giver].day = Today();
    TrainerLevel_AddXp(XP_QUEST);
    gSpecialVar_Result = TRUE;
}

// Counted as they happen.
void DailyQuest_OnCatch(u16 species)
{
    u32 giver;

    for (giver = 0; giver < DAILY_QUEST_GIVERS; giver++)
    {
        const struct DailyQuest *quest = SlotQuest(giver);
        if (sSlots[giver].state != SLOT_ACTIVE)
            continue;
        if (quest->kind == QUEST_CATCH
         || (quest->kind == QUEST_CATCH_TYPE
          && (GetSpeciesType(species, 0) == quest->param || GetSpeciesType(species, 1) == quest->param)))
            sSlots[giver].count++;
    }
}

void DailyQuest_OnTrainerWin(void)
{
    u32 giver;

    for (giver = 0; giver < DAILY_QUEST_GIVERS; giver++)
    {
        if (sSlots[giver].state == SLOT_ACTIVE && SlotQuest(giver)->kind == QUEST_TRAINERS)
            sSlots[giver].count++;
    }
}

// Shows a notice once a quest's goal is reached.
void DailyQuest_Update(void)
{
    u32 giver;
    u8 title[32];

    if ((gMain.vblankCounter1 & 0x3F) != 0x20)
        return;
    for (giver = 0; giver < DAILY_QUEST_GIVERS; giver++)
    {
        const struct DailyQuest *quest = SlotQuest(giver);
        if (sSlots[giver].state != SLOT_ACTIVE || quest->kind == QUEST_SHOW_TYPE || Progress(giver) < quest->goal)
            continue;
        sSlots[giver].state = SLOT_READY;
        Describe(quest, gStringVar4);
        StringCopy(title, COMPOUND_STRING("Quest done! Back to "));
        StringAppend(title, sGivers[giver].place);
        World_ShowToast(title, gStringVar4);
        return;
    }
}

// R > ADVENTURE > DAILY QUESTS.
static EWRAM_DATA u8 sRows[DAILY_QUEST_GIVERS] = {0};

static void GetQuestLine(u16 index, u8 *dest)
{
    u32 giver = sRows[index];

    StringCopy(dest, sGivers[giver].place);
    StringAppend(dest, COMPOUND_STRING(": "));
    Describe(SlotQuest(giver), dest + StringLength(dest));
    if (sSlots[giver].state == SLOT_DONE)
        StringAppend(dest, COMPOUND_STRING(" {COLOR GREEN}{SHADOW LIGHT_GREEN}DONE"));
    else
        AppendProgress(giver, dest);
}

static void GetQuestDetail(u16 index, u8 *dest)
{
    u32 giver = sRows[index];
    const struct DailyQuest *quest = SlotQuest(giver);

    if (sSlots[giver].state == SLOT_DONE)
    {
        StringCopy(dest, COMPOUND_STRING("Turned in. A new request comes tomorrow."));
        return;
    }
    StringCopy(dest, COMPOUND_STRING("Reward: "));
    CopyItemName(quest->item, dest + StringLength(dest));
    StringAppend(dest, COMPOUND_STRING(" and "));
    ConvertIntToDecimalStringN(dest + StringLength(dest), quest->money, STR_CONV_MODE_LEFT_ALIGN, 6);
    StringAppend(dest, COMPOUND_STRING(" money"));
}

void DailyQuest_ShowList(void)
{
    u32 giver, count = 0;

    u16 today = Today();

    for (giver = 0; giver < DAILY_QUEST_GIVERS; giver++)
    {
        if (sSlots[giver].state == SLOT_DONE && sSlots[giver].day != today)
            sSlots[giver].state = SLOT_FREE;
        if (sSlots[giver].state != SLOT_FREE)
            sRows[count++] = giver;
    }
    WorldBoard_Show(COMPOUND_STRING("DAILY QUESTS"), count, GetQuestLine, GetQuestDetail);
}
