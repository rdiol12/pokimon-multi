SolmaraLeague_Vesper_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SolmaraLeague_Vesper_OnLoad
	.byte 0

SolmaraLeague_Vesper_OnLoad:
	call_if_set FLAG_SOL_E4_VESPER, PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	end

SolmaraLeague_Vesper_EventScript_Member::
	lock
	faceplayer
	goto_if_set FLAG_SOL_E4_VESPER, SolmaraLeague_Vesper_EventScript_After
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox SolmaraLeague_Vesper_EventScript_Text_Intro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_SOL_E4_VESPER
	trainerbattle_no_intro TRAINER_SOL_E4_VESPER, SolmaraLeague_Vesper_EventScript_Text_Defeat
	setflag FLAG_SOL_E4_VESPER
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	msgbox SolmaraLeague_Vesper_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Vesper_EventScript_After::
	msgbox SolmaraLeague_Vesper_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Vesper_EventScript_Text_Intro:
	.string "Good evening. I am VESPER of the\n"
	.string "ELITE FOUR.\p"
	.string "Darkness hides many things.\n"
	.string "Let's see what yours hides!$"

SolmaraLeague_Vesper_EventScript_Text_Defeat:
	.string "The night is over.$"

SolmaraLeague_Vesper_EventScript_Text_After:
	.string "Go on. The next room awaits.$"
