BattleLeague_Room4_MapScripts::
	.byte 0

BattleLeague_Room4_EventScript_Trainer::
	lock
	faceplayer
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox BattleLeague_Room4_Text_Intro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_LEAGUE_MORGANA, BattleLeague_Room4_Text_Defeat
	msgbox BattleLeague_Room4_Text_PostBattle, MSGBOX_DEFAULT
	closemessage
	warp MAP_BATTLE_LEAGUE_CHAMPION_ROOM, 6, 7
	waitstate
	release
	end

BattleLeague_Room4_Text_Intro:
	.string "Fufufu... I'm MORGANA.\p"
	.string "Ghosts, minds and shadows.\n"
	.string "You'll never see my next move.$"

BattleLeague_Room4_Text_Defeat:
	.string "The spirits... were wrong about you.$"

BattleLeague_Room4_Text_PostBattle:
	.string "The CHAMPION waits beyond.\n"
	.string "I sense a storm in that room.$"
