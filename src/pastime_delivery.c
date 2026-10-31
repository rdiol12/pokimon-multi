#include "global.h"
#include "event_data.h"
#include "money.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "random.h"
#include "string_util.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Delivery jobs: the PARCEL DESK outside eight Pokémon Centers hands out a
// parcel for another town you've been to, with a time limit that grows with
// the distance (play time, so it doesn't run while the game is off). Deliver
// it in time for full pay, in half the time for an express bonus, late for
// half pay. Every fifth delivery also earns an item.

extern const u8 Delivery_EventScript_Desk[];
extern const u8 Delivery_EventScript_Vigoroth[];

struct DeliveryTown
{
    const u8 *name;
    u16 visitedFlag;
    u8 x, y; // rough place on the region map, for distances
};

static const struct DeliveryTown sTowns[DELIVERY_TOWN_COUNT] =
{
    {COMPOUND_STRING("OLDALE TOWN"),    FLAG_VISITED_OLDALE_TOWN,    6, 11},
    {COMPOUND_STRING("PETALBURG CITY"), FLAG_VISITED_PETALBURG_CITY, 3, 11},
    {COMPOUND_STRING("RUSTBORO CITY"),  FLAG_VISITED_RUSTBORO_CITY,  1, 5},
    {COMPOUND_STRING("SLATEPORT CITY"), FLAG_VISITED_SLATEPORT_CITY, 8, 14},
    {COMPOUND_STRING("MAUVILLE CITY"),  FLAG_VISITED_MAUVILLE_CITY,  8, 8},
    {COMPOUND_STRING("FALLARBOR TOWN"), FLAG_VISITED_FALLARBOR_TOWN, 4, 2},
    {COMPOUND_STRING("FORTREE CITY"),   FLAG_VISITED_FORTREE_CITY,   13, 3},
    {COMPOUND_STRING("LILYCOVE CITY"),  FLAG_VISITED_LILYCOVE_CITY,  20, 5},
};

static const u8 *const sParcels[] =
{
    COMPOUND_STRING("a box of POKéBLOCKS"),
    COMPOUND_STRING("a crate of BERRIES"),
    COMPOUND_STRING("spare BIKE parts"),
    COMPOUND_STRING("a birthday cake"),
    COMPOUND_STRING("a stack of comics"),
    COMPOUND_STRING("a potted plant"),
    COMPOUND_STRING("a big bag of PokéFOOD"),
    COMPOUND_STRING("a rolled-up rug"),
};

// Every fifth delivery, in turn.
static const u16 sBonusItems[] = {ITEM_GREAT_BALL, ITEM_HYPER_POTION, ITEM_RARE_CANDY, ITEM_PP_UP, ITEM_NUGGET, ITEM_ULTRA_BALL, ITEM_BIG_NUGGET};

#define SECONDS_BASE      90
#define SECONDS_PER_STEP  20
#define PAY_BASE          300
#define PAY_PER_STEP      100

#define sSave (gSaveBlock3Ptr->pastime)

// The desk next to each Pokémon Center (arg = town), and a VIGOROTH mover
// at the Slateport head office.
const struct PastimeSpot gDeliverySpots[DELIVERY_TOWN_COUNT + 1] =
{
    {MAP_OLDALE_TOWN,    8, 17,  OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 0, Delivery_EventScript_Desk},
    {MAP_PETALBURG_CITY, 18, 17, OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 1, Delivery_EventScript_Desk},
    {MAP_RUSTBORO_CITY,  18, 39, OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 2, Delivery_EventScript_Desk},
    {MAP_SLATEPORT_CITY, 17, 20, OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 3, Delivery_EventScript_Desk},
    {MAP_MAUVILLE_CITY,  24, 6,  OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 4, Delivery_EventScript_Desk},
    {MAP_FALLARBOR_TOWN, 16, 8,  OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 5, Delivery_EventScript_Desk},
    {MAP_FORTREE_CITY,   7, 8,   OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 6, Delivery_EventScript_Desk},
    {MAP_LILYCOVE_CITY,  26, 15, OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 7, Delivery_EventScript_Desk},
    {MAP_SLATEPORT_CITY, 16, 20, OBJ_EVENT_GFX_VIGOROTH_CARRYING_BOX, MOVEMENT_TYPE_FACE_DOWN, 0, Delivery_EventScript_Vigoroth},
};

