#ifndef GUARD_PASTIME_H
#define GUARD_PASTIME_H

#include "constants/pastime.h"

// Minigames and side activities (docs/pastimes.md). The world features'
// hooks (src/world.c) call these; each activity has its own file.

#define PASTIME_TEXT_SIZE 400
extern u8 gPastimeText[PASTIME_TEXT_SIZE];

void Pastime_Update(void);
bool32 Pastime_TryInteract(s16 x, s16 y);
void Pastime_OnCatch(void);
void Pastime_OnTrainerBattleEnd(void);

// NPCs and objects placed on maps from code. An activity lists them in
// sSpots (src/pastime.c); each shows up while its isShown() says so.
bool32 Pastime_IsPastimeObjectEvent(const struct ObjectEvent *objectEvent);
const u8 *Pastime_GetTalkScript(u8 objectEventId);

// A Pokémon picture box that can share the screen with a multichoice menu.
void Pastime_ShowMonPic(void);
void Pastime_HideMonPic(void);

// Lost and found (pastime_lost.c).
bool32 LostFound_IsItemShown(u32 arg);
bool32 LostFound_IsOwnerShown(u32 arg);
void LostFound_TalkToOwner(void);
void LostFound_Return(void);
void LostFound_PickUp(void);
void LostFound_ShowList(void);

// Museum donations (pastime_museum.c).
bool32 Museum_IsStatueShown(u32 arg);
u16 Museum_GetStatueGraphicsId(u32 arg);
void Museum_FindFossil(void);
void Museum_DonateFossil(void);
void Museum_ShowRare(void);
void Museum_DonatePhotos(void);
void Museum_CheckReward(void);
void Museum_DescribeStatue(void);
void Museum_ShowList(void);

// Delivery jobs (pastime_delivery.c).
void Delivery_Update(void);
void Delivery_Talk(void);
void Delivery_Accept(void);
void Delivery_Deliver(void);
void Delivery_Cancel(void);
void Delivery_ShowList(void);

// Mail courier routes (pastime_mail.c).
bool32 Mail_IsRecipientShown(u32 arg);
void Mail_TalkToPostmaster(void);
void Mail_StartRoute(void);
void Mail_FinishRoute(void);
void Mail_TalkToRecipient(void);
void Mail_ShowList(void);

// Battle quiz hall (pastime_quiz.c).
void Quiz_Start(void);
void Quiz_NextQuestion(void);
void Quiz_Answer(void);
void Quiz_Finish(void);
void Quiz_ShowList(void);

// Pokémon hotel (pastime_hotel.c).
bool32 Hotel_IsGuestShown(u32 arg);
u16 Hotel_GetGuestGraphicsId(u32 arg);
void Hotel_TalkToGuest(void);
void Hotel_ServeGuest(void);
void Hotel_TalkToDesk(void);
void Hotel_Collect(void);
void Hotel_GetUpgrade(void);
void Hotel_Upgrade(void);
void Hotel_ShowList(void);

// Pokémon cafe (pastime_cafe.c).
void Pastime_OnBattleStart(void);
void Cafe_PushMenu(void);
void Cafe_CheckDish(void);
void Cafe_Cook(void);
void Cafe_ShowList(void);
bool32 Cafe_HasIngredients(u32 dish);
void Cafe_UseIngredients(u32 dish);
const u8 *Cafe_GetDishName(u32 dish);
void Cook_CheckEntry(void);
void Cook_CheckDish(void);
void Cook_Play(void);
void Cook_Judge(void);
void Cook_ShowList(void);
void Cafe_OnBattleStart(void);
u16 Pastime_ModifyWildSpecies(u16 species);
u16 BugContest_ModifyWildSpecies(u16 species);
void BugContest_OnBattleStart(void);
void BugContest_OnCatch(u16 species);
void BugContest_Update(void);
void BugContest_GetState(void);
void BugContest_Start(void);
void BugContest_Judge(void);
void BugContest_ShowList(void);
void Trading_PushMenu(void);
void Trading_CheckBarter(void);
void Trading_DoBarter(void);
void Trading_GetSwap(void);
void Trading_CheckSwapMon(void);
void Trading_DoSwap(void);
void Trading_ShowList(void);
void Dojo_GetStatus(void);
void Dojo_Train(void);
void Dojo_PushMoveMenu(void);
void Dojo_CheckMove(void);
void Dojo_PayForMove(void);
void Dojo_ShowList(void);
bool32 Pastime_CanLearnTeachableMove(u16 species, u16 move);
void Pastime_OnWildBattleEnd(void);
void Ranger_OnCatch(u16 species);
void Ranger_OnWildBattleEnd(void);
void Ranger_GetState(void);
void Ranger_StartMission(void);
void Ranger_CompleteMission(void);
void Ranger_RescueLostMon(void);
void Ranger_IsLostMonWaiting(void);
void Ranger_ShowList(void);
void School_PushLessonMenu(void);
void School_StartLesson(void);
void School_NextQuestion(void);
void School_PushAnswers(void);
void School_Answer(void);
void School_Finish(void);
void School_ShowList(void);
void Garden_GetShow(void);
void Garden_CheckShow(void);
void Garden_GiveStarterBerries(void);
void Garden_ShowList(void);
void Garden_Update(void);

// Weather lab (pastime_weather.c).
void Weather_Update(void);
void Weather_GetState(void);
void Weather_Join(void);
void Weather_ShowForecast(void);
void Weather_Report(void);
void Weather_ShowList(void);

// Star map and meteor showers (pastime_stars.c).
void Star_Update(void);
u16 Star_ModifyWildSpecies(u16 species);
void Star_ShowChart(void);
void Star_HideChart(void);
void Star_Look(void);
void Star_PickMeteorite(void);
void Star_TalkToAstronomer(void);
void Star_ShowList(void);

#endif // GUARD_PASTIME_H
