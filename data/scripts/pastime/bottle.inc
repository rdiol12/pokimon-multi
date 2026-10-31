@ Bottle treasure maps (src/pastime_bottle.c).

Bottle_EventScript_Bottle::
	lockall
	playse SE_PIN
	msgbox Bottle_Text_Found, MSGBOX_DEFAULT
	callnative Bottle_Open
	msgbox gPastimeText, MSGBOX_DEFAULT
	callnative Bottle_ShowMap
	msgbox Bottle_Text_Piece, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Bottle_EventScript_Close
	playfanfare MUS_OBTAIN_ITEM
	message Bottle_Text_Whole
	waitmessage
	waitfanfare
	waitbuttonpress
	msgbox Bottle_Text_Riddle, MSGBOX_DEFAULT
Bottle_EventScript_Close:
	callnative Bottle_HideMap
	closemessage
	releaseall
	end

Bottle_EventScript_Dig::
	lockall
	msgbox Bottle_Text_Dig, MSGBOX_DEFAULT
	playse SE_M_DIG
	waitse
	callnative Bottle_Dig
	giveitem VAR_0x8005
	msgbox Bottle_Text_Marlow, MSGBOX_DEFAULT
	releaseall
	end

Bottle_Text_Found:
	.string "A bottle washed up on the beach!\n"
	.string "There's a letter inside…$"

Bottle_Text_Piece:
	.string "There's a torn piece of a map, too.\n"
	.string "That's {STR_VAR_2} of the 4 pieces of this map.$"

Bottle_Text_Whole:
	.string "The pieces fit! The map is whole:\n"
	.string "it shows {STR_VAR_1}!$"

Bottle_Text_Riddle:
	.string "On the back, it says:\p"
	.string "{STR_VAR_3}$"

Bottle_Text_Dig:
	.string "This is the spot marked with an X on\n"
	.string "CAPTAIN MARLOW's map!\p"
	.string "{PLAYER} dug into the sand…$"

Bottle_Text_Marlow:
	.string "There's a note tied to it:\n"
	.string "“Well done, friend.  MARLOW”$"
