Ashcombe_PokemonCenter_1F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Ashcombe_PokemonCenter_1F_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Ashcombe_PokemonCenter_1F_Items:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_HEAL_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_BURN_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_ESCAPE_ROPE
	pokemartlistend

Ashcombe_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

Ashcombe_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_ASHCOMBE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Ashcombe_PokemonCenter_1F_EventScript_Flight::
	setvar VAR_0x8008, 2
	goto Solmara_EventScript_FlightDesk

Ashcombe_PokemonCenter_1F_EventScript_Npc0::
	msgbox Ashcombe_PokemonCenter_1F_EventScript_Npc0_Text, MSGBOX_NPC
	end

Ashcombe_PokemonCenter_1F_EventScript_Npc0_Text:
	.string "My friend is still on Route 202.\n"
	.string "I'll wait here and heal up.$"
