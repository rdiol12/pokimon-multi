@ Cooking contest (src/pastime_cooking.c), next to CAFÉ SOLEIL in Lilycove.

Cook_EventScript_Judge::
	lock
	faceplayer
	callnative Cook_CheckEntry
	goto_if_eq VAR_RESULT, FALSE, Cook_EventScript_Tomorrow
	msgbox Cook_Text_Welcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Cook_EventScript_Bye
Cook_EventScript_PickDish:
	message Cook_Text_WhichDish
	waitmessage
	callnative Cafe_PushMenu
	dynmultistack 0, 0, FALSE, 7, FALSE, 1, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, CAFE_MENU_CANCEL, Cook_EventScript_Bye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Cook_EventScript_Bye
	copyvar VAR_0x8005, VAR_RESULT
	callnative Cook_CheckDish
	goto_if_eq VAR_RESULT, FALSE, Cook_EventScript_Missing
	msgbox Cook_Text_Start, MSGBOX_DEFAULT
	closemessage
	callnative Cook_Play, waitstate=1
	callnative Cook_Judge
	msgbox gPastimeText, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Cook_EventScript_Lost
	playfanfare MUS_LEVEL_UP
	message Cook_Text_RankUp
	waitmessage
	waitfanfare
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox Cook_Text_Board, MSGBOX_DEFAULT
	release
	end

Cook_EventScript_Lost:
	msgbox Cook_Text_Lost, MSGBOX_DEFAULT
	release
	end

Cook_EventScript_Missing:
	msgbox Cook_Text_Missing, MSGBOX_DEFAULT
	goto Cook_EventScript_PickDish

Cook_EventScript_Tomorrow:
	msgbox Cook_Text_Tomorrow, MSGBOX_DEFAULT
	release
	end

Cook_EventScript_Bye:
	msgbox Cook_Text_Bye, MSGBOX_DEFAULT
	release
	end

Cook_Text_Welcome:
	.string "Welcome to the LILYCOVE COOK-OFF!\p"
	.string "You're in the {STR_VAR_1} RANK.\n"
	.string "Cook a dish against three chefs!\p"
	.string "Bring the ingredients for one of\n"
	.string "CAFÉ SOLEIL's dishes. Ready?$"

Cook_Text_WhichDish:
	.string "Which dish will you cook?$"

Cook_Text_Missing:
	.string "You don't have the ingredients\n"
	.string "for the {STR_VAR_2}.$"

Cook_Text_Start:
	.string "The {STR_VAR_2}! Chop, stir, plate:\n"
	.string "press A when the needle is green!$"

Cook_Text_RankUp:
	.string "You're now in the {STR_VAR_1} RANK!$"

Cook_Text_Board:
	.string "Your best score is on the COOK-OFF\n"
	.string "CHAMPS board online. Bon appétit!$"

Cook_Text_Lost:
	.string "So close! Come back tomorrow and\n"
	.string "try again.$"

Cook_Text_Tomorrow:
	.string "Today's COOK-OFF is over.\n"
	.string "Come back tomorrow!$"

Cook_Text_Bye:
	.string "The kitchen's always open!$"
