@ Pokémon cafe (src/pastime_cafe.c): CAFÉ SOLEIL in Lilycove.

Cafe_EventScript_Chef::
	lock
	faceplayer
	msgbox Cafe_Text_Welcome, MSGBOX_DEFAULT
Cafe_EventScript_Menu:
	message Cafe_Text_WhatDish
	waitmessage
	callnative Cafe_PushMenu
	dynmultistack 0, 0, FALSE, 7, FALSE, 1, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, CAFE_MENU_CANCEL, Cafe_EventScript_Bye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Cafe_EventScript_Bye
	copyvar VAR_0x8005, VAR_RESULT
	callnative Cafe_CheckDish
	goto_if_eq VAR_RESULT, FALSE, Cafe_EventScript_Missing
	msgbox gPastimeText, MSGBOX_DEFAULT
	msgbox Cafe_Text_Cook, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Cafe_EventScript_Menu
	callnative Cafe_Cook
	playfanfare MUS_OBTAIN_ITEM
	message Cafe_Text_Served
	waitmessage
	waitfanfare
	msgbox Cafe_Text_Ate, MSGBOX_DEFAULT
	release
	end

Cafe_EventScript_Missing:
	msgbox gPastimeText, MSGBOX_DEFAULT
	msgbox Cafe_Text_Missing, MSGBOX_DEFAULT
	goto Cafe_EventScript_Menu

Cafe_EventScript_Bye:
	msgbox Cafe_Text_Bye, MSGBOX_DEFAULT
	release
	end

Cafe_Text_Welcome:
	.string "Bienvenue to CAFÉ SOLEIL!\p"
	.string "Bring me Berries and I'll cook\n"
	.string "something to power up your team.$"

Cafe_Text_WhatDish:
	.string "What shall I cook?$"

Cafe_Text_Cook:
	.string "Shall I cook it for you?$"

Cafe_Text_Missing:
	.string "You're missing an ingredient or two.\n"
	.string "Berry trees and shops have them!$"

Cafe_Text_Served:
	.string "Bon appétit! {PLAYER}'s team\n"
	.string "shared the {STR_VAR_1}!$"

Cafe_Text_Ate:
	.string "{STR_VAR_2}\n"
	.string "It kicks in at the start of battles.$"

Cafe_Text_Bye:
	.string "À bientôt!$"
