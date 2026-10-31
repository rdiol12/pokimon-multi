#ifndef GUARD_DUNGEON_H
#define GUARD_DUNGEON_H

// The Rift Below, the randomized cave under Route 114 (src/dungeon.c).

void Dungeon_SetupObjects(void);
void Dungeon_DrawMap(void);
void Dungeon_Enter(void);
void Dungeon_GetBest(void);
void Dungeon_GetNextFloor(void);
void Dungeon_Descend(void);
void Dungeon_GetItem(void);
bool32 Dungeon_TryInteract(s16 x, s16 y);
enum Species Dungeon_ModifyWildSpecies(enum Species species);
u8 Dungeon_ModifyWildLevel(u8 level);
void Dungeon_Showcase(u32 floor, u8 *ladderX, u8 *ladderY);

extern const u8 AdventureDungeon_EventScript_Ladder[];

#endif // GUARD_DUNGEON_H
