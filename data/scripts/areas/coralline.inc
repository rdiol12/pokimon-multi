@ Coralline Reef diver in Pacifidlog Town (tools/areas/reef.py).
PacifidlogTown_EventScript_ReefDiver::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE07_GET, PacifidlogTown_EventScript_ReefDiverNoBadge
	msgbox PacifidlogTown_Text_ReefDiverAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PacifidlogTown_EventScript_ReefDiverNo
	msgbox PacifidlogTown_Text_ReefDiverGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_CORALLINE_REEF, 28, 41
	waitstate
	release
	end

PacifidlogTown_EventScript_ReefDiverNoBadge::
	msgbox PacifidlogTown_Text_ReefDiverNoBadge, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_EventScript_ReefDiverNo::
	msgbox PacifidlogTown_Text_ReefDiverNo, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_Text_ReefDiverNoBadge:
	.string "Below us is CORALLINE REEF, and\n"
	.string "past it a city under the sea!\p"
	.string "It's deep, though. Come back once\n"
	.string "TATE and LIZA say you're ready.$"

PacifidlogTown_Text_ReefDiverAsk:
	.string "The MIND BADGE! Then you can handle\n"
	.string "the deep.\p"
	.string "Want me to guide you down to\n"
	.string "CORALLINE REEF?$"

PacifidlogTown_Text_ReefDiverGo:
	.string "Big breath… and down we go!$"

PacifidlogTown_Text_ReefDiverNo:
	.string "The reef will wait for you.$"
