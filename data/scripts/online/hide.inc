@ Hide and seek: HUB > HIDE AND SEEK starts a game (as the seeker) or joins
@ one (to hide); src/online_hide.c runs it and starts the scripts below.
@ Seekers find hiders by talking to them (Netplay_EventScript_TalkToPlayer).

Online_EventScript_Hide::
	callnative Online_HideIsOnScript
	goto_if_eq VAR_RESULT, TRUE, Online_EventScript_HideStop
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HIDE_LIST
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	message Online_Text_HideWhat
	waitmessage
	dynmultipush Online_Text_HidePlay, 0
	dynmultipush Online_Text_HidePoints, 1
	dynmultipush gText_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Online_EventScript_HideBoard
	goto_if_ne VAR_RESULT, 0, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HIDE_JOIN
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	playse SE_WARP_IN
	callnative Online_HideStart
	waitstate
	releaseall
	end

Online_EventScript_HideBoard:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HIDE_BOARD
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto Online_EventScript_End

Online_EventScript_HideStop:
	msgbox Online_Text_HideStop, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	onl_request ONL_REQ_HIDE_QUIT
	closemessage
	callnative Online_HideEnd
	waitstate
	releaseall
	end

@ At the arena: the lobby's countdown, then hiding starts. The seeker
@ counts with their eyes closed.
Online_EventScript_HideLobby::
	lockall
	callnative Online_HideLobbyCountdown, waitstate=1
	call Online_EventScript_Connecting
	onl_request ONL_REQ_HIDE_GO
	goto_if_ne VAR_RESULT, ONLINE_OK, Online_EventScript_HideOff
	goto_if_eq VAR_0x800B, 0, Online_EventScript_HideOff
	playse SE_PIN
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	callnative Online_HideBegin
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_End
	callnative Online_HideSeekerCount, waitstate=1
	playse SE_M_ENCORE2
	msgbox Online_Text_HideReady, MSGBOX_DEFAULT
	goto Online_EventScript_End

Online_EventScript_HideOff:
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	callnative Online_HideEnd
	waitstate
	releaseall
	end

@ The seeker talked to someone.
Online_EventScript_HideTag::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HIDE_TAG
	playse SE_SUCCESS
	msgbox gStringVar4, MSGBOX_DEFAULT
	callnative Online_HideTagged
	goto Online_EventScript_End

Online_EventScript_HideEnd::
	lockall
	playse SE_DING_DONG
	message Online_Text_HideOver
	waitmessage
	waitbuttonpress
	call Online_EventScript_Connecting
	onl_request ONL_REQ_HIDE_END
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	callnative Online_HideEnd
	waitstate
	releaseall
	end

Online_Text_HideWhat:
	.string "What would you like to do?$"

Online_Text_HidePlay:
	.string "PLAY$"

Online_Text_HidePoints:
	.string "POINTS BOARD$"

Online_Text_HideStop:
	.string "Stop playing and go back?$"

Online_Text_HideReady:
	.string "Ready or not, here I come!$"

Online_Text_HideOver:
	.string "Time's up! The game is over.$"
