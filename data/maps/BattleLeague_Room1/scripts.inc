BattleLeague_Room1_MapScripts::
	.byte 0

BattleLeague_Room1_EventScript_Trainer::
	lock
	faceplayer
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox BattleLeague_Room1_Text_Intro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_LEAGUE_KAI, BattleLeague_Room1_Text_Defeat
	msgbox BattleLeague_Room1_Text_PostBattle, MSGBOX_DEFAULT
	closemessage
	warp MAP_BATTLE_LEAGUE_ROOM2, 6, 7
	waitstate
	release
	end

BattleLeague_Room1_Text_Intro:
	.string "Welcome to the BATTLE LEAGUE!\n"
	.string "I'm KAI, the first wall you face.\p"
	.string "My STEEL and FIGHTING POKéMON\n"
	.string "don't bend and don't break.\l"
	.string "Show me what you've got!$"

BattleLeague_Room1_Text_Defeat:
	.string "Hah! You hit harder than steel!$"

BattleLeague_Room1_Text_PostBattle:
	.string "Go on through. The next room\n"
	.string "burns hotter than this one.$"
