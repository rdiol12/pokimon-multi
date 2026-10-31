@ Mount racing: HUB > MOUNT RACING picks a course and joins its next heat;
@ src/online_racing.c runs the race and starts the scripts below.

Online_EventScript_Racing::
	callnative Online_RaceIsOnScript
	goto_if_eq VAR_RESULT, TRUE, Online_EventScript_RaceGiveUp
	callnative Online_HasMount
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_RaceNoMount
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_RACE_LIST
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	message Online_Text_RaceWhat
	waitmessage
	dynmultipush Online_Text_RaceEnter, 0
	dynmultipush Online_Text_RaceTimes, 1
	dynmultipush gText_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Online_EventScript_RaceBoard
	goto_if_ne VAR_RESULT, 0, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_RACE_JOIN
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	playse SE_WARP_IN
	callnative Online_RaceStart
	waitstate
	releaseall
	end

Online_EventScript_RaceBoard:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_RACE_BOARD
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto Online_EventScript_End

Online_EventScript_RaceNoMount:
	msgbox Online_Text_RaceNoMount, MSGBOX_DEFAULT
	goto Online_EventScript_End

Online_EventScript_RaceGiveUp:
	msgbox Online_Text_RaceGiveUp, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	onl_request ONL_REQ_RACE_QUIT
	closemessage
	callnative Online_RaceEnd
	waitstate
	releaseall
	end

Online_EventScript_RaceCountdown::
	lockall
	callnative Online_RaceCountdown, waitstate=1
	releaseall
	end

Online_EventScript_RaceFinish::
	lockall
	callnative Online_RaceSetResult
	playfanfare MUS_OBTAIN_BADGE
	message Online_Text_RaceFinish
	waitmessage
	waitfanfare
	call Online_EventScript_Connecting
	onl_request ONL_REQ_RACE_FINISH
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	callnative Online_RaceEnd
	waitstate
	releaseall
	end

Online_Text_RaceWhat:
	.string "What would you like to do?$"

Online_Text_RaceEnter:
	.string "ENTER THE RACE$"

Online_Text_RaceTimes:
	.string "BEST TIMES$"

Online_Text_RaceNoMount:
	.string "You need a POKéMON you can ride\n"
	.string "to race! Like a RAPIDASH or ARCANINE.$"

Online_Text_RaceGiveUp:
	.string "Give up the race and go back?$"

Online_Text_RaceFinish:
	.string "FINISH! Your time: {STR_VAR_1}!$"
