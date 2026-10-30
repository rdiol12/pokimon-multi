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

#endif // GUARD_WORLD_H
