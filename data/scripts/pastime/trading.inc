@ Trading post (src/pastime_trading.c), at the end of the Slateport market.

Trading_EventScript_Trader::
	lock
	faceplayer
	msgbox Trading_Text_Welcome, MSGBOX_DEFAULT
Trading_EventScript_Menu:
	message Trading_Text_WhatTrade
	waitmessage
	callnative Trading_PushMenu
	dynmultistack 0, 0, FALSE, 5, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, TRADING_MENU_CANCEL, Trading_EventScript_Bye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Trading_EventScript_Bye
	goto_if_eq VAR_RESULT, TRADING_MENU_SWAP, Trading_EventScript_Swap
	copyvar VAR_0x8005, VAR_RESULT
	callnative Trading_CheckBarter
	goto_if_eq VAR_RESULT, TRADING_DONE, Trading_EventScript_AlreadyDone
	goto_if_eq VAR_RESULT, TRADING_MISSING, Trading_EventScript_Missing
	msgbox gPastimeText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Trading_EventScript_Menu
	callnative Trading_DoBarter
	giveitem VAR_0x8006, VAR_0x8007
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox Trading_Text_Deal, MSGBOX_DEFAULT
	goto Trading_EventScript_Menu

Trading_EventScript_AlreadyDone:
	msgbox Trading_Text_AlreadyDone, MSGBOX_DEFAULT
	goto Trading_EventScript_Menu

Trading_EventScript_Missing:
	msgbox gPastimeText, MSGBOX_DEFAULT
	msgbox Trading_Text_Missing, MSGBOX_DEFAULT
	goto Trading_EventScript_Menu

Trading_EventScript_Swap:
	callnative Trading_GetSwap
	goto_if_eq VAR_RESULT, FALSE, Trading_EventScript_SwapDone
	msgbox Trading_Text_SwapOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Trading_EventScript_Menu
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Trading_EventScript_Menu
	callnative Trading_CheckSwapMon
	goto_if_eq VAR_RESULT, TRADING_EGG, Trading_EventScript_SwapEgg
	goto_if_eq VAR_RESULT, TRADING_MISSING, Trading_EventScript_SwapWrongType
	callnative Trading_DoSwap
	playse SE_EXP_MAX
	waitse
	playfanfare MUS_OBTAIN_ITEM
	message Trading_Text_Swapped
	waitmessage
	waitfanfare
	msgbox Trading_Text_TakeCare, MSGBOX_DEFAULT
	release
	end

Trading_EventScript_SwapDone:
	msgbox Trading_Text_SwapDone, MSGBOX_DEFAULT
	goto Trading_EventScript_Menu

Trading_EventScript_SwapEgg:
	msgbox Trading_Text_SwapEgg, MSGBOX_DEFAULT
	goto Trading_EventScript_Menu

Trading_EventScript_SwapWrongType:
	msgbox Trading_Text_SwapWrongType, MSGBOX_DEFAULT
	goto Trading_EventScript_Menu

Trading_EventScript_Bye:
	msgbox Trading_Text_Bye, MSGBOX_DEFAULT
	release
	end

Trading_Text_Welcome:
	.string "Welcome to the TRADING POST!\n"
	.string "New deals every day!$"

Trading_Text_WhatTrade:
	.string "Here's what I've got today.$"

Trading_Text_Deal:
	.string "Pleasure doing business!$"

Trading_Text_AlreadyDone:
	.string "We already made that deal today.\n"
	.string "Come back tomorrow for new ones!$"

Trading_Text_Missing:
	.string "Looks like you don't have enough.$"

Trading_Text_SwapOffer:
	.string "Today I want a {STR_VAR_1}-type\n"
	.string "Pokémon. I'll give you a {STR_VAR_2}\l"
	.string "from far away for it. Deal?$"

Trading_Text_SwapDone:
	.string "I've made today's swap already.\n"
	.string "Come back tomorrow!$"

Trading_Text_SwapEgg:
	.string "An Egg? I can't trade for that!$"

Trading_Text_SwapWrongType:
	.string "{STR_VAR_3} isn't a {STR_VAR_1} type.\n"
	.string "That's not what I'm after.$"

Trading_Text_Swapped:
	.string "{PLAYER} traded {STR_VAR_3}\n"
	.string "for {STR_VAR_2}!$"

Trading_Text_TakeCare:
	.string "Take good care of {STR_VAR_2}!\n"
	.string "It's holding a little gift.$"

Trading_Text_Bye:
	.string "Come back tomorrow for new deals!$"
