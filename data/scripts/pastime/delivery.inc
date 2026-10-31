@ Delivery jobs (src/pastime_delivery.c). VAR_0x8004 = the desk's town.

Delivery_EventScript_Desk::
	lock
	faceplayer
	callnative Delivery_Talk
	switch VAR_RESULT
	case DELIVERY_NO_TOWNS, Delivery_EventScript_NoTowns
	case DELIVERY_DELIVERED, Delivery_EventScript_Deliver
	case DELIVERY_ELSEWHERE, Delivery_EventScript_Elsewhere
	msgbox Delivery_Text_Hello, MSGBOX_DEFAULT
	msgbox gPastimeText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Delivery_EventScript_Declined
	callnative Delivery_Accept
	playfanfare MUS_OBTAIN_ITEM
	message Delivery_Text_GotParcel
	waitmessage
	waitfanfare
	msgbox Delivery_Text_Hurry, MSGBOX_DEFAULT
	release
	end

Delivery_EventScript_Declined:
	msgbox Delivery_Text_Declined, MSGBOX_DEFAULT
	release
	end

Delivery_EventScript_NoTowns:
	msgbox Delivery_Text_NoTowns, MSGBOX_DEFAULT
	release
	end

Delivery_EventScript_Elsewhere:
	msgbox gPastimeText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Delivery_EventScript_KeepGoing
	callnative Delivery_Cancel
	msgbox Delivery_Text_Cancelled, MSGBOX_DEFAULT
	release
	end

Delivery_EventScript_KeepGoing:
	msgbox Delivery_Text_KeepGoing, MSGBOX_DEFAULT
	release
	end

Delivery_EventScript_Deliver:
	msgbox Delivery_Text_ForUs, MSGBOX_DEFAULT
	callnative Delivery_Deliver
	playse SE_SHOP
	msgbox gPastimeText, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Delivery_EventScript_Done
	msgbox Delivery_Text_Bonus, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
Delivery_EventScript_Done:
	release
	end

Delivery_EventScript_Vigoroth::
	lock
	faceplayer
	playmoncry SPECIES_VIGOROTH, CRY_MODE_NORMAL
	msgbox Delivery_Text_Vigoroth, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Delivery_Text_Hello:
	.string "Hi! This is the PARCEL DESK of\n"
	.string "HOENN EXPRESS. Fast and friendly!$"

Delivery_Text_GotParcel:
	.string "{PLAYER} got a parcel for\n{STR_VAR_1}!$"

Delivery_Text_Hurry:
	.string "Hand it to the desk outside their\n"
	.string "POKéMON CENTER. The clock is ticking!\p"
	.string "Check the time left in\n"
	.string "R > PASTIMES > DELIVERY.$"

Delivery_Text_Declined:
	.string "No problem. Come back anytime!$"

Delivery_Text_NoTowns:
	.string "Hi! This is the PARCEL DESK of\n"
	.string "HOENN EXPRESS.\p"
	.string "Our parcels go to towns you've\n"
	.string "visited. Explore a bit, then come\l"
	.string "back for work!$"

Delivery_Text_Cancelled:
	.string "All right, we'll send it ourselves.$"

Delivery_Text_KeepGoing:
	.string "Good luck! Keep moving!$"

Delivery_Text_ForUs:
	.string "Oh, that parcel's for us!\n"
	.string "Let me sign for it…$"

Delivery_Text_Bonus:
	.string "That's five more deliveries!\n"
	.string "Here's a little extra from the boss.$"

Delivery_Text_Vigoroth:
	.string "VIGOROTH: Grooaar!\p"
	.string "It's carrying boxes as fast as\n"
	.string "it can!$"
