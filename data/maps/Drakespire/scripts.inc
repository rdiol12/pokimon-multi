Drakespire_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Drakespire_OnTransition
	.byte 0

Drakespire_OnTransition:
	setflag FLAG_SOL_VISITED_DRAKESPIRE
	end

Drakespire_EventScript_Person0::
	msgbox Drakespire_EventScript_Person0_Text, MSGBOX_NPC
	end

Drakespire_EventScript_Person0_Text:
	.string "VALEN's DRAGONS are the last\n"
	.string "test before VICTORY ROAD. Or the\l"
	.string "first, if you're brave!$"

Drakespire_EventScript_Person1::
	msgbox Drakespire_EventScript_Person1_Text, MSGBOX_NPC
	end

Drakespire_EventScript_Person1_Text:
	.string "DRAGON types fear ICE, FAIRY and\n"
	.string "other DRAGON moves.$"

Drakespire_EventScript_Person2::
	msgbox Drakespire_EventScript_Person2_Text, MSGBOX_NPC
	end

Drakespire_EventScript_Person2_Text:
	.string "Route 208 east goes along the\n"
	.string "coast back to PORT SOLACE.$"

Drakespire_EventScript_Person3::
	msgbox Drakespire_EventScript_Person3_Text, MSGBOX_NPC
	end

Drakespire_EventScript_Person3_Text:
	.string "From the top of the spire you\n"
	.string "can see the whole ring of\l"
	.string "towns, and the LEAGUE in the\l"
	.string "middle!$"

Drakespire_EventScript_GymSign::
	msgbox Drakespire_EventScript_GymSign_Text, MSGBOX_SIGN
	end

Drakespire_EventScript_GymSign_Text:
	.string "DRAKESPIRE POKéMON GYM\n"
	.string "LEADER: VALEN\l"
	.string "Wings of the ancient sky!$"

Drakespire_EventScript_TownSign::
	msgbox Drakespire_EventScript_TownSign_Text, MSGBOX_SIGN
	end

Drakespire_EventScript_TownSign_Text:
	.string "DRAKESPIRE\n"
	.string "Where dragons nest above the\l"
	.string "clouds.$"

