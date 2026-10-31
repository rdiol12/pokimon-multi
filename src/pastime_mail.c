#include "global.h"
#include "event_data.h"
#include "money.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "pokemon.h"
#include "rtc.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Mail courier routes: the PIDGEY & WINGULL POST runs one route a day, in a
// different town each day. Its postmaster there hands out three letters for
// people around town. A PIDGEY or WINGULL line Pokémon in the party flies
// one of them over by itself (and earns a tip). Delivering them all pays,
// plus a stamp item every few routes. The people waiting for mail only
// show up while you carry their letter.

extern const u8 Mail_EventScript_Postmaster[];
extern const u8 Mail_EventScript_Recipient[];

struct MailTown
{
    const u8 *name;
    const u8 *recipients[MAIL_LETTERS];
    const u8 *replies[MAIL_LETTERS];
};

static const struct MailTown sTowns[MAIL_TOWN_COUNT] =
{
    {
        .name = COMPOUND_STRING("RUSTBORO CITY"),
        .recipients = {COMPOUND_STRING("PROF. ELM's cousin"), COMPOUND_STRING("a DEVON worker"), COMPOUND_STRING("a schoolgirl")},
        .replies =
        {
            COMPOUND_STRING("A letter from JOHTO! My cousin\nsays his MARILL learned a new move."),
            COMPOUND_STRING("Ugh, a memo from the boss.\nOvertime again… Thanks, though."),
            COMPOUND_STRING("It's from my pen pal in KALOS!\nShe drew me a picture of FENNEKIN!"),
        },
    },
    {
        .name = COMPOUND_STRING("SLATEPORT CITY"),
        .recipients = {COMPOUND_STRING("a market vendor"), COMPOUND_STRING("a sailor's wife"), COMPOUND_STRING("an old fisherman")},
        .replies =
        {
            COMPOUND_STRING("An order for twelve HEAL BALLS!\nBusiness is booming!"),
            COMPOUND_STRING("A letter from my husband at sea!\nHe's coming home next week!"),
            COMPOUND_STRING("Hm? “You won the fishing contest.”\nHaha! Knew I would!"),
        },
    },
    {
        .name = COMPOUND_STRING("MAUVILLE CITY"),
        .recipients = {COMPOUND_STRING("a bike fan"), COMPOUND_STRING("a GAME CORNER regular"), COMPOUND_STRING("a young TRAINER")},
        .replies =
        {
            COMPOUND_STRING("My new bike bell is on the way!\nRing ring!"),
            COMPOUND_STRING("A letter? Not my winnings?\nOh well. Thanks, kid."),
            COMPOUND_STRING("It's from my mom!\nShe says to eat my vegetables…"),
        },
    },
    {
        .name = COMPOUND_STRING("LILYCOVE CITY"),
        .recipients = {COMPOUND_STRING("a contest star"), COMPOUND_STRING("a department store clerk"), COMPOUND_STRING("a painter")},
        .replies =
        {
            COMPOUND_STRING("Fan mail! Three pages of it!\nMy MILOTIC will be thrilled."),
            COMPOUND_STRING("The new stock list! Time to fill\nthe shelves again."),
            COMPOUND_STRING("The museum wants my painting!\nI must get back to work!"),
        },
    },
};

static const u16 sBirds[] = {SPECIES_PIDGEY, SPECIES_PIDGEOTTO, SPECIES_PIDGEOT, SPECIES_WINGULL, SPECIES_PELIPPER};

// Every third route: a stamp of thanks, in turn.
static const u16 sStampItems[] = {ITEM_PRETTY_FEATHER, ITEM_HEALTH_FEATHER, ITEM_GENIUS_FEATHER, ITEM_SWIFT_FEATHER, ITEM_RARE_CANDY};

#define PAY_ROUTE  600
#define PAY_TIP    200

#define sSave (gSaveBlock3Ptr->pastime)

