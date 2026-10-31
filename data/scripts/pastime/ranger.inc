@ Ranger missions (src/pastime_ranger.c), from the RANGER in Fortree.

Ranger_EventScript_Ranger::
	lock
	faceplayer
	callnative Ranger_GetState
	goto_if_eq VAR_RESULT, RANGER_STATE_ACTIVE, Ranger_EventScript_Active
	goto_if_eq VAR_RESULT, RANGER_STATE_COMPLETE, Ranger_EventScript_Complete
	goto_if_eq VAR_RESULT, RANGER_STATE_DONE_TODAY, Ranger_EventScript_DoneToday
	msgbox Ranger_Text_Hello, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Ranger_EventScript_Bye
	callnative Ranger_StartMission
	msgbox gPastimeText, MSGBOX_DEFAULT
	msgbox Ranger_Text_GoodLuck, MSGBOX_DEFAULT
	release
	end

Ranger_EventScript_Active:
	msgbox Ranger_Text_Active, MSGBOX_DEFAULT
	msgbox gPastimeText, MSGBOX_DEFAULT
	release
	end

Ranger_EventScript_Complete:
	msgbox Ranger_Text_Complete, MSGBOX_DEFAULT
	callnative Ranger_CompleteMission
	msgbox Ranger_Text_Points, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Ranger_EventScript_Bye
	playfanfare MUS_LEVEL_UP
	message Ranger_Text_RankUp
	waitmessage
	waitfanfare
	waitbuttonpress
	giveitem VAR_0x8005
	release
	end

Ranger_EventScript_DoneToday:
	msgbox Ranger_Text_DoneToday, MSGBOX_DEFAULT
	release
	end

Ranger_EventScript_Bye:
	msgbox Ranger_Text_Bye, MSGBOX_DEFAULT
	release
	end

Ranger_EventScript_LostMon::
	lock
	faceplayer
	callnative Ranger_IsLostMonWaiting
	waitmoncry
	goto_if_eq VAR_RESULT, FALSE, Ranger_EventScript_LostMonWaiting
	callnative Ranger_RescueLostMon
	msgbox Ranger_Text_LostMonFound, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox Ranger_Text_LostMonHome, MSGBOX_DEFAULT
	release
	end

Ranger_EventScript_LostMonWaiting:
	release
	end

Ranger_Text_Hello:
	.string "I'm the RANGER of FORTREE.\n"
	.string "You're a {STR_VAR_1}, right?\p"
	.string "Pokémon and people need our help!\n"
	.string "Will you take a mission?$"

Ranger_Text_GoodLuck:
	.string "Come back to me when it's done.\n"
	.string "Good luck, Ranger!$"

Ranger_Text_Active:
	.string "How's your mission going?$"

Ranger_Text_Complete:
	.string "Mission complete? Great work!\n"
	.string "The region thanks you.$"

Ranger_Text_Points:
	.string "You earned {STR_VAR_2} Ranger Points!$"

Ranger_Text_RankUp:
	.string "You've been promoted to\n"
	.string "{STR_VAR_1}! Take this!$"

Ranger_Text_DoneToday:
	.string "That's three missions today.\n"
	.string "Rest up, Ranger!$"

Ranger_Text_Bye:
	.string "Come back any time!$"

Ranger_Text_LostMonFound:
	.string "The lost {STR_VAR_1} ran up to you!\n"
	.string "You called the RANGER…$"

Ranger_Text_LostMonHome:
	.string "The {STR_VAR_1} is on its way home.\n"
	.string "Report back to the RANGER!$"
