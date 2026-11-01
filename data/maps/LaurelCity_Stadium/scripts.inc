LaurelCity_Stadium_MapScripts::
	.byte 0

LaurelCity_Stadium_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_LAUREL_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

LaurelCity_Stadium_EventScript_DeskSign::
	msgbox LaurelCity_Stadium_EventScript_Text_DeskSign, MSGBOX_SIGN
	end

LaurelCity_Stadium_EventScript_NurseSign::
	msgbox LaurelCity_Stadium_EventScript_Text_NurseSign, MSGBOX_SIGN
	end

LaurelCity_Stadium_EventScript_Text_DeskSign:
	.string "ROVER LEAGUE: brackets, entry,\n"
	.string "matches and watching.$"

LaurelCity_Stadium_EventScript_Text_NurseSign:
	.string "Rest your POKéMON before a match.$"

LaurelCity_Stadium_EventScript_Announcer::
	msgbox LaurelCity_Stadium_EventScript_Announcer_Text, MSGBOX_NPC
	end

LaurelCity_Stadium_EventScript_Announcer_Text:
	.string "And here they come, the best\n"
	.string "rovers in HOENN!\p"
	.string "Ahem. Sorry. I practice my\n"
	.string "announcing on visitors.$"

LaurelCity_Stadium_EventScript_Lass::
	msgbox LaurelCity_Stadium_EventScript_Lass_Text, MSGBOX_NPC
	end

LaurelCity_Stadium_EventScript_Lass_Text:
	.string "I watched a rover match at the\n"
	.string "desk. You see it happen, and\l"
	.string "that's how it went for real!$"

LaurelCity_Stadium_EventScript_FatMan::
	msgbox LaurelCity_Stadium_EventScript_FatMan_Text, MSGBOX_NPC
	end

LaurelCity_Stadium_EventScript_FatMan_Text:
	.string "The nurse here heals your team\n"
	.string "before your match. Don't go in\l"
	.string "tired!$"

LaurelCity_Stadium_EventScript_Maniac::
	msgbox LaurelCity_Stadium_EventScript_Maniac_Text, MSGBOX_NPC
	end

LaurelCity_Stadium_EventScript_Maniac_Text:
	.string "Prize money goes up every round.\n"
	.string "Win the final and there's a big\l"
	.string "bonus on top!$"