// Per town: the postmaster (arg = town), then the three recipients
// (arg = town * MAIL_LETTERS + letter).
const struct PastimeSpot gMailSpots[MAIL_TOWN_COUNT * (1 + MAIL_LETTERS)] =
{
    {MAP_RUSTBORO_CITY,  20, 35, OBJ_EVENT_GFX_POKEFAN_M, MOVEMENT_TYPE_FACE_DOWN, 0, Mail_EventScript_Postmaster},
    {MAP_RUSTBORO_CITY,  24, 39, OBJ_EVENT_GFX_SCIENTIST_1, MOVEMENT_TYPE_FACE_DOWN, 0, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_RUSTBORO_CITY,  29, 23, OBJ_EVENT_GFX_DEVON_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 1, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_RUSTBORO_CITY,  14, 46, OBJ_EVENT_GFX_LASS, MOVEMENT_TYPE_FACE_DOWN, 2, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_SLATEPORT_CITY, 19, 12, OBJ_EVENT_GFX_POKEFAN_M, MOVEMENT_TYPE_FACE_DOWN, 1, Mail_EventScript_Postmaster},
    {MAP_SLATEPORT_CITY, 6, 15,  OBJ_EVENT_GFX_MAN_3, MOVEMENT_TYPE_FACE_DOWN, 3, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_SLATEPORT_CITY, 18, 38, OBJ_EVENT_GFX_WOMAN_3, MOVEMENT_TYPE_FACE_DOWN, 4, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_SLATEPORT_CITY, 4, 45,  OBJ_EVENT_GFX_FISHERMAN, MOVEMENT_TYPE_FACE_DOWN, 5, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_MAUVILLE_CITY,  26, 11, OBJ_EVENT_GFX_POKEFAN_M, MOVEMENT_TYPE_FACE_DOWN, 2, Mail_EventScript_Postmaster},
    {MAP_MAUVILLE_CITY,  32, 16, OBJ_EVENT_GFX_CYCLING_TRIATHLETE_M, MOVEMENT_TYPE_FACE_DOWN, 6, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_MAUVILLE_CITY,  16, 7,  OBJ_EVENT_GFX_GENTLEMAN, MOVEMENT_TYPE_FACE_DOWN, 7, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_MAUVILLE_CITY,  11, 8,  OBJ_EVENT_GFX_YOUNGSTER, MOVEMENT_TYPE_FACE_DOWN, 8, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_LILYCOVE_CITY,  23, 16, OBJ_EVENT_GFX_POKEFAN_M, MOVEMENT_TYPE_FACE_DOWN, 3, Mail_EventScript_Postmaster},
    {MAP_LILYCOVE_CITY,  34, 21, OBJ_EVENT_GFX_BEAUTY, MOVEMENT_TYPE_FACE_DOWN, 9, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_LILYCOVE_CITY,  15, 21, OBJ_EVENT_GFX_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN, 10, Mail_EventScript_Recipient, Mail_IsRecipientShown},
    {MAP_LILYCOVE_CITY,  43, 26, OBJ_EVENT_GFX_ARTIST, MOVEMENT_TYPE_FACE_DOWN, 11, Mail_EventScript_Recipient, Mail_IsRecipientShown},
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 TodaysTown(void)
{
    return Today() % MAIL_TOWN_COUNT;
}

static bool32 IsTodaysRoute(void)
{
    return sSave.mailDay == Today() && (sSave.mailLetters & (MAIL_ACTIVE | MAIL_PAID));
}

static u32 LettersLeft(void)
{
    u32 i, count = 0;

    if (!IsTodaysRoute())
        return 0;
    for (i = 0; i < MAIL_LETTERS; i++)
        count += (sSave.mailLetters >> i) & 1;
    return count;
}

bool32 Mail_IsRecipientShown(u32 arg)
{
    return IsTodaysRoute() && sSave.mailTown == arg / MAIL_LETTERS && (sSave.mailLetters >> (arg % MAIL_LETTERS)) & 1;
}

// The first PIDGEY or WINGULL line Pokémon in the party, or PARTY_SIZE.
static u32 FindBird(void)
{
    u32 slot, i;

    for (slot = 0; slot < CalculatePlayerPartyCount(); slot++)
    {
        u16 species = GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_SPECIES_OR_EGG);
        for (i = 0; i < ARRAY_COUNT(sBirds); i++)
        {
            if (sBirds[i] == species)
                return slot;
        }
    }
    return PARTY_SIZE;
}

