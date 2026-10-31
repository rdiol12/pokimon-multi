#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Lost and found: people around Hoenn lost something on a nearby route.
// The lost things lie in plain sight (as item balls); bring one back to its
// owner for a reward. R > PASTIMES > LOST & FOUND lists the owners met and
// the things found.

extern const u8 LostFound_EventScript_Owner[];
extern const u8 LostFound_EventScript_Item[];

struct LostThing
{
    const u8 *name;      // what was lost
    const u8 *owner;     // who lost it
    const u8 *town;      // where the owner waits
    const u8 *where;     // where it was lost, for the list
    const u8 *plea;      // the owner, before it's found
    const u8 *thanks;    // the owner, getting it back
    const u8 *after;     // the owner, afterwards
    u16 reward;
};

static const struct LostThing sThings[LOST_FOUND_COUNT] =
{
    {
        .name = COMPOUND_STRING("HAIR RIBBON"),
        .owner = COMPOUND_STRING("LUCY"),
        .town = COMPOUND_STRING("PETALBURG CITY"),
        .where = COMPOUND_STRING("ROUTE 104, near the flower beds."),
        .plea = COMPOUND_STRING("Waaah… My HAIR RIBBON blew away\n"
                                "on ROUTE 104!\p"
                                "It was red with white spots…\n"
                                "Grandma made it for me!"),
        .thanks = COMPOUND_STRING("My HAIR RIBBON! You found it!\p"
                                  "I'll tie it on extra tight now!\n"
                                  "Here, you can have my bell!"),
        .after = COMPOUND_STRING("Does my ribbon look nice?\n"
                                 "Hehe, I'll never lose it again!"),
        .reward = ITEM_SOOTHE_BELL,
    },
    {
        .name = COMPOUND_STRING("NOTEBOOK"),
        .owner = COMPOUND_STRING("EVAN"),
        .town = COMPOUND_STRING("RUSTBORO CITY"),
        .where = COMPOUND_STRING("PETALBURG WOODS, by the tall grass."),
        .plea = COMPOUND_STRING("Oh no, oh no… My NOTEBOOK!\p"
                                "I was studying bug Pokémon in\n"
                                "PETALBURG WOODS and dropped it.\p"
                                "The TRAINER'S SCHOOL exam is\n"
                                "tomorrow!"),
        .thanks = COMPOUND_STRING("My NOTEBOOK! All my notes!\p"
                                  "You've saved my grades. Please,\n"
                                  "take this. It helps you learn!"),
        .after = COMPOUND_STRING("I passed the exam! Did you know\n"
                                 "WURMPLE evolves in two ways?"),
        .reward = ITEM_LUCKY_EGG,
    },
    {
        .name = COMPOUND_STRING("OLD COMPASS"),
        .owner = COMPOUND_STRING("SAILOR HANK"),
        .town = COMPOUND_STRING("SLATEPORT CITY"),
        .where = COMPOUND_STRING("ROUTE 109, on the beach."),
        .plea = COMPOUND_STRING("Ahoy… I've lost my OLD COMPASS\n"
                                "somewhere on the ROUTE 109 beach.\p"
                                "Fifty years at sea, that compass\n"
                                "and me. Can't sail without it."),
        .thanks = COMPOUND_STRING("Well, I'll be! My OLD COMPASS!\p"
                                  "Take this, matey. The sea gave it\n"
                                  "to me, and now I give it to you."),
        .after = COMPOUND_STRING("The needle points true again.\n"
                                 "Fair winds to you, matey!"),
        .reward = ITEM_MYSTIC_WATER,
    },
    {
        .name = COMPOUND_STRING("TOY CAR"),
        .owner = COMPOUND_STRING("TIMMY"),
        .town = COMPOUND_STRING("MAUVILLE CITY"),
        .where = COMPOUND_STRING("ROUTE 110, south of the bridge."),
        .plea = COMPOUND_STRING("I raced my TOY CAR with a\n"
                                "MINUN on ROUTE 110…\p"
                                "It zoomed off into the grass\n"
                                "and I can't find it!"),
        .thanks = COMPOUND_STRING("Vroom vroom! My TOY CAR!\p"
                                  "It runs on this, you know.\n"
                                  "But you can keep it. Thanks!"),
        .after = COMPOUND_STRING("Vroooom! Nobody's faster than\n"
                                 "my TOY CAR!"),
        .reward = ITEM_MAGNET,
    },
    {
        .name = COMPOUND_STRING("READING GLASSES"),
        .owner = COMPOUND_STRING("GRAMPS OTTO"),
        .town = COMPOUND_STRING("FALLARBOR TOWN"),
        .where = COMPOUND_STRING("ROUTE 113, in the falling ash."),
        .plea = COMPOUND_STRING("Eh? Who's there? I can't see a\n"
                                "thing without my READING GLASSES.\p"
                                "I took them off to wipe the ash\n"
                                "away on ROUTE 113, and… gone."),
        .thanks = COMPOUND_STRING("My READING GLASSES! Oh, now I can\n"
                                  "see what a fine young TRAINER\l"
                                  "you are!\p"
                                  "Here, a spare pair. Wear them\n"
                                  "and think clearly!"),
        .after = COMPOUND_STRING("Now I can read the paper again.\n"
                                 "Mostly I read about you!"),
        .reward = ITEM_WISE_GLASSES,
    },
    {
        .name = COMPOUND_STRING("BIRD WHISTLE"),
        .owner = COMPOUND_STRING("WREN"),
        .town = COMPOUND_STRING("FORTREE CITY"),
        .where = COMPOUND_STRING("ROUTE 119, by the river."),
        .plea = COMPOUND_STRING("I call my SWABLU with a BIRD\n"
                                "WHISTLE, but I dropped it on\l"
                                "ROUTE 119, down by the river.\p"
                                "Now my SWABLU won't come home…"),
        .thanks = COMPOUND_STRING("My BIRD WHISTLE!\n"
                                  "Fweeet! There, SWABLU heard it!\p"
                                  "Thank you! SWABLU wants you to\n"
                                  "have this."),
        .after = COMPOUND_STRING("Fweeet! SWABLU always comes now.\n"
                                 "Thank you again!"),
        .reward = ITEM_SHARP_BEAK,
    },
    {
        .name = COMPOUND_STRING("WEDDING RING"),
        .owner = COMPOUND_STRING("MRS. DIANE"),
        .town = COMPOUND_STRING("LILYCOVE CITY"),
        .where = COMPOUND_STRING("ROUTE 121, by the SAFARI ZONE."),
        .plea = COMPOUND_STRING("Oh dear, oh dear…\p"
                                "My WEDDING RING slipped off on\n"
                                "ROUTE 121, near the SAFARI ZONE.\p"
                                "My husband will be so sad…"),
        .thanks = COMPOUND_STRING("My WEDDING RING!\n"
                                  "I can't thank you enough.\p"
                                  "Please, take this. It's worth\n"
                                  "far less than what you found."),
        .after = COMPOUND_STRING("My husband never even noticed.\n"
                                 "Let's keep it our secret!"),
        .reward = ITEM_BIG_NUGGET,
    },
    {
        .name = COMPOUND_STRING("SHELL CHARM"),
        .owner = COMPOUND_STRING("KAI"),
        .town = COMPOUND_STRING("DEWFORD TOWN"),
        .where = COMPOUND_STRING("ROUTE 104, north of the woods."),
        .plea = COMPOUND_STRING("I made a SHELL CHARM for good\n"
                                "luck before the GYM… and lost\l"
                                "it on ROUTE 104, north of the\l"
                                "woods. Some luck, huh?"),
        .thanks = COMPOUND_STRING("My SHELL CHARM! Maybe it is\n"
                                  "lucky after all!\p"
                                  "I'll make you one too. Here!"),
        .after = COMPOUND_STRING("With my SHELL CHARM back, I beat\n"
                                 "BRAWLY! Well, almost."),
        .reward = ITEM_SHELL_BELL,
    },
};

