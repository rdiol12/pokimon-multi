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

// Team up (HUB > TEAM UP): another player's team fights beside the player
// in big battles. Called as a battle starts.
void HoennMulti_PrepareBattle(void);
bool32 HoennMulti_PartnerInBattle(void);

// Live leader battles: whether the battle starting now is one others can
// watch (a gym leader, the Elite Four or the Champion), and its opponent's
// name (NETPLAY_SOLO_NAME_LENGTH bytes).
bool32 HoennMulti_IsWatchableBattle(void);
void HoennMulti_GetWatchableOpponent(u8 *dest);

// Script natives and request hooks.
void HoennMulti_IsOnline(void);
void HoennMulti_AfterQuickMatch(void);
void HoennMulti_WatchLive(void);
void HoennMulti_StoreTeam(void);
void HoennMulti_DropTeam(void);
void HoennMulti_HasTeam(void);

#endif // GUARD_HOENN_MULTI_H
