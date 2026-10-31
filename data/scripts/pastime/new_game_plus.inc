@ New Game+ (src/pastime_new_game_plus.c), from R > PASTIMES.

Pastime_EventScript_NewGamePlus::
	callnative NewGamePlus_GetState
	goto_if_eq VAR_RESULT, FALSE, NewGamePlus_EventScript_NotYet
	msgbox NewGamePlus_Text_Explain, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NewGamePlus_EventScript_Done
	msgbox NewGamePlus_Text_Sure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NewGamePlus_EventScript_Done
	closemessage
	callnative NewGamePlus_Start, waitstate=1
NewGamePlus_EventScript_Done:
	releaseall
	end

NewGamePlus_EventScript_NotYet:
	msgbox NewGamePlus_Text_NotYet, MSGBOX_DEFAULT
	releaseall
	end

NewGamePlus_Text_NotYet:
	.string "NEW GAME+ opens once you become\n"
	.string "the POKéMON LEAGUE CHAMPION!$"

NewGamePlus_Text_Explain:
	.string "NEW GAME+ starts your journey over\n"
	.string "from the moving truck.\p"
	.string "You keep all your POKéMON (your party\n"
	.string "goes into the PC), your money, your\l"
	.string "POKéDEX, achievements and records.\p"
	.string "Badges, items and the story start\n"
	.string "fresh. New Game+ played: {STR_VAR_1}.\p"
	.string "Start a NEW GAME+?$"

NewGamePlus_Text_Sure:
	.string "Your old adventure stays saved until\n"
	.string "you save the new one. Ready?$"