// Script: talking to a postmaster (VAR_0x8004 = town). VAR_RESULT = MAIL_*,
// STR_VAR_1 = today's town, STR_VAR_2 = letters left.
void Mail_TalkToPostmaster(void)
{
    u32 here = gSpecialVar_0x8004;

    StringCopy(gStringVar1, sTowns[TodaysTown()].name);
    ConvertIntToDecimalStringN(gStringVar2, LettersLeft(), STR_CONV_MODE_LEFT_ALIGN, 1);
    if (IsTodaysRoute())
    {
        if (sSave.mailLetters & MAIL_PAID)
            gSpecialVar_Result = MAIL_DONE_TODAY;
        else if (LettersLeft() == 0)
            gSpecialVar_Result = MAIL_COMPLETE;
        else
            gSpecialVar_Result = MAIL_IN_PROGRESS;
    }
    else if (here != TodaysTown())
        gSpecialVar_Result = MAIL_OTHER_TOWN;
    else
        gSpecialVar_Result = MAIL_NEW_ROUTE;
}

// Script: taking today's letters. VAR_RESULT = TRUE if a bird in the party
// flew one over (STR_VAR_1 = its nickname, STR_VAR_2 = who got it).
void Mail_StartRoute(void)
{
    u32 bird = FindBird();

    sSave.mailDay = Today();
    sSave.mailTown = gSpecialVar_0x8004;
    sSave.mailLetters = MAIL_ACTIVE | ((1 << MAIL_LETTERS) - 1);
    gSpecialVar_Result = bird < PARTY_SIZE;
    if (gSpecialVar_Result)
    {
        GetMonData(&gParties[B_TRAINER_PLAYER][bird], MON_DATA_NICKNAME, gStringVar1);
        StringCopy(gStringVar2, sTowns[sSave.mailTown].recipients[MAIL_LETTERS - 1]);
        sSave.mailLetters &= ~(1 << (MAIL_LETTERS - 1));
        sSave.mailLettersDelivered++;
    }
}

// Script: all letters delivered. Pays (STR_VAR_1 = amount); VAR_RESULT =
// TRUE with VAR_0x8005 = a stamp item every third route.
void Mail_FinishRoute(void)
{
    u32 pay = PAY_ROUTE + (FindBird() < PARTY_SIZE ? PAY_TIP : 0);

    sSave.mailLetters = MAIL_PAID;
    sSave.mailRoutes++;
    AddMoney(&gSaveBlock1Ptr->money, pay);
    ConvertIntToDecimalStringN(gStringVar1, pay, STR_CONV_MODE_LEFT_ALIGN, 5);
    gSpecialVar_Result = sSave.mailRoutes % 3 == 0;
    gSpecialVar_0x8005 = sStampItems[(sSave.mailRoutes / 3 - 1) % ARRAY_COUNT(sStampItems)];
}

// Script: handing a letter over (VAR_0x8004 = recipient). gPastimeText =
// the reply; VAR_RESULT = letters left.
void Mail_TalkToRecipient(void)
{
    u32 arg = gSpecialVar_0x8004;

    sSave.mailLetters &= ~(1 << (arg % MAIL_LETTERS));
    sSave.mailLettersDelivered++;
    StringCopy(gPastimeText, sTowns[arg / MAIL_LETTERS].replies[arg % MAIL_LETTERS]);
    gSpecialVar_Result = LettersLeft();
}

// R > PASTIMES > MAIL -----------------------------------------------------

// Today's letters (while some are left to deliver), then the totals.
static u32 ListedLetters(void)
{
    return LettersLeft() != 0 ? MAIL_LETTERS : 0;
}

static void GetLine(u16 index, u8 *dest)
{
    if (index < ListedLetters())
    {
        const u8 *who = sTowns[sSave.mailTown].recipients[index];
        if (!((sSave.mailLetters >> index) & 1))
            dest = StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
        dest = StringCopy(dest, COMPOUND_STRING("For "));
        StringCopy(dest, who);
        return;
    }
    switch (index - ListedLetters())
    {
    case 0:
        dest = StringCopy(dest, COMPOUND_STRING("Today's route{CLEAR_TO 112}"));
        StringCopy(dest, sTowns[TodaysTown()].name);
        break;
    case 1:
        dest = StringCopy(dest, COMPOUND_STRING("Routes finished{CLEAR_TO 112}"));
        ConvertIntToDecimalStringN(dest, sSave.mailRoutes, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    default:
        dest = StringCopy(dest, COMPOUND_STRING("Letters delivered{CLEAR_TO 112}"));
        ConvertIntToDecimalStringN(dest, sSave.mailLettersDelivered, STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    }
}

void Mail_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("PIDGEY & WINGULL POST"), 3 + ListedLetters(), GetLine, NULL);
}
