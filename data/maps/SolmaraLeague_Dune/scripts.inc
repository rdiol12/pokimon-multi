SolmaraLeague_Dune_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SolmaraLeague_Dune_OnLoad
	.byte 0

SolmaraLeague_Dune_OnLoad:
	call_if_set FLAG_SOL_E4_DUNE, PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	end

SolmaraLeague_Dune_EventScript_Member::
	lock
	faceplayer
	goto_if_set FLAG_SOL_E4_DUNE, SolmaraLeague_Dune_EventScript_After
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox SolmaraLeague_Dune_EventScript_Text_Intro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_SOL_E4_DUNE
	trainerbattle_no_intro TRAINER_SOL_E4_DUNE, SolmaraLeague_Dune_EventScript_Text_Defeat
	setflag FLAG_SOL_E4_DUNE
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	msgbox SolmaraLeague_Dune_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Dune_EventScript_After::
	msgbox SolmaraLeague_Dune_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Dune_EventScript_Text_Intro:
	.string "DUNE's the name. The ground\n"
	.string "under your feet is mine.\p"
	.string "Let's shake things up!$"

SolmaraLeague_Dune_EventScript_Text_Defeat:
	.string "The ground gave way...$"

SolmaraLeague_Dune_EventScript_Text_After:
	.string "Go on. The next room awaits.$"
