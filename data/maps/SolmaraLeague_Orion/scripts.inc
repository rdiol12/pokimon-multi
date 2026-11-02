SolmaraLeague_Orion_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SolmaraLeague_Orion_OnLoad
	.byte 0

SolmaraLeague_Orion_OnLoad:
	call_if_set FLAG_SOL_E4_ORION, PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	end

SolmaraLeague_Orion_EventScript_Member::
	lock
	faceplayer
	goto_if_set FLAG_SOL_E4_ORION, SolmaraLeague_Orion_EventScript_After
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox SolmaraLeague_Orion_EventScript_Text_Intro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_SOL_E4_ORION
	trainerbattle_no_intro TRAINER_SOL_E4_ORION, SolmaraLeague_Orion_EventScript_Text_Defeat
	setflag FLAG_SOL_E4_ORION
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	msgbox SolmaraLeague_Orion_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Orion_EventScript_After::
	msgbox SolmaraLeague_Orion_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Orion_EventScript_Text_Intro:
	.string "I am ORION, last of the ELITE\n"
	.string "FOUR. I saw this battle in the\l"
	.string "stars.\p"
	.string "I did not see who won.$"

SolmaraLeague_Orion_EventScript_Text_Defeat:
	.string "The stars were right about you.$"

SolmaraLeague_Orion_EventScript_Text_After:
	.string "Go on. The next room awaits.$"
