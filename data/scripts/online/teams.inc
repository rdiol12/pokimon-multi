@ 2v2 team ladder: form a team with another player; the battles themselves
@ start from talking to a player of another team ("2v2 team battle").

Online_EventScript_Teams::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_TEAM_VIEW
	switch VAR_0x800B
	case TEAM_INVITED, Online_EventScript_TeamInvited
	case TEAM_ASKED, Online_EventScript_TeamLadder
	case TEAM_TEAMED, Online_EventScript_TeamMenu
	msgbox gStringVar4, MSGBOX_DEFAULT
	message Online_Text_TeamWhat
	waitmessage
	dynmultipush Online_Text_TeamForm, 0
	dynmultipush Online_Text_TeamLadder, 1
	dynmultipush Online_Text_TeamCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Online_EventScript_TeamBoard
	goto_if_ne VAR_RESULT, 0, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_TEAM_WHO
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_TEAM_ASK
	goto Online_EventScript_ShowMessage

Online_EventScript_TeamInvited:
	msgbox gStringVar4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_TeamDecline
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_TEAM_YES
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_EventScript_TeamDecline:
	onl_request ONL_REQ_TEAM_NO
	goto Online_EventScript_End

Online_EventScript_TeamLadder:
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Online_EventScript_TeamBoard

Online_EventScript_TeamMenu:
	msgbox gStringVar4, MSGBOX_DEFAULT
	message Online_Text_TeamWhat
	waitmessage
	dynmultipush Online_Text_TeamLadder, 1
	dynmultipush Online_Text_TeamSplit, 3
	dynmultipush Online_Text_TeamCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Online_EventScript_TeamBoard
	goto_if_ne VAR_RESULT, 3, Online_EventScript_End
	msgbox Online_Text_TeamSure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_TEAM_LEAVE
	goto Online_EventScript_ShowMessage

Online_EventScript_TeamBoard:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_TEAM_BOARD
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto Online_EventScript_End

Online_Text_TeamWhat:
	.string "What would you like to do?$"

Online_Text_TeamForm:
	.string "FORM A TEAM$"

Online_Text_TeamLadder:
	.string "TEAM LADDER$"

Online_Text_TeamSplit:
	.string "SPLIT UP$"

Online_Text_TeamCancel:
	.string "CANCEL$"

Online_Text_TeamSure:
	.string "Split up with {STR_VAR_1}?$"
