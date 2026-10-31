Cobblerock_MinersRest_MapScripts::
	.byte 0

Cobblerock_MinersRest_EventScript_Miner1::
	msgbox Cobblerock_MinersRest_Text_Miner1, MSGBOX_NPC
	end

Cobblerock_MinersRest_EventScript_Miner2::
	msgbox Cobblerock_MinersRest_Text_Miner2, MSGBOX_NPC
	end

Cobblerock_MinersRest_EventScript_Cook::
	lock
	faceplayer
	msgbox Cobblerock_MinersRest_Text_Cook, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox Cobblerock_MinersRest_Text_CookAfter, MSGBOX_DEFAULT
	release
	end

Cobblerock_MinersRest_Text_Miner1:
	.string "A long day in the mine and a hot\n"
	.string "meal after. That's the miner's life.$"

Cobblerock_MinersRest_Text_Miner2:
	.string "When I was a kid, I swore I saw a\n"
	.string "POKéMON made of jewels in the old\l"
	.string "shaft. Nobody believed me.$"

Cobblerock_MinersRest_Text_Cook:
	.string "You look worn out! Sit down and\n"
	.string "rest a while.$"

Cobblerock_MinersRest_Text_CookAfter:
	.string "There! Good as new. Off you go.$"
