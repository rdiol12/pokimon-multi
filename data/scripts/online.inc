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
	dynmultipush Online_Text_HubCoFish, ONLINE_HUB_COFISH
	dynmultipush Online_Text_HubShops, ONLINE_HUB_SHOPS
	dynmultipush Online_Text_HubPageant, ONLINE_HUB_PAGEANT
	dynmultipush Online_Text_HubDuo, ONLINE_HUB_DUO
	dynmultipush Online_Text_HubMentors, ONLINE_HUB_MENTORS
	dynmultipush Online_Text_HubPQuests, ONLINE_HUB_PQUESTS
	dynmultipush Online_Text_HubBetting, ONLINE_HUB_BETTING
	dynmultipush Online_Text_HubDoubles, ONLINE_HUB_DOUBLES
	dynmultipush Online_Text_HubTeams, ONLINE_HUB_TEAMS
	dynmultipush Online_Text_HubClose, ONLINE_HUB_CLOSE
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case ONLINE_HUB_GOALS, Online_EventScript_Goals
	case ONLINE_HUB_SIGHTINGS, Online_EventScript_Sightings
	case ONLINE_HUB_FISHING, Online_EventScript_Fishing
	case ONLINE_HUB_COFISH, Online_EventScript_CoFish
	case ONLINE_HUB_SHOPS, Online_EventScript_Shops
	case ONLINE_HUB_PAGEANT, Online_EventScript_Pageant
	case ONLINE_HUB_DUO, Online_EventScript_Duo
	case ONLINE_HUB_MENTORS, Online_EventScript_Mentors
	case ONLINE_HUB_PQUESTS, Online_EventScript_PQuests
	case ONLINE_HUB_BETTING, Online_EventScript_Betting
	case ONLINE_HUB_DOUBLES, Online_EventScript_Doubles
	case ONLINE_HUB_TEAMS, Online_EventScript_Teams
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

Online_Text_HubCoFish:
	.string "CO-OP FISHING$"

Online_Text_HubShops:
	.string "PLAYER SHOPS$"

Online_Text_HubPageant:
	.string "POKéMON PAGEANT$"

Online_Text_HubDuo:
	.string "DUO QUESTS$"

Online_Text_HubMentors:
	.string "MENTORS$"

Online_Text_HubPQuests:
	.string "PLAYER QUESTS$"

Online_Text_HubBetting:
	.string "BETTING$"

Online_Text_HubDoubles:
	.string "DOUBLES LADDER$"

Online_Text_HubTeams:
	.string "TEAM LADDER$"

Online_Text_HubClose:
	.string "CLOSE$"

	.include "data/scripts/online/goals.inc"
	.include "data/scripts/online/sightings.inc"
	.include "data/scripts/online/fishing.inc"
	.include "data/scripts/online/coop_fishing.inc"
	.include "data/scripts/online/shops.inc"
	.include "data/scripts/online/pageant.inc"
	.include "data/scripts/online/duo.inc"
	.include "data/scripts/online/mentors.inc"
	.include "data/scripts/online/pquests.inc"
	.include "data/scripts/online/betting.inc"
	.include "data/scripts/online/teams.inc"

@ Ranked doubles: the ladder (the battles start from talking to a player).
Online_EventScript_Doubles::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_DOUBLES_BOARD
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto Online_EventScript_End
