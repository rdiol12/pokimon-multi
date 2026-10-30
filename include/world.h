#ifndef GUARD_WORLD_H
#define GUARD_WORLD_H

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

// Day/night (world_daynight.c).
void DayNight_UpdateFlags(void);

// Mounts (world_mount.c).
bool32 Mount_IsRideable(u16 species);
bool32 Mount_IsRiding(void);
u16 Mount_GetGfx(void);
u8 Mount_Attach(u8 objectId, u16 gfx);
void Mount_Detach(u8 spriteId);
void Mount_Update(void);
void Mount_Toggle(void);

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
// Sends the hardcore run's progress to the server (netplay_world.c).
void World_OnHardcoreProgress(void);

// Shiny chaining (world_shiny.c).
void ShinyChain_OnWildSpecies(enum Species species);
u32 ShinyChain_ExtraRolls(void);
u32 ShinyChain_BonusFor(u32 count);
void ShinyChain_OnWildBattleEnd(void);

// A two-line notice at the top of the screen (world_achievements.c).
void World_ShowToast(const u8 *title, const u8 *text);

// The R button menu.
bool32 World_TryOpenMenu(void);

#endif // GUARD_WORLD_H
