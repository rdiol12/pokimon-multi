#include "global.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "guide.h"
#include "netplay.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "constants/area_ids.h"
#include "constants/adventure.h"
#include "constants/flags.h"
#include "constants/life.h"
#include "constants/map_groups.h"
#include "constants/mechanics.h"
#include "constants/region_map_sections.h"

// The guide (docs/guide.md): every new area and activity, where it starts,
// what it needs and whether it's done. R > GUIDE lists them and marks them
// on a map (src/guide_screen.c); the overworld tells the player when a badge
// opens something new.
//
// Save data is one bit per entry (struct GuideSaveData.seen): set when the
// player goes to the place or looks at it in the guide. Everything else comes
// from the flags the features already keep, so adding an entry is one line.

extern const u8 Guide_EventScript_NewInGuide[];
extern const u8 Guide_EventScript_SendInvites[];

static bool8 CinderValvesDone(void)
{
    return FlagGet(FLAG_CINDER_VALVE_WEST) && FlagGet(FLAG_CINDER_VALVE_EAST) && FlagGet(FLAG_CINDER_VALVE_SOUTH);
}

static bool8 AmberwoodLanternsDone(void)
{
    return FlagGet(FLAG_AMBERWOOD_LANTERN_1) && FlagGet(FLAG_AMBERWOOD_LANTERN_2) && FlagGet(FLAG_AMBERWOOD_LANTERN_3);
}

#define AREA(_name, _where, _mapsec, _map, _badges, _done) \
    {COMPOUND_STRING(_name), COMPOUND_STRING(_where), _mapsec, _map, 0, _done, NULL, _badges}

#define ACTIVITY(_name, _where, _mapsec, _map, _badges, _started) \
    {COMPOUND_STRING(_name), COMPOUND_STRING(_where), _mapsec, _map, _started, 0, NULL, _badges}

