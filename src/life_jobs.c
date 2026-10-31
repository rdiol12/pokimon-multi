#include "global.h"
#include "event_data.h"
#include "item.h"
#include "life.h"
#include "money.h"
#include "battle_main.h"
#include "data.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "world.h"
#include "constants/items.h"
#include "constants/characters.h"

// Pokémon jobs (docs/life.md): the job agent in the Mauville Pokémon
// Center sends a party Pokémon out on a timed errand. It leaves the party
// (it's kept in struct LifeSaveData), and when the time is up the agent
// hands it back with EXP, a little friendship and what it earned. Pokémon
// of the job's types do a great job and bring back twice as much.

STATIC_ASSERT(sizeof(struct BoxPokemon) == sizeof(((struct LifeJob *)0)->mon), LifeJobBoxMonSize);

#define JOB_MON(job) ((struct BoxPokemon *)(job)->mon)

struct JobReward
{
    u16 item;
    u8 min;
    u8 max;
};

struct Job
{
    const u8 *name;
    u8 hours;
    u8 types[3];
    u16 money;
    const struct JobReward *rewards;
    u8 rewardCount;
};

static const struct JobReward sBerryFarmRewards[] =
{
    {ITEM_ORAN_BERRY, 2, 4}, {ITEM_PECHA_BERRY, 2, 4}, {ITEM_SITRUS_BERRY, 1, 2}, {ITEM_LUM_BERRY, 1, 1},
};
static const struct JobReward sDeliveryRewards[] =
{
    {ITEM_POKE_BALL, 2, 4}, {ITEM_GREAT_BALL, 1, 3}, {ITEM_POTION, 2, 3}, {ITEM_REPEAT_BALL, 1, 2},
};
static const struct JobReward sLifeguardRewards[] =
{
    {ITEM_PEARL, 1, 2}, {ITEM_HEART_SCALE, 1, 2}, {ITEM_BIG_PEARL, 1, 1}, {ITEM_DIVE_BALL, 1, 3},
};
static const struct JobReward sPowerPlantRewards[] =
{
    {ITEM_PP_UP, 1, 1}, {ITEM_ELIXIR, 1, 1}, {ITEM_THUNDER_STONE, 1, 1}, {ITEM_CELL_BATTERY, 1, 1},
};
static const struct JobReward sCaveSurveyRewards[] =
{
    {ITEM_HARD_STONE, 1, 1}, {ITEM_NUGGET, 1, 1}, {ITEM_STAR_PIECE, 1, 1}, {ITEM_EVERSTONE, 1, 1},
    {ITEM_FIRE_STONE, 1, 1}, {ITEM_WATER_STONE, 1, 1}, {ITEM_LEAF_STONE, 1, 1},
};
static const struct JobReward sNightWatchRewards[] =
{
    {ITEM_DUSK_BALL, 2, 3}, {ITEM_DUSK_STONE, 1, 1}, {ITEM_MOON_STONE, 1, 1}, {ITEM_RARE_CANDY, 1, 1},
    {ITEM_SPELL_TAG, 1, 1},
};

static const struct Job sJobs[LIFE_JOB_COUNT] =
{
    [LIFE_JOB_BERRY_FARM] = {COMPOUND_STRING("BERRY FARM HELP"), 1, {TYPE_GRASS, TYPE_BUG, TYPE_GROUND}, 300, sBerryFarmRewards, ARRAY_COUNT(sBerryFarmRewards)},
    [LIFE_JOB_DELIVERY] = {COMPOUND_STRING("PARCEL DELIVERY"), 2, {TYPE_FLYING, TYPE_NORMAL, TYPE_ELECTRIC}, 900, sDeliveryRewards, ARRAY_COUNT(sDeliveryRewards)},
    [LIFE_JOB_LIFEGUARD] = {COMPOUND_STRING("LIFEGUARD DUTY"), 2, {TYPE_WATER, TYPE_ICE, TYPE_WATER}, 700, sLifeguardRewards, ARRAY_COUNT(sLifeguardRewards)},
    [LIFE_JOB_POWER_PLANT] = {COMPOUND_STRING("POWER PLANT SHIFT"), 4, {TYPE_ELECTRIC, TYPE_STEEL, TYPE_FIRE}, 1600, sPowerPlantRewards, ARRAY_COUNT(sPowerPlantRewards)},
    [LIFE_JOB_CAVE_SURVEY] = {COMPOUND_STRING("CAVE SURVEY"), 4, {TYPE_ROCK, TYPE_GROUND, TYPE_FIGHTING}, 1200, sCaveSurveyRewards, ARRAY_COUNT(sCaveSurveyRewards)},
    [LIFE_JOB_NIGHT_WATCH] = {COMPOUND_STRING("NIGHT WATCH"), 8, {TYPE_DARK, TYPE_GHOST, TYPE_PSYCHIC}, 2500, sNightWatchRewards, ARRAY_COUNT(sNightWatchRewards)},
};

