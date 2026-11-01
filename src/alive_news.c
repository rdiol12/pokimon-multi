#include "global.h"
#include "alive.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "data.h"
#include "follower_helper.h"
#include "fieldmap.h"
#include "netplay_ext.h"
#include "overworld.h"
#include "random.h"
#include "region_map.h"
#include "rovers.h"
#include "save_ext.h"
#include "script.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_objects.h"
#include "constants/metatile_labels.h"
#include "constants/region_map_sections.h"

// Living news (docs/alive.md): a town crier in every city shouts the
// headlines and hands out the paper, and TVs show the news. Online the
// news is what really happened on the server (server/alive.py ANEWS);
// offline it's the weather, the player's own deeds and the town's gossip.

#define NEWS_MAX      12
#define NEWS_LENGTH   72
#define FETCH_EVERY   (60 * 60 * 3) // frames between server fetches

static EWRAM_EXT_DATA u8 sServerNews[NEWS_MAX][NEWS_LENGTH] = {0};
static EWRAM_EXT_DATA u16 sServerAge[NEWS_MAX] = {0}; // minutes
static EWRAM_EXT_DATA u8 sLocalNews[4][NEWS_LENGTH] = {0};

static EWRAM_DATA struct
{
    u8 serverCount;
    u8 localCount;
    u8 waiting;
    u8 crierSpawned;
    u16 fetchTimer;
    u16 shoutTimer;
    u8 shown[NEWS_MAX]; // the headlines on the board right now, by index into the list
} sNews = {0};

// The headlines, newest first: the server's, then ours.
static EWRAM_EXT_DATA u8 sList[NEWS_MAX + 8][NEWS_LENGTH] = {0};
static EWRAM_DATA u8 sListCount = 0;

void AliveNews_AddLocal(const u8 *headline)
{
    memmove(sLocalNews[1], sLocalNews[0], sizeof(sLocalNews[0]) * 3);
    StringCopyN(sLocalNews[0], headline, NEWS_LENGTH - 1);
    sLocalNews[0][NEWS_LENGTH - 1] = EOS;
    if (sNews.localCount < 4)
        sNews.localCount++;
}

static void MapNameOf(u8 *dest, u16 map)
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF);
    GetMapName(dest, header->regionMapSectionId, 0);
}

static void Add(const u8 *fmt)
{
    if (sListCount >= ARRAY_COUNT(sList))
        return;
    StringExpandPlaceholders(sList[sListCount], fmt);
    sListCount++;
}

