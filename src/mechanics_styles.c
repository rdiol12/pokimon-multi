#include "global.h"
#include "battle.h"
#include "battle_scripts.h"
#include "battle_setup.h"
#include "fpmath.h"
#include "event_data.h"
#include "mechanics.h"
#include "random.h"
#include "constants/characters.h"
#include "constants/battle_string_ids.h"

// Battle styles: once the style master in the Fallarbor Battle Tent has
// taught them, SELECT in the move menu switches the move between normal,
// AGILE and STRONG style. A style costs 1 extra PP.
//   AGILE:  0.75x power, goes first among moves of the same priority.
//   STRONG: 1.33x power, goes last among moves of the same priority.
// The choice goes to the battle engine with the chosen move (see
// STYLE_RET_MASK), so it would survive a link battle, but styles are off in
// link battles so online PvP stays fair to everyone.

static EWRAM_DATA u8 sChoice[MAX_BATTLERS_COUNT] = {0}; // in the move menu
static EWRAM_DATA u8 sStyle[MAX_BATTLERS_COUNT] = {0};  // this turn's
static EWRAM_DATA bool8 sPractice = FALSE; // the master's practice battle

static const u8 *const sStyleNames[] =
{
    [STYLE_NONE]   = COMPOUND_STRING("TYPE"),
    [STYLE_AGILE]  = COMPOUND_STRING("AGILE"),
    [STYLE_STRONG] = COMPOUND_STRING("STRONG"),
};

bool32 Mechanics_CanUseBattleStyles(void)
{
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_RECORDED))
        return FALSE;
    return sPractice || FlagGet(FLAG_MECH_BATTLE_STYLES);
}

const u8 *Mechanics_GetStyleName(u8 style)
{
    return sStyleNames[style < ARRAY_COUNT(sStyleNames) ? style : STYLE_NONE];
}

// Player controller --------------------------------------------------------

void Mechanics_ResetStyleChoice(u32 battler)
{
    sChoice[battler] = STYLE_NONE;
}

// SELECT in the move menu. Returns TRUE if the choice changed.
bool32 Mechanics_CycleStyleChoice(u32 battler, u32 pp)
{
    if (!Mechanics_CanUseBattleStyles())
        return FALSE;
    if (pp < 2 && sChoice[battler] == STYLE_NONE)
        return FALSE;
    sChoice[battler] = (sChoice[battler] + 1) % 3;
    if (pp < 2)
        sChoice[battler] = STYLE_NONE;
    return TRUE;
}

u8 Mechanics_GetStyleChoice(u32 battler)
{
    return sChoice[battler];
}

u32 Mechanics_GetStyleReturnBits(u32 battler)
{
    if (!Mechanics_CanUseBattleStyles())
        return 0;
    return sChoice[battler] << STYLE_RET_SHIFT;
}

// Battle engine -------------------------------------------------------------

// A trainer who knows styles (the master) picks one now and then.
static u8 ChooseOpponentStyle(u32 battler)
{
    if (!sPractice || GetBattlerSide(battler) != B_SIDE_OPPONENT)
        return STYLE_NONE;
    if (gBattleMons[battler].pp[gBattleStruct->chosenMovePositions[battler]] < 2)
        return STYLE_NONE;
    return Random() % 3;
}

// When a battler's move is chosen: `returnValue` is the controller's byte
// with the move index.
void Mechanics_SetBattlerStyle(u32 battler, u8 returnValue)
{
    u8 style = (returnValue & STYLE_RET_MASK) >> STYLE_RET_SHIFT;

    if (style > STYLE_STRONG || !Mechanics_CanUseBattleStyles())
        style = STYLE_NONE;
    if (GetBattlerSide(battler) == B_SIDE_OPPONENT)
        style = ChooseOpponentStyle(battler);
    else if (gBattleMons[battler].pp[gBattleStruct->chosenMovePositions[battler]] < 2)
        style = STYLE_NONE;
    sStyle[battler] = style;
}

void Mechanics_ClearBattlerStyle(u32 battler)
{
    sStyle[battler] = STYLE_NONE;
}

// Base power modifier, in UQ4.12.
u32 Mechanics_GetStylePowerModifier(u32 battler)
{
    switch (sStyle[battler])
    {
    case STYLE_AGILE:
        return UQ_4_12(0.75);
    case STYLE_STRONG:
        return UQ_4_12(1.33);
    default:
        return UQ_4_12(1.0);
    }
}

// Within a priority bracket: 1 goes before 0, 0 before -1.
s32 Mechanics_GetStyleOrder(u32 battler)
{
    if (sStyle[battler] == STYLE_AGILE)
        return 1;
    if (sStyle[battler] == STYLE_STRONG)
        return -1;
    return 0;
}

u32 Mechanics_GetStyleExtraPP(u32 battler)
{
    return sStyle[battler] != STYLE_NONE;
}

// At the start of the turn (like Quick Claw): "X readies an AGILE move!"
bool32 Mechanics_TryAnnounceBattleStyle(u32 battler)
{
    if (sStyle[battler] == STYLE_NONE)
        return FALSE;
    gBattleTextBuff3[0] = sStyle[battler] == STYLE_AGILE ? MECH_MSG_STYLE_AGILE : MECH_MSG_STYLE_STRONG;
    gBattleTextBuff3[1] = EOS;
    BattleScriptExecute(BattleScript_MechanicsMessage);
    return TRUE;
}

static const u8 *const sBattleMessages[] =
{
    [MECH_MSG_STYLE_AGILE]  = COMPOUND_STRING("{B_ATK_NAME_WITH_PREFIX} moves in AGILE style!\nIt's quick, but lighter!"),
    [MECH_MSG_STYLE_STRONG] = COMPOUND_STRING("{B_ATK_NAME_WITH_PREFIX} moves in STRONG style!\nIt's slow, but it hits hard!"),
};

// STRINGID_MECH_MESSAGE: the text for gBattleTextBuff3[0].
const u8 *Mechanics_GetBattleMessage(u8 id)
{
    if (id < ARRAY_COUNT(sBattleMessages))
        return sBattleMessages[id];
    return COMPOUND_STRING("");
}

// The style master ----------------------------------------------------------

// Script: a practice battle where both sides may use styles. Follow with
// Mechanics_StartFacilityBattle; Mechanics_EndStylePractice afterwards.
void Mechanics_SetupStylePractice(void)
{
    u8 level = Mechanics_GetPartyBattleLevel();
    u16 trainerId = Mechanics_PickFacilityTrainer(level);

    TRAINER_BATTLE_PARAM.opponentA = trainerId;
    Mechanics_FillFacilityParty(trainerId, 2, level);
    sPractice = TRUE;
}

void Mechanics_EndStylePractice(void)
{
    sPractice = FALSE;
}