// In story order: the list sorts by status, then keeps this order.
const struct GuideEntry gGuideEntries[] =
{
    ACTIVITY("DAILY QUESTS", "People by the POKéMON CENTERS of PETALBURG,\nSLATEPORT and FORTREE ask a favor every day.",
             MAPSEC_PETALBURG_CITY, 0, 0, 0),
    ACTIVITY("LOST AND FOUND", "People in PETALBURG and other towns lost\nthings on the routes nearby.",
             MAPSEC_PETALBURG_CITY, 0, 0, 0),
#ifdef FLAG_BAMBOO_TEA_DONE
    AREA("WHISPERSTEM VALLEY", "A guide in PETALBURG CITY leads you into\nthe bamboo valley.",
         MAPSEC_PETALBURG_CITY, MAP_WHISPERSTEM_VALLEY, 0, FLAG_BAMBOO_TEA_DONE),
#endif
    ACTIVITY("PARCEL DELIVERIES", "HOENN EXPRESS desks stand outside the\nPOKéMON CENTERS, starting in OLDALE.",
             MAPSEC_OLDALE_TOWN, 0, 0, 0),
    ACTIVITY("CRAFTING", "The crafter outside DEVON CORP. in RUSTBORO\nturns Apricorns into items.",
             MAPSEC_RUSTBORO_CITY, 0, 0, FLAG_ADVENTURE_CRAFTING),
    ACTIVITY("TRAINER SKILLS", "The teacher in RUSTBORO's POKéMON SCHOOL\nteaches a skill for every second badge.",
             MAPSEC_RUSTBORO_CITY, 0, 1, FLAG_MECH_MET_SKILL_TEACHER),
    ACTIVITY("MAIL ROUTES", "The PIDGEY & WINGULL POST in RUSTBORO,\nSLATEPORT, MAUVILLE and LILYCOVE.",
             MAPSEC_RUSTBORO_CITY, 0, 0, 0),
    ACTIVITY("UNDERGROUND MINING", "The digger by the rock in RUSTURF TUNNEL\nlends out digging tools.",
             MAPSEC_RUSTURF_TUNNEL, 0, 0, 0),
    ACTIVITY("TREASURE HUNTS", "An old treasure hunter in DEWFORD hands out\na new treasure map every day.",
             MAPSEC_DEWFORD_TOWN, 0, 0, 0),
#ifdef FLAG_GULL_CAPTAIN_BEATEN
    AREA("GULLWING POINT", "A boatman in DEWFORD TOWN takes you to the\ncape with the lighthouse and pirate ship.",
         MAPSEC_DEWFORD_TOWN, MAP_GULLWING_POINT, 0, FLAG_GULL_CAPTAIN_BEATEN),
#endif
#ifdef FLAG_CARNIVAL_EGG_DONE
    AREA("SUNFAIR CARNIVAL", "A sailor in SLATEPORT takes you to the\nfairground by the sea.",
         MAPSEC_SLATEPORT_CITY, MAP_SUNFAIR_CARNIVAL, 0, FLAG_CARNIVAL_EGG_DONE),
#endif
    ACTIVITY("RULE BATTLES", "A referee in SLATEPORT's BATTLE TENT runs\nINVERSE battles and other rule sets.",
             MAPSEC_SLATEPORT_CITY, 0, 0, FLAG_MECH_MET_REFEREE),
    ACTIVITY("POKéMON JOBS", "The job agent in MAUVILLE's POKéMON CENTER\nsends your POKéMON out on errands.",
             MAPSEC_MAUVILLE_CITY, 0, 0, 0),
    ACTIVITY("BATTLE QUIZ", "The QUIZMASTER in MAUVILLE asks about type\nmatchups. Ten questions a round.",
             MAPSEC_MAUVILLE_CITY, 0, 0, 0),
    ACTIVITY("EVOLUTION SAGE", "A sage in a MAUVILLE house evolves trade\nPOKéMON without a trade.",
             MAPSEC_MAUVILLE_CITY, 0, 0, FLAG_MECH_MET_EVO_SAGE),
    ACTIVITY("LEVEL CUPS", "A host in VERDANTURF's BATTLE TENT runs\nlevel-capped cups.",
             MAPSEC_VERDANTURF_TOWN, 0, 0, FLAG_MECH_MET_CUP_HOST),
    ACTIVITY("HOENN RANCH", "The ranch hand next to the ROUTE 117 DAY\nCARE takes you to the ranch.",
             MAPSEC_ROUTE_117, MAP_ADVENTURE_RANCH, 0, 0),
    {COMPOUND_STRING("SIGNATURE MOVES"), COMPOUND_STRING("PROF. BIRCH's aide in LITTLEROOT found\nsomething about your first POKéMON."),
     MAPSEC_LITTLEROOT_TOWN, 0, FLAG_MECH_SIG_STARTED, FLAG_MECH_SIG_TAUGHT, NULL, 3},
    ACTIVITY("THE RIFT BELOW", "A spelunker outside METEOR FALLS on\nROUTE 114 lets you dive into the cave.",
             MAPSEC_ROUTE_114, MAP_ADVENTURE_DUNGEON, 0, 0),
    {COMPOUND_STRING("BATTLE STYLES"), COMPOUND_STRING("The style master in FALLARBOR's BATTLE TENT\nteaches the AGILE and STRONG styles."),
     MAPSEC_FALLARBOR_TOWN, 0, 0, FLAG_MECH_BATTLE_STYLES, NULL, 0},
    AREA("EMBER SPRINGS", "A hiker in LAVARIDGE TOWN leads you up to\nthe hot springs.",
         MAPSEC_LAVARIDGE_TOWN, MAP_EMBER_SPRINGS, 4, FLAG_EMBER_QUEST_DONE),
    {COMPOUND_STRING("CINDER CAVERNS"), COMPOUND_STRING("The volcano cave on the north hill of\nEMBER SPRINGS, above LAVARIDGE."),
     MAPSEC_LAVARIDGE_TOWN, MAP_CINDER_CAVERNS_1F, 0, 0, CinderValvesDone, 4},
    AREA("SUNSCAR DESERT", "A guide in MAUVILLE CITY leads you to the\ndesert and its ruins.",
         MAPSEC_MAUVILLE_CITY, MAP_SUNSCAR_DESERT, 4, FLAG_SUNSCAR_RIDDLE_3),
    AREA("SKYROPE CANYON", "A hiker in FALLARBOR TOWN leads you to the\nrope bridges.",
         MAPSEC_FALLARBOR_TOWN, MAP_SKYROPE_CANYON, 5, FLAG_SKYROPE_BRIDGE_FIXED),
    AREA("ELDERWOOD FOREST", "A guide on the east side of FORTREE CITY\ntakes you into the old forest.",
         MAPSEC_FORTREE_CITY, MAP_ELDERWOOD_FOREST, 6, FLAG_ELDERWOOD_QUEST_DONE),
#ifdef FLAG_MIRE_STEW_DONE
    AREA("MIREBOUGH MARSH", "A guide in FORTREE CITY takes you to the\nrainy marsh.",
         MAPSEC_FORTREE_CITY, MAP_MIREBOUGH_MARSH, 6, FLAG_MIRE_STEW_DONE),
#endif
    ACTIVITY("CAFE AND COOK-OFF", "The chef of CAFE SOLEIL and the COOK-OFF\njudge stand on the LILYCOVE waterfront.",
             MAPSEC_LILYCOVE_CITY, 0, 0, 0),
    ACTIVITY("MUSEUM DONATIONS", "The curator in the LILYCOVE MUSEUM takes\nfossils and rare finds.",
             MAPSEC_LILYCOVE_CITY, 0, 0, 0),
    ACTIVITY("PHOTO SAFARI", "A photographer at the SAFARI ZONE on\nROUTE 121 lends you a camera.",
             MAPSEC_ROUTE_121, 0, 0, 0),
    ACTIVITY("LEGEND QUESTS", "People on ROUTES 110 to 121 tell old\nlegends. R > QUESTS keeps track.",
             MAPSEC_ROUTE_119, 0, 0, 0),
    AREA("FROSTVEIN CAVE", "A sailor in MOSSDEEP CITY takes you to the\nice cave.",
         MAPSEC_MOSSDEEP_CITY, MAP_FROSTVEIN_CAVE_1F, 7, FLAG_FROSTVEIN_FRIGIBAX),
#ifdef FLAG_ABYSS_TEMPLE_OPEN
    AREA("CORALLINE REEF", "A diver in PACIFIDLOG TOWN takes you down\nto the reef, the wreck and sunken ABYSSIA.",
         MAPSEC_PACIFIDLOG_TOWN, MAP_CORALLINE_REEF, 7, FLAG_ABYSS_TEMPLE_OPEN),
#endif
#ifdef FLAG_HOLLOW_MANOR_KEY
    AREA("HOLLOWMERE", "An old ferryman in LILYCOVE CITY rows you\nthere, but only at night.",
         MAPSEC_LILYCOVE_CITY, MAP_HOLLOWMERE, 7, FLAG_HOLLOW_MANOR_KEY),
#endif
    AREA("STARFALL ISLE", "A sailor in LILYCOVE CITY sails to the\nisland of rare POKéMON.",
         MAPSEC_LILYCOVE_CITY, MAP_STARFALL_ISLE, 8, FLAG_STARFALL_SKWOVET_REWARD),
    ACTIVITY("STARFALL HOTEL", "The bellhop in STARFALL ISLE's plaza makes\nyou the hotel's manager.",
             MAPSEC_LILYCOVE_CITY, MAP_STARFALL_ISLE_HOTEL, 8, 0),
    ACTIVITY("YOUR HOUSE", "An empty house on STARFALL ISLE, east of\nthe explorer's. Furnish it!",
             MAPSEC_LILYCOVE_CITY, MAP_STARFALL_ISLE_PLAYER_HOUSE, 8, 0),
    {COMPOUND_STRING("AMBERWOOD HOLLOW"), COMPOUND_STRING("The boatman on STARFALL ISLE rows you to\nthe autumn woods."),
     MAPSEC_LILYCOVE_CITY, MAP_AMBERWOOD_HOLLOW, 0, 0, AmberwoodLanternsDone, 8},
    AREA("STARFALL CAVE", "A cave on STARFALL ISLE where a wish-maker\nis said to sleep.",
         MAPSEC_LILYCOVE_CITY, MAP_STARFALL_ISLE_CAVE, GUIDE_CHAMPION, FLAG_STARFALL_JIRACHI_DONE),
    AREA("MOLTEN HEART", "Deep in CINDER CAVERNS, past the guard on\nthe landing. Something stirs in the lava.",
         MAPSEC_LAVARIDGE_TOWN, MAP_CINDER_CAVERNS_HEART, GUIDE_CHAMPION, FLAG_CINDER_HEATRAN_DONE),
    AREA("TEAM ECLIPSE", "A scientist at the MOSSDEEP SPACE CENTER\nneeds help. A new villain team is out.",
         MAPSEC_MOSSDEEP_CITY, 0, GUIDE_CHAMPION, FLAG_LIFE_ECLIPSE_BEATEN_0 + 5),
    ACTIVITY("HALL OF FAME WALL", "The curator in the POKéMON LEAGUE shows\nevery champion's team.",
             MAPSEC_EVER_GRANDE_CITY, 0, GUIDE_CHAMPION, 0),
};

