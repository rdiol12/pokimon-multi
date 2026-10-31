@ Frostvein Cave boat in Mossdeep City (tools/areas/icecave.py).
MossdeepCity_EventScript_FrostveinSailor::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE07_GET, MossdeepCity_EventScript_FrostveinSailorNoBadge
	msgbox MossdeepCity_Text_FrostveinSailorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MossdeepCity_EventScript_FrostveinSailorNo
	msgbox MossdeepCity_Text_FrostveinSailorGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_FROSTVEIN_CAVE_1F, 20, 28
	waitstate
	release
	end

MossdeepCity_EventScript_FrostveinSailorNoBadge::
	msgbox MossdeepCity_Text_FrostveinSailorNoBadge, MSGBOX_DEFAULT
	release
	end

MossdeepCity_EventScript_FrostveinSailorNo::
	msgbox MossdeepCity_Text_FrostveinSailorNo, MSGBOX_DEFAULT
	release
	end

MossdeepCity_Text_FrostveinSailorNoBadge:
	.string "North of here there's an islet with\n"
	.string "a cave that never thaws.\p"
	.string "It's a dangerous trip. Show me the\n"
	.string "MIND BADGE and I'll sail you there.$"

MossdeepCity_Text_FrostveinSailorAsk:
	.string "The MIND BADGE! Then you can handle\n"
	.string "the cold.\p"
	.string "Shall I sail you to FROSTVEIN CAVE?$"

MossdeepCity_Text_FrostveinSailorGo:
	.string "Wrap up warm!$"

MossdeepCity_Text_FrostveinSailorNo:
	.string "The ice will still be there\n"
	.string "tomorrow. It always is.$"
