@ Whisperstem Valley guide in Petalburg City (tools/areas/whisperstem.py).
PetalburgCity_EventScript_WhisperstemGuide::
	lock
	faceplayer
	msgbox PetalburgCity_Text_WhisperstemAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PetalburgCity_EventScript_WhisperstemNo
	msgbox PetalburgCity_Text_WhisperstemGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_WHISPERSTEM_VALLEY, 29, 44
	waitstate
	release
	end

PetalburgCity_EventScript_WhisperstemNo::
	msgbox PetalburgCity_Text_WhisperstemNo, MSGBOX_DEFAULT
	release
	end

PetalburgCity_Text_WhisperstemAsk:
	.string "West of town there's a hidden\n"
	.string "valley full of whispering bamboo.\p"
	.string "There's a teahouse, a koi pond…\n"
	.string "Shall I show you the way?$"

PetalburgCity_Text_WhisperstemGo:
	.string "Follow me!$"

PetalburgCity_Text_WhisperstemNo:
	.string "Any time. The bamboo isn't going\n"
	.string "anywhere.$"
