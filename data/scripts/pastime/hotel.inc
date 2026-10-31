@ Pokémon hotel (src/pastime_hotel.c): the Starfall Hotel on Starfall Isle.

Hotel_EventScript_Bellhop::
	lock
	faceplayer
	msgbox Hotel_Text_Bellhop, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Hotel_EventScript_BellhopBye
	msgbox Hotel_Text_RightThisWay, MSGBOX_DEFAULT
	closemessage
	playse SE_EXIT
	warp MAP_STARFALL_ISLE_HOTEL, 0
	waitstate
	release
	end

Hotel_EventScript_BellhopBye:
	msgbox Hotel_Text_BellhopBye, MSGBOX_DEFAULT
	release
	end

Hotel_EventScript_Desk::
	lock
	faceplayer
	callnative Hotel_TalkToDesk
	switch VAR_RESULT
	case HOTEL_DESK_DONE, Hotel_EventScript_DeskDone
	case HOTEL_DESK_WAITING, Hotel_EventScript_DeskWaiting
	msgbox Hotel_Text_DeskCollect, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Hotel_EventScript_DeskMenu
	callnative Hotel_Collect
	playse SE_SHOP
	msgbox gPastimeText, MSGBOX_DEFAULT
	msgbox Hotel_Text_CheckedOut, MSGBOX_DEFAULT
	goto Hotel_EventScript_DeskMenu

Hotel_EventScript_DeskWaiting:
	msgbox Hotel_Text_DeskWaiting, MSGBOX_DEFAULT
	goto Hotel_EventScript_DeskMenu

Hotel_EventScript_DeskDone:
	msgbox Hotel_Text_DeskDone, MSGBOX_DEFAULT
Hotel_EventScript_DeskMenu:
	callnative Hotel_GetUpgrade
	goto_if_eq VAR_RESULT, 2, Hotel_EventScript_DeskBye
	msgbox Hotel_Text_Upgrade, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Hotel_EventScript_DeskBye
	callnative Hotel_GetUpgrade
	goto_if_eq VAR_RESULT, FALSE, Hotel_EventScript_NoMoney
	callnative Hotel_Upgrade
	playfanfare MUS_LEVEL_UP
	msgbox Hotel_Text_Upgraded, MSGBOX_DEFAULT
	waitfanfare
Hotel_EventScript_DeskBye:
	msgbox Hotel_Text_DeskBye, MSGBOX_DEFAULT
	release
	end

Hotel_EventScript_NoMoney:
	msgbox Hotel_Text_NoMoney, MSGBOX_DEFAULT
	release
	end

Hotel_EventScript_Guest::
	lock
	faceplayer
	callnative Hotel_TalkToGuest
	switch VAR_RESULT
	case HOTEL_GUEST_SERVED, Hotel_EventScript_GuestServed
	case HOTEL_GUEST_CANT, Hotel_EventScript_GuestCant
	msgbox gPastimeText, MSGBOX_DEFAULT
	msgbox Hotel_Text_Give, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Hotel_EventScript_GuestLater
	callnative Hotel_ServeGuest
	playse SE_SHOP
	msgbox Hotel_Text_Tip, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Hotel_EventScript_GuestDone
	msgbox Hotel_Text_StarGift, MSGBOX_DEFAULT
Hotel_EventScript_GuestDone:
	release
	end

Hotel_EventScript_GuestServed:
	msgbox gPastimeText, MSGBOX_DEFAULT
	release
	end

Hotel_EventScript_GuestCant:
	msgbox gPastimeText, MSGBOX_DEFAULT
	msgbox Hotel_Text_DontHave, MSGBOX_DEFAULT
	release
	end

Hotel_EventScript_GuestLater:
	msgbox Hotel_Text_Later, MSGBOX_DEFAULT
	release
	end

Hotel_Text_Bellhop:
	.string "Welcome back, manager!\n"
	.string "The STARFALL HOTEL awaits.\p"
	.string "Shall we go in?$"

Hotel_Text_RightThisWay:
	.string "Right this way!$"

Hotel_Text_BellhopBye:
	.string "Enjoy the island air, manager!$"

Hotel_Text_DeskCollect:
	.string "Manager! You've served {STR_VAR_1} of\n"
	.string "{STR_VAR_2} guests today.\p"
	.string "Shall we check them out and collect\n"
	.string "today's room money and tips?$"

Hotel_Text_CheckedOut:
	.string "The guests have checked out.\n"
	.string "New ones arrive tomorrow!$"

Hotel_Text_DeskWaiting:
	.string "Manager! {STR_VAR_2} guests are in the\n"
	.string "lobby, and nobody's been served yet.\p"
	.string "Talk to them and bring what they\n"
	.string "ask for. Happy guests pay more!$"

Hotel_Text_DeskDone:
	.string "That's all for today, manager.\n"
	.string "New guests arrive tomorrow!$"

Hotel_Text_Upgrade:
	.string "We're a {STR_VAR_3}-star hotel.\n"
	.string "For ¥{STR_VAR_1}, we can renovate to\l"
	.string "{STR_VAR_2} stars. More guests, better rates!\p"
	.string "Shall we renovate?$"

Hotel_Text_Upgraded:
	.string "The STARFALL HOTEL is now a\n"
	.string "{STR_VAR_2}-star hotel!$"

Hotel_Text_NoMoney:
	.string "We can't afford that yet, manager.$"

Hotel_Text_DeskBye:
	.string "Good luck out there, manager!$"

Hotel_Text_Give:
	.string "Give the {STR_VAR_1}?$"

Hotel_Text_Tip:
	.string "Thank you so much!\n"
	.string "Here's a tip of ¥{STR_VAR_2} for the desk.$"

Hotel_Text_StarGift:
	.string "STEVEN: And take this. A METAL COAT\n"
	.string "from my collection. It's in your BAG.$"

Hotel_Text_DontHave:
	.string "You don't have a {STR_VAR_1}…\n"
	.string "Better find one!$"

Hotel_Text_Later:
	.string "Oh. Maybe later, then.$"
