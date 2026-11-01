#include "global.h"
#include "alive.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "metatile_behavior.h"
#include "constants/metatile_behaviors.h"
#include "fieldmap.h"
#include "data.h"
#include "follower_helper.h"
#include "item.h"
#include "money.h"
#include "netplay_ext.h"
#include "random.h"
#include "save_ext.h"
#include "script.h"
#include "string_util.h"
#include "task.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/metatile_labels.h"

// Towns that grow (docs/alive.md): every town project is funded by the
// players together (server/alive.py APROJ). Once it's built, it's on the
// map for everyone: a garden, a park or a plaza where there was empty
// grass, with a caretaker who has a little something for visitors each
// day. Offline, the player can fund one alone (a smaller sum).

struct Project
{
    u16 map;
    const u8 *name;
    u8 pattern;     // PATTERN_*
    u8 w, h;        // the site, in metatiles
    u32 offlineGoal;
    u16 gift;       // the caretaker's daily gift
};

enum { PATTERN_GARDEN, PATTERN_PARK, PATTERN_PLAZA };

static const struct Project sProjects[ALIVE_PROJECT_COUNT] =
{
    [ALIVE_PROJECT_OLDALE_GARDEN]     = {MAP_OLDALE_TOWN, COMPOUND_STRING("flower garden"), PATTERN_GARDEN, 4, 3, 15000, ITEM_ORAN_BERRY},
    [ALIVE_PROJECT_PETALBURG_PARK]    = {MAP_PETALBURG_CITY, COMPOUND_STRING("pond park"), PATTERN_PARK, 5, 3, 20000, ITEM_PECHA_BERRY},
    [ALIVE_PROJECT_RUSTBORO_STATUE]   = {MAP_RUSTBORO_CITY, COMPOUND_STRING("founder's plaza"), PATTERN_PLAZA, 5, 3, 25000, ITEM_HARD_STONE},
    [ALIVE_PROJECT_SLATEPORT_STALLS]  = {MAP_SLATEPORT_CITY, COMPOUND_STRING("market garden"), PATTERN_GARDEN, 5, 3, 30000, ITEM_SITRUS_BERRY},
    [ALIVE_PROJECT_MAUVILLE_PLAZA]    = {MAP_MAUVILLE_CITY, COMPOUND_STRING("city plaza"), PATTERN_PLAZA, 6, 3, 30000, ITEM_CELL_BATTERY},
    [ALIVE_PROJECT_FORTREE_BRIDGE]    = {MAP_FORTREE_CITY, COMPOUND_STRING("canopy garden"), PATTERN_GARDEN, 4, 3, 35000, ITEM_MIRACLE_SEED},
    [ALIVE_PROJECT_LILYCOVE_PROMENADE] = {MAP_LILYCOVE_CITY, COMPOUND_STRING("seaside promenade"), PATTERN_PLAZA, 6, 3, 40000, ITEM_PEARL},
    [ALIVE_PROJECT_DEWFORD_PIER]      = {MAP_DEWFORD_TOWN, COMPOUND_STRING("beach park"), PATTERN_PARK, 4, 3, 20000, ITEM_SHOAL_SALT},
};

static const u32 sServerGoals[ALIVE_PROJECT_COUNT] = {150000, 200000, 250000, 300000, 300000, 350000, 400000, 200000};

#define FLOWERS  0x004 // General: red flowers
#define COBBLE   0x1D9 // General: light path (MB_NORMAL)
#define GRASS    METATILE_General_Grass
#define SIGN     0x003 // General: a little sign
#define SAND     METATILE_General_SandPit_Center

static EWRAM_DATA struct
{
    s8 project;        // the project on this map (-1 none)
    s16 siteX, siteY;  // its site's top-left
    u8 siteW, siteH;   // and size (smaller than planned if the town is cramped)
    u8 siteFound;
    u8 foremanSpawned;
    u8 waiting;        // an APROJ request is out
    u8 rpcGive;        // ... and it's a gift
    u32 given, goal;   // online totals, as last heard
    u8 online;         // the totals above are from the server
    u8 justFinished;
    u8 top[PLAYER_NAME_LENGTH + 1];
} sProj = {0};

bool32 AliveProjects_IsDone(u32 p)
{
    return (gSaveBlockExt.alive.projectsDone >> p) & 1;
}

static void SetDone(u32 p)
{
    gSaveBlockExt.alive.projectsDone |= 1 << p;
}

static s32 ProjectHere(void)
{
    u16 here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;
    for (i = 0; i < ALIVE_PROJECT_COUNT; i++)
    {
        if (sProjects[i].map == here)
            return i;
    }
    return -1;
}

