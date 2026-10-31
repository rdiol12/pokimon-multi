#include "global.h"
#include "battle_main.h"
#include "event_data.h"
#include "malloc.h"
#include "move.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "rtc.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "task.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/map_groups.h"
#include "constants/moves.h"

// Battle dojo: the DOJO MASTER outside Dewford Gym runs three training
// sessions a day. Each is five timing meter drills; each good drill earns
// 2 Dojo Points. The points buy moves the Pokémon Center relearner can't
// teach (to any Pokémon of the move's type, or that has it on its tutor
// list), and the points earned in total
// raise the player's belt, which unlocks the stronger moves.

extern const u8 Dojo_EventScript_Master[];

static const u8 *const sBeltNames[DOJO_BELT_COUNT] =
{
    COMPOUND_STRING("WHITE"), COMPOUND_STRING("YELLOW"), COMPOUND_STRING("GREEN"),
    COMPOUND_STRING("BLUE"), COMPOUND_STRING("BLACK"),
};

// Dojo Points earned in total for each belt.
static const u16 sBeltPoints[DOJO_BELT_COUNT] = {0, 20, 60, 120, 200};

static const struct { u16 move; u8 cost; u8 belt; } sMoves[DOJO_MOVE_COUNT] =
{
    {MOVE_FIRE_PUNCH,    6, 0},
    {MOVE_ICE_PUNCH,     6, 0},
    {MOVE_THUNDER_PUNCH, 6, 0},
    {MOVE_SEED_BOMB,     6, 0},
    {MOVE_ZEN_HEADBUTT,  8, 1},
    {MOVE_AQUA_TAIL,     8, 1},
    {MOVE_KNOCK_OFF,     10, 1},
    {MOVE_IRON_HEAD,     10, 2},
    {MOVE_DRAIN_PUNCH,   10, 2},
    {MOVE_HYPER_VOICE,   10, 2},
    {MOVE_EARTH_POWER,   12, 3},
    {MOVE_HEAT_WAVE,     12, 3},
    {MOVE_GUNK_SHOT,     12, 3},
    {MOVE_STEALTH_ROCK,  14, 4},
    {MOVE_SUPERPOWER,    16, 4},
    {MOVE_BODY_PRESS,    16, 4},
};

#define sSave (gSaveBlock3Ptr->pastime)

// The move the master is teaching right now (MOVE_NONE when he isn't).
static EWRAM_DATA u16 sTeaching = MOVE_NONE;

