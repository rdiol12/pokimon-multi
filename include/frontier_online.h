#ifndef GUARD_FRONTIER_ONLINE_H
#define GUARD_FRONTIER_ONLINE_H

// The online Battle Frontier (src/frontier_online.c).

void FrontierOnline_Update(void);
void FrontierOnline_FormatScore(u8 *dest, u8 board, u32 value, u32 extra);
void FrontierOnline_Showcase(u32 factory, u32 arena, u32 pyramid);

#endif // GUARD_FRONTIER_ONLINE_H
