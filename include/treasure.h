#ifndef GUARD_TREASURE_H
#define GUARD_TREASURE_H

// Treasure hunts (src/treasure.c).

void Treasure_Talk(void);
void Treasure_HasMap(void);
void Treasure_OpenMap(void);
void Treasure_CloseMap(void);
bool32 Treasure_TryInteract(s16 x, s16 y);
void Treasure_Dig(void);
void Treasure_Showcase(u32 spot);

extern const u8 Adventure_EventScript_TreasureDig[];
extern const u8 Adventure_EventScript_TreasureTooLate[];

#endif // GUARD_TREASURE_H