static bool32 HasEvents(s16 x, s16 y)
{
    const struct MapEvents *events = gMapHeader.events;
    u32 i;
    for (i = 0; i < events->warpCount; i++)
    {
        if (events->warps[i].x + MAP_OFFSET == x && events->warps[i].y + MAP_OFFSET == y)
            return TRUE;
    }
    for (i = 0; i < events->bgEventCount; i++)
    {
        if (events->bgEvents[i].x + MAP_OFFSET == x && events->bgEvents[i].y + MAP_OFFSET == y)
            return TRUE;
    }
    for (i = 0; i < events->coordEventCount; i++)
    {
        if (events->coordEvents[i].x + MAP_OFFSET == x && events->coordEvents[i].y + MAP_OFFSET == y)
            return TRUE;
    }
    for (i = 0; i < events->objectEventCount; i++)
    {
        if (events->objectEvents[i].x + MAP_OFFSET == x && events->objectEvents[i].y + MAP_OFFSET == y)
            return TRUE;
    }
    return FALSE;
}

static bool8 sAnyGround; // FindSite's second pass: any plain ground will do

// Plain grass, nothing on it: somewhere to build. Towns with no grass to
// spare build on plain paving or sand instead.
static bool32 IsOpenGrass(s16 x, s16 y)
{
    u32 behavior;
    if (MapGridGetCollisionAt(x, y) != 0 || HasEvents(x, y))
        return FALSE;
    if (MapGridGetMetatileIdAt(x, y) == GRASS)
        return TRUE;
    if (!sAnyGround || MapGridGetMetatileIdAt(x, y) >= NUM_METATILES_IN_PRIMARY)
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    return behavior == MB_NORMAL || behavior == MB_SAND;
}

// The empty patch of grass nearest the middle of town with room for the
// site and a free row around it (so no path gets blocked). Always the same
// spot for a town, so everyone builds in the same place.
static bool32 FindSite(u32 w, u32 h, s16 *outX, s16 *outY)
{
    s32 mw = gMapHeader.mapLayout->width, mh = gMapHeader.mapLayout->height;
    s32 cx = mw / 2 + MAP_OFFSET, cy = mh / 2 + MAP_OFFSET;
    s32 x, y, i, j;
    u32 best = 0xFFFF;

    for (y = MAP_OFFSET + 1; y + h + 1 < (u32)(mh + MAP_OFFSET); y++)
    {
        for (x = MAP_OFFSET + 1; x + w + 1 < (u32)(mw + MAP_OFFSET); x++)
        {
            bool32 ok = TRUE;
            u32 d = abs(x + (s32)w / 2 - cx) + abs(y + (s32)h / 2 - cy);
            if (d >= best)
                continue;
            for (j = -1; j <= (s32)h && ok; j++)
            {
                for (i = -1; i <= (s32)w && ok; i++)
                {
                    bool32 inside = (i >= 0 && i < (s32)w && j >= 0 && j < (s32)h);
                    if (inside ? !IsOpenGrass(x + i, y + j) : (MapGridGetCollisionAt(x + i, y + j) != 0 && !IsOpenGrass(x + i, y + j)))
                        ok = FALSE;
                }
            }
            if (ok)
            {
                best = d;
                *outX = x;
                *outY = y;
            }
        }
    }
    return best != 0xFFFF;
}

static u16 PatternTile(u32 pattern, u32 i, u32 j, u32 w, u32 h)
{
    bool32 edge = (i == 0 || j == 0 || i == w - 1 || j == h - 1);
    switch (pattern)
    {
    case PATTERN_GARDEN:
        // Flower beds with a paved path down the middle.
        if (i == w / 2)
            return COBBLE;
        return FLOWERS;
    case PATTERN_PARK:
        // Paving round a flower bed, a sign at the corner.
        if (i == 0 && j == 0)
            return SIGN;
        return edge ? COBBLE : FLOWERS;
    default:
        // A paved plaza with flowers at the corners.
        if ((i == 0 || i == w - 1) && (j == 0 || j == h - 1))
            return FLOWERS;
        if (i == w / 2 && j == 0)
            return SIGN;
        return COBBLE;
    }
}

static void Build(u32 p)
{
    const struct Project *proj = &sProjects[p];
    u32 i, j;
    for (j = 0; j < sProj.siteH; j++)
    {
        for (i = 0; i < sProj.siteW; i++)
        {
            u16 tile = PatternTile(proj->pattern, i, j, sProj.siteW, sProj.siteH);
            MapGridSetMetatileIdAt(sProj.siteX + i, sProj.siteY + j, tile | (tile == SIGN ? MAPGRID_COLLISION_MASK : 0));
        }
    }
}

