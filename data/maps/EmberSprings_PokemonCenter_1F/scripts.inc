EmberSprings_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

EmberSprings_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_EMBER_SPRINGS_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

EmberSprings_PokemonCenter_1F_EventScript_Npc0::
	msgbox EmberSprings_PokemonCenter_1F_Text_Npc0, MSGBOX_NPC
	end

EmberSprings_PokemonCenter_1F_Text_Npc0:
	.string "A soak in the springs and a rest at"
	.string "the CENTER. Perfect holiday!$"

EmberSprings_PokemonCenter_1F_EventScript_Npc1::
	msgbox EmberSprings_PokemonCenter_1F_Text_Npc1, MSGBOX_NPC
	end

EmberSprings_PokemonCenter_1F_Text_Npc1:
	.string "The SPRING KEEPER looks worried"
	.string "about the steam vents.$"
