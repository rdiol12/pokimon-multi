@ Mentors: newer trainers pick a mentor; both get gifts for each badge.

Online_EventScript_Mentors::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_MENTOR_VIEW
	switch VAR_0x800B
	case MENTOR_APPRENTICE, Online_EventScript_MentorApprentice
	case MENTOR_ON_LIST, Online_EventScript_MentorMentor
	case MENTOR_OFF_LIST, Online_EventScript_MentorMentor
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Online_Text_MentorFind, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_MENTOR_FIND
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_MENTOR_PICK
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_EventScript_MentorApprentice:
	message gStringVar4
	waitmessage
	dynmultipush Online_Text_MentorGifts, 0
	dynmultipush Online_Text_MentorLeave, 1
	dynmultipush Online_Text_HubClose, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, Online_EventScript_MentorGifts
	goto_if_ne VAR_RESULT, 1, Online_EventScript_End
	msgbox Online_Text_MentorSure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_MENTOR_LEAVE
	goto Online_EventScript_ShowMessage

Online_EventScript_MentorGifts:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_MENTOR_GIFTS
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_EventScript_MentorMentor:
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	message Online_Text_MentorWhat
	waitmessage
	dynmultipush Online_Text_MentorCheer, 0
	dynmultipush Online_Text_MentorGifts, 1
	goto_if_eq VAR_0x800B, MENTOR_ON_LIST, Online_EventScript_MentorOnList
	dynmultipush Online_Text_MentorJoinList, 2
	goto Online_EventScript_MentorMenu
Online_EventScript_MentorOnList:
	dynmultipush Online_Text_MentorLeaveList, 2
Online_EventScript_MentorMenu:
	dynmultipush Online_Text_HubClose, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Online_EventScript_MentorCheer
	case 1, Online_EventScript_MentorGifts
	case 2, Online_EventScript_MentorVolunteer
	goto Online_EventScript_End

Online_EventScript_MentorCheer:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_MENTOR_WHO
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_MENTOR_CHEER
	playse SE_APPLAUSE
	goto Online_EventScript_ShowMessage

Online_EventScript_MentorVolunteer:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_MENTOR_VOLUNTEER
	goto Online_EventScript_ShowMessage

Online_Text_MentorFind:
	.string "Look for a mentor?$"

Online_Text_MentorWhat:
	.string "What will you do?$"

Online_Text_MentorCheer:
	.string "CHEER ON$"

Online_Text_MentorGifts:
	.string "GIFTS$"

Online_Text_MentorLeave:
	.string "SAY GOODBYE$"

Online_Text_MentorJoinList:
	.string "JOIN THE LIST$"

Online_Text_MentorLeaveList:
	.string "LEAVE THE LIST$"

Online_Text_MentorSure:
	.string "Say goodbye to {STR_VAR_1}?$"
