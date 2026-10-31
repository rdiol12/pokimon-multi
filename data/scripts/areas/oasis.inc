@ Mirage Dunes caravan master on Route 111 (tools/areas/oasis.py).
Route111_EventScript_MirageGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE08_GET, Route111_EventScript_MirageNoBadge
	msgbox Route111_Text_MirageAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route111_EventScript_MirageNo
	msgbox Route111_Text_MirageGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_MIRAGE_DUNES, 28, 50
	waitstate
	release
	end

Route111_EventScript_MirageNoBadge::
	msgbox Route111_Text_MirageNoBadge, MSGBOX_DEFAULT
	release
	end

Route111_EventScript_MirageNo::
	msgbox Route111_Text_MirageNo, MSGBOX_DEFAULT
	release
	end

Route111_Text_MirageNoBadge:
	.string "I lead caravans across the MIRAGE\n"
	.string "DUNES, beyond this desert.\p"
	.string "The deep dunes are for the strongest\n"
	.string "trainers only. Come back with all\l"
	.string "eight badges.$"

Route111_Text_MirageAsk:
	.string "Eight badges! Our caravan lost its\n"
	.string "way to the MIRAGE OASIS. The dunes\l"
	.string "are full of false water.\p"
	.string "Will you ride out with me and help\n"
	.string "find the real oasis?$"

Route111_Text_MirageGo:
	.string "Hold on to your hat!$"

Route111_Text_MirageNo:
	.string "The caravan waits for no one… but\n"
	.string "I'll wait for you.$"
