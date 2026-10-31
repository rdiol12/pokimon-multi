@ Battle dojo (src/pastime_dojo.c), outside Dewford Gym.

Dojo_EventScript_Master::
	lock
	faceplayer
	callnative Dojo_GetStatus
	msgbox Dojo_Text_Welcome, MSGBOX_DEFAULT
Dojo_EventScript_Menu:
	callnative Dojo_GetStatus
	message Dojo_Text_WhatToDo
	waitmessage
	dynmultipush Dojo_Text_Train, DOJO_MENU_TRAIN
	dynmultipush Dojo_Text_Learn, DOJO_MENU_LEARN
	dynmultipush Dojo_Text_Cancel, DOJO_MENU_CANCEL
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, DOJO_MENU_TRAIN, Dojo_EventScript_Train
	goto_if_eq VAR_RESULT, DOJO_MENU_LEARN, Dojo_EventScript_Learn
	goto Dojo_EventScript_Bye

Dojo_EventScript_Train:
	callnative Dojo_GetStatus
	goto_if_eq VAR_RESULT, 0, Dojo_EventScript_Tired
	msgbox Dojo_Text_TrainRules, MSGBOX_DEFAULT
	closemessage
	callnative Dojo_Train, waitstate=1
	msgbox Dojo_Text_TrainDone, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Dojo_EventScript_Menu
	playfanfare MUS_LEVEL_UP
	message Dojo_Text_NewBelt
	waitmessage
	waitfanfare
	waitbuttonpress
	goto Dojo_EventScript_Menu

Dojo_EventScript_Tired:
	msgbox Dojo_Text_Tired, MSGBOX_DEFAULT
	goto Dojo_EventScript_Menu

Dojo_EventScript_Learn:
	message Dojo_Text_WhichMove
	waitmessage
	callnative Dojo_PushMoveMenu
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, DOJO_MENU_CANCEL, Dojo_EventScript_Menu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Dojo_EventScript_Menu
	copyvar VAR_0x8007, VAR_RESULT
	callnative Dojo_CheckMove
	goto_if_eq VAR_RESULT, DOJO_NO_BELT, Dojo_EventScript_NoBelt
	goto_if_eq VAR_RESULT, DOJO_NO_POINTS, Dojo_EventScript_NoPoints
	msgbox Dojo_Text_TeachWhich, MSGBOX_DEFAULT
	call MoveTutor_EventScript_OpenBox
	goto_if_eq VAR_RESULT, FALSE, Dojo_EventScript_Menu
	callnative Dojo_PayForMove
	msgbox Dojo_Text_Taught, MSGBOX_DEFAULT
	goto Dojo_EventScript_Menu

Dojo_EventScript_NoBelt:
	msgbox Dojo_Text_NoBelt, MSGBOX_DEFAULT
	goto Dojo_EventScript_Learn

Dojo_EventScript_NoPoints:
	msgbox Dojo_Text_NoPoints, MSGBOX_DEFAULT
	goto Dojo_EventScript_Learn

Dojo_EventScript_Bye:
	msgbox Dojo_Text_Bye, MSGBOX_DEFAULT
	release
	end

Dojo_Text_Welcome:
	.string "Hoo-hah! Welcome to the BATTLE DOJO!\p"
	.string "You wear the {STR_VAR_1} belt and\n"
	.string "have {STR_VAR_2} Dojo Points.$"

Dojo_Text_WhatToDo:
	.string "Train, or learn a move?$"

Dojo_Text_Train:
	.string "TRAIN$"
Dojo_Text_Learn:
	.string "LEARN A MOVE$"
Dojo_Text_Cancel:
	.string "CANCEL$"

Dojo_Text_TrainRules:
	.string "Five drills! Press A when the\n"
	.string "needle hits green. Hoo-hah!$"

Dojo_Text_TrainDone:
	.string "You earned {STR_VAR_1} Dojo Points!\n"
	.string "You have {STR_VAR_2} now.$"

Dojo_Text_NewBelt:
	.string "Splendid! You've earned the\n"
	.string "{STR_VAR_3} belt!$"

Dojo_Text_Tired:
	.string "That's three sessions today.\n"
	.string "Rest up and come back tomorrow!$"

Dojo_Text_WhichMove:
	.string "Which move shall I teach?$"

Dojo_Text_TeachWhich:
	.string "{STR_VAR_1} for {STR_VAR_3} Dojo Points.\n"
	.string "Any {STR_VAR_2}-type Pokémon can\l"
	.string "learn it. Which one?$"

Dojo_Text_Taught:
	.string "Hoo-hah! Use it well!$"

Dojo_Text_NoBelt:
	.string "{STR_VAR_1} is for the {STR_VAR_2}\n"
	.string "belt and up. Keep training!$"

Dojo_Text_NoPoints:
	.string "{STR_VAR_1} costs {STR_VAR_3} Dojo\n"
	.string "Points. Keep training!$"

Dojo_Text_Bye:
	.string "Train hard! Hoo-hah!$"
