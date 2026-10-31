#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "list_menu.h"
#include "malloc.h"
#include "mechanics.h"
#include "pokemon.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "constants/flags.h"

// Trainer skills: the teacher in the Rustboro Pokémon School teaches the
// player a new skill for every second badge. One skill is on at a time;
// the teacher switches it.

struct Skill
{
    const u8 *name;
    u16 badgeFlag; // learned with this badge
    const u8 *help;
};

static const struct Skill sSkills[SKILL_COUNT + 1] =
{
    [SKILL_FIELD_MEDIC] =
    {
        COMPOUND_STRING("FIELD MEDIC"), FLAG_BADGE01_GET,
        COMPOUND_STRING("FIELD MEDIC: in each battle, the\n"
                        "first item you use on a POKéMON\l"
                        "isn't used up.\p"
                        "That's an extra item every battle!"),
    },
    [SKILL_BALL_EXPERT] =
    {
        COMPOUND_STRING("BALL EXPERT"), FLAG_BADGE03_GET,
        COMPOUND_STRING("BALL EXPERT: your throws are so good,\n"
                        "every POKé BALL catches a quarter\l"
                        "more easily."),
    },
    [SKILL_BIG_EARNER] =
    {
        COMPOUND_STRING("BIG EARNER"), FLAG_BADGE05_GET,
        COMPOUND_STRING("BIG EARNER: trainers you beat pay a\n"
                        "quarter more prize money."),
    },
    [SKILL_SECOND_WIND] =
    {
        COMPOUND_STRING("SECOND WIND"), FLAG_BADGE07_GET,
        COMPOUND_STRING("SECOND WIND: after every trainer\n"
                        "battle you win, your POKéMON get\l"
                        "back a fifth of their HP."),
    },
};

static EWRAM_DATA bool8 sItemKept = FALSE; // FIELD MEDIC, this battle

static bool32 IsSkillLearned(u8 skill)
{
    return FlagGet(sSkills[skill].badgeFlag);
}

bool32 Mechanics_HasSkill(u8 skill)
{
    return gSaveBlock3Ptr->mechanics.skill == skill && IsSkillLearned(skill)
        && !(gMain.inBattle && (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK)));
}

// Each overworld frame: a new battle gets a new free item.
void Mechanics_UpdateSkills(void)
{
    sItemKept = FALSE;
}

// party_menu.c, using an item on a Pokémon in battle: TRUE keeps the item in the Bag.
bool32 Mechanics_TryKeepBattleItem(u16 item)
{
    if (!gMain.inBattle || sItemKept || !Mechanics_HasSkill(SKILL_FIELD_MEDIC))
        return FALSE;
    sItemKept = TRUE;
    return TRUE;
}

u32 Mechanics_BoostCatchOdds(u32 odds)
{
    if (Mechanics_HasSkill(SKILL_BALL_EXPERT))
        return odds * 5 / 4;
    return odds;
}

u32 Mechanics_BoostPrizeMoney(u32 money)
{
    if (Mechanics_HasSkill(SKILL_BIG_EARNER))
        return money * 5 / 4;
    return money;
}

// After a trainer battle (World_OnTrainerBattleEnd).
void Mechanics_OnTrainerBattleEnd(void)
{
    u32 i;

    if (gBattleOutcome != B_OUTCOME_WON || !Mechanics_HasSkill(SKILL_SECOND_WIND))
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 hp = GetMonData(mon, MON_DATA_HP);
        u32 maxHP = GetMonData(mon, MON_DATA_MAX_HP);

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG) || hp == 0)
            continue;
        hp = min(maxHP, hp + max(1, maxHP / 5));
        SetMonData(mon, MON_DATA_HP, &hp);
    }
}

// The teacher ----------------------------------------------------------------

// Script: VAR_RESULT = how many skills are learned; gStringVar1 = the one on.
void Mechanics_CountSkills(void)
{
    u32 i;

    gSpecialVar_Result = 0;
    for (i = 1; i <= SKILL_COUNT; i++)
        gSpecialVar_Result += IsSkillLearned(i);
    if (gSaveBlock3Ptr->mechanics.skill != SKILL_NONE && IsSkillLearned(gSaveBlock3Ptr->mechanics.skill))
        StringCopy(gStringVar1, sSkills[gSaveBlock3Ptr->mechanics.skill].name);
    else
        StringCopy(gStringVar1, COMPOUND_STRING("none"));
}

// Script: one dynmulti row per skill learned, the one on marked.
void Mechanics_PushSkillRows(void)
{
    u32 i;

    for (i = 1; i <= SKILL_COUNT; i++)
    {
        struct ListMenuItem item = {.name = Alloc(100), .id = i};
        u8 *end;

        if (!IsSkillLearned(i))
            continue;
        end = StringCopy((u8 *)item.name, sSkills[i].name);
        if (gSaveBlock3Ptr->mechanics.skill == i)
            StringCopy(end, COMPOUND_STRING(" {COLOR GREEN}ON"));
        MultichoiceDynamic_PushElement(item);
    }
}

// Script: VAR_0x8005 = SKILL_*. gStringVar4 = what it does,
// gStringVar1 = its name.
void Mechanics_GetSkillHelp(void)
{
    StringCopy(gStringVar4, sSkills[gSpecialVar_0x8005].help);
    StringCopy(gStringVar1, sSkills[gSpecialVar_0x8005].name);
}

// Script: VAR_0x8005 = SKILL_*.
void Mechanics_SetSkill(void)
{
    gSaveBlock3Ptr->mechanics.skill = gSpecialVar_0x8005;
}