void AliveProjects_OnMapLoad(void)
{
    s32 p = ProjectHere();
    sProj.project = p;
    sProj.siteFound = FALSE;
    if (p < 0)
        return;
    if (gAliveShowcase.scene == ALIVE_SHOW_PROJECT && gAliveShowcase.arg == p)
        SetDone(p);
    // The planned size on grass, then smaller, then on any plain ground.
    {
        static const u8 sSizes[][2] = {{0, 0}, {4, 3}, {3, 3}, {3, 2}};
        u32 pass, k;
        for (pass = 0; pass < 2 && !sProj.siteFound; pass++)
        {
            sAnyGround = pass;
            for (k = 0; k < ARRAY_COUNT(sSizes) && !sProj.siteFound; k++)
            {
                u32 w = k == 0 ? sProjects[p].w : sSizes[k][0], h = k == 0 ? sProjects[p].h : sSizes[k][1];
                if (k != 0 && w >= sProjects[p].w && h >= sProjects[p].h)
                    continue;
                if (FindSite(w, h, &sProj.siteX, &sProj.siteY))
                {
                    sProj.siteW = w;
                    sProj.siteH = h;
                    sProj.siteFound = TRUE;
                }
            }
        }
        sAnyGround = FALSE;
    }
    if (!sProj.siteFound)
        return;
    if (AliveProjects_IsDone(p))
        Build(p);
}

void AliveProjects_OnMapTransition(void)
{
    sProj.foremanSpawned = FALSE;
    sProj.online = FALSE;
}

static void SpawnForeman(void)
{
    s16 x, y;
    sProj.foremanSpawned = TRUE;
    if (sProj.project < 0 || !sProj.siteFound || Alive_IsQuiet() || !Alive_RoomForObjects(1))
        return;
    // Beside the site: building it, or looking after it.
    if (Alive_FindSpotNear(sProj.siteX - 1, sProj.siteY + 1, 2, &x, &y))
        Alive_Spawn(LOCALID_ALIVE_FOREMAN, AliveProjects_IsDone(sProj.project) ? OBJ_EVENT_GFX_GENTLEMAN : OBJ_EVENT_GFX_HIKER,
                    x, y, DIR_SOUTH);
}

bool32 AliveProjects_IsForeman(struct ObjectEvent *obj)
{
    return obj->localId == LOCALID_ALIVE_FOREMAN;
}

// --- Talking to the foreman -------------------------------------------------

static void Task_WaitRpc(u8 taskId);

static void StartRpc(bool32 give, u32 amount)
{
    if (!Npx_IsOnline() || !NpxRpc_Begin("APROJ"))
    {
        ScriptContext_Enable();
        return;
    }
    if (give)
    {
        NpxRpc_Word("GIVE");
        NpxRpc_Num(sProj.project);
        NpxRpc_Num(amount);
    }
    else
    {
        NpxRpc_Word("INFO");
    }
    NpxRpc_Send();
    sProj.waiting = TRUE;
    sProj.rpcGive = give;
    CreateTask(Task_WaitRpc, 80);
}

