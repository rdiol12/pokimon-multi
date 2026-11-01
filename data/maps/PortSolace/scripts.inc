PortSolace_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PortSolace_OnTransition
	.byte 0

PortSolace_OnTransition:
	setflag FLAG_SOL_VISITED_PORT_SOLACE
	end

PortSolace_EventScript_Ship::
	lock
	faceplayer
	msgbox PortSolace_EventScript_Text_ShipAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PortSolace_EventScript_ShipNo
	msgbox PortSolace_EventScript_Text_ShipGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SOLMARA_SHIP_LOUNGE, 2, 11
	waitstate
	release
	end

PortSolace_EventScript_ShipNo::
	msgbox PortSolace_EventScript_Text_ShipNo, MSGBOX_DEFAULT
	release
	end

PortSolace_EventScript_Ranger::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	goto_if_eq VAR_RESULT, 8, PortSolace_EventScript_RangerOpen
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox PortSolace_EventScript_Text_Ranger, MSGBOX_DEFAULT
	release
	end

PortSolace_EventScript_RangerOpen::
	msgbox PortSolace_EventScript_Text_RangerOpen, MSGBOX_DEFAULT
	release
	end

PortSolace_EventScript_VictoryCheck::
	lockall
	callnative Solmara_GetBadgeCount
	goto_if_eq VAR_RESULT, 8, PortSolace_EventScript_VictoryOk
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox PortSolace_EventScript_Text_VictoryNo, MSGBOX_DEFAULT
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, PortSolace_EventScript_StepBack
	waitmovement 0
	releaseall
	end

PortSolace_EventScript_VictoryOk::
	releaseall
	end

PortSolace_EventScript_StepBack:
	walk_down
	step_end

PortSolace_EventScript_Text_ShipAsk:
	.string "The S.S. MERIDIAN sails for\n"
	.string "LILYCOVE CITY in HOENN. All aboard?$"

PortSolace_EventScript_Text_ShipGo:
	.string "Welcome aboard! Make yourself at\n"
	.string "home in the lounge.$"

PortSolace_EventScript_Text_ShipNo:
	.string "We sail all day. Come back any\n"
	.string "time!$"

PortSolace_EventScript_Text_Ranger:
	.string "VICTORY ROAD climbs to the\n"
	.string "SOLMARA LEAGUE.\p"
	.string "Only trainers with all eight\n"
	.string "SOLMARA badges may enter.\p"
	.string "You have {STR_VAR_1}. Come back with\n"
	.string "all eight!$"

PortSolace_EventScript_Text_RangerOpen:
	.string "Eight SOLMARA badges! VICTORY ROAD\n"
	.string "is open to you. Good luck up there!$"

PortSolace_EventScript_Text_VictoryNo:
	.string "VICTORY ROAD is for trainers with\n"
	.string "all eight SOLMARA badges.\p"
	.string "You have {STR_VAR_1}.$"

PortSolace_EventScript_Fisher::
	msgbox PortSolace_EventScript_Fisher_Text, MSGBOX_NPC
	end

PortSolace_EventScript_Fisher_Text:
	.string "Everyone who sails in from HOENN\n"
	.string "walks through HARBOR SQUARE.\p"
	.string "Wait here and you'll meet a\n"
	.string "traveling partner in no time!$"

PortSolace_EventScript_Swimmer::
	msgbox PortSolace_EventScript_Swimmer_Text, MSGBOX_NPC
	end

PortSolace_EventScript_Swimmer_Text:
	.string "SOLMARA has eight gyms, and you\n"
	.string "can take them in any order!\p"
	.string "The leaders size up your badges\n"
	.string "and raise their game to match.$"

PortSolace_EventScript_OldMan::
	msgbox PortSolace_EventScript_OldMan_Text, MSGBOX_NPC
	end

PortSolace_EventScript_OldMan_Text:
	.string "The towns form a great ring. Head\n"
	.string "east or west, you'll come home\l"
	.string "to PORT SOLACE either way.$"

PortSolace_EventScript_Twin::
	msgbox PortSolace_EventScript_Twin_Text, MSGBOX_NPC
	end

PortSolace_EventScript_Twin_Text:
	.string "Every POKéMON CENTER here has a\n"
	.string "flight desk. Fly to any town you\l"
	.string "have been to before!$"

PortSolace_EventScript_Deckhand::
	msgbox PortSolace_EventScript_Deckhand_Text, MSGBOX_NPC
	end

PortSolace_EventScript_Deckhand_Text:
	.string "The S.S. MERIDIAN sails to\n"
	.string "LILYCOVE and back all day long.$"

PortSolace_EventScript_SquareSign::
	msgbox PortSolace_EventScript_SquareSign_Text, MSGBOX_SIGN
	end

PortSolace_EventScript_SquareSign_Text:
	.string "HARBOR SQUARE\n"
	.string "Meet, trade and battle with\l"
	.string "trainers from near and far!$"

PortSolace_EventScript_GymSign::
	msgbox PortSolace_EventScript_GymSign_Text, MSGBOX_SIGN
	end

PortSolace_EventScript_GymSign_Text:
	.string "PORT SOLACE POKéMON GYM\n"
	.string "LEADER: MARINA\l"
	.string "The tide that never turns!$"

PortSolace_EventScript_CaveSign::
	msgbox PortSolace_EventScript_CaveSign_Text, MSGBOX_SIGN
	end

PortSolace_EventScript_CaveSign_Text:
	.string "VICTORY ROAD\n"
	.string "To the SOLMARA LEAGUE.\l"
	.string "Eight SOLMARA badges needed.$"

