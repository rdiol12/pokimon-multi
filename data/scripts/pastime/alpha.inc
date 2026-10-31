@ Alpha taming (src/pastime_alpha.c): the alphas on the routes.

Alpha_EventScript_Alpha::
	lock
	callnative Alpha_GetInfo
	playmoncry VAR_0x8005, CRY_MODE_ENCOUNTER
	msgbox Alpha_Text_Alpha, MSGBOX_DEFAULT
	waitmoncry
	msgbox Alpha_Text_TryCalm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Alpha_EventScript_Leave
	msgbox Alpha_Text_HowTo, MSGBOX_DEFAULT
	closemessage
	callnative Alpha_Calm, waitstate=1
	goto_if_eq VAR_RESULT, FALSE, Alpha_EventScript_Attacks
	msgbox Alpha_Text_Calm, MSGBOX_DEFAULT
	callnative Alpha_Tame
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, Alpha_EventScript_NoRoom
	playfanfare MUS_OBTAIN_ITEM
	message Alpha_Text_Joined
	waitmessage
	waitfanfare
	waitbuttonpress
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, Alpha_EventScript_ToPC
	closemessage
	release
	end

Alpha_EventScript_ToPC:
	msgbox Alpha_Text_ToPC, MSGBOX_DEFAULT
	release
	end

Alpha_EventScript_NoRoom:
	msgbox Alpha_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

Alpha_EventScript_Attacks:
	msgbox Alpha_Text_Attacks, MSGBOX_DEFAULT
	closemessage
	setwildbattle SPECIES_LOUDRED, 1 @ a single battle; Alpha_PrepareBattle makes the Pokémon
	callnative Alpha_PrepareBattle
	dowildbattle
	release
	end

Alpha_EventScript_Leave:
	release
	end

Alpha_Text_Alpha:
	.string "A huge {STR_VAR_1} stands in the way!\n"
	.string "It's an ALPHA, Lv. {STR_VAR_2}!$"

Alpha_Text_TryCalm:
	.string "It looks restless…\n"
	.string "Try to calm it?$"

Alpha_Text_HowTo:
	.string "Move slowly… Press A while the\n"
	.string "marker is in the zone, every time!$"

Alpha_Text_Calm:
	.string "The {STR_VAR_1} relaxed and sniffed\n"
	.string "{PLAYER}'s hand…$"

Alpha_Text_Joined:
	.string "The ALPHA {STR_VAR_1} decided to\n"
	.string "come along with {PLAYER}!$"

Alpha_Text_ToPC:
	.string "There's no room in the party, so it\n"
	.string "was sent to the PC.$"

Alpha_Text_NoRoom:
	.string "…but there's no room for it in the\n"
	.string "party or the PC. It wandered off.$"

Alpha_Text_Attacks:
	.string "The {STR_VAR_1} got startled!\n"
	.string "It attacks!$"
