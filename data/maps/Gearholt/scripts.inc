Gearholt_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Gearholt_OnTransition
	.byte 0

Gearholt_OnTransition:
	setflag FLAG_SOL_VISITED_GEARHOLT
	end

Gearholt_EventScript_Person0::
	msgbox Gearholt_EventScript_Person0_Text, MSGBOX_NPC
	end

Gearholt_EventScript_Person0_Text:
	.string "This square used to be the ore\n"
	.string "yard. Now trainers come here\l"
	.string "to battle each other!$"

Gearholt_EventScript_Person1::
	msgbox Gearholt_EventScript_Person1_Text, MSGBOX_NPC
	end

Gearholt_EventScript_Person1_Text:
	.string "I watch the battles in the yard\n"
	.string "every day. Talk to a trainer to\l"
	.string "challenge them!$"

Gearholt_EventScript_Person2::
	msgbox Gearholt_EventScript_Person2_Text, MSGBOX_NPC
	end

Gearholt_EventScript_Person2_Text:
	.string "BRANT's STEEL types shrug off\n"
	.string "almost anything. Fire and\l"
	.string "Fighting moves melt them down!$"

Gearholt_EventScript_Person3::
	msgbox Gearholt_EventScript_Person3_Text, MSGBOX_NPC
	end

Gearholt_EventScript_Person3_Text:
	.string "Route 202 north goes through the\n"
	.string "dunes to ASHCOMBE. Bring water!$"

Gearholt_EventScript_Person4::
	msgbox Gearholt_EventScript_Person4_Text, MSGBOX_NPC
	end

Gearholt_EventScript_Person4_Text:
	.string "In my day we had one gym and\n"
	.string "one road. Now the whole ring is\l"
	.string "open from the start!$"

Gearholt_EventScript_GymSign::
	msgbox Gearholt_EventScript_GymSign_Text, MSGBOX_SIGN
	end

Gearholt_EventScript_GymSign_Text:
	.string "GEARHOLT POKéMON GYM\n"
	.string "LEADER: BRANT\l"
	.string "Forged in fire, tempered in steel!$"

Gearholt_EventScript_TownSign::
	msgbox Gearholt_EventScript_TownSign_Text, MSGBOX_SIGN
	end

Gearholt_EventScript_TownSign_Text:
	.string "GEARHOLT\n"
	.string "The town that never stops\l"
	.string "working.\p"
	.string "BATTLE YARD: the square is open\n"
	.string "to all trainers. Battle away!$"

