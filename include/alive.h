#ifndef GUARD_ALIVE_H
#define GUARD_ALIVE_H

#include "constants/alive.h"

// The living world (docs/alive.md): wild Pokémon you can see, townsfolk
// with daily routines who remember you and chat with each other, living
// news, ambient critters, town projects, surprise events, weather that
// changes the map and resident town Pokémon.

struct ObjectEvent;
struct ScriptContext;

// --- Hooks the rest of the game calls ------------------------------------

// World_OverworldUpdate (src/world.c). TRUE if it started a script.
bool32 Alive_OverworldUpdate(void);
// World_OnMapTransition (src/world.c): a new map's header is loaded.
void Alive_OnMapTransition(void);
// RunOnLoadMapScript (src/script.c): the map's metatiles are loaded and
// may be changed (town projects, flooding, dry ponds).
void Alive_OnMapLoad(void);
// GetInteractedObjectEventScript (src/field_control_avatar.c): the script
// for talking to one of our objects, or a townsperson's remark first;
// NULL to talk as usual.
const u8 *Alive_GetTalkScript(struct ObjectEvent *obj, const u8 *script);
// Player bumped into an object (TryTriggerOverworldWildEncounter's spot):
// TRUE if it was a snow drift we cleared.
bool32 Alive_OnBump(struct ObjectEvent *obj);
// ScrCmd_pokemart: TRUE if the shop is closed for the night (a script was
// started instead).
bool32 Alive_IsShopClosed(void);

// --- Visible wild Pokémon (alive_wild.c) ---------------------------------

// TRUE: wild Pokémon only come out of the grass as random encounters, as in
// the original game (the automated tests, or the player's own choice).
bool32 Alive_ClassicEncounters(void);
void Alive_SetClassicEncounters(bool32 classic);
// The overworld encounter code asks for packs: TRUE (and a tile next to the
// pack's leader) if the next spawn joins a pack.
bool32 Alive_WildTakePackTile(s32 *x, s32 *y);
bool32 Alive_WildIsPackSpawn(void);
bool32 Alive_WildCreatePackMon(void);
// A wild Pokémon just appeared on the map.
void Alive_WildOnSpawned(struct ObjectEvent *owe, s32 x, s32 y);
// The battle with an overworld Pokémon starts: a sleeping one starts asleep.
void Alive_WildOnBattleStart(struct ObjectEvent *owe);
bool32 Alive_WildIsAsleep(struct ObjectEvent *owe);

// --- Shared helpers (alive.c) -------------------------------------------

u32 Alive_Hour(void);
u32 Alive_Day(void);
bool32 Alive_IsNight(void);
bool32 Alive_IsTown(void);
bool32 Alive_IsRoute(void);
bool32 Alive_IsOutdoors(void);
bool32 Alive_IsQuiet(void); // test boots: nothing turns up
struct ObjectEvent *Alive_Obj(u8 localId);
void Alive_Remove(u8 localId);
u8 Alive_Spawn(u8 localId, u16 gfx, s16 x, s16 y, u32 facing);
bool32 Alive_Busy(struct ObjectEvent *obj);
void Alive_Do(struct ObjectEvent *obj, u8 action);
bool32 Alive_CanStandAt(s16 x, s16 y);
bool32 Alive_FindSpotNear(s16 cx, s16 cy, u32 radius, s16 *outX, s16 *outY);
bool32 Alive_FindSpotOnScreen(u32 minDist, s16 *outX, s16 *outY);
bool32 Alive_RoomForObjects(u32 count);
u32 Alive_PlayerDistance(s16 x, s16 y);
bool32 Alive_StepToward(struct ObjectEvent *obj, s16 tx, s16 ty, bool32 fast);
u16 Alive_MonGfx(u16 species);
bool32 Alive_IsHumanGfx(u16 gfx);
bool32 Alive_IsTownsperson(struct ObjectEvent *obj);
const u8 *Alive_SpeakerName(struct ObjectEvent *obj);
void Alive_ShowCaption(const u8 *speaker, const u8 *text);
void Alive_Wrap(u8 *s, u32 width, bool32 msgbox);
bool32 Alive_IsCaptionShown(void);
void Alive_CloseCaption(void);
void Alive_Emote(struct ObjectEvent *obj, u8 emotion);
void Alive_ShowZzz(struct ObjectEvent *obj);
void Alive_UpdateZzz(void);
u32 Alive_Badges(void);
u16 Alive_LeadSpecies(void);
u32 Alive_MapSeed(void);

