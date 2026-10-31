@ Roaming trainers, charging wild Pokémon and the quest boards
@ (src/rovers.c, docs/rovers.md).

Rovers_EventScript_Talk::
	lock
	callnative Rovers_BufferTalk
	goto_if_eq VAR_RESULT, 1, Rovers_EventScript_Duel
	goto_if_eq VAR_RESULT, 2, Rovers_EventScript_Busy
	faceplayer
	callnative Rovers_IsFriend
	goto_if_eq VAR_RESULT, TRUE, Rovers_EventScript_FriendHello
	msgbox Rovers_Text_Hello, MSGBOX_DEFAULT
Rovers_EventScript_AfterHello:
	callnative Rovers_BufferTalkStats
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
	callnative Rovers_TradeOffer
	goto_if_eq VAR_RESULT, FALSE, Rovers_EventScript_ByeNow
	msgbox Rovers_Text_TradeAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Rovers_EventScript_ByeNow
	msgbox Rovers_Text_TradePick, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Rovers_EventScript_ByeNow
	callnative Rovers_TradeCheck
	goto_if_eq VAR_RESULT, 1, Rovers_EventScript_TradeEgg
	goto_if_eq VAR_RESULT, 2, Rovers_EventScript_TradeWrongType
	goto_if_eq VAR_RESULT, 3, Rovers_EventScript_TradeLastMon
	callnative Rovers_PrepareTrade
	special DoInGameTradeScene
	waitstate
	callnative Rovers_AfterTrade
	msgbox Rovers_Text_TradeThanks, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_FriendHello:
	msgbox Rovers_Text_FriendHello, MSGBOX_DEFAULT
	callnative Rovers_GiftFetch, waitstate=1
	goto_if_eq VAR_0x8004, ITEM_NONE, Rovers_EventScript_AfterHello
	msgbox Rovers_Text_FriendGift, MSGBOX_DEFAULT
	giveitem VAR_0x8004, VAR_0x8005
	goto Rovers_EventScript_AfterHello

@ A rival the player beat walks up for a rematch: no saying no.
Rovers_EventScript_Rival::
	lock
	faceplayer
	msgbox Rovers_Text_RivalChallenge, MSGBOX_DEFAULT
	closemessage
	callnative Rovers_StartChallenge, waitstate=1
	msgbox Rovers_Text_RivalAfter, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_TradeEgg:
	msgbox Rovers_Text_TradeEgg, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_TradeWrongType:
	msgbox Rovers_Text_TradeWrongType, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_TradeLastMon:
	msgbox Rovers_Text_TradeLastMon, MSGBOX_DEFAULT
	release
	end

Rovers_EventScript_ByeNow:
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

Rovers_Text_FriendHello:
	.string "Hey, it's you! Good to see you\n"
	.string "out here, friend!$"

Rovers_Text_FriendGift:
	.string "I made something for you.\n"
	.string "Here, take it!$"

Rovers_Text_RivalChallenge:
	.string "{STR_VAR_1}: There you are!\p"
	.string "I've been training ever since you\n"
	.string "beat me. This time I'll win!$"

Rovers_Text_RivalAfter:
	.string "I'll keep training.\n"
	.string "Next time it's my turn!$"

Rovers_Text_TradeAsk:
	.string "Want to trade, then?\n"
	.string "I'd give my {STR_VAR_2} for a\p"
	.string "{STR_VAR_3}-type Pokémon.$"

Rovers_Text_TradePick:
	.string "Which one will you trade?$"

Rovers_Text_TradeEgg:
	.string "An Egg? I'd rather have\n"
	.string "a Pokémon I can battle with.$"

Rovers_Text_TradeWrongType:
	.string "{STR_VAR_2}? I'm only after\n"
	.string "{STR_VAR_3}-type Pokémon.$"

Rovers_Text_TradeLastMon:
	.string "Then you'd have nobody left\n"
	.string "to battle with!$"

Rovers_Text_TradeThanks:
	.string "Thanks! Take good care of it.\n"
	.string "I'll train mine hard!$"

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
