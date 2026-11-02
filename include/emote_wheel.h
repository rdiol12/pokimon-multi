#ifndef GUARD_EMOTE_WHEEL_H
#define GUARD_EMOTE_WHEEL_H

// The emote wheel (L in the overworld while online, src/netplay.c). Emote ids
// go over the network as they are, so never renumber them; old games ignore
// ids they don't know.
#define EMOTE_COUNT 24
#define EMOTE_NONE  0xFF

bool32 EmoteWheel_IsPhrase(u8 emote);
const u8 *EmoteWheel_GetText(u8 emote);
// Opens the wheel; onChosen gets the emote, or EMOTE_NONE if it was closed.
void EmoteWheel_Open(void (*onChosen)(u8 emote));
bool32 EmoteWheel_IsOpen(void);
// Pops the emote's bubble up above an object event on the current map.
void EmoteWheel_ShowBalloon(u8 localId, u8 emote);

#endif // GUARD_EMOTE_WHEEL_H
