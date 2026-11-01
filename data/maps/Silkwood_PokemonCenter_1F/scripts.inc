Silkwood_PokemonCenter_1F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Silkwood_PokemonCenter_1F_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Silkwood_PokemonCenter_1F_Items:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_NET_BALL
	.2byte ITEM_NEST_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REPEL
	pokemartlistend

Silkwood_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

Silkwood_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_SILKWOOD_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Silkwood_PokemonCenter_1F_EventScript_Flight::
	setvar VAR_0x8008, 3
	goto Solmara_EventScript_FlightDesk

Silkwood_PokemonCenter_1F_EventScript_Npc0::
	msgbox Silkwood_PokemonCenter_1F_EventScript_Npc0_Text, MSGBOX_NPC
	end

Silkwood_PokemonCenter_1F_EventScript_Npc0_Text:
	.string "I counted forty BUG POKéMON on\n"
	.string "the way here. Then I lost count.$"
