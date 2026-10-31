@ Detective cases (src/pastime_detective.c), from the agency in Mauville.

Detective_EventScript_Detective::
	lock
	faceplayer
	callnative Detective_GetState
	goto_if_eq VAR_RESULT, DETECTIVE_OPEN, Detective_EventScript_Open
	goto_if_eq VAR_RESULT, DETECTIVE_ALL_SOLVED, Detective_EventScript_AllSolved
	msgbox Detective_Text_NewCase, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Detective_EventScript_Bye
	callnative Detective_OpenCase
	playse SE_SELECT
	msgbox gPastimeText, MSGBOX_DEFAULT
	msgbox Detective_Text_GoAsk, MSGBOX_DEFAULT
	release
	end

Detective_EventScript_Open:
	msgbox Detective_Text_Open, MSGBOX_DEFAULT
	message Detective_Text_WhoDidIt
	waitmessage
	callnative Detective_PushSuspects
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, DETECTIVE_MENU_NOT_YET, Detective_EventScript_Bye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Detective_EventScript_Bye
	callnative Detective_Accuse
	goto_if_eq VAR_RESULT, FALSE, Detective_EventScript_Wrong
	msgbox gPastimeText, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_BADGE
	message Detective_Text_Solved
	waitmessage
	waitfanfare
	waitbuttonpress
	giveitem VAR_0x8005
	release
	end

Detective_EventScript_Wrong:
	playse SE_FAILURE
	msgbox Detective_Text_Wrong, MSGBOX_DEFAULT
	release
	end

Detective_EventScript_AllSolved:
	msgbox Detective_Text_AllSolved, MSGBOX_DEFAULT
	release
	end

Detective_EventScript_Bye:
	msgbox Detective_Text_Bye, MSGBOX_DEFAULT
	release
	end

Detective_EventScript_Witness::
	lock
	faceplayer
	callnative Detective_TalkToWitness
	msgbox gPastimeText, MSGBOX_DEFAULT
	release
	end

Detective_Text_NewCase:
	.string "DETECTIVE AGENCY. Ah, a new\n"
	.string "assistant! I have a case for you:\p"
	.string "“{STR_VAR_1}”.\n"
	.string "Will you take it?$"

Detective_Text_GoAsk:
	.string "Go and ask around. Three witnesses\n"
	.string "saw something. Then tell me who\l"
	.string "did it!$"

Detective_Text_Open:
	.string "“{STR_VAR_1}”…\n"
	.string "You've found {STR_VAR_2} of 3 clues.$"

Detective_Text_WhoDidIt:
	.string "So, who did it?$"

Detective_Text_Solved:
	.string "Case closed! Fine work, {PLAYER}!\n"
	.string "You're a {STR_VAR_3} now.$"

Detective_Text_Wrong:
	.string "{STR_VAR_1}? No… That doesn't fit the\n"
	.string "evidence. The trail's gone cold.\p"
	.string "Come back and we'll open the case\n"
	.string "again from the start.$"

Detective_Text_AllSolved:
	.string "Every case solved! HOENN is safe,\n"
	.string "thanks to you, ACE DETECTIVE!$"

Detective_Text_Bye:
	.string "A good detective never rushes.$"