const struct PastimeSpot gDojoSpots[1] =
{
    {MAP_DEWFORD_TOWN, 13, 16, OBJ_EVENT_GFX_BLACK_BELT, MOVEMENT_TYPE_FACE_DOWN, 0, Dojo_EventScript_Master},
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 GetBelt(void)
{
    u32 belt = DOJO_BELT_COUNT - 1;

    while (sSave.dojoEarned < sBeltPoints[belt])
        belt--;
    return belt;
}

// CanLearnTeachableMove (pokemon.c): the master teaches his moves to any
// Pokémon of the move's type, not just the ones on its tutor list.
bool32 Pastime_CanLearnTeachableMove(u16 species, u16 move)
{
    u32 type;

    if (sTeaching == MOVE_NONE || move != sTeaching)
        return FALSE;
    type = GetMoveType(move);
    return gSpeciesInfo[species].types[0] == type || gSpeciesInfo[species].types[1] == type;
}

// Script: gStringVar1 = the belt, gStringVar2 = the points,
// VAR_RESULT = sessions left today.
void Dojo_GetStatus(void)
{
    sTeaching = MOVE_NONE;
    if (sSave.dojoDay != Today() + 1) // + 1, so 0 means never
    {
        sSave.dojoDay = Today() + 1;
        sSave.dojoSessions = 0;
    }
    StringCopy(gStringVar1, sBeltNames[GetBelt()]);
    ConvertIntToDecimalStringN(gStringVar2, sSave.dojoPoints, STR_CONV_MODE_LEFT_ALIGN, 5);
    gSpecialVar_Result = DOJO_SESSIONS - sSave.dojoSessions;
}

static void Task_Train(u8 taskId)
{
    s32 hits = TimingMeter_GetResult();
    u32 oldBelt;

    if (hits < 0)
        return;
    oldBelt = GetBelt();
    sSave.dojoSessions++;
    sSave.dojoPoints = min(sSave.dojoPoints + hits * 2, 9999);
    sSave.dojoEarned = min(sSave.dojoEarned + hits * 2, 0xFFFF);
    gSpecialVar_0x8006 = hits;
    gSpecialVar_Result = GetBelt() != oldBelt;
    ConvertIntToDecimalStringN(gStringVar1, hits * 2, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, sSave.dojoPoints, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringCopy(gStringVar3, sBeltNames[GetBelt()]);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Script: `callnative Dojo_Train, waitstate=1`. VAR_0x8006 = good drills,
// VAR_RESULT = TRUE if the belt went up. gStringVar1 = points won,
// gStringVar2 = points now, gStringVar3 = the belt.
void Dojo_Train(void)
{
    CreateTask(Task_Train, 80);
    TimingMeter_Start(DOJO_DRILLS, 3, 2 + min(GetBelt(), 2));
}

static void PushMenuItem(const u8 *name, u32 id)
{
    struct ListMenuItem item;
    u8 *buffer = Alloc(StringLength(name) + 1);

    StringCopy(buffer, name);
    item.name = buffer;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

static u8 *AppendMove(u8 *dest, u32 i)
{
    if (GetBelt() < sMoves[i].belt || sSave.dojoPoints < sMoves[i].cost)
        dest = StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
    dest = StringCopy(dest, GetMoveName(sMoves[i].move));
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 84}"));
    dest = ConvertIntToDecimalStringN(dest, sMoves[i].cost, STR_CONV_MODE_RIGHT_ALIGN, 2);
    return StringCopy(dest, COMPOUND_STRING(" DP"));
}

// Script: pushes the moves onto the dynmultichoice stack.
void Dojo_PushMoveMenu(void)
{
    u8 text[64];
    u32 i;

    for (i = 0; i < DOJO_MOVE_COUNT; i++)
    {
        AppendMove(text, i);
        PushMenuItem(text, i);
    }
    PushMenuItem(COMPOUND_STRING("CANCEL"), DOJO_MENU_CANCEL);
}

// Script: VAR_0x8007 = the move picked. VAR_RESULT = DOJO_OK, DOJO_NO_BELT
// or DOJO_NO_POINTS. VAR_0x8005 = the move, for MoveTutor_EventScript_OpenBox.
void Dojo_CheckMove(void)
{
    u32 i = gSpecialVar_0x8007;

    gSpecialVar_0x8005 = sMoves[i].move;
    StringCopy(gStringVar1, GetMoveName(sMoves[i].move));
    StringCopy(gStringVar2, sBeltNames[sMoves[i].belt]);
    ConvertIntToDecimalStringN(gStringVar3, sMoves[i].cost, STR_CONV_MODE_LEFT_ALIGN, 2);
    if (GetBelt() < sMoves[i].belt)
        gSpecialVar_Result = DOJO_NO_BELT;
    else if (sSave.dojoPoints < sMoves[i].cost)
        gSpecialVar_Result = DOJO_NO_POINTS;
    else
        gSpecialVar_Result = DOJO_OK;
    if (gSpecialVar_Result == DOJO_OK)
    {
        sTeaching = sMoves[i].move;
        StringCopy(gStringVar2, gTypesInfo[GetMoveType(sTeaching)].name);
    }
}

// Script: the move (VAR_0x8007) was taught.
void Dojo_PayForMove(void)
{
    sTeaching = MOVE_NONE;
    sSave.dojoPoints -= sMoves[gSpecialVar_0x8007].cost;
    if (sSave.dojoTaught < 0xFFFF)
        sSave.dojoTaught++;
}

// R > PASTIMES > DOJO -----------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    static const u8 *const labels[] =
    {
        COMPOUND_STRING("Belt"),
        COMPOUND_STRING("Dojo Points"),
        COMPOUND_STRING("Points earned"),
        COMPOUND_STRING("Sessions left today"),
        COMPOUND_STRING("Moves learned"),
    };
    u32 value;

    if (index >= ARRAY_COUNT(labels))
    {
        AppendMove(dest, index - ARRAY_COUNT(labels));
        return;
    }
    dest = StringCopy(dest, labels[index]);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 128}"));
    switch (index)
    {
    case 0:
        StringCopy(dest, sBeltNames[GetBelt()]);
        return;
    case 1:
        value = sSave.dojoPoints;
        break;
    case 2:
        value = sSave.dojoEarned;
        break;
    case 3:
        value = sSave.dojoDay == Today() + 1 ? DOJO_SESSIONS - sSave.dojoSessions : DOJO_SESSIONS;
        break;
    default:
        value = sSave.dojoTaught;
        break;
    }
    ConvertIntToDecimalStringN(dest, value, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void GetDetail(u16 index, u8 *dest)
{
    u32 belt = GetBelt();

    if (index == 0 && belt + 1 < DOJO_BELT_COUNT)
    {
        dest = StringCopy(dest, sBeltNames[belt + 1]);
        dest = StringCopy(dest, COMPOUND_STRING(" belt at "));
        dest = ConvertIntToDecimalStringN(dest, sBeltPoints[belt + 1], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringCopy(dest, COMPOUND_STRING(" points earned"));
    }
    else if (index >= 5)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Needs the "));
        dest = StringCopy(dest, sBeltNames[sMoves[index - 5].belt]);
        StringCopy(dest, COMPOUND_STRING(" belt"));
    }
    else
    {
        StringCopy(dest, COMPOUND_STRING("The master is outside Dewford Gym"));
    }
}

void Dojo_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("BATTLE DOJO"), 5 + DOJO_MOVE_COUNT, GetLine, GetDetail);
}