static void BuildList(void)
{
    u32 i;
    static const u8 *const sFronts[] = {COMPOUND_STRING("Rainstorm"), COMPOUND_STRING("Sandstorm"), COMPOUND_STRING("Snow squall")};

    sListCount = 0;
    for (i = 0; i < sNews.serverCount; i++)
        StringCopy(sList[sListCount++], sServerNews[i]);
    for (i = 0; i < sNews.localCount; i++)
        StringCopy(sList[sListCount++], sLocalNews[i]);

    // The forecast, from the roaming weather fronts.
    i = Alive_Day() % 3;
    MapNameOf(gStringVar1, RoamingWeather_GetFrontMap(i, 3));
    StringCopy(gStringVar2, sFronts[i]);
    Add(COMPOUND_STRING("FORECAST: {STR_VAR_2} heading for {STR_VAR_1}."));

    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        Add(COMPOUND_STRING("CHAMPION {PLAYER} still unbeaten at the LEAGUE!"));
    else if (Alive_Badges() != 0)
    {
        ConvertIntToDecimalStringN(gStringVar1, Alive_Badges(), STR_CONV_MODE_LEFT_ALIGN, 1);
        Add(COMPOUND_STRING("Local TRAINER {PLAYER} wins BADGE no. {STR_VAR_1}!"));
    }
    if (gSaveBlockExt.alive.helped != 0)
        Add(COMPOUND_STRING("Helpful TRAINER praised on the routes again."));

    // The rovers' week, when the server can't say.
    if (sNews.serverCount == 0)
    {
        StringCopy(gStringVar1, gRoverCast[(Alive_Day() / 7 * 31) % ROVER_COUNT].name);
        StringCopy(gStringVar2, gRoverCast[(Alive_Day() * 17 + 3) % ROVER_COUNT].name);
        Add(COMPOUND_STRING("ROVER LEAGUE: {STR_VAR_1} the favorite this week."));
        Add(COMPOUND_STRING("{STR_VAR_2} spotted training on the routes."));
    }
    // A bit of town life every day.
    {
        static const u8 *const sDaily[] =
        {
            COMPOUND_STRING("SLATEPORT MARKET: BERRIES fresh off the boat."),
            COMPOUND_STRING("Wild POKéMON seen sleeping in the grass at night."),
            COMPOUND_STRING("Town projects need your help! Ask the foreman."),
            COMPOUND_STRING("POKé MARTS remind shoppers: closed 10 PM-6 AM."),
            COMPOUND_STRING("Fishermen report record catches off DEWFORD."),
            COMPOUND_STRING("FORTREE: TROPIUS fruit harvest a big success."),
            COMPOUND_STRING("Hikers warn: dry ponds hide lost treasures."),
        };
        Add(sDaily[Alive_Day() % ARRAY_COUNT(sDaily)]);
        Add(sDaily[(Alive_Day() + 3) % ARRAY_COUNT(sDaily)]);
    }
}

// --- Server ---------------------------------------------------------------

static void Fetch(void)
{
    if (!Npx_IsOnline() || sNews.waiting || !NpxRpc_Begin("ANEWS"))
        return;
    NpxRpc_Send();
    sNews.waiting = TRUE;
}

static void UpdateFetch(void)
{
    u8 state;
    u32 i, rows;

    if (!sNews.waiting)
    {
        if (sNews.fetchTimer != 0)
            sNews.fetchTimer--;
        else if (Alive_IsTown())
        {
            sNews.fetchTimer = FETCH_EVERY;
            Fetch();
        }
        return;
    }
    state = NpxRpc_Poll();
    if (state == NPX_RPC_WAITING)
        return;
    sNews.waiting = FALSE;
    if (state == NPX_RPC_DONE && gNetplayExtMailbox.status == 0)
    {
        Npx_TakeAnswer();
        rows = min(Npx_RowCount(), NEWS_MAX);
        for (i = 0; i < rows; i++)
        {
            sServerAge[i] = Npx_Num(i, 0);
            Npx_CopyField(sServerNews[i], i, 1, NEWS_LENGTH - 1);
        }
        sNews.serverCount = rows;
    }
    NpxRpc_Release();
}

// --- The town crier -------------------------------------------------------

static bool32 CrierHere(void)
{
    // In cities and towns with a Pokémon Center, by day.
    u32 hour = Alive_Hour();
    return Alive_IsTown() && hour >= 7 && hour < 21;
}

static void SpawnCrier(void)
{
    const struct MapEvents *events = gMapHeader.events;
    u32 i;
    s16 x, y;

    sNews.crierSpawned = TRUE;
    if (!CrierHere() || !Alive_RoomForObjects(1))
        return;
    for (i = 0; i < events->warpCount; i++)
    {
        s16 wx = events->warps[i].x + MAP_OFFSET, wy = events->warps[i].y + MAP_OFFSET;
        if (MapGridGetMetatileIdAt(wx, wy) != METATILE_General_Door_PokeCenter)
            continue;
        if (Alive_FindSpotNear(wx - 2, wy + 2, 2, &x, &y))
            Alive_Spawn(LOCALID_ALIVE_CRIER, OBJ_EVENT_GFX_REPORTER_M, x, y, DIR_SOUTH);
        return;
    }
}

bool32 AliveNews_IsCrier(struct ObjectEvent *obj)
{
    return obj->localId == LOCALID_ALIVE_CRIER;
}

