#include "global.h"
#include "event_data.h"
#include "malloc.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "script_menu.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"

// Detective cases: the DETECTIVE in Mauville hands out cases one at a time.
// Each case happens in a town, where three witnesses (shown only while the
// case is open) each give a clue. Back at the agency the player names the
// culprit from three suspects: right solves the case, wrong closes it
// unsolved (it can be opened again).

extern const u8 Detective_EventScript_Detective[];
extern const u8 Detective_EventScript_Witness[];

struct Case
{
    const u8 *title;
    const u8 *intro;
    const u8 *suspects[DETECTIVE_SUSPECTS];
    u8 culprit;
    const u8 *clues[DETECTIVE_CLUES];
    const u8 *solution;
    u16 reward;
};

static const struct Case sCases[DETECTIVE_CASES] =
{
    {
        .title = COMPOUND_STRING("THE VANISHED PIE"),
        .intro = COMPOUND_STRING("A berry pie went missing from a\nwindowsill in PETALBURG CITY.\pThree suspects: a ZIGZAGOON, the\nbaker's son and a PELIPPER."),
        .suspects = {COMPOUND_STRING("ZIGZAGOON"), COMPOUND_STRING("BAKER'S SON"), COMPOUND_STRING("PELIPPER")},
        .culprit = 0,
        .clues = {
            COMPOUND_STRING("I saw tracks in the flower bed,\ngoing this way, then that way…\pZig, then zag!"),
            COMPOUND_STRING("The baker's son? He was with me\nat the GYM all morning!"),
            COMPOUND_STRING("PELIPPER was out at sea with the\nfishing boats. I saw it myself!"),
        },
        .solution = COMPOUND_STRING("The zig-zag tracks gave it away!\nZIGZAGOON took the pie!"),
        .reward = ITEM_NUGGET,
    },
    {
        .title = COMPOUND_STRING("THE LOST WIND CHIME"),
        .intro = COMPOUND_STRING("A wind chime vanished from a tree\nhouse in FORTREE CITY.\pThree suspects: a CHIMECHO, a\nKECLEON and a traveling painter."),
        .suspects = {COMPOUND_STRING("CHIMECHO"), COMPOUND_STRING("KECLEON"), COMPOUND_STRING("PAINTER")},
        .culprit = 1,
        .clues = {
            COMPOUND_STRING("CHIMECHO? It slept at the POKéMON\nCENTER all day, the NURSE said so."),
            COMPOUND_STRING("The painter was selling pictures\nat the market from dawn to dusk."),
            COMPOUND_STRING("I still hear the chime ringing by\nthe wall… but there's nothing there!\pAnd that bit of wall keeps\nchanging color…"),
        },
        .solution = COMPOUND_STRING("A wall that changes color… KECLEON\nwas hiding with the chime!"),
        .reward = ITEM_PP_UP,
    },
    {
        .title = COMPOUND_STRING("THE SOAKED LAUNDRY"),
        .intro = COMPOUND_STRING("Someone soaked the clean laundry\nin SLATEPORT CITY.\pThree suspects: a WINGULL, a kid\nwith a water gun and a MARILL."),
        .suspects = {COMPOUND_STRING("WINGULL"), COMPOUND_STRING("KID"), COMPOUND_STRING("MARILL")},
        .culprit = 0,
        .clues = {
            COMPOUND_STRING("The water came down from above,\nlike a little rain cloud!"),
            COMPOUND_STRING("My water gun? I lent it to my\ncousin in DEWFORD last week!"),
            COMPOUND_STRING("MARILL was playing on the beach\nwith me. It never left!"),
        },
        .solution = COMPOUND_STRING("Water from above… WINGULL's WATER\nGUN from the sky!"),
        .reward = ITEM_BIG_PEARL,
    },
    {
        .title = COMPOUND_STRING("THE MIDNIGHT HOWL"),
        .intro = COMPOUND_STRING("A loud howl wakes LAVARIDGE TOWN\nevery night.\pThree suspects: a POOCHYENA, an\nold man and a WHISMUR."),
        .suspects = {COMPOUND_STRING("POOCHYENA"), COMPOUND_STRING("OLD MAN"), COMPOUND_STRING("WHISMUR")},
        .culprit = 2,
        .clues = {
            COMPOUND_STRING("POOCHYENA sleeps inside my house.\nI lock the door every night!"),
            COMPOUND_STRING("The old man? He's hard of hearing.\nHe didn't even notice the noise!"),
            COMPOUND_STRING("It was SO loud, my windows rattled!\nLike a little Pokémon practicing\lto be a big one…"),
        },
        .solution = COMPOUND_STRING("Practicing to be loud… WHISMUR\nwants to be a LOUDRED!"),
        .reward = ITEM_LUCKY_EGG,
    },
};

