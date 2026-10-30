BattleLeague_ChampionRoom_MapScripts::
	.byte 0

BattleLeague_ChampionRoom_EventScript_Trainer::
	lock
	faceplayer
	playbgm MUS_ENCOUNTER_CHAMPION, FALSE
	msgbox BattleLeague_ChampionRoom_Text_Intro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_LEAGUE_REN, BattleLeague_ChampionRoom_Text_Defeat
	msgbox BattleLeague_ChampionRoom_Text_PostBattle, MSGBOX_DEFAULT
	addvar VAR_BATTLE_LEAGUE_CLEARS, 1
	setflag FLAG_BATTLE_LEAGUE_CLEARED
	closemessage
	warp MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F, 9, 5
	waitstate
	release
	end

BattleLeague_ChampionRoom_Text_Intro:
	.string "So you made it through all four.\p"
	.string "I'm REN, CHAMPION of the\n"
	.string "BATTLE LEAGUE.\p"
	.string "Only the strongest trainers ever\n"
	.string "reach this room. Let's make this\l"
	.string "a battle worth remembering!$"

BattleLeague_ChampionRoom_Text_Defeat:
	.string "Incredible... You are the\n"
	.string "BATTLE LEAGUE CHAMPION!$"

BattleLeague_ChampionRoom_Text_PostBattle:
	.string "Your victory has been recorded.\p"
	.string "Come back anytime. The LEAGUE\n"
	.string "will be waiting for a rematch!$"
