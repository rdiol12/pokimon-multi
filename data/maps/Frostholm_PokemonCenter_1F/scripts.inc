Frostholm_PokemonCenter_1F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Frostholm_PokemonCenter_1F_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Frostholm_PokemonCenter_1F_Items:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_TIMER_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_ESCAPE_ROPE
	pokemartlistend

Frostholm_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

Frostholm_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_FROSTHOLM_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Frostholm_PokemonCenter_1F_EventScript_Flight::
	setvar VAR_0x8008, 6
	goto Solmara_EventScript_FlightDesk

Frostholm_PokemonCenter_1F_EventScript_Npc0::
	msgbox Frostholm_PokemonCenter_1F_EventScript_Npc0_Text, MSGBOX_NPC
	end

Frostholm_PokemonCenter_1F_EventScript_Npc0_Text:
	.string "Come in, come in! Warm up by\n"
	.string "the heater, all of you!$"
