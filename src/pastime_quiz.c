#include "global.h"
#include "battle_main.h"
#include "battle_util.h"
#include "event_data.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "pokemon.h"
#include "random.h"
#include "rtc.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Battle quiz hall: the QUIZMASTER in Mauville asks ten type matchup
// questions a round ("A FIRE move hits FERROTHORN. How effective?"), with
// the defending Pokémon's picture. Right answers score 10 points plus the
// current streak. The first round each day wins a prize by score; the best
// score goes to the server's QUIZ CHAMPS leaderboard.

extern const u8 Quiz_EventScript_Quizmaster[];

// Defenders: well-known Pokémon with a good spread of types.
static const u16 sDefenders[] =
{
    SPECIES_PIKACHU, SPECIES_CHARIZARD, SPECIES_GYARADOS, SPECIES_GENGAR, SPECIES_SNORLAX,
    SPECIES_DRAGONITE, SPECIES_STEELIX, SPECIES_SCIZOR, SPECIES_TYRANITAR, SPECIES_SKARMORY,
    SPECIES_SWAMPERT, SPECIES_GARDEVOIR, SPECIES_SABLEYE, SPECIES_SALAMENCE, SPECIES_METAGROSS,
    SPECIES_LUDICOLO, SPECIES_ALTARIA, SPECIES_CLAYDOL, SPECIES_GARCHOMP, SPECIES_LUCARIO,
    SPECIES_TOGEKISS, SPECIES_ROTOM, SPECIES_FERROTHORN, SPECIES_CHANDELURE, SPECIES_EXCADRILL,
    SPECIES_SYLVEON, SPECIES_AEGISLASH, SPECIES_DECIDUEYE, SPECIES_TOXAPEX, SPECIES_MIMIKYU,
    SPECIES_CORVIKNIGHT, SPECIES_DRAGAPULT, SPECIES_ANNIHILAPE, SPECIES_KINGAMBIT, SPECIES_AZUMARILL,
    SPECIES_BRONZONG, SPECIES_MAGNEZONE, SPECIES_WHISCASH, SPECIES_SHEDINJA, SPECIES_MARSHADOW,
};

static const u8 *const sAnswerNames[] =
{
    [QUIZ_SUPER]     = COMPOUND_STRING("super effective"),
    [QUIZ_NORMAL]    = COMPOUND_STRING("normal damage"),
    [QUIZ_NOT_VERY]  = COMPOUND_STRING("not very effective"),
    [QUIZ_NO_EFFECT] = COMPOUND_STRING("no effect"),
};

// The daily prize by score (a perfect round with full streaks is 190).
static const struct { u16 score; u16 item; } sPrizes[] =
{
    {160, ITEM_RARE_CANDY},
    {120, ITEM_PP_UP},
    {80,  ITEM_ULTRA_BALL},
    {40,  ITEM_GREAT_BALL},
};

struct QuizRound
{
    u8 question;
    u8 correct;
    u8 streak;
    u8 answer;
    u16 species;
    u8 moveType;
    u16 score;
};