static u32 MinutesNow(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days * 24 * 60 + gLocalTime.hours * 60 + gLocalTime.minutes;
}

static u32 MinutesLeft(const struct LifeJob *job)
{
    u32 now = MinutesNow();
    u32 length = sJobs[job->job].hours * 60;
    s32 done = now - job->startMinute; // negative if the clock was turned back

    if (done < 0)
        done = 0;
    return (u32)done >= length ? 0 : length - done;
}

static bool32 IsGoodAtJob(struct BoxPokemon *mon, u32 jobId)
{
    enum Species species = GetBoxMonData(mon, MON_DATA_SPECIES);
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sJobs[jobId].types); i++)
    {
        if (GetSpeciesType(species, 0) == sJobs[jobId].types[i] || GetSpeciesType(species, 1) == sJobs[jobId].types[i])
            return TRUE;
    }
    return FALSE;
}

static void FormatTimeLeft(u8 *dest, u32 minutes)
{
    u8 *end = ConvertIntToDecimalStringN(dest, minutes / 60, STR_CONV_MODE_LEFT_ALIGN, 2);
    *end++ = CHAR_COLON;
    ConvertIntToDecimalStringN(end, minutes % 60, STR_CONV_MODE_LEADING_ZEROS, 2);
}

// VAR_0x8005 = job id: puts its name in STR_VAR_1, its length in
// STR_VAR_2 and its best types in STR_VAR_3.
void LifeJobs_Describe(void)
{
    const struct Job *job = &sJobs[gSpecialVar_0x8005 % LIFE_JOB_COUNT];
    u8 *end;

    StringCopy(gStringVar1, job->name);
    ConvertIntToDecimalStringN(gStringVar2, job->hours, STR_CONV_MODE_LEFT_ALIGN, 1);
    end = StringCopy(gStringVar3, gTypesInfo[job->types[0]].name);
    if (job->types[1] != job->types[0])
    {
        end = StringAppend(gStringVar3, COMPOUND_STRING(", "));
        end = StringAppend(gStringVar3, gTypesInfo[job->types[1]].name);
    }
    if (job->types[2] != job->types[0])
    {
        end = StringAppend(gStringVar3, COMPOUND_STRING(" and "));
        StringAppend(gStringVar3, gTypesInfo[job->types[2]].name);
    }
}

// VAR_RESULT: whether a job slot is free.
void LifeJobs_HasFreeSlot(void)
{
    u32 i;
    gSpecialVar_Result = FALSE;
    for (i = 0; i < LIFE_JOB_SLOTS; i++)
    {
        if (!gSaveBlock3Ptr->life.jobs[i].active)
            gSpecialVar_Result = TRUE;
    }
}

// Sends party slot VAR_0x8004 on job VAR_0x8005. VAR_RESULT: LIFE_JOB_SENT,
// LIFE_JOB_LAST_MON or LIFE_JOB_EGG; STR_VAR_1 is its name, STR_VAR_2 the
// job and STR_VAR_3 its hours.
void LifeJobs_Send(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    struct LifeJob *job = NULL;
    u32 i, others = 0;

    if (GetMonData(mon, MON_DATA_IS_EGG))
    {
        gSpecialVar_Result = LIFE_JOB_EGG;
        return;
    }
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *other = &gParties[B_TRAINER_PLAYER][i];
        if (i != gSpecialVar_0x8004 && GetMonData(other, MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(other, MON_DATA_IS_EGG) && GetMonData(other, MON_DATA_HP) != 0)
            others++;
    }
    if (others == 0)
    {
        gSpecialVar_Result = LIFE_JOB_LAST_MON;
        return;
    }
    for (i = 0; i < LIFE_JOB_SLOTS && job == NULL; i++)
    {
        if (!gSaveBlock3Ptr->life.jobs[i].active)
            job = &gSaveBlock3Ptr->life.jobs[i];
    }
    if (job == NULL)
        return;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    memcpy(job->mon, &mon->box, sizeof(job->mon));
    job->job = gSpecialVar_0x8005 % LIFE_JOB_COUNT;
    job->startMinute = MinutesNow();
    job->active = TRUE;
    StringCopy(gStringVar2, sJobs[job->job].name);
    ConvertIntToDecimalStringN(gStringVar3, sJobs[job->job].hours, STR_CONV_MODE_LEFT_ALIGN, 1);
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    gSpecialVar_Result = LIFE_JOB_SENT;
}

static void PayOut(struct LifeJob *job, bool32 great)
{
    const struct Job *info = &sJobs[job->job];
    const struct JobReward *reward = &info->rewards[Random() % info->rewardCount];
    u32 count = reward->min + Random() % (reward->max - reward->min + 1);
    u32 money = info->money;
    u8 *end;

    if (great)
    {
        count *= 2;
        money *= 2;
    }
    AddBagItem(reward->item, count);
    AddMoney(&gSaveBlock1Ptr->money, money);
    end = ConvertIntToDecimalStringN(gStringVar3, count, STR_CONV_MODE_LEFT_ALIGN, 2);
    *end++ = CHAR_SPACE;
    end = CopyItemNameHandlePlural(reward->item, end, count);
    end = StringCopy(end, COMPOUND_STRING(" and ¥"));
    ConvertIntToDecimalStringN(end, money, STR_CONV_MODE_LEFT_ALIGN, 5);
}

