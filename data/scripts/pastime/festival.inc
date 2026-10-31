@ Seasonal festivals (src/pastime_festival.c), in Lilycove City.

Festival_EventScript_Host::
	lock
	faceplayer
	callnative Festival_TalkToHost
	goto_if_eq VAR_RESULT, FESTIVAL_DONE, Festival_EventScript_Done
	goto_if_eq VAR_RESULT, FESTIVAL_PRIZE, Festival_EventScript_Prize
	msgbox Festival_Text_Welcome, MSGBOX_DEFAULT
	release
	end

Festival_EventScript_Prize:
	playfanfare MUS_LEVEL_UP
	message Festival_Text_AllStamps
	waitmessage
	waitfanfare
	waitbuttonpress
	giveitem VAR_0x8005
	msgbox Festival_Text_Enjoy, MSGBOX_DEFAULT
	release
	end

Festival_EventScript_Done:
	msgbox Festival_Text_Done, MSGBOX_DEFAULT
	release
	end

Festival_EventScript_Stall::
	lock
	faceplayer
	callnative Festival_VisitStall
	goto_if_eq VAR_RESULT, FALSE, Festival_EventScript_StallAgain
	msgbox Festival_Text_Stall, MSGBOX_DEFAULT
	playse SE_SELECT
	msgbox Festival_Text_Stamped, MSGBOX_DEFAULT
	release
	end

Festival_EventScript_StallAgain:
	msgbox Festival_Text_StallAgain, MSGBOX_DEFAULT
	release
	end

Festival_Text_Welcome:
	.string "Welcome to the {STR_VAR_1}!\p"
	.string "Visit the three stalls around town\n"
	.string "and get a stamp at each one.\p"
	.string "Your card has {STR_VAR_2} of 3 stamps.\n"
	.string "Bring me a full card for a prize!$"

Festival_Text_AllStamps:
	.string "All three stamps! You've really\n"
	.string "enjoyed the {STR_VAR_1}!$"

Festival_Text_Enjoy:
	.string "Thanks for coming! See you at the\n"
	.string "next festival!$"

Festival_Text_Done:
	.string "Enjoy the {STR_VAR_1}!\n"
	.string "See you next season!$"

Festival_Text_Stall:
	.string "Happy {STR_VAR_2}!\n"
	.string "Here, have {STR_VAR_1}!$"

Festival_Text_Stamped:
	.string "Your stamp card got a stamp!$"

Festival_Text_StallAgain:
	.string "You've got our stamp already.\n"
	.string "Enjoy the festival!$"
