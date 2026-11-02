Duskmere_PokemonCenter_1F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Duskmere_PokemonCenter_1F_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Duskmere_PokemonCenter_1F_Items:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_DUSK_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_ESCAPE_ROPE
	pokemartlistend

Duskmere_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

Duskmere_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_DUSKMERE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Duskmere_PokemonCenter_1F_EventScript_Flight::
	setvar VAR_0x8008, 5
	goto Solmara_EventScript_FlightDesk

Duskmere_PokemonCenter_1F_EventScript_Npc0::
	msgbox Duskmere_PokemonCenter_1F_EventScript_Npc0_Text, MSGBOX_NPC
	end

Duskmere_PokemonCenter_1F_EventScript_Npc0_Text:
	.string "Ghosts love a crowd. The more\n"
	.string "players in town, the more they\l"
	.string "come out to play...$"
