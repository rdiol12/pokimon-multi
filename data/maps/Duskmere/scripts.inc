Duskmere_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Duskmere_OnTransition
	.byte 0

Duskmere_OnTransition:
	setflag FLAG_SOL_VISITED_DUSKMERE
	end

Duskmere_EventScript_Person0::
	msgbox Duskmere_EventScript_Person0_Text, MSGBOX_NPC
	end

Duskmere_EventScript_Person0_Text:
	.string "In the fog you can hardly tell\n"
	.string "the other trainers from the\l"
	.string "ghosts. Say hello anyway!$"

Duskmere_EventScript_Person1::
	msgbox Duskmere_EventScript_Person1_Text, MSGBOX_NPC
	end

Duskmere_EventScript_Person1_Text:
	.string "MORWYN's GHOST types vanish\n"
	.string "when you swing at them. DARK\l"
	.string "and GHOST moves hit them hard.$"

Duskmere_EventScript_Person2::
	msgbox Duskmere_EventScript_Person2_Text, MSGBOX_NPC
	end

Duskmere_EventScript_Person2_Text:
	.string "Strange shapes swim under the\n"
	.string "fog at night. Try fishing!$"

Duskmere_EventScript_Person3::
	msgbox Duskmere_EventScript_Person3_Text, MSGBOX_NPC
	end

Duskmere_EventScript_Person3_Text:
	.string "Route 206 to the south climbs\n"
	.string "into the snow. FROSTHOLM is cold!$"

Duskmere_EventScript_GymSign::
	msgbox Duskmere_EventScript_GymSign_Text, MSGBOX_SIGN
	end

Duskmere_EventScript_GymSign_Text:
	.string "DUSKMERE POKéMON GYM\n"
	.string "LEADER: MORWYN\l"
	.string "The mist sees all who enter.$"

Duskmere_EventScript_TownSign::
	msgbox Duskmere_EventScript_TownSign_Text, MSGBOX_SIGN
	end

Duskmere_EventScript_TownSign_Text:
	.string "DUSKMERE\n"
	.string "The town in the mist.$"