static const u8 *const sRankNames[] =
{
    COMPOUND_STRING("NEWCOMER"), COMPOUND_STRING("ROOKIE"), COMPOUND_STRING("SLEUTH"),
    COMPOUND_STRING("INSPECTOR"), COMPOUND_STRING("ACE DETECTIVE"),
};

#define sSave (gSaveBlock3Ptr->pastime)

static bool32 IsWitnessShown(u32 arg)
{
    return sSave.detectiveCase == arg / DETECTIVE_CLUES + 1;
}

#define WITNESS(map, x, y, gfx, n) {map, x, y, gfx, MOVEMENT_TYPE_FACE_DOWN, n, Detective_EventScript_Witness, IsWitnessShown}

const struct PastimeSpot gDetectiveSpots[1 + DETECTIVE_CASES * DETECTIVE_CLUES] =
{
    {MAP_MAUVILLE_CITY, 17, 10, OBJ_EVENT_GFX_GENTLEMAN, MOVEMENT_TYPE_FACE_DOWN, 0, Detective_EventScript_Detective},
    WITNESS(MAP_PETALBURG_CITY, 5, 7, OBJ_EVENT_GFX_WOMAN_1, 0),
    WITNESS(MAP_PETALBURG_CITY, 16, 13, OBJ_EVENT_GFX_BOY_1, 1),
    WITNESS(MAP_PETALBURG_CITY, 13, 20, OBJ_EVENT_GFX_FISHERMAN, 2),
    WITNESS(MAP_FORTREE_CITY, 2, 10, OBJ_EVENT_GFX_NURSE, 3),
    WITNESS(MAP_FORTREE_CITY, 34, 9, OBJ_EVENT_GFX_WOMAN_3, 4),
    WITNESS(MAP_FORTREE_CITY, 20, 12, OBJ_EVENT_GFX_LITTLE_GIRL, 5),
    WITNESS(MAP_SLATEPORT_CITY, 19, 7, OBJ_EVENT_GFX_WOMAN_2, 6),
    WITNESS(MAP_SLATEPORT_CITY, 22, 14, OBJ_EVENT_GFX_BOY_2, 7),
    WITNESS(MAP_SLATEPORT_CITY, 12, 15, OBJ_EVENT_GFX_TUBER_F, 8),
    WITNESS(MAP_LAVARIDGE_TOWN, 7, 10, OBJ_EVENT_GFX_WOMAN_4, 9),
    WITNESS(MAP_LAVARIDGE_TOWN, 14, 11, OBJ_EVENT_GFX_MAN_3, 10),
    WITNESS(MAP_LAVARIDGE_TOWN, 9, 17, OBJ_EVENT_GFX_LASS, 11),
};

static u32 CountBits(u32 bits)
{
    u32 count = 0;

    for (; bits != 0; bits >>= 1)
        count += bits & 1;
    return count;
}

static s32 NextCase(void)
{
    u32 i;

    for (i = 0; i < DETECTIVE_CASES; i++)
    {
        if (!(sSave.detectiveSolved & (1 << i)))
            return i;
    }
    return -1;
}

// Script: VAR_RESULT = DETECTIVE_OPEN (a case is open: gStringVar1 = its
// title, gStringVar2 = clues found), DETECTIVE_NEW (gStringVar1 = the next
// case's title, gPastimeText = its story) or DETECTIVE_ALL_SOLVED.
// gStringVar3 = the rank.
void Detective_GetState(void)
{
    s32 next = NextCase();

    StringCopy(gStringVar3, sRankNames[CountBits(sSave.detectiveSolved)]);
    if (sSave.detectiveCase != 0)
    {
        StringCopy(gStringVar1, sCases[sSave.detectiveCase - 1].title);
        ConvertIntToDecimalStringN(gStringVar2, CountBits(sSave.detectiveClues), STR_CONV_MODE_LEFT_ALIGN, 1);
        gSpecialVar_Result = DETECTIVE_OPEN;
    }
    else if (next < 0)
    {
        gSpecialVar_Result = DETECTIVE_ALL_SOLVED;
    }
    else
    {
        StringCopy(gStringVar1, sCases[next].title);
        StringCopy(gPastimeText, sCases[next].intro);
        gSpecialVar_Result = DETECTIVE_NEW;
    }
}

