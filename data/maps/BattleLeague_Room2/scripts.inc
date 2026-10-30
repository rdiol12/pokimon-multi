BattleLeague_Room2_MapScripts::
	.byte 0

BattleLeague_Room2_EventScript_Trainer::
	lock
	faceplayer
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox BattleLeague_Room2_Text_Intro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_LEAGUE_NOVA, BattleLeague_Room2_Text_Defeat
	msgbox BattleLeague_Room2_Text_PostBattle, MSGBOX_DEFAULT
	closemessage
	warp MAP_BATTLE_LEAGUE_ROOM3, 6, 7
	waitstate
	release
	end

BattleLeague_Room2_Text_Intro:
	.string "I'm NOVA. Fire and lightning,\n"
	.string "that's my whole style.\p"
	.string "Try not to get scorched!$"

BattleLeague_Room2_Text_Defeat:
	.string "You put out my spark...$"

BattleLeague_Room2_Text_PostBattle:
	.string "The cold room is next.\n"
	.string "Bundle up, challenger.$"