const u8 gGuideEntryCount = ARRAY_COUNT(gGuideEntries);

STATIC_ASSERT(ARRAY_COUNT(gGuideEntries) <= GUIDE_MAX_ENTRIES, GuideEntriesFitInSave);

#define sSeen (gSaveBlock3Ptr->guide.seen)

static u32 CountBadges(void)
{
    u32 i, n = 0;

    for (i = 0; i < NUM_BADGES; i++)
    {
        if (FlagGet(FLAG_BADGE01_GET + i))
            n++;
    }
    return n;
}

static bool32 IsUnlocked(const struct GuideEntry *entry)
{
    if (entry->badges == GUIDE_CHAMPION)
        return FlagGet(FLAG_SYS_GAME_CLEAR);
    return CountBadges() >= entry->badges;
}

static bool32 IsDone(const struct GuideEntry *entry)
{
    if (entry->isDone != NULL)
        return entry->isDone();
    return entry->doneFlag != 0 && FlagGet(entry->doneFlag);
}

static bool32 HasQuest(const struct GuideEntry *entry)
{
    return entry->isDone != NULL || entry->doneFlag != 0;
}

enum GuideStatus Guide_GetStatus(u32 i)
{
    const struct GuideEntry *entry = &gGuideEntries[i];

    if (IsDone(entry))
        return GUIDE_DONE;
    if (!IsUnlocked(entry))
        return GUIDE_LOCKED;
    if (!(sSeen[i / 8] & (1 << (i % 8))) && !(entry->startedFlag != 0 && FlagGet(entry->startedFlag)))
        return GUIDE_NEW;
    return HasQuest(entry) ? GUIDE_TODO : GUIDE_KNOWN;
}