// --- Features (each in its own file) ------------------------------------

void AliveTown_OnMapTransition(void);
void AliveTown_Update(void);
bool32 AliveTown_IsResident(struct ObjectEvent *obj);
const u8 *AliveTown_GetTalkScript(struct ObjectEvent *obj);

const u8 *AliveTalk_GetRemarkScript(struct ObjectEvent *obj, const u8 *script);
void AliveTalk_UpdateChatter(void);
void AliveTalk_OnMapTransition(void);

void AliveNews_OnMapTransition(void);
void AliveNews_Update(void);
bool32 AliveNews_IsCrier(struct ObjectEvent *obj);
void AliveNews_AddLocal(const u8 *headline);

void AliveAmbient_OnMapTransition(void);
void AliveAmbient_Update(void);
bool32 AliveAmbient_IsCritter(struct ObjectEvent *obj);

void AliveProjects_OnMapLoad(void);
void AliveProjects_OnMapTransition(void);
void AliveProjects_Update(void);
bool32 AliveProjects_IsForeman(struct ObjectEvent *obj);
bool32 AliveProjects_IsDone(u32 project);

void AliveEvents_OnMapTransition(void);
void AliveEvents_Update(void);
bool32 AliveEvents_IsIncidentObject(struct ObjectEvent *obj);
const u8 *AliveEvents_GetTalkScript(struct ObjectEvent *obj);

void AliveWeather_OnMapLoad(void);
void AliveWeather_OnMapTransition(void);
void AliveWeather_Update(void);
bool32 AliveWeather_OnBump(struct ObjectEvent *obj);
bool32 AliveWeather_IsWeatherObject(struct ObjectEvent *obj);
const u8 *AliveWeather_GetTalkScript(struct ObjectEvent *obj);

void AliveWild_OnMapTransition(void);
void AliveWild_Update(void);

// --- Script natives -----------------------------------------------------

void Alive_ResumeTalk(struct ScriptContext *ctx);
void AliveTown_BufferMonText(void);
void AliveNews_ShowPaper(void);
void AliveNews_BufferHeadline(void);
void AliveProjects_BufferStatus(void);
void AliveProjects_Donate(void);
void AliveProjects_Refresh(void);
void AliveProjects_AfterDonate(void);
void AliveProjects_BuildNow(void);
void AliveTalk_BufferRemark(void);
void AliveEvents_ChildState(void);
void AliveEvents_ChildFollow(void);
void AliveEvents_ChildReward(void);
void AliveEvents_BufferStuck(void);
void AliveEvents_RescueStuck(void);
void AliveEvents_OpenCrate(void);
void AliveEvents_BufferHurt(void);
void AliveEvents_HealHurt(void);
void AliveEvents_HurtJoins(void);
void AliveEvents_HurtLeaves(void);
void AliveWeather_TakeFind(void);

// --- Screenshot scenes (tools/alive_shots/) -----------------------------

struct AliveShowcase
{
    u8 scene;       // what to force on the next map: ALIVE_SHOW_*
    u8 arg;
    u8 living;      // 1: the living world runs on a test boot too (screenshots)
    u8 weather;     // a weather to force on the next map (0: none)
    u16 species;
    u16 pad;
};
extern struct AliveShowcase gAliveShowcase;

#define ALIVE_SHOW_NONE      0
#define ALIVE_SHOW_INCIDENT  1 // arg: ALIVE_INCIDENT_*
#define ALIVE_SHOW_CHATTER   2 // the next chatter starts at once
#define ALIVE_SHOW_CRITTERS  3 // critters and a flyover at once
#define ALIVE_SHOW_PROJECT   4 // arg: project, finished
#define ALIVE_SHOW_PACK      5 // species: a pack of these at once
#define ALIVE_SHOW_REMARK    6 // the next townsperson talked to makes a remark
#define ALIVE_SHOW_DRY       7 // today is a dry spell

#endif // GUARD_ALIVE_H
