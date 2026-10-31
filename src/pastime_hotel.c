#include "global.h"
#include "event_data.h"
#include "item.h"
#include "money.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "rtc.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Pokémon hotel: you're the new manager of the STARFALL HOTEL. Each day
// guests check into the lobby (two, plus one per star), each wanting
// something: a drink, a Berry, a Potion... Bring it from your bag and they
// tip. At the desk, collect the day's room money and tips once a day, and
// spend money to raise the hotel's stars: more guests, higher room rates,
// and at five stars a famous guest.

extern const u8 Hotel_EventScript_Bellhop[];
extern const u8 Hotel_EventScript_Desk[];
extern const u8 Hotel_EventScript_Guest[];

struct HotelGuest
{
    u16 graphicsId;
    const u8 *name;
};

static const struct HotelGuest sGuests[] =
{
    {OBJ_EVENT_GFX_GENTLEMAN,  COMPOUND_STRING("Mr. Bellamy")},
    {OBJ_EVENT_GFX_BEAUTY,     COMPOUND_STRING("Ms. Rosa")},
    {OBJ_EVENT_GFX_HIKER,      COMPOUND_STRING("Hiker Bo")},
    {OBJ_EVENT_GFX_OLD_WOMAN,  COMPOUND_STRING("Granny Fay")},
    {OBJ_EVENT_GFX_TUBER_F,    COMPOUND_STRING("Little Mia")},
    {OBJ_EVENT_GFX_SAILOR,     COMPOUND_STRING("Sailor Jim")},
    {OBJ_EVENT_GFX_PICNICKER,  COMPOUND_STRING("Camper Liz")},
    {OBJ_EVENT_GFX_EXPERT_M,   COMPOUND_STRING("Master Ito")},
    {OBJ_EVENT_GFX_RICH_BOY,   COMPOUND_STRING("Young Rex")},
    {OBJ_EVENT_GFX_POKEFAN_F,  COMPOUND_STRING("Mrs. Plum")},
    {OBJ_EVENT_GFX_ARTIST,     COMPOUND_STRING("Painter Leo")},
    {OBJ_EVENT_GFX_SCIENTIST_1, COMPOUND_STRING("Dr. Kemp")},
};

// The five-star famous guest, in place of the last regular one.
static const struct HotelGuest sStarGuest = {OBJ_EVENT_GFX_STEVEN, COMPOUND_STRING("STEVEN")};

static const u16 sRequests[] =
{
    ITEM_FRESH_WATER, ITEM_SODA_POP, ITEM_LEMONADE, ITEM_MOOMOO_MILK, ITEM_LAVA_COOKIE,
    ITEM_ORAN_BERRY, ITEM_PECHA_BERRY, ITEM_CHERI_BERRY, ITEM_POTION, ITEM_ANTIDOTE,
    ITEM_POKE_BALL, ITEM_SUPER_POTION, ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_REPEL,
};

static const u16 sRoomRates[HOTEL_MAX_STARS + 1] = {0, 150, 250, 400, 600, 900};
static const u32 sUpgradeCosts[HOTEL_MAX_STARS + 1] = {0, 3000, 8000, 20000, 45000, 0};

#define sSave (gSaveBlock3Ptr->pastime)