#define BIT(i) (1 << (i))
#define sSave (gSaveBlock3Ptr->pastime)

// Owners first, then the lost things (arg = index in sThings).
const struct PastimeSpot gLostFoundSpots[LOST_FOUND_COUNT * 2] =
{
    {MAP_PETALBURG_CITY, 22, 10, OBJ_EVENT_GFX_LITTLE_GIRL,  MOVEMENT_TYPE_FACE_DOWN, 0, LostFound_EventScript_Owner, LostFound_IsOwnerShown},
    {MAP_RUSTBORO_CITY,  21, 48, OBJ_EVENT_GFX_SCHOOL_KID_M, MOVEMENT_TYPE_FACE_DOWN, 1, LostFound_EventScript_Owner, LostFound_IsOwnerShown},
    {MAP_SLATEPORT_CITY, 17, 31, OBJ_EVENT_GFX_SAILOR,       MOVEMENT_TYPE_FACE_DOWN, 2, LostFound_EventScript_Owner, LostFound_IsOwnerShown},
    {MAP_MAUVILLE_CITY,  17, 16, OBJ_EVENT_GFX_BOY_1,        MOVEMENT_TYPE_FACE_DOWN, 3, LostFound_EventScript_Owner, LostFound_IsOwnerShown},
    {MAP_FALLARBOR_TOWN, 11, 17, OBJ_EVENT_GFX_OLD_MAN,      MOVEMENT_TYPE_FACE_DOWN, 4, LostFound_EventScript_Owner, LostFound_IsOwnerShown},
    {MAP_FORTREE_CITY,   34, 18, OBJ_EVENT_GFX_GIRL_3,       MOVEMENT_TYPE_FACE_DOWN, 5, LostFound_EventScript_Owner, LostFound_IsOwnerShown},
    {MAP_LILYCOVE_CITY,  30, 28, OBJ_EVENT_GFX_WOMAN_2,      MOVEMENT_TYPE_FACE_DOWN, 6, LostFound_EventScript_Owner, LostFound_IsOwnerShown},
    {MAP_DEWFORD_TOWN,    9, 12, OBJ_EVENT_GFX_TUBER_M,      MOVEMENT_TYPE_FACE_DOWN, 7, LostFound_EventScript_Owner, LostFound_IsOwnerShown},
    {MAP_ROUTE104,       13, 44, OBJ_EVENT_GFX_ITEM_BALL,    MOVEMENT_TYPE_NONE,      0, LostFound_EventScript_Item,  LostFound_IsItemShown},
    {MAP_PETALBURG_WOODS,13, 19, OBJ_EVENT_GFX_ITEM_BALL,    MOVEMENT_TYPE_NONE,      1, LostFound_EventScript_Item,  LostFound_IsItemShown},
    {MAP_ROUTE109,       24, 20, OBJ_EVENT_GFX_ITEM_BALL,    MOVEMENT_TYPE_NONE,      2, LostFound_EventScript_Item,  LostFound_IsItemShown},
    {MAP_ROUTE110,       12, 69, OBJ_EVENT_GFX_ITEM_BALL,    MOVEMENT_TYPE_NONE,      3, LostFound_EventScript_Item,  LostFound_IsItemShown},
    {MAP_ROUTE113,       36, 12, OBJ_EVENT_GFX_ITEM_BALL,    MOVEMENT_TYPE_NONE,      4, LostFound_EventScript_Item,  LostFound_IsItemShown},
    {MAP_ROUTE119,        9, 74, OBJ_EVENT_GFX_ITEM_BALL,    MOVEMENT_TYPE_NONE,      5, LostFound_EventScript_Item,  LostFound_IsItemShown},
    {MAP_ROUTE121,       31, 16, OBJ_EVENT_GFX_ITEM_BALL,    MOVEMENT_TYPE_NONE,      6, LostFound_EventScript_Item,  LostFound_IsItemShown},
    {MAP_ROUTE104,       29, 24, OBJ_EVENT_GFX_ITEM_BALL,    MOVEMENT_TYPE_NONE,      7, LostFound_EventScript_Item,  LostFound_IsItemShown},
};

