#ifndef GUARD_WIDESCREEN_H
#define GUARD_WIDESCREEN_H

// Widescreen on our emulator (extended hardware v6, emulator/README.md). When the player turns
// widescreen on, the frame is 320 wide and the overworld fills it: the map, people and Pokémon
// show 40 more pixels on each side. Every other screen (battles, menus, the bag...) stays 240
// wide in the middle, with black bars.

// Map columns the side areas show past each edge of the normal view (40 px is 2.5 metatiles)
#define WIDE_EXTRA_METATILES 3

extern bool8 gFieldWide;
// Metatile columns object events spawn and stay in past each side of the normal range
#define FIELD_WIDE_EXTRA (gFieldWide ? WIDE_EXTRA_METATILES : 0)

bool32 Widescreen_Available(void);
// Layers drawn in the side areas (bits 0-3 BG0-BG3, PKX_WIDE_OBJ for sprites). ResetBgs clears it.
void Widescreen_SetLayers(u32 layers);
// A 32x32 background whose tilemap buffer is 64x32 (0x1000 bytes): CopyBgTilemapBufferToVram
// copies the right half to the extended map memory. ResetBgs clears it.
void Widescreen_SetBgWideMap(u32 bg, bool32 wide);
bool32 Widescreen_IsBgWideMap(u32 bg);
void Widescreen_CopyRightHalf(u32 bg, const void *tilemap);
void Widescreen_Reset(void);
// How far past the normal 240x160 view the field shows, in pixels (0 when it isn't wide)
s32 Widescreen_FieldMargin(void);

#endif // GUARD_WIDESCREEN_H
