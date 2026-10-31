@ Player-made quests: hide items from the bag where you stand, with a hint,
@ or take someone's quest and go find their treasure.

Online_EventScript_PQuests::
	dynmultipush Online_Text_PQuestFind, 0
	dynmultipush Online_Text_PQuestHide, 1
	dynmultipush Online_Text_PQuestMine, 2
	dynmultipush Online_Text_HubClose, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Online_EventScript_PQuestFind
	case 1, Online_EventScript_PQuestHide
	case 2, Online_EventScript_PQuestMine
	goto Online_EventScript_End

Online_EventScript_PQuestFind:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PQUEST_LIST
	goto_if_eq VAR_0x800B, 1, Online_EventScript_PQuestTaken
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PQUEST_SHOW
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Online_Text_PQuestTake, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PQUEST_TAKE
	goto Online_EventScript_ShowMessage

@ Already on a quest: what it is, and a chance to give up.
Online_EventScript_PQuestTaken:
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Online_Text_PQuestGiveUp, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	onl_request_or_fail ONL_REQ_PQUEST_DROP
	goto Online_EventScript_ShowMessage

Online_EventScript_PQuestHide:
	message Online_Text_PQuestWhich
	waitmessage
	callnative Online_ChooseBagItem, waitstate=1
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	message Online_Text_PQuestHowMany
	waitmessage
	call Online_EventScript_ShopCount
	goto_if_eq VAR_RESULT, 3, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PQUEST_HINTS
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PQUEST_MAKE
	playse SE_M_DIG
	goto Online_EventScript_ShowMessage

Online_EventScript_PQuestMine:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PQUEST_MINE
	goto_if_eq VAR_0x800A, 0, Online_EventScript_ShowMessage
	msgbox gStringVar4, MSGBOX_DEFAULT
	message Online_Text_PQuestDigUp
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PQUEST_CANCEL
	goto Online_EventScript_ShowMessage

@ Started by the game (src/online_pquests.c) as the player gets close.
Online_EventScript_PQuestOnMap::
	lockall
	playse SE_PIN
	msgbox Online_Text_PQuestOnMap, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Online_EventScript_PQuestClose::
	lockall
	playse SE_SUPER_EFFECTIVE
	msgbox Online_Text_PQuestClose, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Online_EventScript_PQuestSpot::
	lockall
	delay 20
	onl_request ONL_REQ_PQUEST_FIND
	goto_if_ne VAR_RESULT, ONLINE_OK, Online_EventScript_PQuestMissed
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_EventScript_PQuestMissed:
	callnative Online_PlayerQuestRetry
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Online_EventScript_End

Online_Text_PQuestFind:
	.string "FIND A TREASURE$"

Online_Text_PQuestHide:
	.string "HIDE A TREASURE$"

Online_Text_PQuestMine:
	.string "MY TREASURES$"

Online_Text_PQuestTake:
	.string "Look for it?$"

Online_Text_PQuestGiveUp:
	.string "Give up this quest?$"

Online_Text_PQuestWhich:
	.string "What will you hide here?$"

Online_Text_PQuestHowMany:
	.string "How many {STR_VAR_1}?$"

Online_Text_PQuestDigUp:
	.string "Dig one up again?$"

Online_Text_PQuestOnMap:
	.string "This is where the treasure is hidden!\n"
	.string "Look around…$"

Online_Text_PQuestClose:
	.string "Something glints nearby!$"
