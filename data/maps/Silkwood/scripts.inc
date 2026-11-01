Silkwood_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Silkwood_OnTransition
	.byte 0

Silkwood_OnTransition:
	setflag FLAG_SOL_VISITED_SILKWOOD
	end

Silkwood_EventScript_Person0::
	msgbox Silkwood_EventScript_Person0_Text, MSGBOX_NPC
	end

Silkwood_EventScript_Person0_Text:
	.string "Players swap rare BUG POKéMON\n"
	.string "right here in the square. I got\l"
	.string "a SCIZOR from someone yesterday!$"

Silkwood_EventScript_Person1::
	msgbox Silkwood_EventScript_Person1_Text, MSGBOX_NPC
	end

Silkwood_EventScript_Person1_Text:
	.string "THISTLE's gym is full of bushes.\n"
	.string "Plenty of room to get around\l"
	.string "other challengers though!$"

Silkwood_EventScript_Person2::
	msgbox Silkwood_EventScript_Person2_Text, MSGBOX_NPC
	end

Silkwood_EventScript_Person2_Text:
	.string "West is Route 204, through the\n"
	.string "marsh to ROSEMEAD. Wear boots!$"

Silkwood_EventScript_Person3::
	msgbox Silkwood_EventScript_Person3_Text, MSGBOX_NPC
	end

Silkwood_EventScript_Person3_Text:
	.string "FIRE and FLYING moves are what\n"
	.string "you want against BUG types.$"

Silkwood_EventScript_GymSign::
	msgbox Silkwood_EventScript_GymSign_Text, MSGBOX_SIGN
	end

Silkwood_EventScript_GymSign_Text:
	.string "SILKWOOD POKéMON GYM\n"
	.string "LEADER: THISTLE\l"
	.string "Small wings, big dreams!$"

Silkwood_EventScript_TownSign::
	msgbox Silkwood_EventScript_TownSign_Text, MSGBOX_SIGN
	end

Silkwood_EventScript_TownSign_Text:
	.string "SILKWOOD\n"
	.string "Where the forest hums.$"

