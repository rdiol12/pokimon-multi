@ Gullwing Point boatman in Dewford Town (tools/areas/gullwing.py).
DewfordTown_EventScript_GullwingBoatman::
	lock
	faceplayer
	msgbox DewfordTown_Text_GullwingBoatmanAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_EventScript_GullwingBoatmanNo
	msgbox DewfordTown_Text_GullwingBoatmanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_GULLWING_POINT, 15, 45
	waitstate
	release
	end

DewfordTown_EventScript_GullwingBoatmanNo::
	msgbox DewfordTown_Text_GullwingBoatmanNo, MSGBOX_DEFAULT
	release
	end

DewfordTown_Text_GullwingBoatmanAsk:
	.string "I row folk out to GULLWING POINT.\n"
	.string "There's a lighthouse, a fishing\l"
	.string "village… and pirates, they say.\p"
	.string "Want to go?$"

DewfordTown_Text_GullwingBoatmanGo:
	.string "Heave ho!$"

DewfordTown_Text_GullwingBoatmanNo:
	.string "The boat's here when you want it.$"
