Frostholm_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Frostholm_OnTransition
	.byte 0

Frostholm_OnTransition:
	setflag FLAG_SOL_VISITED_FROSTHOLM
	end

Frostholm_EventScript_Person0::
	msgbox Frostholm_EventScript_Person0_Text, MSGBOX_NPC
	end

Frostholm_EventScript_Person0_Text:
	.string "Snowball fights in the square\n"
	.string "every winter! Last year forty\l"
	.string "trainers joined in.$"

Frostholm_EventScript_Person1::
	msgbox Frostholm_EventScript_Person1_Text, MSGBOX_NPC
	end

Frostholm_EventScript_Person1_Text:
	.string "SIGRID's ICE types hate FIRE,\n"
	.string "FIGHTING, ROCK and STEEL moves.$"

Frostholm_EventScript_Person2::
	msgbox Frostholm_EventScript_Person2_Text, MSGBOX_NPC
	end

Frostholm_EventScript_Person2_Text:
	.string "South, Route 207 runs through\n"
	.string "the crags to DRAKESPIRE, home\l"
	.string "of the DRAGON gym.$"

Frostholm_EventScript_Person3::
	msgbox Frostholm_EventScript_Person3_Text, MSGBOX_NPC
	end

Frostholm_EventScript_Person3_Text:
	.string "The gym floor is ice! Watch out,\n"
	.string "you slide until you hit\l"
	.string "something. Or someone!$"

Frostholm_EventScript_GymSign::
	msgbox Frostholm_EventScript_GymSign_Text, MSGBOX_SIGN
	end

Frostholm_EventScript_GymSign_Text:
	.string "FROSTHOLM POKéMON GYM\n"
	.string "LEADER: SIGRID\l"
	.string "Cold heart, clear mind!$"

Frostholm_EventScript_TownSign::
	msgbox Frostholm_EventScript_TownSign_Text, MSGBOX_SIGN
	end

Frostholm_EventScript_TownSign_Text:
	.string "FROSTHOLM\n"
	.string "Warm hearts in the cold north.$"

