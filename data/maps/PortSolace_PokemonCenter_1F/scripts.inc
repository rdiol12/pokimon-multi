PortSolace_PokemonCenter_1F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart PortSolace_PokemonCenter_1F_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
PortSolace_PokemonCenter_1F_Items:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_DIVE_BALL
	.2byte ITEM_NET_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_ESCAPE_ROPE
	pokemartlistend

PortSolace_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

PortSolace_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_PORT_SOLACE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

PortSolace_PokemonCenter_1F_EventScript_Flight::
	setvar VAR_0x8008, 0
	goto Solmara_EventScript_FlightDesk

PortSolace_PokemonCenter_1F_EventScript_Npc0::
	msgbox PortSolace_PokemonCenter_1F_EventScript_Npc0_Text, MSGBOX_NPC
	end

PortSolace_PokemonCenter_1F_EventScript_Npc0_Text:
	.string "I came over on the ship with a\n"
	.string "friend. We split up to cover\l"
	.string "more ground!$"

PortSolace_PokemonCenter_1F_EventScript_Npc1::
	msgbox PortSolace_PokemonCenter_1F_EventScript_Npc1_Text, MSGBOX_NPC
	end

PortSolace_PokemonCenter_1F_EventScript_Npc1_Text:
	.string "The flight desk only goes to\n"
	.string "towns you have visited. Walk\l"
	.string "there once first!$"
