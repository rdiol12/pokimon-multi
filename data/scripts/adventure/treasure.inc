@ Treasure hunts (src/treasure.c): the old treasure hunter in Dewford Town
@ hands out one treasure map a day; A on the X digs the treasure up.

Adventure_EventScript_TreasureHunter::
	lock
	faceplayer
	callnative Treasure_Talk
	switch VAR_RESULT
	case TREASURE_TALK_FIRST, Adventure_EventScript_TreasureFirst
	case TREASURE_TALK_NEW, Adventure_EventScript_TreasureNew
	case TREASURE_TALK_DONE, Adventure_EventScript_TreasureDone
	msgbox Adventure_Text_TreasureStill, MSGBOX_DEFAULT
	goto Adventure_EventScript_TreasureShowMap

Adventure_EventScript_TreasureFirst:
	msgbox Adventure_Text_TreasureFirst, MSGBOX_DEFAULT
	goto Adventure_EventScript_TreasureGiveMap

Adventure_EventScript_TreasureNew:
	msgbox Adventure_Text_TreasureNew, MSGBOX_DEFAULT
Adventure_EventScript_TreasureGiveMap:
	playfanfare MUS_OBTAIN_ITEM
	message Adventure_Text_TreasureGotMap
	waitfanfare
	waitmessage
Adventure_EventScript_TreasureShowMap:
	callnative Treasure_OpenMap
	msgbox Adventure_Text_TreasureRiddle, MSGBOX_DEFAULT
	callnative Treasure_CloseMap
	msgbox Adventure_Text_TreasureHint, MSGBOX_DEFAULT
	release
	end

Adventure_EventScript_TreasureDone:
	msgbox Adventure_Text_TreasureDone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Adventure_EventScript_TreasureBye
	closemessage
	setvar VAR_0x8004, WORLD_BOARD_TREASURE
	callnative WorldOnline_ShowBoard, waitstate=1
Adventure_EventScript_TreasureBye:
	release
	end

@ R > ADVENTURE > TREASURE MAP.
Adventure_EventScript_TreasureMap::
	callnative Treasure_HasMap
	goto_if_eq VAR_RESULT, FALSE, Adventure_EventScript_TreasureNoMap
	callnative Treasure_OpenMap
	msgbox Adventure_Text_TreasureRiddle, MSGBOX_DEFAULT
	callnative Treasure_CloseMap
	releaseall
	end

Adventure_EventScript_TreasureNoMap:
	msgbox Adventure_Text_TreasureNoMap, MSGBOX_DEFAULT
	releaseall
	end

Adventure_EventScript_TreasureDig::
	lockall
	playse SE_M_DIG
	msgbox Adventure_Text_TreasureDigging, MSGBOX_DEFAULT
	callnative Treasure_Dig
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	msgbox Adventure_Text_TreasureChest, MSGBOX_DEFAULT
	waitfanfare
	giveitem VAR_0x8000
	msgbox Adventure_Text_TreasureCount, MSGBOX_DEFAULT
	releaseall
	end

Adventure_EventScript_TreasureTooLate::
	lockall
	msgbox Adventure_Text_TreasureTooLate, MSGBOX_DEFAULT
	releaseall
	end

Adventure_Text_TreasureFirst:
	.string "Ahoy! You've the look of an explorer.\p"
	.string "I've hidden treasure all over HOENN\n"
	.string "in my day, more than I can dig up\l"
	.string "with these old bones.\p"
	.string "Every day I'll hand you one map.\n"
	.string "Find the X and press A to dig!$"

Adventure_Text_TreasureNew:
	.string "A new day, a new map. Here, take\n"
	.string "today's!$"

Adventure_Text_TreasureGotMap:
	.string "{PLAYER} received a TREASURE MAP!$"

Adventure_Text_TreasureStill:
	.string "Still looking? Let's have another\n"
	.string "look at that map.$"

Adventure_Text_TreasureRiddle:
	.string "{STR_VAR_1}$"

Adventure_Text_TreasureHint:
	.string "You can look at it again any time:\n"
	.string "R, then ADVENTURE, TREASURE MAP.$"

Adventure_Text_TreasureDone:
	.string "You dug up today's treasure! Come\n"
	.string "back tomorrow for a new map.\p"
	.string "Want to see the TREASURE HUNTERS?$"

Adventure_Text_TreasureNoMap:
	.string "No treasure map for today yet.\n"
	.string "The old treasure hunter in DEWFORD\l"
	.string "TOWN hands one out every day.$"

Adventure_Text_TreasureDigging:
	.string "{PLAYER} dug where the X was…\p"
	.string "There's something down there!$"

Adventure_Text_TreasureChest:
	.string "{PLAYER} dug up an old treasure chest!$"

Adventure_Text_TreasureCount:
	.string "Treasures found: {STR_VAR_2}.$"

Adventure_Text_TreasureTooLate:
	.string "The map is from another day…\n"
	.string "Nothing's left here to dig up.$"
