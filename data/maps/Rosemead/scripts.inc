Rosemead_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Rosemead_OnTransition
	.byte 0

Rosemead_OnTransition:
	setflag FLAG_SOL_VISITED_ROSEMEAD
	end

Rosemead_EventScript_Person0::
	msgbox Rosemead_EventScript_Person0_Text, MSGBOX_NPC
	end

Rosemead_EventScript_Person0_Text:
	.string "Every flower bed in town was\n"
	.string "planted by a different trainer\l"
	.string "passing through!$"

Rosemead_EventScript_Person1::
	msgbox Rosemead_EventScript_Person1_Text, MSGBOX_NPC
	end

Rosemead_EventScript_Person1_Text:
	.string "BELLE's FAIRY types laugh at\n"
	.string "DRAGON moves. STEEL and POISON\l"
	.string "are what they fear.$"

Rosemead_EventScript_Person2::
	msgbox Rosemead_EventScript_Person2_Text, MSGBOX_NPC
	end

Rosemead_EventScript_Person2_Text:
	.string "West is Route 205 through the\n"
	.string "canyon. DUSKMERE is foggy and\l"
	.string "a bit spooky!$"

Rosemead_EventScript_Person3::
	msgbox Rosemead_EventScript_Person3_Text, MSGBOX_NPC
	end

Rosemead_EventScript_Person3_Text:
	.string "Players picnic in the square on\n"
	.string "sunny days. Bring a friend!$"

Rosemead_EventScript_GymSign::
	msgbox Rosemead_EventScript_GymSign_Text, MSGBOX_SIGN
	end

Rosemead_EventScript_GymSign_Text:
	.string "ROSEMEAD POKéMON GYM\n"
	.string "LEADER: BELLE\l"
	.string "Grace that hits hard!$"

Rosemead_EventScript_TownSign::
	msgbox Rosemead_EventScript_TownSign_Text, MSGBOX_SIGN
	end

Rosemead_EventScript_TownSign_Text:
	.string "ROSEMEAD\n"
	.string "The garden of SOLMARA.$"