static u32 PlaySeconds(void)
{
    return gSaveBlock2Ptr->playTimeHours * 3600 + gSaveBlock2Ptr->playTimeMinutes * 60 + gSaveBlock2Ptr->playTimeSeconds;
}

static u32 Distance(u32 a, u32 b)
{
    s32 dx = sTowns[a].x - sTowns[b].x, dy = sTowns[a].y - sTowns[b].y;
    return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

static bool32 IsCarrying(void)
{
    return sSave.deliveryTo < DELIVERY_TOWN_COUNT;
}

static u32 SecondsLeft(void)
{
    u32 elapsed = PlaySeconds() - sSave.deliveryStart;
    return elapsed < sSave.deliveryLimit ? sSave.deliveryLimit - elapsed : 0;
}

static u8 *AppendTime(u8 *dest, u32 seconds)
{
    dest = ConvertIntToDecimalStringN(dest, seconds / 60, STR_CONV_MODE_LEFT_ALIGN, 3);
    *dest++ = CHAR_COLON;
    return ConvertIntToDecimalStringN(dest, seconds % 60, STR_CONV_MODE_LEADING_ZEROS, 2);
}

// Called a few times a second: tells the player once when time runs out.
void Delivery_Update(void)
{
    if (sSave.deliveryFrom == 0 && sSave.deliveryTo == 0 && sSave.deliveryLimit == 0)
    {
        // A save from before delivery jobs.
        sSave.deliveryFrom = sSave.deliveryTo = DELIVERY_NONE;
    }
    if (IsCarrying() && !sSave.deliveryLate && SecondsLeft() == 0)
    {
        sSave.deliveryLate = TRUE;
        World_ShowToast(COMPOUND_STRING("PARCEL LATE!"), COMPOUND_STRING("Deliver it anyway for half pay."));
    }
}

// Script: talking to a desk (VAR_0x8004 = town). VAR_RESULT = DELIVERY_*,
// gPastimeText = what the clerk says, VAR_0x8005 = the offered town.
void Delivery_Talk(void)
{
    u32 here = gSpecialVar_0x8004, i, count = 0, pick;
    u8 *p = gPastimeText;
    u8 towns[DELIVERY_TOWN_COUNT];

    if (IsCarrying())
    {
        if (sSave.deliveryTo == here)
        {
            gSpecialVar_Result = DELIVERY_DELIVERED;
            return;
        }
        gSpecialVar_Result = DELIVERY_ELSEWHERE;
        p = StringCopy(p, COMPOUND_STRING("You're carrying "));
        p = StringCopy(p, sParcels[sSave.deliveryParcel]);
        p = StringCopy(p, COMPOUND_STRING("\nfor "));
        p = StringCopy(p, sTowns[sSave.deliveryTo].name);
        if (SecondsLeft() != 0)
        {
            p = StringCopy(p, COMPOUND_STRING(".\pTime left: "));
            p = AppendTime(p, SecondsLeft());
            StringCopy(p, COMPOUND_STRING(".\nWant to give up the job?"));
        }
        else
        {
            StringCopy(p, COMPOUND_STRING(".\pIt's late, but they'll still pay\nhalf. Want to give up the job?"));
        }
        return;
    }
    for (i = 0; i < DELIVERY_TOWN_COUNT; i++)
    {
        if (i != here && FlagGet(sTowns[i].visitedFlag))
            towns[count++] = i;
    }
    if (count == 0)
    {
        gSpecialVar_Result = DELIVERY_NO_TOWNS;
        return;
    }
    pick = towns[Random() % count];
    gSpecialVar_Result = DELIVERY_OFFER;
    gSpecialVar_0x8005 = pick;
    gSpecialVar_0x8006 = Random() % ARRAY_COUNT(sParcels);
    p = StringCopy(p, COMPOUND_STRING("I've got "));
    p = StringCopy(p, sParcels[gSpecialVar_0x8006]);
    p = StringCopy(p, COMPOUND_STRING("\nfor the desk in "));
    p = StringCopy(p, sTowns[pick].name);
    p = StringCopy(p, COMPOUND_STRING(".\pTime limit: "));
    p = AppendTime(p, SECONDS_BASE + SECONDS_PER_STEP * Distance(here, pick));
    p = StringCopy(p, COMPOUND_STRING(". It pays ¥"));
    p = ConvertIntToDecimalStringN(p, PAY_BASE + PAY_PER_STEP * Distance(here, pick), STR_CONV_MODE_LEFT_ALIGN, 5);
    StringCopy(p, COMPOUND_STRING(".\nWill you take it?"));
}

// Script: the player took the offered parcel.
void Delivery_Accept(void)
{
    sSave.deliveryFrom = gSpecialVar_0x8004;
    sSave.deliveryTo = gSpecialVar_0x8005;
    sSave.deliveryParcel = gSpecialVar_0x8006;
    sSave.deliveryLate = FALSE;
    sSave.deliveryStart = PlaySeconds();
    sSave.deliveryLimit = SECONDS_BASE + SECONDS_PER_STEP * Distance(sSave.deliveryFrom, sSave.deliveryTo);
    StringCopy(gStringVar1, sTowns[sSave.deliveryTo].name);
}

// Script: handing the parcel over. Pays, gPastimeText = the clerk's words,
// VAR_RESULT = TRUE with VAR_0x8005 = an item every fifth delivery.
void Delivery_Deliver(void)
{
    u32 pay = PAY_BASE + PAY_PER_STEP * Distance(sSave.deliveryFrom, sSave.deliveryTo);
    u32 elapsed = PlaySeconds() - sSave.deliveryStart;
    u8 *p = gPastimeText;

    p = StringCopy(p, COMPOUND_STRING("Delivered in "));
    p = AppendTime(p, elapsed);
    if (elapsed * 2 <= sSave.deliveryLimit)
    {
        pay += pay / 2;
        p = StringCopy(p, COMPOUND_STRING("!\nEXPRESS! That's a bonus for you!"));
    }
    else if (elapsed <= sSave.deliveryLimit)
    {
        p = StringCopy(p, COMPOUND_STRING(".\nRight on time. Thank you!"));
    }
    else
    {
        pay /= 2;
        p = StringCopy(p, COMPOUND_STRING("…\nLate, so half pay. Sorry!"));
    }
    p = StringCopy(p, COMPOUND_STRING("\p{PLAYER} got ¥"));
    p = ConvertIntToDecimalStringN(p, pay, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringCopy(p, COMPOUND_STRING("!"));
    AddMoney(&gSaveBlock1Ptr->money, pay);
    if (sSave.deliveries < 0xFFFF)
        sSave.deliveries++;
    sSave.deliveryFrom = sSave.deliveryTo = DELIVERY_NONE;
    gSpecialVar_Result = sSave.deliveries % 5 == 0;
    gSpecialVar_0x8005 = sBonusItems[(sSave.deliveries / 5 - 1) % ARRAY_COUNT(sBonusItems)];
}

void Delivery_Cancel(void)
{
    sSave.deliveryFrom = sSave.deliveryTo = DELIVERY_NONE;
}

// R > PASTIMES > DELIVERY -------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index == 0)
    {
        if (!IsCarrying())
        {
            StringCopy(dest, COMPOUND_STRING("No parcel. Visit a PARCEL DESK!"));
            return;
        }
        dest = StringCopy(dest, COMPOUND_STRING("To "));
        dest = StringCopy(dest, sTowns[sSave.deliveryTo].name);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 128}"));
        if (SecondsLeft() == 0)
            StringCopy(dest, COMPOUND_STRING("{COLOR RED}{SHADOW LIGHT_RED}LATE"));
        else
            AppendTime(StringCopy(dest, COMPOUND_STRING("Left ")), SecondsLeft());
    }
    else if (index == 1 && IsCarrying())
    {
        dest = StringCopy(dest, COMPOUND_STRING("Carrying "));
        StringCopy(dest, sParcels[sSave.deliveryParcel]);
    }
    else
    {
        dest = StringCopy(dest, COMPOUND_STRING("Parcels delivered{CLEAR_TO 128}"));
        ConvertIntToDecimalStringN(dest, sSave.deliveries, STR_CONV_MODE_LEFT_ALIGN, 5);
    }
}

void Delivery_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("DELIVERY JOBS"), IsCarrying() ? 3 : 2, GetLine, NULL);
}
