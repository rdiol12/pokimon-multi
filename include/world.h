#ifndef GUARD_WORLD_H
#define GUARD_WORLD_H

#include "constants/world.h"

// World and gameplay features that are not tied to one screen: day/night
// events, roaming weather fronts and the hooks the overworld calls for them.
// See docs/world.md.

// Called by the overworld after the map header's weather is loaded and
// before the map's ON_TRANSITION script runs.
void World_OnMapTransition(void);
// Called once per overworld frame. Returns TRUE if it took over input.
bool32 World_OverworldUpdate(void);
// Lets the weather swap in a weather-only species for a wild encounter.
enum Species World_ModifyWildSpecies(enum Species species, u8 level);

// Roaming weather (world_weather.c).
enum OverworldWeather RoamingWeather_GetForMap(u8 mapGroup, u8 mapNum);
void RoamingWeather_ApplyToCurrentMap(void);
void RoamingWeather_Update(void);
enum Species RoamingWeather_ModifyWildSpecies(enum Species species, u8 level);
void RoamingWeather_GetForecast(void);
u16 RoamingWeather_GetFrontMap(u32 front, u32 hoursAhead);

// Day/night (world_daynight.c).
void DayNight_UpdateFlags(void);
enum DayNightTag
{
    DAYNIGHT_BOTH,
    DAYNIGHT_DAY,   // out in the morning, by day and in the evening
    DAYNIGHT_NIGHT, // out in the evening and at night
};
struct WildPokemon;
enum DayNightTag DayNight_GetSpeciesTag(u32 species);
// Whether a wild Pokemon of this species is out on this map at this hour.
bool32 DayNight_IsSpeciesOut(u32 species);
// Rerolls a wild slot (with choose(arg)) while its Pokemon is asleep.
u32 DayNight_PickSlot(const struct WildPokemon *mons, u32 index, u32 (*choose)(u32 arg), u32 arg);
// The hour (0-23) and minute the game runs on. Online, this is the
// server's clock, so everybody shares one sky; offline, the RTC.
bool32 DayNight_GetServerClock(s32 *hours, s32 *minutes);
// Lights the windows in the evening and at night (after a map's palettes
// load, and every overworld frame so they switch on as the hour turns).
void DayNight_UpdateLights(void);

// Mounts (world_mount.c).
bool32 Mount_IsRideable(u16 species);
bool32 Mount_IsRiding(void);
u16 Mount_GetGfx(void);
u8 Mount_Attach(u8 objectId, u16 gfx);
void Mount_Detach(u8 spriteId);
void Mount_Update(void);
void Mount_Toggle(void);

// Legendary quests (world_legends.c).
void Legends_Update(void);
bool32 Legends_TryInteract(s16 x, s16 y);
void Legends_ShowList(void);

// Player housing (world_house.c).
void House_Update(void);
bool32 House_TryInteract(s16 x, s16 y);
bool32 World_TryInteract(s16 x, s16 y);

// Online (world_online.c).
void WorldOnline_Update(void);
// What another player's follower shows of its injuries (INJURY_LOOKS_*), by its object's local id.
u32 WorldOnline_GetFollowerInjuryLooks(u8 localId);
void World_OnWildBattleStart(void);
void WorldOnline_OnCatch(void);
const u8 *WorldOnline_GetOpenHouse(u16 *playerId, u8 *name);
bool32 Mount_IsAttached(u8 spriteId, u8 objectId);
u32 Records_SizeOf(u16 species, u32 personality);
void Records_FormatSize(u8 *dest, u32 mm);
bool32 WorldBoard_IsOpen(void);

// Full-screen list window (world_board.c).
typedef void (*WorldBoardLineFunc)(u16 index, u8 *dest);
void WorldBoard_Show(const u8 *title, u16 count, WorldBoardLineFunc getLine, WorldBoardLineFunc getDetail);
void WorldBoard_Refresh(u16 count);

// Battle hooks (battle_setup.c).
void World_OnWildBattleEnd(void);
void World_OnTrainerBattleEnd(void);

// Hardcore mode (world_hardcore.c).
bool32 Hardcore_IsOn(void);
bool32 Hardcore_IsCatchBlocked(void);
void Hardcore_OnBattleEnd(bool32 wild);
bool32 Hardcore_Update(void);
// Online (netplay_world.c): the hardcore run's progress, and new records
// for the leaderboards (board: WORLD_BOARD_*).
void World_OnHardcoreProgress(void);
void World_SubmitScore(u8 board, u32 value, u32 extra);

// Shiny chaining (world_shiny.c).
void ShinyChain_OnWildSpecies(enum Species species);
u32 ShinyChain_ExtraRolls(void);
u32 ShinyChain_BonusFor(u32 count);
void ShinyChain_OnWildBattleEnd(void);

// A two-line notice at the top of the screen (world_achievements.c).
void World_ShowToast(const u8 *title, const u8 *text);

// Timing meter minigame (world_meter.c).
void TimingMeter_Start(u8 rounds, u8 zoneTiles, u8 speed);
s32 TimingMeter_GetResult(void);
s32 TimingMeter_GetLastOffset(void);

// Fishing: the reel meter replaces the "press A on a bite" check; the zone
// is 4 tiles wide with the Old Rod, 3 with the Good Rod and 2 with the Super.
#define WORLD_FISHING_REEL_METER TRUE
#define MAX_ZONE_TILES_FOR_ROD(rod) (4 - (rod))

// Fishing and berry records (world_records.c).
void Records_OnFishHooked(void);
void Records_OnWildBattleEnd(void);
void Records_OnBerriesPicked(u32 count);
void Records_ShowList(void);

// The R button menu.
bool32 World_TryOpenMenu(void);

#endif // GUARD_WORLD_H
