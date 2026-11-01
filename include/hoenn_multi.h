#ifndef GUARD_HOENN_MULTI_H
#define GUARD_HOENN_MULTI_H

// Changes to the original Hoenn for many players at once (docs/hoenn_multi.md).

// Called every overworld frame (from Netplay_OverworldUpdate), online or not.
void HoennMulti_OverworldUpdate(void);

// Whether other players are drawn on the current map right now: not in private
// or per-player places (your house, secret bases, gyms, the League, the
// Frontier's battle rooms) and not during one of your own cutscenes.
bool32 HoennMulti_ShowOtherPlayers(void);

// Whether `mover` walks straight through `other`: nobody bumps into another
// player or their following Pokémon.
bool32 HoennMulti_PassesThrough(const struct ObjectEvent *mover, const struct ObjectEvent *other);

// Whether the player has been to this map on their own (friend travel only
// goes to places the player has already reached).
bool32 HoennMulti_HasVisitedMap(u8 mapGroup, u8 mapNum);
void HoennMulti_SetVisited(u8 mapGroup, u8 mapNum);

#endif // GUARD_HOENN_MULTI_H
