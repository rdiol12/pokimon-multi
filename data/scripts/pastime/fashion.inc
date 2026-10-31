@ Pokémon fashion (src/pastime_fashion.c), at Slateport's Pokémon Fan Club.

Fashion_EventScript_Stylist::
	lock
	faceplayer
	msgbox Fashion_Text_Welcome, MSGBOX_DEFAULT
Fashion_EventScript_Menu:
	message Fashion_Text_WhatToDo
	waitmessage
	dynmultipush Fashion_Text_Buy, FASHION_MENU_BUY
	dynmultipush Fashion_Text_Dress, FASHION_MENU_DRESS
	dynmultipush Fashion_Text_Judge, FASHION_MENU_JUDGE
	dynmultipush Fashion_Text_Cancel, FASHION_MENU_CANCEL
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case FASHION_MENU_BUY, Fashion_EventScript_Buy
	case FASHION_MENU_DRESS, Fashion_EventScript_Dress
	case FASHION_MENU_JUDGE, Fashion_EventScript_Judge
	goto Fashion_EventScript_Bye

Fashion_EventScript_Buy:
	message Fashion_Text_WhichBuy
	waitmessage
	callnative Fashion_PushBuyMenu
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, FASHION_CANCEL, Fashion_EventScript_Menu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Fashion_EventScript_Menu
	copyvar VAR_0x8005, VAR_RESULT
	callnative Fashion_Buy
	goto_if_eq VAR_RESULT, FASHION_BUY_NO_MONEY, Fashion_EventScript_NoMoney
	playse SE_SHOP
	msgbox Fashion_Text_Bought, MSGBOX_DEFAULT
	goto Fashion_EventScript_Menu

Fashion_EventScript_NoMoney:
	msgbox Fashion_Text_NoMoney, MSGBOX_DEFAULT
	goto Fashion_EventScript_Menu

Fashion_EventScript_Dress:
	message Fashion_Text_WhichWear
	waitmessage
	callnative Fashion_PushDressMenu
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, FASHION_CANCEL, Fashion_EventScript_Menu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Fashion_EventScript_Menu
	copyvar VAR_0x8005, VAR_RESULT
	callnative Fashion_Dress
	goto_if_eq VAR_0x8005, FASHION_TAKE_OFF, Fashion_EventScript_TookOff
	playse SE_M_ATTRACT
	msgbox Fashion_Text_Dressed, MSGBOX_DEFAULT
	goto Fashion_EventScript_Menu

Fashion_EventScript_TookOff:
	msgbox Fashion_Text_TookOff, MSGBOX_DEFAULT
	goto Fashion_EventScript_Menu

Fashion_EventScript_Judge:
	callnative Fashion_Judge
	msgbox Fashion_Text_Theme, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FASHION_JUDGE_NO_MON, Fashion_EventScript_NoLook
	goto_if_eq VAR_RESULT, FASHION_JUDGE_DONE, Fashion_EventScript_JudgedToday
	goto_if_eq VAR_RESULT, FASHION_JUDGE_MISS, Fashion_EventScript_Miss
	playfanfare MUS_LEVEL_UP
	message Fashion_Text_Match
	waitmessage
	waitfanfare
	waitbuttonpress
	giveitem VAR_0x8005
	goto Fashion_EventScript_Bye

Fashion_EventScript_NoLook:
	msgbox Fashion_Text_NoLook, MSGBOX_DEFAULT
	goto Fashion_EventScript_Menu

Fashion_EventScript_JudgedToday:
	msgbox Fashion_Text_JudgedToday, MSGBOX_DEFAULT
	goto Fashion_EventScript_Bye

Fashion_EventScript_Miss:
	msgbox Fashion_Text_Miss, MSGBOX_DEFAULT
	goto Fashion_EventScript_Menu

Fashion_EventScript_Bye:
	msgbox Fashion_Text_Bye, MSGBOX_DEFAULT
	release
	end

Fashion_Text_Welcome:
	.string "Darling! I'm the FAN CLUB's stylist.\n"
	.string "Every POKéMON deserves to look its\l"
	.string "very best!$"

Fashion_Text_WhatToDo:
	.string "What shall we do today?$"

Fashion_Text_Buy:
	.string "BUY$"
Fashion_Text_Dress:
	.string "DRESS UP$"
Fashion_Text_Judge:
	.string "TODAY'S LOOK$"
Fashion_Text_Cancel:
	.string "CANCEL$"

Fashion_Text_WhichBuy:
	.string "Which one catches your eye?$"

Fashion_Text_Bought:
	.string "The {STR_VAR_1}! A fine choice.$"

Fashion_Text_NoMoney:
	.string "Oh dear, you can't quite afford\n"
	.string "the {STR_VAR_1}.$"

Fashion_Text_WhichWear:
	.string "What should your lead POKéMON wear?$"

Fashion_Text_Dressed:
	.string "{STR_VAR_1} is wearing the {STR_VAR_2}.\n"
	.string "Simply darling!$"

Fashion_Text_TookOff:
	.string "{STR_VAR_1} is back to its natural\n"
	.string "look. Lovely all the same!$"

Fashion_Text_Theme:
	.string "Today's theme is {STR_VAR_1}!$"

Fashion_Text_NoLook:
	.string "Your lead POKéMON isn't wearing\n"
	.string "anything! Dress it up first.$"

Fashion_Text_JudgedToday:
	.string "I've already given out today's prize\n"
	.string "to you. Come back tomorrow!$"

Fashion_Text_Miss:
	.string "Hmm… Pretty, but not quite today's\n"
	.string "theme. Try another look!$"

Fashion_Text_Match:
	.string "{STR_VAR_2} is the very picture of\n"
	.string "{STR_VAR_1}! Here's today's prize!$"

Fashion_Text_Bye:
	.string "Do come again, darling!$"