void Detective_OpenCase(void)
{
    sSave.detectiveCase = NextCase() + 1;
    sSave.detectiveClues = 0;
}

// Script: witness VAR_0x8004's clue in gPastimeText; VAR_RESULT = TRUE if
// it's new.
void Detective_TalkToWitness(void)
{
    u32 caseId = gSpecialVar_0x8004 / DETECTIVE_CLUES, clue = gSpecialVar_0x8004 % DETECTIVE_CLUES;
    u8 *p;

    StringCopy(gPastimeText, sCases[caseId].clues[clue]);
    gSpecialVar_Result = !(sSave.detectiveClues & (1 << clue));
    sSave.detectiveClues |= 1 << clue;
    if (!gSpecialVar_Result)
        return;
    p = StringCopy(gStringVar4, COMPOUND_STRING("Clue "));
    p = ConvertIntToDecimalStringN(p, CountBits(sSave.detectiveClues), STR_CONV_MODE_LEFT_ALIGN, 1);
    p = StringCopy(p, COMPOUND_STRING(" of 3 noted. "));
    StringCopy(p, CountBits(sSave.detectiveClues) == DETECTIVE_CLUES ? COMPOUND_STRING("Back to MAUVILLE!") : COMPOUND_STRING("Keep asking!"));
    World_ShowToast(sCases[caseId].title, gStringVar4);
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

// Script: pushes the open case's suspects, then NOT YET.
void Detective_PushSuspects(void)
{
    const struct Case *c = &sCases[sSave.detectiveCase - 1];
    u32 i;

    for (i = 0; i < DETECTIVE_SUSPECTS; i++)
        PushMenuItem(c->suspects[i], i);
    PushMenuItem(COMPOUND_STRING("NOT YET"), DETECTIVE_MENU_NOT_YET);
}

// Script: the player names suspect VAR_RESULT. VAR_RESULT = TRUE if right
// (gPastimeText = how it was done, VAR_0x8005 = the reward, gStringVar3 =
// the new rank); either way the case closes.
void Detective_Accuse(void)
{
    u32 caseId = sSave.detectiveCase - 1;
    const struct Case *c = &sCases[caseId];

    StringCopy(gStringVar1, c->suspects[gSpecialVar_Result]);
    sSave.detectiveCase = 0;
    sSave.detectiveClues = 0;
    if (gSpecialVar_Result != c->culprit)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    sSave.detectiveSolved |= 1 << caseId;
    StringCopy(gPastimeText, c->solution);
    StringCopy(gStringVar3, sRankNames[CountBits(sSave.detectiveSolved)]);
    gSpecialVar_0x8005 = c->reward;
    gSpecialVar_Result = TRUE;
}

// R > PASTIMES > DETECTIVE ---------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < DETECTIVE_CASES)
    {
        dest = StringCopy(dest, sCases[index].title);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 136}"));
        if (sSave.detectiveSolved & (1 << index))
            StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}SOLVED"));
        else if (sSave.detectiveCase == index + 1)
            StringCopy(dest, COMPOUND_STRING("{COLOR BLUE}{SHADOW LIGHT_BLUE}OPEN"));
        else
            StringCopy(dest, COMPOUND_STRING("-"));
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Rank{CLEAR_TO 56}"));
    StringCopy(dest, sRankNames[CountBits(sSave.detectiveSolved)]);
}

static void GetDetail(u16 index, u8 *dest)
{
    u8 *p;

    if (index < DETECTIVE_CASES && sSave.detectiveCase == index + 1)
    {
        p = StringCopy(dest, COMPOUND_STRING("Clues found: "));
        p = ConvertIntToDecimalStringN(p, CountBits(sSave.detectiveClues), STR_CONV_MODE_LEFT_ALIGN, 1);
        StringCopy(p, COMPOUND_STRING(" of 3"));
        return;
    }
    StringCopy(dest, COMPOUND_STRING("The DETECTIVE AGENCY is in MAUVILLE"));
}

void Detective_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("CASE FILES"), DETECTIVE_CASES + 1, GetLine, GetDetail);
}
