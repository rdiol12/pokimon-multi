#ifndef GUARD_DAILY_QUEST_H
#define GUARD_DAILY_QUEST_H

#include "constants/adventure.h"

// Daily quests from three people in Petalburg, Slateport and Fortree
// (docs/adventure.md).

void DailyQuest_Talk(void);
void DailyQuest_Accept(void);
void DailyQuest_Claim(void);
void DailyQuest_ShowList(void);
void DailyQuest_Update(void);
void DailyQuest_OnCatch(u16 species);
void DailyQuest_OnTrainerWin(void);

#endif // GUARD_DAILY_QUEST_H
