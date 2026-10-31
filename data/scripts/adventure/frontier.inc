@ The online Battle Frontier (src/frontier_online.c): the records keeper
@ outside the Ranking Hall, and R > ADVENTURE > FRONTIER RECORDS.

Adventure_EventScript_FrontierKeeper::
	lock
	faceplayer
	msgbox Adventure_Text_FrontierKeeper, MSGBOX_DEFAULT
	call Adventure_EventScript_FrontierBoards
	release
	end

Adventure_EventScript_FrontierRecords::
	call Adventure_EventScript_FrontierBoards
	releaseall
	end

Adventure_EventScript_FrontierBoards:
	message Adventure_Text_FrontierWhich
	waitmessage
	dynmultipush Adventure_Text_FrontierFactory, WORLD_BOARD_FACTORY
	dynmultipush Adventure_Text_FrontierArena, WORLD_BOARD_ARENA
	dynmultipush Adventure_Text_FrontierPyramid, WORLD_BOARD_PYRAMID
	dynmultipush Adventure_Text_FrontierCancel, 254
	dynmultistack 0, 0, FALSE, 4, FALSE, WORLD_BOARD_FACTORY, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 254, Adventure_EventScript_FrontierDone
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Adventure_EventScript_FrontierDone
	closemessage
	copyvar VAR_0x8004, VAR_RESULT
	callnative WorldOnline_ShowBoard, waitstate=1
Adventure_EventScript_FrontierDone:
	closemessage
	return

Adventure_Text_FrontierKeeper:
	.string "I keep the online records of the\n"
	.string "BATTLE FACTORY, ARENA and PYRAMID.\p"
	.string "Every TRAINER's best streak goes\n"
	.string "on the boards, from all over!$"

Adventure_Text_FrontierWhich:
	.string "Which records?$"

Adventure_Text_FrontierFactory:
	.string "BATTLE FACTORY$"
Adventure_Text_FrontierArena:
	.string "BATTLE ARENA$"
Adventure_Text_FrontierPyramid:
	.string "BATTLE PYRAMID$"
Adventure_Text_FrontierCancel:
	.string "CANCEL$"
