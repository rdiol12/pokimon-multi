#ifndef GUARD_QOL_H
#define GUARD_QOL_H

// Quality-of-life options (OPTION menu, page two): auto-run, autosave,
// colour-blind palettes and remapped buttons. See docs/qol.md.

#define COLOR_MODE_NORMAL     0
#define COLOR_MODE_RED_WEAK   1 // protanopia
#define COLOR_MODE_GREEN_WEAK 2 // deuteranopia
#define COLOR_MODE_BLUE_WEAK  3 // tritanopia
#define COLOR_MODE_COUNT      4

// The buttons that can be remapped, in this order. The D-pad stays put.
enum {
    QOL_KEY_A,
    QOL_KEY_B,
    QOL_KEY_START,
    QOL_KEY_SELECT,
    QOL_KEY_L,
    QOL_KEY_R,
    QOL_KEY_COUNT,
};

extern const u16 gQoLKeyBits[QOL_KEY_COUNT];

// map[job] = the key (QOL_KEY_*) that does that job.
void QoL_DecodeKeyMap(u16 packed, u8 *map);
u16 QoL_EncodeKeyMap(const u8 *map);
u16 QoL_RemapKeys(u16 keys);

bool32 QoL_WantsToRun(u16 heldKeys);
bool32 QoL_TryAutosave(void);

// NULL when colours are normal, else a table of 0x8000 corrected BGR555 colours.
const u16 *QoL_GetColorTable(void);

#endif // GUARD_QOL_H