// The bellhop outside, the front desk, then the guests in the lobby.
const struct PastimeSpot gHotelSpots[HOTEL_SPOT_COUNT] =
{
    {MAP_STARFALL_ISLE, 35, 37, OBJ_EVENT_GFX_LINK_RECEPTIONIST, MOVEMENT_TYPE_FACE_DOWN, 0, Hotel_EventScript_Bellhop},
    {MAP_STARFALL_ISLE_HOTEL, 4, 4, OBJ_EVENT_GFX_LINK_RECEPTIONIST, MOVEMENT_TYPE_FACE_DOWN, 0, Hotel_EventScript_Desk},
    {MAP_STARFALL_ISLE_HOTEL, 1, 4, 0, MOVEMENT_TYPE_FACE_RIGHT, 0, Hotel_EventScript_Guest, Hotel_IsGuestShown, Hotel_GetGuestGraphicsId},
    {MAP_STARFALL_ISLE_HOTEL, 7, 4, 0, MOVEMENT_TYPE_FACE_LEFT,  1, Hotel_EventScript_Guest, Hotel_IsGuestShown, Hotel_GetGuestGraphicsId},
    {MAP_STARFALL_ISLE_HOTEL, 1, 6, 0, MOVEMENT_TYPE_FACE_RIGHT, 2, Hotel_EventScript_Guest, Hotel_IsGuestShown, Hotel_GetGuestGraphicsId},
    {MAP_STARFALL_ISLE_HOTEL, 7, 6, 0, MOVEMENT_TYPE_FACE_LEFT,  3, Hotel_EventScript_Guest, Hotel_IsGuestShown, Hotel_GetGuestGraphicsId},
    {MAP_STARFALL_ISLE_HOTEL, 2, 8, 0, MOVEMENT_TYPE_FACE_UP,    4, Hotel_EventScript_Guest, Hotel_IsGuestShown, Hotel_GetGuestGraphicsId},
    {MAP_STARFALL_ISLE_HOTEL, 6, 8, 0, MOVEMENT_TYPE_FACE_UP,    5, Hotel_EventScript_Guest, Hotel_IsGuestShown, Hotel_GetGuestGraphicsId},
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

// A new day: new guests.
static void CheckDay(void)
{
    if (sSave.hotelStars == 0)
        sSave.hotelStars = 1;
    if (sSave.hotelDay != Today())
    {
        sSave.hotelDay = Today();
        sSave.hotelServed = 0;
        sSave.hotelCollected = FALSE;
    }
}

static u32 GuestCount(void)
{
    return min(2 + sSave.hotelStars, HOTEL_MAX_GUESTS);
}

static bool32 IsStarGuest(u32 i)
{
    return sSave.hotelStars >= HOTEL_MAX_STARS && i == GuestCount() - 1;
}

// Today's guests and requests come from the day, so they stay put.
static const struct HotelGuest *GetGuest(u32 i)
{
    if (IsStarGuest(i))
        return &sStarGuest;
    return &sGuests[(sSave.hotelDay * 5 + i * 7) % ARRAY_COUNT(sGuests)];
}

static u16 GetRequest(u32 i)
{
    if (IsStarGuest(i))
        return ITEM_LAVA_COOKIE;
    return sRequests[(sSave.hotelDay * 3 + i * 11) % ARRAY_COUNT(sRequests)];
}

static u32 CountServed(void)
{
    u32 i, count = 0;

    for (i = 0; i < GuestCount(); i++)
        count += (sSave.hotelServed >> i) & 1;
    return count;
}

// Room money: every guest pays the rate; served guests pay double.
static u32 RoomMoney(void)
{
    return sRoomRates[sSave.hotelStars] * (GuestCount() + CountServed());
}

bool32 Hotel_IsGuestShown(u32 arg)
{
    CheckDay();
    return arg < GuestCount() && !sSave.hotelCollected;
}

u16 Hotel_GetGuestGraphicsId(u32 arg)
{
    return GetGuest(arg)->graphicsId;
}

// Script: talking to a guest (VAR_0x8004). VAR_RESULT = HOTEL_GUEST_*,
// gPastimeText = what they say, VAR_0x8005 = the item they want.
void Hotel_TalkToGuest(void)
{
    u32 i = gSpecialVar_0x8004;
    const struct HotelGuest *guest = GetGuest(i);
    u16 item = GetRequest(i);
    u8 *p = gPastimeText;

    CheckDay();
    gSpecialVar_0x8005 = item;
    CopyItemName(item, gStringVar1);
    if ((sSave.hotelServed >> i) & 1)
    {
        gSpecialVar_Result = HOTEL_GUEST_SERVED;
        p = StringCopy(p, guest->name);
        StringCopy(p, IsStarGuest(i) ? COMPOUND_STRING(": A fine hotel. I'll tell\nthe DEVON board about it.")
                                     : COMPOUND_STRING(": What lovely service!\nI'll be back next year."));
        return;
    }
    p = StringCopy(p, guest->name);
    if (IsStarGuest(i))
        p = StringCopy(p, COMPOUND_STRING(": Ah, the manager. I hear\nthe LAVA COOKIES here are rare…\pCould I trouble you for one?"));
    else
        p = StringCopy(p, COMPOUND_STRING(": Excuse me, manager!\nCould I get a {STR_VAR_1}, please?"));
    StringExpandPlaceholders(gStringVar4, gPastimeText);
    StringCopy(gPastimeText, gStringVar4);
    gSpecialVar_Result = CheckBagHasItem(item, 1) ? HOTEL_GUEST_ASKS : HOTEL_GUEST_CANT;
}

// Script: the player handed the item over. STR_VAR_2 = the tip.
void Hotel_ServeGuest(void)
{
    u32 i = gSpecialVar_0x8004;
    u32 tip = GetItemPrice(GetRequest(i)) / 2 + 100 * sSave.hotelStars;

    if (IsStarGuest(i))
    {
        tip += 5000;
        AddBagItem(ITEM_METAL_COAT, 1);
    }
    RemoveBagItem(GetRequest(i), 1);
    sSave.hotelServed |= 1 << i;
    sSave.hotelTips += tip;
    if (sSave.hotelGuests < 0xFFFF)
        sSave.hotelGuests++;
    ConvertIntToDecimalStringN(gStringVar2, tip, STR_CONV_MODE_LEFT_ALIGN, 5);
    gSpecialVar_Result = IsStarGuest(i);
}

// Script: the front desk. VAR_RESULT = HOTEL_DESK_*. STR_VAR_1 = served
// guests, STR_VAR_2 = guests today, STR_VAR_3 = stars.
void Hotel_TalkToDesk(void)
{
    CheckDay();
    ConvertIntToDecimalStringN(gStringVar1, CountServed(), STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertIntToDecimalStringN(gStringVar2, GuestCount(), STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertIntToDecimalStringN(gStringVar3, sSave.hotelStars, STR_CONV_MODE_LEFT_ALIGN, 1);
    if (sSave.hotelCollected)
        gSpecialVar_Result = HOTEL_DESK_DONE;
    else if (CountServed() == 0)
        gSpecialVar_Result = HOTEL_DESK_WAITING;
    else
        gSpecialVar_Result = HOTEL_DESK_COLLECT;
}

// Script: collecting today's money. The guests check out. gPastimeText =
// the day's takings.
void Hotel_Collect(void)
{
    u32 rooms = RoomMoney(), total = rooms + sSave.hotelTips;
    u8 *p = gPastimeText;

    p = StringCopy(p, COMPOUND_STRING("Rooms: ¥"));
    p = ConvertIntToDecimalStringN(p, rooms, STR_CONV_MODE_LEFT_ALIGN, 6);
    p = StringCopy(p, COMPOUND_STRING("   Tips: ¥"));
    p = ConvertIntToDecimalStringN(p, sSave.hotelTips, STR_CONV_MODE_LEFT_ALIGN, 6);
    p = StringCopy(p, COMPOUND_STRING("\n{PLAYER} collected ¥"));
    p = ConvertIntToDecimalStringN(p, total, STR_CONV_MODE_LEFT_ALIGN, 6);
    StringCopy(p, COMPOUND_STRING("!"));
    AddMoney(&gSaveBlock1Ptr->money, total);
    sSave.hotelEarnings += total;
    sSave.hotelTips = 0;
    sSave.hotelCollected = TRUE;
}

// Script: VAR_RESULT = FALSE at five stars; otherwise STR_VAR_1 = the cost
// of the next star and VAR_RESULT = TRUE if the player can pay it.
void Hotel_GetUpgrade(void)
{
    u32 cost = sUpgradeCosts[sSave.hotelStars];

    if (sSave.hotelStars >= HOTEL_MAX_STARS)
    {
        gSpecialVar_Result = 2;
        return;
    }
    ConvertIntToDecimalStringN(gStringVar1, cost, STR_CONV_MODE_LEFT_ALIGN, 6);
    ConvertIntToDecimalStringN(gStringVar2, sSave.hotelStars + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    gSpecialVar_Result = IsEnoughMoney(&gSaveBlock1Ptr->money, cost);
}

void Hotel_Upgrade(void)
{
    RemoveMoney(&gSaveBlock1Ptr->money, sUpgradeCosts[sSave.hotelStars]);
    sSave.hotelStars++;
}

// R > PASTIMES > HOTEL ----------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    u32 i;

    if (index == 0)
    {
        dest = StringCopy(dest, COMPOUND_STRING("STARFALL HOTEL{CLEAR_TO 112}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.hotelStars, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringCopy(dest, sSave.hotelStars == 1 ? COMPOUND_STRING(" star") : COMPOUND_STRING(" stars"));
        return;
    }
    if (index == 1)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Guests served{CLEAR_TO 112}"));
        ConvertIntToDecimalStringN(dest, sSave.hotelGuests, STR_CONV_MODE_LEFT_ALIGN, 5);
        return;
    }
    if (index == 2)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Earned{CLEAR_TO 112}¥"));
        ConvertIntToDecimalStringN(dest, sSave.hotelEarnings, STR_CONV_MODE_LEFT_ALIGN, 7);
        return;
    }
    i = index - 3;
    if (sSave.hotelCollected)
    {
        StringCopy(dest, COMPOUND_STRING("Today's guests have checked out."));
        return;
    }
    if ((sSave.hotelServed >> i) & 1)
        dest = StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
    dest = StringCopy(dest, GetGuest(i)->name);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 96}"));
    CopyItemName(GetRequest(i), dest);
}

void Hotel_ShowList(void)
{
    CheckDay();
    WorldBoard_Show(COMPOUND_STRING("POKéMON HOTEL"), 3 + (sSave.hotelCollected ? 1 : GuestCount()), GetLine, NULL);
}
