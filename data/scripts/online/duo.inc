@ Duo quests: team up with another player and catch Pokémon together.

Online_EventScript_Duo::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_DUO_VIEW
	switch VAR_0x800B
	case DUO_INVITED, Online_EventScript_DuoInvited
	case DUO_ASKED, Online_EventScript_ShowMessage
	case DUO_ACTIVE, Online_EventScript_DuoBoard
	case DUO_REWARD, Online_EventScript_DuoBoard
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Online_Text_DuoFind, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_DUO_WHO
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_DUO_ASK
	goto Online_EventScript_ShowMessage

Online_EventScript_DuoInvited:
	msgbox gStringVar4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_DuoDecline
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_DUO_YES
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_EventScript_DuoDecline:
	onl_request ONL_REQ_DUO_NO
	goto Online_EventScript_End

Online_EventScript_DuoBoard:
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto_if_ne VAR_0x800B, DUO_REWARD, Online_EventScript_DuoMenu
	msgbox Online_Text_DuoReward, MSGBOX_DEFAULT
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_DUO_CLAIM
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
Online_EventScript_DuoMenu:
	message Online_Text_DuoKeepGoing
	waitmessage
	dynmultipush Online_Text_DuoKeep, 0
	dynmultipush Online_Text_DuoSplit, 1
	dynmultistack 0, 0, FALSE, 2, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_ne VAR_RESULT, 1, Online_EventScript_End
	msgbox Online_Text_DuoSure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_DUO_LEAVE
	goto Online_EventScript_ShowMessage

Online_Text_DuoFind:
	.string "Look for a partner?$"

Online_Text_DuoReward:
	.string "You finished a duo quest!\n"
	.string "Here's your reward!$"

Online_Text_DuoKeepGoing:
	.string "Good luck with the quest!$"

Online_Text_DuoKeep:
	.string "KEEP GOING$"

Online_Text_DuoSplit:
	.string "SPLIT UP$"

Online_Text_DuoSure:
	.string "Split up with {STR_VAR_1}?$"