// Not static: the screenshot driver reads the answer.
EWRAM_DATA struct QuizRound gQuizRound = {0};
#define sRound gQuizRound

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gQuizSpots[1] =
{
    {MAP_MAUVILLE_CITY, 21, 10, OBJ_EVENT_GFX_PSYCHIC_M, MOVEMENT_TYPE_FACE_DOWN, 0, Quiz_EventScript_Quizmaster},
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 Effectiveness(u32 moveType, u16 species)
{
    uq4_12_t mod = GetTypeModifier(moveType, gSpeciesInfo[species].types[0]);

    if (gSpeciesInfo[species].types[1] != gSpeciesInfo[species].types[0])
        mod = uq4_12_multiply(mod, GetTypeModifier(moveType, gSpeciesInfo[species].types[1]));
    if (mod == UQ_4_12(0))
        return QUIZ_NO_EFFECT;
    if (mod < UQ_4_12(1))
        return QUIZ_NOT_VERY;
    if (mod > UQ_4_12(1))
        return QUIZ_SUPER;
    return QUIZ_NORMAL;
}

static u32 RandomType(void)
{
    u32 type;

    do
        type = TYPE_NORMAL + Random() % (TYPE_FAIRY - TYPE_NORMAL + 1);
    while (type == TYPE_MYSTERY);
    return type;
}

void Quiz_Start(void)
{
    memset(&sRound, 0, sizeof(sRound));
}

// Script: VAR_RESULT = FALSE when the round is over; otherwise
// VAR_0x8005 = the defender (for showmonpic), gPastimeText = the question.
void Quiz_NextQuestion(void)
{
    u32 tries = 0;
    u8 *p = gPastimeText;

    if (sRound.question >= QUIZ_QUESTIONS)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    // Normal damage is the dull answer: make it come up less often.
    do
    {
        sRound.species = sDefenders[Random() % ARRAY_COUNT(sDefenders)];
        sRound.moveType = RandomType();
        sRound.answer = Effectiveness(sRound.moveType, sRound.species);
    } while (sRound.answer == QUIZ_NORMAL && ++tries < 3);
    sRound.question++;
    gSpecialVar_Result = TRUE;
    gSpecialVar_0x8005 = sRound.species;
    p = StringCopy(p, COMPOUND_STRING("Question "));
    p = ConvertIntToDecimalStringN(p, sRound.question, STR_CONV_MODE_LEFT_ALIGN, 2);
    p = StringCopy(p, COMPOUND_STRING(": a "));
    p = StringCopy(p, gTypesInfo[sRound.moveType].name);
    p = StringCopy(p, COMPOUND_STRING(" move\nhits "));
    p = StringCopy(p, GetSpeciesName(sRound.species));
    p = StringCopy(p, COMPOUND_STRING(" ("));
    p = StringCopy(p, gTypesInfo[gSpeciesInfo[sRound.species].types[0]].name);
    if (gSpeciesInfo[sRound.species].types[1] != gSpeciesInfo[sRound.species].types[0])
    {
        p = StringCopy(p, COMPOUND_STRING("/"));
        p = StringCopy(p, gTypesInfo[gSpeciesInfo[sRound.species].types[1]].name);
    }
    StringCopy(p, COMPOUND_STRING(")!"));
}

// Script: VAR_RESULT = the answer picked. Sets VAR_RESULT = TRUE if right,
// gPastimeText = the quizmaster's reply.
void Quiz_Answer(void)
{
    u8 *p = gPastimeText;

    if (gSpecialVar_Result == sRound.answer)
    {
        sRound.correct++;
        sRound.score += 10 + sRound.streak * 2;
        sRound.streak++;
        gSpecialVar_Result = TRUE;
        p = StringCopy(p, COMPOUND_STRING("Correct! "));
        if (sRound.streak >= 3)
        {
            p = ConvertIntToDecimalStringN(p, sRound.streak, STR_CONV_MODE_LEFT_ALIGN, 2);
            p = StringCopy(p, COMPOUND_STRING(" in a row! "));
        }
    }
    else
    {
        sRound.streak = 0;
        gSpecialVar_Result = FALSE;
        p = StringCopy(p, COMPOUND_STRING("Bzzt! "));
    }
    p = StringCopy(p, COMPOUND_STRING("It's "));
    p = StringCopy(p, sAnswerNames[sRound.answer]);
    p = StringCopy(p, COMPOUND_STRING(".\nScore: "));
    ConvertIntToDecimalStringN(p, sRound.score, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Script: the round is over. gPastimeText = the result; VAR_RESULT = TRUE
// with VAR_0x8005 = today's prize.
void Quiz_Finish(void)
{
    u32 i;
    u8 *p = gPastimeText;

    p = StringCopy(p, COMPOUND_STRING("That's the round! "));
    p = ConvertIntToDecimalStringN(p, sRound.correct, STR_CONV_MODE_LEFT_ALIGN, 2);
    p = StringCopy(p, COMPOUND_STRING(" of 10 right,\nfor "));
    p = ConvertIntToDecimalStringN(p, sRound.score, STR_CONV_MODE_LEFT_ALIGN, 3);
    p = StringCopy(p, COMPOUND_STRING(" points!"));
    if (sRound.score > sSave.quizBest)
    {
        sSave.quizBest = sRound.score;
        p = StringCopy(p, COMPOUND_STRING("\pA new personal best!"));
    }
    if (sSave.quizRounds < 0xFFFF)
        sSave.quizRounds++;
    if (sRound.correct == QUIZ_QUESTIONS && sSave.quizPerfect < 0xFFFF)
        sSave.quizPerfect++;
    World_SubmitScore(WORLD_BOARD_QUIZ, sRound.score, sRound.correct);

    gSpecialVar_Result = FALSE;
    if (sSave.quizPrizeDay == Today() + 1)
        return;
    for (i = 0; i < ARRAY_COUNT(sPrizes); i++)
    {
        if (sRound.score >= sPrizes[i].score)
        {
            sSave.quizPrizeDay = Today() + 1; // + 1, so 0 means never
            gSpecialVar_Result = TRUE;
            gSpecialVar_0x8005 = sPrizes[i].item;
            return;
        }
    }
}

// R > PASTIMES > QUIZ -----------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    static const u8 *const labels[] =
    {
        COMPOUND_STRING("Best score"),
        COMPOUND_STRING("Rounds played"),
        COMPOUND_STRING("Perfect rounds"),
        COMPOUND_STRING("Today's prize"),
    };
    const u16 values[] = {sSave.quizBest, sSave.quizRounds, sSave.quizPerfect};

    dest = StringCopy(dest, labels[index]);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 128}"));
    if (index < ARRAY_COUNT(values))
        ConvertIntToDecimalStringN(dest, values[index], STR_CONV_MODE_LEFT_ALIGN, 5);
    else
        StringCopy(dest, sSave.quizPrizeDay == Today() + 1 ? COMPOUND_STRING("Won") : COMPOUND_STRING("Up for grabs"));
}

void Quiz_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("BATTLE QUIZ HALL"), 4, GetLine, NULL);
}
