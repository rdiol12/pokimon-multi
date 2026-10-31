#include "global.h"
#include "battle.h"
#include "battle_message.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/characters.h"
#include "mechanics.h"
#include "constants/battle_string_ids.h"

// Battle commentary: in PvP (link) battles and tournaments, an announcer
// calls the action. It opens the battle, then after each turn picks the
// biggest thing that happened (a Pokémon down to its side's last, a KO,
// Dynamax or Terastal, a critical hit, a super effective hit) and says a
// line about it. The line goes through STRINGID_MECH_MESSAGE with the
// Pokémon in gBattleTextBuff1, so both games of a link battle print it.

enum
{
    EVENT_NONE,
    EVENT_SUPER_EFFECTIVE,
    EVENT_CRITICAL_HIT,
    EVENT_TERASTAL,
    EVENT_DYNAMAX,
    EVENT_KO,
    EVENT_LAST_MON,
};

enum
{
    LINE_OPENING_PVP,
    LINE_OPENING_CUP,
    LINE_SUPER_EFFECTIVE,
    LINE_SUPER_EFFECTIVE_2,
    LINE_CRITICAL_HIT,
    LINE_CRITICAL_HIT_2,
    LINE_TERASTAL,
    LINE_DYNAMAX,
    LINE_KO,
    LINE_KO_2,
    LINE_LAST_MON,
};

static const u8 *const sLines[] =
{
    [LINE_OPENING_PVP]       = COMPOUND_STRING("ANNOUNCER: Welcome, everyone, to a\nlive POKéMON battle! Here we go!"),
    [LINE_OPENING_CUP]       = COMPOUND_STRING("ANNOUNCER: The tournament is on,\nand the crowd is packed in today!"),
    [LINE_SUPER_EFFECTIVE]   = COMPOUND_STRING("ANNOUNCER: {B_BUFF1}\nfound the weak spot! Ouch!"),
    [LINE_SUPER_EFFECTIVE_2] = COMPOUND_STRING("ANNOUNCER: Super effective!\n{B_BUFF1} knows its types!"),
    [LINE_CRITICAL_HIT]      = COMPOUND_STRING("ANNOUNCER: A critical hit from\n{B_BUFF1}! Right on target!"),
    [LINE_CRITICAL_HIT_2]    = COMPOUND_STRING("ANNOUNCER: What a critical hit!\n{B_BUFF1} is on fire!"),
    [LINE_TERASTAL]          = COMPOUND_STRING("ANNOUNCER: {B_BUFF1}\nshines like a jewel! Amazing!"),
    [LINE_DYNAMAX]           = COMPOUND_STRING("ANNOUNCER: {B_BUFF1}\nis HUGE! The stadium is shaking!"),
    [LINE_KO]                = COMPOUND_STRING("ANNOUNCER: {B_BUFF1}\nis down! What a battle this is!"),
    [LINE_KO_2]              = COMPOUND_STRING("ANNOUNCER: The crowd gasps!\n{B_BUFF1} can't go on!"),
    [LINE_LAST_MON]          = COMPOUND_STRING("ANNOUNCER: It all comes down to\n{B_BUFF1}! Last one left!"),
};

static EWRAM_DATA u8 sEvent = EVENT_NONE;
static EWRAM_DATA u8 sBattler = 0;
static EWRAM_DATA u8 sPartyIndex = 0;
static EWRAM_DATA u8 sLastMonCalled = 0; // a bit per side

static bool32 IsCommentaryOn(void)
{
    return (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK)) || Mechanics_IsCupActive();
}

static void Say(u32 line, u32 battler, u32 partyIndex)
{
    PREPARE_MON_NICK_WITH_PREFIX_BUFFER(gBattleTextBuff1, battler, partyIndex);
    gBattleTextBuff3[0] = MECH_MSG_COMMENTARY + line;
    gBattleTextBuff3[1] = EOS;
}

// PrepareStringBattle: notes what happened this turn.
void Mechanics_OnBattleString(u16 stringId, u32 battler)
{
    u32 event, subject = gBattlerAttacker;

    if (!IsCommentaryOn())
        return;
    switch (stringId)
    {
    case STRINGID_SUPEREFFECTIVE:
    case STRINGID_SUPEREFFECTIVEONDEF:
        event = EVENT_SUPER_EFFECTIVE;
        break;
    case STRINGID_CRITICALHIT:
    case STRINGID_CRITICALHITONDEF:
        event = EVENT_CRITICAL_HIT;
        break;
    case STRINGID_PKMNTERASTALLIZEDINTO:
        event = EVENT_TERASTAL;
        break;
    case STRINGID_TIMETODYNAMAX:
        event = EVENT_DYNAMAX;
        break;
    case STRINGID_BATTLERFAINTED:
        event = EVENT_KO;
        subject = gBattlerFainted;
        break;
    default:
        return;
    }
    if (event < sEvent)
        return;
    sEvent = event;
    sBattler = subject;
    sPartyIndex = gBattlerPartyIndexes[subject];
}

// Before the first turn. TRUE if the announcer opens the battle.
bool32 Mechanics_TryOpeningCommentary(void)
{
    sEvent = EVENT_NONE;
    sLastMonCalled = 0;
    if (!IsCommentaryOn())
        return FALSE;
    Say(Mechanics_IsCupActive() ? LINE_OPENING_CUP : LINE_OPENING_PVP, 0, gBattlerPartyIndexes[0]);
    return TRUE;
}

// A side whose only Pokémon left able to fight is out on the field.
static bool32 IsDownToLastMon(u32 battler)
{
    struct Pokemon *party = GetBattlerParty(battler);
    u32 i, alive = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&party[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&party[i], MON_DATA_IS_EGG)
         && GetMonData(&party[i], MON_DATA_HP) != 0)
            alive++;
    }
    return alive == 1 && IsBattlerAlive(battler);
}

// At the end of each turn. TRUE if the announcer has something to say.
bool32 Mechanics_TryTurnCommentary(void)
{
    u32 event = sEvent, battler;
    u32 variant = gBattleResults.battleTurnCounter & 1;

    sEvent = EVENT_NONE;
    if (!IsCommentaryOn())
        return FALSE;
    for (battler = 0; battler < gBattlersCount && !(gBattleTypeFlags & BATTLE_TYPE_DOUBLE); battler++)
    {
        u32 side = GetBattlerSide(battler);

        if (!(sLastMonCalled & (1 << side)) && IsDownToLastMon(battler))
        {
            sLastMonCalled |= 1 << side;
            Say(LINE_LAST_MON, battler, gBattlerPartyIndexes[battler]);
            return TRUE;
        }
    }
    switch (event)
    {
    case EVENT_SUPER_EFFECTIVE:
        Say(LINE_SUPER_EFFECTIVE + variant, sBattler, sPartyIndex);
        return TRUE;
    case EVENT_CRITICAL_HIT:
        Say(LINE_CRITICAL_HIT + variant, sBattler, sPartyIndex);
        return TRUE;
    case EVENT_TERASTAL:
        Say(LINE_TERASTAL, sBattler, sPartyIndex);
        return TRUE;
    case EVENT_DYNAMAX:
        Say(LINE_DYNAMAX, sBattler, sPartyIndex);
        return TRUE;
    case EVENT_KO:
        Say(LINE_KO + variant, sBattler, sPartyIndex);
        return TRUE;
    }
    return FALSE;
}

const u8 *Mechanics_GetCommentary(u8 line)
{
    if (line < ARRAY_COUNT(sLines))
        return sLines[line];
    return COMPOUND_STRING("");
}
