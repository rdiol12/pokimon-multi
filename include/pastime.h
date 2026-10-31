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

#endif // GUARD_PASTIME_H