void Guide_MarkSeen(u32 i)
{
    if (i < gGuideEntryCount)
        sSeen[i / 8] |= 1 << (i % 8);
}

u32 Guide_CountStatus(enum GuideStatus status)
{
    u32 i, n = 0;

    for (i = 0; i < gGuideEntryCount; i++)
    {
        if (Guide_GetStatus(i) == status)
            n++;
    }
    return n;
}

static const u8 *const sBadgeNames[NUM_BADGES] =
{
    COMPOUND_STRING("STONE BADGE"),
    COMPOUND_STRING("KNUCKLE BADGE"),
    COMPOUND_STRING("DYNAMO BADGE"),
    COMPOUND_STRING("HEAT BADGE"),
    COMPOUND_STRING("BALANCE BADGE"),
    COMPOUND_STRING("FEATHER BADGE"),
    COMPOUND_STRING("MIND BADGE"),
    COMPOUND_STRING("RAIN BADGE"),
};

// "Needs the HEAT BADGE." for a locked entry.
const u8 *Guide_GetNeedsText(u32 i)
{
    const struct GuideEntry *entry = &gGuideEntries[i];

    if (entry->badges == GUIDE_CHAMPION)
        return COMPOUND_STRING("Opens once you are the CHAMPION.");
    if (entry->badges == 0 || entry->badges > NUM_BADGES)
        return COMPOUND_STRING("");
    StringCopy(gStringVar3, COMPOUND_STRING("Needs the "));
    StringAppend(gStringVar3, sBadgeNames[entry->badges - 1]);
    StringAppend(gStringVar3, COMPOUND_STRING("."));
    return gStringVar3;
}

