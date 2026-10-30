BattleLeague_Room3_MapScripts::
	.byte 0

BattleLeague_Room3_EventScript_Trainer::
	lock
	faceplayer
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox BattleLeague_Room3_Text_Intro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_LEAGUE_RYU, BattleLeague_Room3_Text_Defeat
	msgbox BattleLeague_Room3_Text_PostBattle, MSGBOX_DEFAULT
	closemessage
	warp MAP_BATTLE_LEAGUE_ROOM4, 6, 7
	waitstate
	release
	end

BattleLeague_Room3_Text_Intro:
	.string "I am RYU. My DRAGONS were\n"
	.string "raised in the ice of Meteor Falls.\p"
	.string "Few have survived their breath.$"

BattleLeague_Room3_Text_Defeat:
	.string "My dragons... bow to you.$"

BattleLeague_Room3_Text_PostBattle:
	.string "One room remains before\n"
	.string "the CHAMPION. Tread carefully.$"
