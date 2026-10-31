@ The tournament host in the Verdanturf Battle Tent (src/mechanics_cups.c):
@ level-capped cups of three rounds against Battle Frontier trainers.
Mechanics_EventScript_CupHost::
	lockall
	faceplayer
	goto_if_set FLAG_MECH_MET_CUP_HOST, Mechanics_EventScript_CupMenu
	setflag FLAG_MECH_MET_CUP_HOST
	msgbox Mechanics_Text_CupIntro, MSGBOX_DEFAULT
Mechanics_EventScript_CupMenu:
	message Mechanics_Text_CupWhich
	waitmessage
	callnative Mechanics_PushCupRows
	dynmultipush Mechanics_Text_RefereeNotNow, 0
	dynmultistack 0, 0, FALSE, 4, FALSE, 1, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, Mechanics_EventScript_CupBye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Mechanics_EventScript_CupBye
	copyvar VAR_0x8005, VAR_RESULT
	callnative Mechanics_GetCupHelp
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Mechanics_Text_CupReady, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Mechanics_EventScript_CupMenu
	callnative Mechanics_EnterCup
	goto_if_eq VAR_RESULT, 0, Mechanics_EventScript_CupNoEntrants
	special HealPlayerParty
	msgbox Mechanics_Text_CupBegin, MSGBOX_DEFAULT
	setvar VAR_0x8006, 0
Mechanics_EventScript_CupRound:
	callnative Mechanics_SetupCupRound
	call_if_eq VAR_0x8006, 0, Mechanics_EventScript_CupRound1
	call_if_eq VAR_0x8006, 1, Mechanics_EventScript_CupRound2
	call_if_eq VAR_0x8006, 2, Mechanics_EventScript_CupRound3
	closemessage
	callnative Mechanics_StartFacilityBattle
	waitstate
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, Mechanics_EventScript_CupLost
	special HealPlayerParty
	addvar VAR_0x8006, 1
	goto_if_lt VAR_0x8006, 3, Mechanics_EventScript_CupRound
	callnative Mechanics_WinCup
	msgbox Mechanics_Text_CupChampion, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, ITEM_NONE, Mechanics_EventScript_CupBye
	msgbox Mechanics_Text_CupPrize, MSGBOX_DEFAULT
	giveitem VAR_RESULT
	goto Mechanics_EventScript_CupBye

Mechanics_EventScript_CupRound1:
	msgbox Mechanics_Text_CupRound1, MSGBOX_DEFAULT
	return

Mechanics_EventScript_CupRound2:
	msgbox Mechanics_Text_CupRound2, MSGBOX_DEFAULT
	return

Mechanics_EventScript_CupRound3:
	msgbox Mechanics_Text_CupRound3, MSGBOX_DEFAULT
	return

Mechanics_EventScript_CupLost:
	callnative Mechanics_LoseCup
	special HealPlayerParty
	msgbox Mechanics_Text_CupLost, MSGBOX_DEFAULT
	goto Mechanics_EventScript_CupBye

Mechanics_EventScript_CupNoEntrants:
	msgbox Mechanics_Text_CupNoEntrants, MSGBOX_DEFAULT
	goto Mechanics_EventScript_CupMenu

Mechanics_EventScript_CupBye:
	msgbox Mechanics_Text_CupBye, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_Text_CupIntro:
	.string "Hello, hello! I host the VERDANTURF\n"
	.string "LEVEL CUPS!\p"
	.string "Three rounds in a row, and every\n"
	.string "POKéMON battles at the cup's level\l"
	.string "or lower. Win all three to be the\l"
	.string "champion!\p"
	.string "Your POKéMON go back to normal\n"
	.string "after the cup, and nobody gains EXP.$"

Mechanics_Text_CupWhich:
	.string "Which cup will you enter?$"

Mechanics_Text_CupReady:
	.string "Will you enter this cup?$"

Mechanics_Text_CupNoEntrants:
	.string "Oh dear… None of your POKéMON may\n"
	.string "enter this cup.$"

Mechanics_Text_CupBegin:
	.string "Your POKéMON are now level {STR_VAR_2}\n"
	.string "or lower. Let the {STR_VAR_1} begin!$"

Mechanics_Text_CupRound1:
	.string "ROUND ONE!\n"
	.string "{PLAYER} versus {STR_VAR_3}!$"

Mechanics_Text_CupRound2:
	.string "On to the SEMIFINAL!\n"
	.string "{PLAYER} versus {STR_VAR_3}!$"

Mechanics_Text_CupRound3:
	.string "This is it, the FINAL!\n"
	.string "{PLAYER} versus {STR_VAR_3}!$"

Mechanics_Text_CupChampion:
	.string "And the {STR_VAR_1} champion is…\n"
	.string "{PLAYER}! Congratulations!$"

Mechanics_Text_CupPrize:
	.string "Your first win in this cup earns\n"
	.string "a special prize!$"

Mechanics_Text_CupLost:
	.string "What a fight! Your POKéMON are back\n"
	.string "to their usual levels. Come again!$"

Mechanics_Text_CupBye:
	.string "The cups are always open!$"
