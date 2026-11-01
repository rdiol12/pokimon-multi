SolmaraLeague_Nettle_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SolmaraLeague_Nettle_OnLoad
	.byte 0

SolmaraLeague_Nettle_OnLoad:
	call_if_set FLAG_SOL_E4_NETTLE, PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	end

SolmaraLeague_Nettle_EventScript_Member::
	lock
	faceplayer
	goto_if_set FLAG_SOL_E4_NETTLE, SolmaraLeague_Nettle_EventScript_After
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox SolmaraLeague_Nettle_EventScript_Text_Intro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_SOL_E4_NETTLE
	trainerbattle_no_intro TRAINER_SOL_E4_NETTLE, SolmaraLeague_Nettle_EventScript_Text_Defeat
	setflag FLAG_SOL_E4_NETTLE
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	msgbox SolmaraLeague_Nettle_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Nettle_EventScript_After::
	msgbox SolmaraLeague_Nettle_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Nettle_EventScript_Text_Intro:
	.string "I'm NETTLE! My POKéMON sting, bite\n"
	.string "and poison.\p"
	.string "Still want to battle? Of course\n"
	.string "you do!$"

SolmaraLeague_Nettle_EventScript_Text_Defeat:
	.string "Ouch! That stung!$"

SolmaraLeague_Nettle_EventScript_Text_After:
	.string "Go on. The next room awaits.$"