bool32 LostFound_IsOwnerShown(u32 arg)
{
    return TRUE;
}

bool32 LostFound_IsItemShown(u32 arg)
{
    return !(sSave.lostFound & BIT(arg));
}

// Script: talking to an owner (VAR_0x8004). VAR_RESULT = LOST_FOUND_*,
// gPastimeText = what they say, VAR_0x8005 = the reward.
void LostFound_TalkToOwner(void)
{
    u32 i = gSpecialVar_0x8004;
    const struct LostThing *thing = &sThings[i];

    sSave.lostHeard |= BIT(i);
    StringCopy(gStringVar1, thing->name);
    gSpecialVar_0x8005 = thing->reward;
    if (sSave.lostReturned & BIT(i))
    {
        gSpecialVar_Result = LOST_FOUND_RETURNED;
        StringCopy(gPastimeText, thing->after);
    }
    else if (sSave.lostFound & BIT(i))
    {
        gSpecialVar_Result = LOST_FOUND_CARRYING;
        StringCopy(gPastimeText, thing->thanks);
    }
    else
    {
        gSpecialVar_Result = LOST_FOUND_MISSING;
        StringCopy(gPastimeText, thing->plea);
    }
}

// Script: the reward was given.
void LostFound_Return(void)
{
    sSave.lostReturned |= BIT(gSpecialVar_0x8004);
}

