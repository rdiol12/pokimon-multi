@ Roaming trainers, charging wild Pokémon and the quest boards
@ (src/rovers.c, docs/rovers.md).

Rovers_EventScript_Talk::
	lock
	callnative Rovers_BufferTalk
	goto_if_eq VAR_RESULT, 1, Rovers_EventScript_Duel
	goto_if_eq VAR_RESULT, 2, Rovers_EventScript_Busy
	faceplayer
	msgbox Rovers_Text_Hello, MSGBOX_DEFAULT
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Rovers_Text_AskBattle, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Rovers_EventScript_Bye
	msgbox Rovers_Text_Go, MSGBOX_DEFAULT
	closemessage
	callnative Rovers_StartChallenge, waitstate=1
	msgbox Rovers_Text_After, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_Bye:
	msgbox Rovers_Text_Bye, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_Busy:
	msgbox Rovers_Text_Busy, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_Duel:
	msgbox Rovers_Text_DuelAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Rovers_EventScript_DuelNo
	msgbox Rovers_Text_DuelWatch, MSGBOX_DEFAULT
	closemessage
	callnative Rovers_StartWatch, waitstate=1
	goto_if_eq VAR_RESULT, FALSE, Rovers_EventScript_DuelRightWon
	msgbox Rovers_Text_DuelLeftWon, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_DuelRightWon:
	msgbox Rovers_Text_DuelRightWon, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_DuelNo:
	release
	end

Rovers_EventScript_Charger::
	lock
	callnative Rovers_BufferCharger
	playmoncry VAR_0x8004, CRY_MODE_ENCOUNTER
	goto_if_eq VAR_RESULT, TRUE, Rovers_EventScript_MajorIntro
	msgbox Rovers_Text_ChargerAttacks, MSGBOX_DEFAULT
	goto Rovers_EventScript_ChargerBattle

Rovers_EventScript_MajorIntro:
	msgbox Rovers_Text_MajorAttacks, MSGBOX_DEFAULT
Rovers_EventScript_ChargerBattle:
	waitmoncry
	closemessage
	callnative Rovers_SetupChargerBattle
	dowildbattle
	callnative Rovers_AfterChargerBattle
	release
	end

Rovers_EventScript_Board::
	lockall
	msgbox Rovers_Text_BoardIntro, MSGBOX_DEFAULT
	closemessage
	callnative Rovers_BoardFetch, waitstate=1
	callnative Rovers_BoardShowJobs
	goto_if_eq VAR_RESULT, FALSE, Rovers_EventScript_BoardOffline
	goto_if_eq VAR_0x8005, 0, Rovers_EventScript_BoardJobs
	playfanfare MUS_OBTAIN_ITEM
	msgbox Rovers_Text_BoardReward, MSGBOX_DEFAULT
	waitfanfare
Rovers_EventScript_BoardJobs:
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Rovers_Text_BoardAskRankings, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Rovers_EventScript_BoardDone
	closemessage
	callnative Rovers_BoardRankingsFetch, waitstate=1
	callnative Rovers_BoardShowRankings
	goto_if_eq VAR_RESULT, FALSE, Rovers_EventScript_BoardOffline
	msgbox gStringVar4, MSGBOX_DEFAULT
Rovers_EventScript_BoardDone:
	releaseall
	end

Rovers_EventScript_BoardOffline:
	msgbox Rovers_Text_BoardOffline, MSGBOX_DEFAULT
	releaseall
	end

Rovers_Text_Hello:
	.string "I'm {STR_VAR_1} the {STR_VAR_2}!\n"
	.string "I'm traveling all over HOENN.\p"
	.string "I've got {STR_VAR_3} badges so far.$"

Rovers_Text_AskBattle:
	.string "Want to battle me?$"

Rovers_Text_Go:
	.string "All right! Show me what you've got!$"

Rovers_Text_After:
	.string "That was fun!\n"
	.string "See you on the road!$"

Rovers_Text_Bye:
	.string "Maybe next time.\n"
	.string "I've got Pokémon to catch!$"

Rovers_Text_Busy:
	.string "Shh! There's a wild Pokémon!$"

Rovers_Text_DuelAsk:
	.string "{STR_VAR_1} and {STR_VAR_2} are in the\n"
	.string "middle of a battle!\p"
	.string "Watch it?$"

Rovers_Text_DuelWatch:
	.string "You stop to watch…$"

Rovers_Text_DuelLeftWon:
	.string "{STR_VAR_1} won the battle!$"

Rovers_Text_DuelRightWon:
	.string "{STR_VAR_2} won the battle!$"

Rovers_Text_ChargerAttacks:
	.string "A wild {STR_VAR_1} burst out and\n"
	.string "attacked!$"

Rovers_Text_MajorAttacks:
	.string "The rampaging {STR_VAR_1} attacks!\p"
	.string "Catch it or knock it out to\n"
	.string "clear the quest board's job!$"

Rovers_Text_BoardIntro:
	.string "QUEST BOARD\n"
	.string "Dangerous wild Pokémon around here.$"

Rovers_Text_BoardReward:
	.string "You collected ¥{STR_VAR_2} for the\n"
	.string "jobs you cleared!$"

Rovers_Text_BoardAskRankings:
	.string "See the traveling trainers'\n"
	.string "rankings and news?$"

Rovers_Text_BoardOffline:
	.string "The board is updated online.\n"
	.string "Connect to see the latest jobs.$"
