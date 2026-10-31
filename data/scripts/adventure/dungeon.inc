@ The spelunker outside Meteor Falls on Route 114 lets the player down into
@ the Rift Below (data/maps/AdventureDungeon, src/dungeon.c).

Adventure_EventScript_Spelunker::
	lock
	faceplayer
	callnative Dungeon_GetBest
	goto_if_eq VAR_RESULT, 0, Adventure_EventScript_SpelunkerFirst
	msgbox Adventure_Text_SpelunkerBest, MSGBOX_DEFAULT
	goto Adventure_EventScript_SpelunkerMenu

Adventure_EventScript_SpelunkerFirst:
	msgbox Adventure_Text_SpelunkerFirst, MSGBOX_DEFAULT
Adventure_EventScript_SpelunkerMenu:
	message Adventure_Text_SpelunkerAsk
	waitmessage
	dynmultipush Adventure_Text_SpelunkerDiveOption, 0
	dynmultipush Adventure_Text_SpelunkerBoardOption, 1
	dynmultipush Adventure_Text_SpelunkerCancelOption, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Adventure_EventScript_SpelunkerDive
	case 1, Adventure_EventScript_SpelunkerBoard
	msgbox Adventure_Text_SpelunkerBye, MSGBOX_DEFAULT
	release
	end

Adventure_EventScript_SpelunkerDive:
	msgbox Adventure_Text_SpelunkerDive, MSGBOX_DEFAULT
	closemessage
	callnative Dungeon_Enter
	playse SE_EXIT
	warp MAP_ADVENTURE_DUNGEON, 16, 27
	waitstate
	release
	end

Adventure_EventScript_SpelunkerBoard:
	closemessage
	setvar VAR_0x8004, WORLD_BOARD_DUNGEON
	callnative WorldOnline_ShowBoard, waitstate=1
	release
	end

Adventure_Text_SpelunkerFirst:
	.string "Under these falls there's a rift\n"
	.string "that goes down and down.\p"
	.string "The tunnels shift every time you\n"
	.string "climb down, so no map ever helps.\p"
	.string "The deeper you go, the better the\n"
	.string "finds, and the tougher the POKéMON.\p"
	.string "Find the ladder on each floor to\n"
	.string "go deeper. The way you came in\l"
	.string "always leads back up here.$"

Adventure_Text_SpelunkerBest:
	.string "Back for more? Your deepest dive so\n"
	.string "far is B{STR_VAR_1}F.$"

Adventure_Text_SpelunkerAsk:
	.string "Want to climb down?$"

Adventure_Text_SpelunkerDive:
	.string "Every dive starts at B1F.\n"
	.string "Good luck down there!$"

Adventure_Text_SpelunkerDiveOption:
	.string "CLIMB DOWN$"
Adventure_Text_SpelunkerBoardOption:
	.string "DEEPEST DIVERS$"
Adventure_Text_SpelunkerCancelOption:
	.string "CANCEL$"

Adventure_Text_SpelunkerBye:
	.string "The rift will be here.$"
