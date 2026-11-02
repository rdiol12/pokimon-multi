Rosemead_PokemonCenter_1F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Rosemead_PokemonCenter_1F_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Rosemead_PokemonCenter_1F_Items:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_LOVE_BALL
	.2byte ITEM_FRIEND_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_ESCAPE_ROPE
	pokemartlistend

Rosemead_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

Rosemead_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_ROSEMEAD_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Rosemead_PokemonCenter_1F_EventScript_Flight::
	setvar VAR_0x8008, 4
	goto Solmara_EventScript_FlightDesk

Rosemead_PokemonCenter_1F_EventScript_Npc0::
	msgbox Rosemead_PokemonCenter_1F_EventScript_Npc0_Text, MSGBOX_NPC
	end

Rosemead_PokemonCenter_1F_EventScript_Npc0_Text:
	.string "Have you counted your badges?\n"
	.string "Only the order you choose is\l"
	.string "yours. The challenge grows!$"
