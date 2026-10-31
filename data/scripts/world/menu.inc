@ The R button menu: riding, achievements, records, photo mode and hunts.

World_EventScript_Menu::
	lockall
	dynmultipush World_Text_MenuRide, WORLD_MENU_RIDE
	dynmultipush World_Text_MenuAchievements, WORLD_MENU_ACHIEVEMENTS
	dynmultipush World_Text_MenuRecords, WORLD_MENU_RECORDS
	dynmultipush World_Text_MenuQuests, WORLD_MENU_QUESTS
	dynmultipush World_Text_MenuBoards, WORLD_MENU_BOARDS
	dynmultipush World_Text_MenuHunt, WORLD_MENU_HUNT
	dynmultipush World_Text_MenuPhoto, WORLD_MENU_PHOTO
	dynmultipush World_Text_MenuAdventure, WORLD_MENU_ADVENTURE
	dynmultipush World_Text_MenuClose, WORLD_MENU_CLOSE
	dynmultistack 0, 0, FALSE, 6, FALSE, WORLD_MENU_RIDE, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case WORLD_MENU_RIDE, World_EventScript_Ride
	case WORLD_MENU_ADVENTURE, Adventure_EventScript_Menu
	case WORLD_MENU_ACHIEVEMENTS, World_EventScript_Achievements
	case WORLD_MENU_RECORDS, World_EventScript_Records
	case WORLD_MENU_QUESTS, World_EventScript_Quests
	case WORLD_MENU_BOARDS, World_EventScript_Boards
	case WORLD_MENU_HUNT, World_EventScript_Hunt
	case WORLD_MENU_PHOTO, World_EventScript_Photo
	releaseall
	end

World_EventScript_Ride::
	callnative Mount_Toggle
	switch VAR_RESULT
	case MOUNT_RODE_OFF, World_EventScript_RodeOff
	case MOUNT_NONE_IN_PARTY, World_EventScript_NothingToRide
	case MOUNT_NOT_HERE, World_EventScript_CantRideHere
	releaseall
	end

World_EventScript_Achievements::
	callnative Achievements_ShowList, waitstate=1
	releaseall
	end

World_EventScript_Records::
	callnative Records_ShowList, waitstate=1
	releaseall
	end

World_EventScript_Quests::
	callnative Legends_ShowList, waitstate=1
	releaseall
	end

World_EventScript_Boards::
	dynmultipush World_Text_BoardFish, WORLD_BOARD_FISH
	dynmultipush World_Text_BoardBerries, WORLD_BOARD_BERRIES
	dynmultipush World_Text_BoardHardcore, WORLD_BOARD_HARDCORE
	dynmultipush World_Text_BoardHunt, WORLD_BOARD_HUNT
	dynmultistack 0, 0, FALSE, 4, FALSE, WORLD_BOARD_FISH, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, World_EventScript_MenuDone
	copyvar VAR_0x8004, VAR_RESULT
World_EventScript_ShowBoard:
	callnative WorldOnline_ShowBoard, waitstate=1
World_EventScript_MenuDone:
	releaseall
	end

World_EventScript_Hunt::
	callnative WorldOnline_GetHunt
	goto_if_eq VAR_RESULT, 0, World_EventScript_HuntOffline
	goto_if_eq VAR_RESULT, 1, World_EventScript_HuntWaiting
	msgbox World_Text_HuntOn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, World_EventScript_MenuDone
	setvar VAR_0x8004, WORLD_BOARD_HUNT
	goto World_EventScript_ShowBoard

World_EventScript_HuntOffline:
	msgbox World_Text_HuntOffline, MSGBOX_DEFAULT
	releaseall
	end

World_EventScript_HuntWaiting:
	msgbox World_Text_HuntWaiting, MSGBOX_DEFAULT
	releaseall
	end

World_EventScript_Photo::
	delay 8
	callnative WorldOnline_TakePhoto
	delay 4
	playse SE_CLICK
	waitse
	goto_if_eq VAR_RESULT, FALSE, World_EventScript_PhotoSaved
	msgbox World_Text_PhotoUploaded, MSGBOX_DEFAULT
	releaseall
	end

World_EventScript_PhotoSaved:
	msgbox World_Text_PhotoSaved, MSGBOX_DEFAULT
	releaseall
	end

World_EventScript_RodeOff:
	waitmoncry
	releaseall
	end

World_EventScript_NothingToRide:
	msgbox World_Text_NothingToRide, MSGBOX_DEFAULT
	releaseall
	end

World_EventScript_CantRideHere:
	msgbox World_Text_CantRideHere, MSGBOX_DEFAULT
	releaseall
	end

World_Text_MenuRide:
	.string "RIDE$"
World_Text_MenuAdventure:
	.string "ADVENTURE$"
World_Text_MenuAchievements:
	.string "ACHIEVEMENTS$"
World_Text_MenuRecords:
	.string "RECORDS$"
World_Text_MenuQuests:
	.string "QUESTS$"
World_Text_MenuBoards:
	.string "LEADERBOARDS$"
World_Text_BoardFish:
	.string "BIGGEST FISH$"
World_Text_BoardBerries:
	.string "BERRY PICKERS$"
World_Text_BoardHardcore:
	.string "HARDCORE LADDER$"
World_Text_BoardHunt:
	.string "SAFARI HUNT$"

World_Text_HuntOn:
	.string "SAFARI HUNT: catch the biggest\n"
	.string "{STR_VAR_1}! {STR_VAR_2} minutes left.\p"
	.string "See the standings?$"

World_Text_HuntOffline:
	.string "Safari hunts are played online.\n"
	.string "Pick JOIN ONLINE on the title screen.$"

World_Text_HuntWaiting:
	.string "Waiting for the server to announce\n"
	.string "the next hunt…$"

World_Text_PhotoUploaded:
	.string "Snap! The photo is in the online\n"
	.string "gallery.$"

World_Text_PhotoSaved:
	.string "Snap! Photos are saved by the\n"
	.string "netplay bridge, and shared online.$"

World_Text_MenuPhoto:
	.string "PHOTO$"
World_Text_MenuHunt:
	.string "SAFARI HUNT$"
World_Text_MenuClose:
	.string "CLOSE$"

World_Text_NothingToRide:
	.string "None of your Pokémon can carry you.\p"
	.string "Big Pokémon like PONYTA, ARCANINE,\n"
	.string "RHYHORN or MUDSDALE can be ridden.$"

World_Text_CantRideHere:
	.string "{STR_VAR_1} can't carry you here.$"
