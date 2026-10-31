@ Cobblerock guide in Rustboro City (tools/areas/cobblerock.py).
RustboroCity_EventScript_CobblerockGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE01_GET, RustboroCity_EventScript_CobblerockNoBadge
	msgbox RustboroCity_Text_CobblerockAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_EventScript_CobblerockNo
	msgbox RustboroCity_Text_CobblerockGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_COBBLEROCK, 24, 38
	waitstate
	release
	end

RustboroCity_EventScript_CobblerockNoBadge::
	msgbox RustboroCity_Text_CobblerockNoBadge, MSGBOX_DEFAULT
	release
	end

RustboroCity_EventScript_CobblerockNo::
	msgbox RustboroCity_Text_CobblerockNo, MSGBOX_DEFAULT
	release
	end

RustboroCity_Text_CobblerockNoBadge:
	.string "I'm a miner from COBBLEROCK, up in\n"
	.string "the hills. The mine's no place for\l"
	.string "beginners, though.\p"
	.string "Beat ROXANNE first, then I'll take\n"
	.string "you up.$"

RustboroCity_Text_CobblerockAsk:
	.string "ROXANNE's badge! You can handle\n"
	.string "the mine.\p"
	.string "Want to come up to COBBLEROCK?$"

RustboroCity_Text_CobblerockGo:
	.string "Up we go!$"

RustboroCity_Text_CobblerockNo:
	.string "Suit yourself. I'll be here.$"
