@ Sixth batch of online features: START > ONLINE > HUB, and the notices the
@ server sends. See src/online.c and docs/online.md.

.macro onl_request req:req
	setvar VAR_0x8004, \req
	callnative Online_Request, waitstate=1
.endm

@ Shows the server's reason and ends the script.
.macro onl_request_or_fail req:req
	onl_request \req
	goto_if_ne VAR_RESULT, ONLINE_OK, Online_EventScript_Failed
.endm

.macro onl_board
	callnative Online_ShowBoard, waitstate=1
.endm

.macro onl_choose
	callnative Online_Choose, waitstate=1
.endm

Online_EventScript_Hub::
	lockall
	dynmultipush Online_Text_HubGoals, ONLINE_HUB_GOALS
	dynmultipush Online_Text_HubSightings, ONLINE_HUB_SIGHTINGS
	dynmultipush Online_Text_HubFishing, ONLINE_HUB_FISHING
	dynmultipush Online_Text_HubClose, ONLINE_HUB_CLOSE
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case ONLINE_HUB_GOALS, Online_EventScript_Goals
	case ONLINE_HUB_SIGHTINGS, Online_EventScript_Sightings
	case ONLINE_HUB_FISHING, Online_EventScript_Fishing
	releaseall
	end

Online_EventScript_Notice::
	lockall
	playse SE_EXP_MAX
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Online_EventScript_Failed::
	msgbox gStringVar4, MSGBOX_DEFAULT
Online_EventScript_End::
	closemessage
	releaseall
	end

Online_EventScript_Connecting::
	message Online_Text_Connecting
	waitmessage
	return

@ Shows the server's message, if it sent one, and ends the script.
Online_EventScript_ShowMessage::
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Online_EventScript_End

Online_Text_Connecting:
	.string "Connecting to the server…$"

Online_Text_HubGoals:
	.string "CATCH GOALS$"

Online_Text_HubSightings:
	.string "LEGEND SIGHTINGS$"

Online_Text_HubFishing:
	.string "FISHING TOURNEY$"

Online_Text_HubClose:
	.string "CLOSE$"

	.include "data/scripts/online/goals.inc"
	.include "data/scripts/online/sightings.inc"
	.include "data/scripts/online/fishing.inc"