static void Task_WaitRpc(u8 taskId)
{
    u8 state = NpxRpc_Poll();
    if (state == NPX_RPC_WAITING)
        return;
    sProj.waiting = FALSE;
    gSpecialVar_0x8005 = FALSE;
    if (state == NPX_RPC_DONE && gNetplayExtMailbox.status == 0)
    {
        Npx_TakeAnswer();
        if (sProj.rpcGive)
        {
            sProj.given = Npx_Num(0, 0);
            sProj.goal = Npx_Num(0, 1);
            if (Npx_Num(0, 2))
                SetDone(sProj.project);
            sProj.justFinished = Npx_Num(0, 3);
        }
        else if ((u32)sProj.project < Npx_RowCount())
        {
            sProj.given = Npx_Num(sProj.project, 1);
            sProj.goal = Npx_Num(sProj.project, 2);
            if (Npx_Num(sProj.project, 3))
                SetDone(sProj.project);
            Npx_CopyField(sProj.top, sProj.project, 4, PLAYER_NAME_LENGTH);
        }
        sProj.online = TRUE;
        gSpecialVar_0x8005 = TRUE;
    }
    else if (state == NPX_RPC_DONE && sProj.rpcGive)
    {
        // Refused (already built): the message is the answer.
        Npx_AsciiToGame(gStringVar4, (const char *)gNetplayExtMailbox.resp, 100);
    }
    NpxRpc_Release();
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Progress(u32 *given, u32 *goal)
{
    if (sProj.online)
    {
        *given = sProj.given;
        *goal = sProj.goal ? sProj.goal : sServerGoals[sProj.project];
    }
    else
    {
        *given = gSaveBlockExt.alive.donated[sProj.project];
        *goal = sProjects[sProj.project].offlineGoal;
    }
}

// Script (waitstate): asks the server how the project is doing.
void AliveProjects_Refresh(void)
{
    StartRpc(FALSE, 0);
}

// Script: STR_VAR_4 what the foreman says; VAR_RESULT 0 building, 1 built
// (a gift for today in VAR_0x8004), 2 built (gift already taken).
void AliveProjects_BufferStatus(void)
{
    const struct Project *proj;
    u32 given, goal;
    struct AliveSave *save = &gSaveBlockExt.alive;

    if (sProj.project < 0)
    {
        gSpecialVar_Result = 2;
        StringCopy(gStringVar4, COMPOUND_STRING("Lovely day, isn't it?"));
        return;
    }
    proj = &sProjects[sProj.project];
    StringCopy(gStringVar1, proj->name);
    if (AliveProjects_IsDone(sProj.project))
    {
        u32 bit = 1u << (24 + sProj.project);
        u32 day = Alive_Day() + 1;
        if (save->giftDay != (u16)day)
        {
            save->giftDay = day;
            save->giftTowns = 0;
        }
        if (sProj.top[0] != EOS && sProj.top[0] != 0)
        {
            StringCopy(gStringVar2, sProj.top);
            StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Welcome to our new {STR_VAR_1}!\nEveryone built it together.\pThe biggest gift came from\n{STR_VAR_2}. Bless them!"));
        }
        else
        {
            StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Welcome to our new {STR_VAR_1}!\nEveryone built it together.\pI look after it now. Isn't it\nlovely?"));
        }
        if (save->giftTowns & bit)
            gSpecialVar_Result = 2;
        else
        {
            save->giftTowns |= bit;
            gSpecialVar_0x8004 = proj->gift;
            gSpecialVar_Result = 1;
        }
        return;
    }
    Progress(&given, &goal);
    ConvertIntToDecimalStringN(gStringVar2, given * 100 / max(goal, 1), STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar3, goal - min(given, goal), STR_CONV_MODE_LEFT_ALIGN, 7);
    if (sProj.online)
        StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("We're building a {STR_VAR_1} here!\nEvery TRAINER on the server helps.\pIt's {STR_VAR_2}% funded. We still need\n¥{STR_VAR_3}."));
    else
        StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("We're building a {STR_VAR_1} here!\pIt's {STR_VAR_2}% funded. We still need\n¥{STR_VAR_3}."));
    gSpecialVar_Result = 0;
}

// Script (waitstate): gives VAR_0x8004 money. VAR_RESULT 0 not enough money,
// 1 thanks, 2 thanks and it's finished, 3 refused (STR_VAR_4 says why).
void AliveProjects_Donate(void)
{
    u32 amount = gSpecialVar_0x8004, given, goal;
    struct AliveSave *save = &gSaveBlockExt.alive;

    if (sProj.project < 0 || !IsEnoughMoney(&gSaveBlock1Ptr->money, amount))
    {
        gSpecialVar_Result = 0;
        ScriptContext_Enable();
        return;
    }
    if (Npx_IsOnline())
    {
        gSpecialVar_Result = 1;
        sProj.justFinished = FALSE;
        StartRpc(TRUE, amount); // enables the script when the answer comes
        return;
    }
    gSpecialVar_0x8005 = 2; // nothing for AliveProjects_AfterDonate to do
    RemoveMoney(&gSaveBlock1Ptr->money, amount);
    save->donated[sProj.project] += amount;
    Progress(&given, &goal);
    if (given >= goal)
    {
        SetDone(sProj.project);
        gSpecialVar_Result = 2;
    }
    else
    {
        gSpecialVar_Result = 1;
    }
    ScriptContext_Enable();
}

// Script: after an online gift. Takes the money if the server took it.
void AliveProjects_AfterDonate(void)
{
    if (gSpecialVar_0x8005 == 2 || gSpecialVar_Result == 0)
        return; // offline, or not enough money: already done
    if (!gSpecialVar_0x8005)
    {
        gSpecialVar_Result = 3;
        return;
    }
    RemoveMoney(&gSaveBlock1Ptr->money, gSpecialVar_0x8004);
    gSaveBlockExt.alive.donated[sProj.project] += gSpecialVar_0x8004;
    gSpecialVar_Result = sProj.justFinished || AliveProjects_IsDone(sProj.project) ? 2 : 1;
}

// Script: a newly finished project appears at once.
void AliveProjects_BuildNow(void)
{
    if (sProj.project >= 0 && sProj.siteFound)
    {
        Build(sProj.project);
        DrawWholeMapView();
    }
}

void AliveProjects_Update(void)
{
    if (!sProj.foremanSpawned)
        SpawnForeman();
}
