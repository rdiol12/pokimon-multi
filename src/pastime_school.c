#include "global.h"
#include "event_data.h"
#include "malloc.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "script_menu.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Trainer school: a new teacher at the Rustboro Trainer's School gives five
// lessons (status, weather, held items, abilities, battle basics), each a
// five-question test. Passing (4 right) the first time gives the lesson's
// prize; passing all five graduates the player, for an Amulet Coin.

extern const u8 School_EventScript_Teacher[];

struct Question
{
    const u8 *text;
    const u8 *answers[SCHOOL_ANSWERS];
    u8 right;
};

#define Q(t, a, b, c, d, r) {COMPOUND_STRING(t), {COMPOUND_STRING(a), COMPOUND_STRING(b), COMPOUND_STRING(c), COMPOUND_STRING(d)}, r}

static const struct Question sQuestions[SCHOOL_LESSONS][SCHOOL_QUESTIONS] =
{
    { // Status conditions
        Q("Which status keeps a Pokémon from\nmoving until it wakes up?", "SLEEP", "BURN", "POISON", "PARALYSIS", 0),
        Q("A burned Pokémon's physical moves\ndo how much damage?", "HALF", "DOUBLE", "NORMAL", "NONE", 0),
        Q("Which of these types can never\nbe poisoned?", "WATER", "STEEL", "FLYING", "ROCK", 1),
        Q("Paralysis halves a Pokémon's…", "ATTACK", "DEFENSE", "SPEED", "ACCURACY", 2),
        Q("Which type can never be burned?", "WATER", "ICE", "GROUND", "FIRE", 3),
    },
    { // Weather
        Q("Rain powers up moves of\nwhich type?", "GRASS", "WATER", "ELECTRIC", "ICE", 1),
        Q("Which move never misses\nin the rain?", "THUNDER", "FIRE BLAST", "BLIZZARD", "SURF", 0),
        Q("In harsh sunlight, SOLAR BEAM…", "MISSES", "IS WEAKER", "FIRES AT ONCE", "FAILS", 2),
        Q("Which type takes no damage\nfrom a sandstorm?", "WATER", "FIRE", "GRASS", "ROCK", 3),
        Q("Which Ability makes it rain\nwhen the Pokémon comes out?", "DROUGHT", "DRIZZLE", "SAND STREAM", "SWIFT SWIM", 1),
    },
    { // Held items
        Q("LEFTOVERS restore how much\nmax HP each turn?", "1/16", "1/8", "1/4", "1/2", 0),
        Q("Which item lets a Pokémon at full\nHP survive any one hit?", "QUICK CLAW", "SHELL BELL", "FOCUS SASH", "KING'S ROCK", 2),
        Q("CHOICE BAND raises Attack, but\nthe holder…", "LOSES HP", "IS SLOWER", "MISSES MORE", "USES 1 MOVE", 3),
        Q("Which Berry cures any\nstatus condition?", "ORAN", "LUM", "SITRUS", "CHERI", 1),
        Q("A Pokémon holding an EVERSTONE…", "WON'T EVOLVE", "TAKES LESS", "HEALS", "GETS MORE EXP", 0),
    },
    { // Abilities
        Q("LEVITATE makes a Pokémon immune\nto moves of which type?", "FLYING", "GROUND", "ROCK", "FIGHTING", 1),
        Q("INTIMIDATE lowers the foe's…", "SPEED", "DEFENSE", "ATTACK", "SP. ATK", 2),
        Q("Which Ability heals the Pokémon\nwhen a Water move hits it?", "TORRENT", "SWIFT SWIM", "DAMP", "WATER ABSORB", 3),
        Q("BLAZE powers up Fire moves when\nthe Pokémon's HP is…", "UNDER 1/3", "FULL", "OVER 1/2", "UNDER 1/4", 0),
        Q("At full HP, a Pokémon with\nSTURDY can't be…", "POISONED", "KO'D IN 1 HIT", "CRITICAL HIT", "SWITCHED", 1),
    },
    { // Battle basics
        Q("A move of the user's own type\ngets how much more power?", "x1.25", "x1.5", "x2", "NONE", 1),
        Q("QUICK ATTACK usually moves first\nbecause it has…", "MORE PP", "HIGH POWER", "PRIORITY", "NO MISS", 2),
        Q("A critical hit does how much\ndamage?", "x1.5", "x2", "x3", "x1.25", 0),
        Q("Without priority moves, which\nstat decides who moves first?", "LEVEL", "ATTACK", "HP", "SPEED", 3),
        Q("Switching a Pokémon out resets\nits…", "STAT CHANGES", "HP", "PP", "STATUS", 0),
    },
};

static const u8 *const sLessonNames[SCHOOL_LESSONS] =
{
    COMPOUND_STRING("STATUS"), COMPOUND_STRING("WEATHER"), COMPOUND_STRING("HELD ITEMS"),
    COMPOUND_STRING("ABILITIES"), COMPOUND_STRING("BATTLE BASICS"),
};

static const u16 sLessonPrizes[SCHOOL_LESSONS] =
{
    ITEM_FULL_HEAL, ITEM_DAMP_ROCK, ITEM_SHELL_BELL, ITEM_BOTTLE_CAP, ITEM_EXPERT_BELT,
};

struct SchoolTest
{
    u8 lesson;
    u8 question; // 1 to SCHOOL_QUESTIONS once started
    u8 correct;
    u8 right;    // the right answer of the current question
};

