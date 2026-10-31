@ Pokémon orchestra (src/pastime_orchestra.c), in Verdanturf Town.

Orchestra_EventScript_Conductor::
	lock
	faceplayer
	msgbox Orchestra_Text_Welcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Orchestra_EventScript_Bye
	msgbox Orchestra_Text_Rules, MSGBOX_DEFAULT
	message Orchestra_Text_Listen
	waitmessage
	callnative Orchestra_Play, waitstate=1
	callnative Orchestra_Finish
	msgbox Orchestra_Text_Score, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Orchestra_EventScript_Prize
	playfanfare MUS_LEVEL_UP
	message Orchestra_Text_Record
	waitmessage
	waitfanfare
	waitbuttonpress
Orchestra_EventScript_Prize:
	goto_if_eq VAR_0x8005, ITEM_NONE, Orchestra_EventScript_Bye
	msgbox Orchestra_Text_Prize, MSGBOX_DEFAULT
	giveitem VAR_0x8005
Orchestra_EventScript_Bye:
	msgbox Orchestra_Text_Bye, MSGBOX_DEFAULT
	release
	end

Orchestra_Text_Welcome:
	.string "Ah, a new musician! I conduct the\n"
	.string "POKéMON ORCHESTRA.\p"
	.string "Would you like to play with my\n"
	.string "quartet?$"

Orchestra_Text_Rules:
	.string "My four players sit on the four\n"
	.string "directions of your + Control Pad.\p"
	.string "They'll play a tune. Play it back,\n"
	.string "note for note!\p"
	.string "Every time you get it right, the\n"
	.string "tune grows by one note.$"

Orchestra_Text_Listen:
	.string "Listen closely…$"

Orchestra_Text_Score:
	.string "Bravo! You played back a tune of\n"
	.string "{STR_VAR_1} notes. Your record: {STR_VAR_2}.$"

Orchestra_Text_Record:
	.string "A new record! Magnificent!$"

Orchestra_Text_Prize:
	.string "Such talent deserves a gift!$"

Orchestra_Text_Bye:
	.string "Come and play with us again!$"