static void Shout(void)
{
    struct ObjectEvent *crier = Alive_Obj(LOCALID_ALIVE_CRIER);
    u8 text[NEWS_LENGTH + 24];

    if (crier == NULL || Alive_PlayerDistance(crier->currentCoords.x, crier->currentCoords.y) > 7 || Alive_IsCaptionShown())
        return;
    BuildList();
    if (sListCount == 0)
        return;
    gSaveBlockExt.alive.newsSeen++;
    StringCopy(text, COMPOUND_STRING("Extra! "));
    StringAppend(text, sList[gSaveBlockExt.alive.newsSeen % sListCount]);
    Alive_Emote(crier, FOLLOWER_EMOTION_SURPRISE);
    Alive_Do(crier, GetJumpInPlaceMovementAction(crier->facingDirection));
    Alive_ShowCaption(COMPOUND_STRING("CRIER"), text);
}

void AliveNews_OnMapTransition(void)
{
    sNews.crierSpawned = FALSE;
    sNews.shoutTimer = 120 + Random() % 240;
    if (sNews.fetchTimer > 60)
        sNews.fetchTimer = 60; // a new town: ask again soon
}

void AliveNews_Update(void)
{
    UpdateFetch();
    if (Alive_IsQuiet())
        return;
    if (!sNews.crierSpawned)
        SpawnCrier();
    if (gAliveShowcase.scene == ALIVE_SHOW_CHATTER)
        return; // the chatter's turn
    if (sNews.shoutTimer != 0)
        sNews.shoutTimer--;
    else
    {
        sNews.shoutTimer = 600 + Random() % 600;
        Shout();
    }
}

// --- The paper and the TV -------------------------------------------------

static void PaperLine(u16 index, u8 *dest)
{
    StringCopyN(dest, sList[index], 30);
    dest[30] = EOS;
    if (StringLength(sList[index]) > 30)
        StringAppend(dest, COMPOUND_STRING("…"));
}

static void PaperDetail(u16 index, u8 *dest)
{
    StringCopy(dest, sList[index]);
    if (index < sNews.serverCount)
    {
        u32 minutes = sServerAge[index];
        StringAppend(dest, COMPOUND_STRING("\n"));
        if (minutes < 60)
        {
            ConvertIntToDecimalStringN(dest + StringLength(dest), minutes, STR_CONV_MODE_LEFT_ALIGN, 2);
            StringAppend(dest, COMPOUND_STRING(" min. ago, on the server"));
        }
        else
        {
            ConvertIntToDecimalStringN(dest + StringLength(dest), min(minutes / 60, 999), STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(dest, COMPOUND_STRING(" hr. ago, on the server"));
        }
    }
}

// Script (callnative, waitstate): the paper, as a full-screen list.
void AliveNews_ShowPaper(void)
{
    BuildList();
    WorldBoard_Show(COMPOUND_STRING("HOENN DAILY NEWS"), sListCount, PaperLine, PaperDetail);
}

// Script: STR_VAR_4 two headlines for the TV; VAR_RESULT FALSE if the TV
// shows its usual programmes this time.
void AliveNews_BufferHeadline(void)
{
    u32 a;

    BuildList();
    gSpecialVar_Result = FALSE;
    if (sListCount == 0 || Alive_IsQuiet() || (Random() % 3 == 0 && sNews.serverCount == 0))
        return;
    a = Random() % sListCount;
    StringCopy(gStringVar4, COMPOUND_STRING("HOENN NEWS!\p"));
    StringAppend(gStringVar4, sList[a]);
    if (sListCount > 1)
    {
        StringAppend(gStringVar4, COMPOUND_STRING("\p"));
        StringAppend(gStringVar4, sList[(a + 1) % sListCount]);
    }
    Alive_Wrap(gStringVar4, 34, TRUE);
    gSpecialVar_Result = TRUE;
}