// Script: picking up a lost thing. STR_VAR_1 = what, STR_VAR_2 = whose,
// STR_VAR_3 = where they live. VAR_RESULT = TRUE if the owner is known.
void LostFound_PickUp(void)
{
    u32 i = gSpecialVar_0x8004;

    sSave.lostFound |= BIT(i);
    StringCopy(gStringVar1, sThings[i].name);
    StringCopy(gStringVar2, sThings[i].owner);
    StringCopy(gStringVar3, sThings[i].town);
    gSpecialVar_Result = (sSave.lostHeard & BIT(i)) != 0;
}

// R > PASTIMES > LOST & FOUND --------------------------------------------

static bool32 IsListed(u32 i)
{
    return ((sSave.lostHeard | sSave.lostFound) & BIT(i)) != 0;
}

static u32 ListedIndex(u16 row)
{
    u32 i;

    for (i = 0; i < LOST_FOUND_COUNT; i++)
    {
        if (IsListed(i) && row-- == 0)
            return i;
    }
    return 0;
}

static void GetLine(u16 row, u8 *dest)
{
    u32 i = ListedIndex(row);

    if (sSave.lostReturned & BIT(i))
        dest = StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
    dest = StringCopy(dest, sThings[i].name);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 96}"));
    if (sSave.lostReturned & BIT(i))
        StringCopy(dest, COMPOUND_STRING("Returned"));
    else if (sSave.lostFound & BIT(i))
        StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Found! Take it back"));
    else
        StringCopy(dest, COMPOUND_STRING("Still lost"));
}

// The board's line buffer is 64 bytes: keep these short.
static void GetDetail(u16 row, u8 *dest)
{
    u32 i = ListedIndex(row);

    if (sSave.lostReturned & BIT(i))
    {
        dest = StringCopy(dest, COMPOUND_STRING("Given back to "));
        StringCopy(dest, sThings[i].owner);
    }
    else if (sSave.lostFound & BIT(i))
    {
        dest = StringCopy(dest, COMPOUND_STRING("Bring it to "));
        dest = StringCopy(dest, sThings[i].owner);
        dest = StringCopy(dest, COMPOUND_STRING(", "));
        StringCopy(dest, sThings[i].town);
    }
    else
    {
        dest = StringCopy(dest, COMPOUND_STRING("Lost on "));
        StringCopy(dest, sThings[i].where);
    }
}

static EWRAM_DATA u8 sTitle[40] = {0};

void LostFound_ShowList(void)
{
    u32 i, count = 0, returned = 0;
    u8 *p;

    for (i = 0; i < LOST_FOUND_COUNT; i++)
    {
        count += IsListed(i);
        returned += (sSave.lostReturned & BIT(i)) != 0;
    }
    p = StringCopy(sTitle, COMPOUND_STRING("LOST & FOUND   Returned "));
    p = ConvertIntToDecimalStringN(p, returned, STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(p, LOST_FOUND_COUNT, STR_CONV_MODE_LEFT_ALIGN, 1);
    WorldBoard_Show(sTitle, count, GetLine, GetDetail);
}