// Hands back the first finished job. VAR_RESULT: 0 none finished, 1 back
// in the party, 2 sent to the PC (the party was full). STR_VAR_1: name,
// STR_VAR_2: job, STR_VAR_3: what it brought. VAR_0x8006: a great job;
// VAR_0x8007: the new level if it grew.
void LifeJobs_Collect(void)
{
    u32 i;

    gSpecialVar_Result = 0;
    for (i = 0; i < LIFE_JOB_SLOTS; i++)
    {
        struct LifeJob *job = &gSaveBlock3Ptr->life.jobs[i];
        struct Pokemon mon;
        u32 exp, level, oldLevel, friendship;
        bool32 great;

        if (!job->active || MinutesLeft(job) != 0)
            continue;

        BoxMonToMon(JOB_MON(job), &mon);
        great = IsGoodAtJob(JOB_MON(job), job->job);
        oldLevel = GetMonData(&mon, MON_DATA_LEVEL);
        if (oldLevel < MAX_LEVEL)
        {
            // About a level for a 4-hour job at level 20.
            exp = GetMonData(&mon, MON_DATA_EXP) + sJobs[job->job].hours * oldLevel * (great ? 30 : 20) + 100;
            level = gExperienceTables[gSpeciesInfo[GetMonData(&mon, MON_DATA_SPECIES)].growthRate][MAX_LEVEL];
            if (exp > level)
                exp = level;
            SetMonData(&mon, MON_DATA_EXP, &exp);
            CalculateMonStats(&mon);
        }
        friendship = GetMonData(&mon, MON_DATA_FRIENDSHIP) + 3;
        if (friendship > MAX_FRIENDSHIP)
            friendship = MAX_FRIENDSHIP;
        SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);
        level = GetMonData(&mon, MON_DATA_LEVEL);
        gSpecialVar_0x8006 = great;
        gSpecialVar_0x8007 = level > oldLevel ? level : 0;

        GetMonData(&mon, MON_DATA_NICKNAME, gStringVar1);
        StringGet_Nickname(gStringVar1);
        StringCopy(gStringVar2, sJobs[job->job].name);
        PayOut(job, great);
        if (CalculatePlayerPartyCount() < PARTY_SIZE)
        {
            gParties[B_TRAINER_PLAYER][gPartiesCount[B_TRAINER_PLAYER]] = mon;
            CalculatePlayerPartyCount();
            gSpecialVar_Result = 1;
        }
        else
        {
            CopyMonToPC(&mon);
            gSpecialVar_Result = 2;
        }
        job->active = FALSE;
        return;
    }
}

static void JobLine(u16 index, u8 *dest)
{
    struct LifeJob *job = &gSaveBlock3Ptr->life.jobs[index];
    u32 left;

    if (!job->active)
    {
        StringCopy(dest, COMPOUND_STRING("- free -"));
        return;
    }
    GetBoxMonData(JOB_MON(job), MON_DATA_NICKNAME, dest);
    StringGet_Nickname(dest);
    dest = StringAppend(dest, COMPOUND_STRING(": "));
    dest = StringAppend(dest, sJobs[job->job].name);
    left = MinutesLeft(job);
    if (left == 0)
    {
        StringAppend(dest, COMPOUND_STRING(" (done!)"));
    }
    else
    {
        dest = StringAppend(dest, COMPOUND_STRING(" ("));
        FormatTimeLeft(dest + StringLength(dest), left);
        StringAppend(dest, COMPOUND_STRING(")"));
    }
}

static void JobDetail(u16 index, u8 *dest)
{
    struct LifeJob *job = &gSaveBlock3Ptr->life.jobs[index];

    if (!job->active)
        StringCopy(dest, COMPOUND_STRING("Send a Pokémon at the MAUVILLE {PKMN} CENTER."));
    else if (MinutesLeft(job) == 0)
        StringCopy(dest, COMPOUND_STRING("Finished! Pick it up in MAUVILLE."));
    else if (IsGoodAtJob(JOB_MON(job), job->job))
        StringCopy(dest, COMPOUND_STRING("A natural at this job! Double pay."));
    else
        StringCopy(dest, COMPOUND_STRING("Working hard. Hours:minutes left."));
}

// R > JOBS and the agent's SEE JOBS: the job slots on a board.
void LifeJobs_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("POKéMON JOBS"), LIFE_JOB_SLOTS, JobLine, JobDetail);
}

// For the screenshot driver: every job's time is up.
void LifeJobs_FinishAll(void)
{
    u32 i;
    for (i = 0; i < LIFE_JOB_SLOTS; i++)
        gSaveBlock3Ptr->life.jobs[i].startMinute -= 24 * 60;
}
