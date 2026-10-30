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

// The R button menu.
bool32 World_TryOpenMenu(void);

#endif // GUARD_WORLD_H
