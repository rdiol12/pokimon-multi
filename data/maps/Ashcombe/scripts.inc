Ashcombe_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Ashcombe_OnTransition
	.byte 0

Ashcombe_OnTransition:
	setflag FLAG_SOL_VISITED_ASHCOMBE
	end

Ashcombe_EventScript_Person0::
	msgbox Ashcombe_EventScript_Person0_Text, MSGBOX_NPC
	end

Ashcombe_EventScript_Person0_Text:
	.string "Whole groups of trainers soak\n"
	.string "in the spring together after\l"
	.string "beating CINDRA. Lovely to see!$"

Ashcombe_EventScript_Person1::
	msgbox Ashcombe_EventScript_Person1_Text, MSGBOX_NPC
	end

Ashcombe_EventScript_Person1_Text:
	.string "CINDRA's FIRE types are tough,\n"
	.string "but WATER and ROCK moves cool\l"
	.string "them right down.$"

Ashcombe_EventScript_Person2::
	msgbox Ashcombe_EventScript_Person2_Text, MSGBOX_NPC
	end

Ashcombe_EventScript_Person2_Text:
	.string "Route 203 north leads into\n"
	.string "thick woods. SILKWOOD is up\l"
	.string "there, full of BUG types!$"

Ashcombe_EventScript_Person3::
	msgbox Ashcombe_EventScript_Person3_Text, MSGBOX_NPC
	end

Ashcombe_EventScript_Person3_Text:
	.string "Ash falls on the town every\n"
	.string "morning. We sweep it into the\l"
	.string "gardens. Great for berries!$"

Ashcombe_EventScript_GymSign::
	msgbox Ashcombe_EventScript_GymSign_Text, MSGBOX_SIGN
	end

Ashcombe_EventScript_GymSign_Text:
	.string "ASHCOMBE POKéMON GYM\n"
	.string "LEADER: CINDRA\l"
	.string "Burning bright, burning together!$"

Ashcombe_EventScript_TownSign::
	msgbox Ashcombe_EventScript_TownSign_Text, MSGBOX_SIGN
	end

Ashcombe_EventScript_TownSign_Text:
	.string "ASHCOMBE\n"
	.string "Hot springs for all, under the\l"
	.string "smoking peak.$"

Ashcombe_EventScript_Spring::
	lock
	faceplayer
	msgbox Ashcombe_EventScript_Text_SpringAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Ashcombe_EventScript_SpringNo
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	callnative Area_SoakInHotSpring
	waitfanfare
	fadescreen FADE_FROM_BLACK
	msgbox Ashcombe_EventScript_Text_SpringDone, MSGBOX_DEFAULT
	release
	end

Ashcombe_EventScript_SpringNo::
	msgbox Ashcombe_EventScript_Text_SpringNo, MSGBOX_DEFAULT
	release
	end

Ashcombe_EventScript_Text_SpringAsk:
	.string "The spring is big enough for\n"
	.string "everyone. Care for a soak?$"

Ashcombe_EventScript_Text_SpringDone:
	.string "Your POKéMON are rested and look\n"
	.string "happier than ever!$"

Ashcombe_EventScript_Text_SpringNo:
	.string "The water's always warm.$"