// Overworld -----------------------------------------------------------------

static EWRAM_DATA struct
{
    u8 unlocked[GUIDE_MAX_ENTRIES / 8]; // what was open when we last looked
    bool8 ready;                        // unlocked[] has been filled once
    bool8 invitePending;                // INVITE FRIENDS on the title menu
    u8 timer;
} sGuide = {0};

static void MarkVisited(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;

    for (i = 0; i < gGuideEntryCount; i++)
    {
        if (gGuideEntries[i].map != 0 && gGuideEntries[i].map == map)
            Guide_MarkSeen(i);
    }
}

// The first entry that opened since the last look, or GUIDE_NONE. Puts its
// name in gStringVar1 and how many others opened with it in gStringVar2.
static u32 FindNewlyUnlocked(void)
{
    u32 i, first = GUIDE_NONE, others = 0;

    for (i = 0; i < gGuideEntryCount; i++)
    {
        bool32 now = IsUnlocked(&gGuideEntries[i]) && !IsDone(&gGuideEntries[i]);
        bool32 before = (sGuide.unlocked[i / 8] >> (i % 8)) & 1;

        if (now)
            sGuide.unlocked[i / 8] |= 1 << (i % 8);
        if (!now || before || !sGuide.ready)
            continue;
        if (first == GUIDE_NONE)
            first = i;
        else
            others++;
    }
    sGuide.ready = TRUE;
    if (first != GUIDE_NONE)
    {
        StringCopy(gStringVar1, gGuideEntries[first].name);
        ConvertIntToDecimalStringN(gStringVar2, others, STR_CONV_MODE_LEFT_ALIGN, 2);
        gSpecialVar_0x8005 = others;
    }
    return first;
}

static bool32 IsFree(void)
{
    return !ArePlayerFieldControlsLocked() && IsPlayerStandingStill();
}

bool32 Guide_OverworldUpdate(void)
{
    if (++sGuide.timer < 30)
        return FALSE;
    sGuide.timer = 0;
    MarkVisited();
    if (!IsFree())
        return FALSE;
    if (sGuide.invitePending && Netplay_IsOnline())
    {
        sGuide.invitePending = FALSE;
        ScriptContext_SetupScript(Guide_EventScript_SendInvites);
        return TRUE;
    }
    if (FindNewlyUnlocked() != GUIDE_NONE)
    {
        ScriptContext_SetupScript(Guide_EventScript_NewInGuide);
        return TRUE;
    }
    return FALSE;
}

void Guide_JoinOnlineAndInvite(void)
{
    Netplay_JoinOnline();
    sGuide.invitePending = TRUE;
}
