SolmaraLeague_Champion_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SolmaraLeague_Champion_OnTransition
	.byte 0

SolmaraLeague_Champion_OnTransition:
	clearflag FLAG_SOL_CHAMPION_THIS_RUN
	end

SolmaraLeague_Champion_EventScript_Champion::
	lock
	faceplayer
	goto_if_set FLAG_SOL_CHAMPION_THIS_RUN, SolmaraLeague_Champion_EventScript_After
	playbgm MUS_ENCOUNTER_CHAMPION, FALSE
	msgbox SolmaraLeague_Champion_EventScript_Text_Intro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_SOL_CHAMPION_SOLENNE
	trainerbattle_no_intro TRAINER_SOL_CHAMPION_SOLENNE, SolmaraLeague_Champion_EventScript_Text_Defeat
	setflag FLAG_SOL_CHAMPION_THIS_RUN
	callnative Solmara_RecordLeagueClear
	goto_if_set FLAG_SOL_CHAMPION, SolmaraLeague_Champion_EventScript_Again
	setflag FLAG_SOL_CHAMPION
	msgbox SolmaraLeague_Champion_EventScript_Text_First, MSGBOX_DEFAULT
	giveitem ITEM_MASTER_BALL
	msgbox SolmaraLeague_Champion_EventScript_Text_Hall, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Champion_EventScript_Again::
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox SolmaraLeague_Champion_EventScript_Text_Again, MSGBOX_DEFAULT
	giveitem ITEM_RARE_CANDY, 3
	msgbox SolmaraLeague_Champion_EventScript_Text_Hall, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Champion_EventScript_After::
	msgbox SolmaraLeague_Champion_EventScript_Text_Hall, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Champion_EventScript_Text_Intro:
	.string "So you crossed the whole ring and\n"
	.string "climbed VICTORY ROAD.\p"
	.string "I am SOLENNE, Champion of\n"
	.string "SOLMARA. Every trainer who came\l"
	.string "over from HOENN has to get past\l"
	.string "me. Very few have!\p"
	.string "Show me everything!$"

SolmaraLeague_Champion_EventScript_Text_Defeat:
	.string "Magnificent! You are SOLMARA's\n"
	.string "new Champion!$"

SolmaraLeague_Champion_EventScript_Text_First:
	.string "The first time is special. Take\n"
	.string "this, Champion.$"

SolmaraLeague_Champion_EventScript_Text_Again:
	.string "That's {STR_VAR_1} wins at this LEAGUE!\n"
	.string "You keep getting stronger.$"

SolmaraLeague_Champion_EventScript_Text_Hall:
	.string "Go through to the hall. Your\n"
	.string "name belongs there.$"
