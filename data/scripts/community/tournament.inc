@ Online tournaments: START > ONLINE > COMMUNITY > TOURNAMENT.
@ The server (server/community_tournament.py) writes the text.

	.set TOUR_NOT_IN, 2
	.set TOUR_SIGNED_UP, 3
	.set TOUR_PLAYING, 4
	.set TOUR_PRIZE, 5
	.set TOUR_OUT, 6

Community_EventScript_Tournament::
	lockall
	setvar VAR_0x8004, COMMUNITY_REQ_TOUR_STATUS
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	copyvar VAR_0x8008, VAR_RESULT
	callnative Community_BufferRows
	goto_if_eq VAR_0x8008, TOUR_PRIZE, Community_EventScript_TourPrize
	msgbox Community_Text_TourStatus, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8008, TOUR_SIGNED_UP, Community_EventScript_TourSignedUp
	goto_if_eq VAR_0x8008, TOUR_PLAYING, Community_EventScript_TourBracketOrDone
	msgbox Community_Text_TourJoin, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Community_EventScript_TourBracketOrDone
	setvar VAR_0x8004, COMMUNITY_REQ_TOUR_JOIN
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	callnative Community_BufferRows
	playfanfare MUS_OBTAIN_ITEM
	msgbox Community_Text_TourJoined, MSGBOX_DEFAULT
	waitfanfare
	goto Community_EventScript_TourBracketOrDone

Community_EventScript_TourSignedUp:
	msgbox Community_Text_TourWithdraw, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Community_EventScript_TourBracketOrDone
	setvar VAR_0x8004, COMMUNITY_REQ_TOUR_LEAVE
	callnative Community_ScriptRequest, waitstate=1
	msgbox Community_Text_TourWithdrawn, MSGBOX_DEFAULT
	releaseall
	end

Community_EventScript_TourBracketOrDone:
	msgbox Community_Text_TourSeeBracket, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Community_EventScript_Done
	setvar VAR_0x8004, COMMUNITY_REQ_TOUR_BRACKET
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	callnative Community_ShowRows, waitstate=1
	releaseall
	end

Community_EventScript_TourPrize:
	msgbox Community_Text_TourStatus, MSGBOX_DEFAULT
	setvar VAR_0x8004, 2
	callnative Community_GetRespArg
	copyvar VAR_0x8006, VAR_RESULT
	giveitem VAR_0x8006
	releaseall
	end

@ Shown when the server pairs you with someone (src/community_tournament.c).
Community_EventScript_TourMatchReady::
	lockall
	playse SE_PIN
	msgbox Community_Text_TourMatchReady, MSGBOX_DEFAULT
	releaseall
	end

Community_Text_TourStatus:
	.string "{STR_VAR_1}\n"
	.string "{STR_VAR_2}\p"
	.string "{STR_VAR_3}$"

Community_Text_TourJoin:
	.string "Sign up for the tournament?$"

Community_Text_TourJoined:
	.string "{STR_VAR_1}$"

Community_Text_TourWithdraw:
	.string "Withdraw from the tournament?$"

Community_Text_TourWithdrawn:
	.string "You've withdrawn from the cup.$"

Community_Text_TourSeeBracket:
	.string "See the bracket?$"

Community_Text_TourMatchReady:
	.string "TOURNAMENT: {STR_VAR_2}\p"
	.string "Your match against {STR_VAR_1} is ready!\n"
	.string "Find them and challenge them.$"