// Not static: the screenshot driver reads the right answer.
EWRAM_DATA struct SchoolTest gSchoolTest = {0};
#define sTest gSchoolTest

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gSchoolSpots[1] =
{
    {MAP_RUSTBORO_CITY_POKEMON_SCHOOL, 11, 3, OBJ_EVENT_GFX_WOMAN_3, MOVEMENT_TYPE_FACE_DOWN, 0, School_EventScript_Teacher},
};

static void PushMenuItem(const u8 *name, u32 id)
{
    struct ListMenuItem item;
    u8 *buffer = Alloc(StringLength(name) + 1);

    StringCopy(buffer, name);
    item.name = buffer;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

// Script: pushes the lessons onto the dynmultichoice stack (passed ones
// marked). VAR_RESULT = TRUE if the player has graduated.
void School_PushLessonMenu(void)
{
    u8 text[32];
    u32 i;

    for (i = 0; i < SCHOOL_LESSONS; i++)
    {
        u8 *p = StringCopy(text, sLessonNames[i]);
        if (sSave.schoolPassed & (1 << i))
            StringCopy(p, COMPOUND_STRING(" {COLOR GREEN}{SHADOW LIGHT_GREEN}PASSED"));
        PushMenuItem(text, i);
    }
    PushMenuItem(COMPOUND_STRING("CANCEL"), SCHOOL_MENU_CANCEL);
    gSpecialVar_Result = sSave.schoolPassed == (1 << SCHOOL_LESSONS) - 1;
}

// Script: starts the lesson VAR_0x8005. gStringVar1 = its name.
void School_StartLesson(void)
{
    memset(&sTest, 0, sizeof(sTest));
    sTest.lesson = gSpecialVar_0x8005;
    StringCopy(gStringVar1, sLessonNames[sTest.lesson]);
    if (sSave.schoolTests < 0xFFFF)
        sSave.schoolTests++;
}

// Script: the next question in gPastimeText; VAR_RESULT = FALSE when the
// test is over.
void School_NextQuestion(void)
{
    const struct Question *q;
    u8 *p = gPastimeText;

    if (sTest.question == SCHOOL_QUESTIONS)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    q = &sQuestions[sTest.lesson][sTest.question++];
    sTest.right = q->right;
    p = StringCopy(p, COMPOUND_STRING("Q"));
    p = ConvertIntToDecimalStringN(p, sTest.question, STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, COMPOUND_STRING(". "));
    StringCopy(p, q->text);
    gSpecialVar_Result = TRUE;
}

// Script: pushes the current question's answers.
void School_PushAnswers(void)
{
    const struct Question *q = &sQuestions[sTest.lesson][sTest.question - 1];
    u32 i;

    for (i = 0; i < SCHOOL_ANSWERS; i++)
        PushMenuItem(q->answers[i], i);
}

// Script: VAR_RESULT = the answer picked; becomes TRUE if it's right.
// gStringVar2 = the right answer.
void School_Answer(void)
{
    const struct Question *q = &sQuestions[sTest.lesson][sTest.question - 1];

    StringCopy(gStringVar2, q->answers[q->right]);
    if (gSpecialVar_Result == q->right)
    {
        sTest.correct++;
        gSpecialVar_Result = TRUE;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

// Script: the test is over. gStringVar1 = the score; VAR_RESULT =
// SCHOOL_FAILED, SCHOOL_PASSED (again) or SCHOOL_PASSED_FIRST, and then
// VAR_0x8005 = the prize; VAR_0x8006 = TRUE if the player just graduated.
void School_Finish(void)
{
    bool32 wasGraduate = sSave.schoolPassed == (1 << SCHOOL_LESSONS) - 1;

    ConvertIntToDecimalStringN(gStringVar1, sTest.correct, STR_CONV_MODE_LEFT_ALIGN, 1);
    gSpecialVar_0x8006 = FALSE;
    if (sTest.correct < SCHOOL_PASS_MARK)
    {
        gSpecialVar_Result = SCHOOL_FAILED;
        return;
    }
    if (sSave.schoolPassed & (1 << sTest.lesson))
    {
        gSpecialVar_Result = SCHOOL_PASSED;
        return;
    }
    sSave.schoolPassed |= 1 << sTest.lesson;
    gSpecialVar_Result = SCHOOL_PASSED_FIRST;
    gSpecialVar_0x8005 = sLessonPrizes[sTest.lesson];
    gSpecialVar_0x8006 = !wasGraduate && sSave.schoolPassed == (1 << SCHOOL_LESSONS) - 1;
}

// R > PASTIMES > SCHOOL ---------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < SCHOOL_LESSONS)
    {
        dest = StringCopy(dest, sLessonNames[index]);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 112}"));
        StringCopy(dest, (sSave.schoolPassed & (1 << index)) ? COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}PASSED") : COMPOUND_STRING("-"));
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Tests taken{CLEAR_TO 112}"));
    ConvertIntToDecimalStringN(dest, sSave.schoolTests, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void GetDetail(u16 index, u8 *dest)
{
    if (sSave.schoolPassed == (1 << SCHOOL_LESSONS) - 1)
        StringCopy(dest, COMPOUND_STRING("Graduated from the Trainer's School!"));
    else
        StringCopy(dest, COMPOUND_STRING("Lessons at the Rustboro Trainer's School"));
}

void School_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("TRAINER'S SCHOOL"), SCHOOL_LESSONS + 1, GetLine, GetDetail);
}
