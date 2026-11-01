SolmaraShip_Lounge_MapScripts::
	.byte 0

SolmaraShip_Lounge_EventScript_Steward::
	lock
	faceplayer
	message SolmaraShip_Lounge_EventScript_Text_Where
	waitmessage
	dynmultipush SolmaraShip_Lounge_EventScript_Text_Solace, 0
	dynmultipush SolmaraShip_Lounge_EventScript_Text_Lilycove, 1
	dynmultipush SolmaraShip_Lounge_EventScript_Text_Stay, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, SolmaraShip_Lounge_EventScript_ToSolace
	goto_if_eq VAR_RESULT, 1, SolmaraShip_Lounge_EventScript_ToLilycove
	msgbox SolmaraShip_Lounge_EventScript_Text_StayOn, MSGBOX_DEFAULT
	release
	end

SolmaraShip_Lounge_EventScript_ToSolace::
	msgbox SolmaraShip_Lounge_EventScript_Text_Land, MSGBOX_DEFAULT
	closemessage
	playse SE_SHIP
	warp MAP_PORT_SOLACE, 21, 31
	waitstate
	release
	end

SolmaraShip_Lounge_EventScript_ToLilycove::
	msgbox SolmaraShip_Lounge_EventScript_Text_Land, MSGBOX_DEFAULT
	closemessage
	playse SE_SHIP
	warp MAP_LILYCOVE_CITY_HARBOR, 13, 12
	waitstate
	release
	end

SolmaraShip_Lounge_EventScript_Text_Where:
	.string "Welcome aboard the S.S. MERIDIAN!\n"
	.string "Where shall I let you off?$"

SolmaraShip_Lounge_EventScript_Text_Solace:
	.string "PORT SOLACE$"
SolmaraShip_Lounge_EventScript_Text_Lilycove:
	.string "LILYCOVE$"
SolmaraShip_Lounge_EventScript_Text_Stay:
	.string "STAY ABOARD$"
SolmaraShip_Lounge_EventScript_Text_StayOn:
	.string "Enjoy the lounge! Talk to me\n"
	.string "whenever you're ready to land.$"

SolmaraShip_Lounge_EventScript_Text_Land:
	.string "Land ho! Mind your step.$"

SolmaraShip_Lounge_EventScript_Npc0::
	msgbox SolmaraShip_Lounge_EventScript_Npc0_Text, MSGBOX_NPC
	end

SolmaraShip_Lounge_EventScript_Npc0_Text:
	.string "Half the trainers on this ship\n"
	.string "are heading to SOLMARA for the\l"
	.string "first time. Exciting, isn't it?$"

SolmaraShip_Lounge_EventScript_Npc1::
	msgbox SolmaraShip_Lounge_EventScript_Npc1_Text, MSGBOX_NPC
	end

SolmaraShip_Lounge_EventScript_Npc1_Text:
	.string "I always trade with someone on\n"
	.string "the crossing. You meet the\l"
	.string "nicest people at sea!$"

SolmaraShip_Lounge_EventScript_Npc2::
	msgbox SolmaraShip_Lounge_EventScript_Npc2_Text, MSGBOX_NPC
	end

SolmaraShip_Lounge_EventScript_Npc2_Text:
	.string "The crossing is quick, but the\n"
	.string "buffet is quicker. Gone already!$"

SolmaraShip_Lounge_EventScript_Npc3::
	msgbox SolmaraShip_Lounge_EventScript_Npc3_Text, MSGBOX_NPC
	end

SolmaraShip_Lounge_EventScript_Npc3_Text:
	.string "WINGULL: Pihyuu! It stowed away!$"
