@ The R button menu: riding, achievements, records, photo mode and hunts.

World_EventScript_Menu::
	lockall
	dynmultipush World_Text_MenuRide, WORLD_MENU_RIDE
	dynmultipush World_Text_MenuAchievements, WORLD_MENU_ACHIEVEMENTS
	dynmultipush World_Text_MenuRecords, WORLD_MENU_RECORDS
	dynmultipush World_Text_MenuClose, WORLD_MENU_CLOSE
	dynmultistack 0, 0, FALSE, 6, FALSE, WORLD_MENU_RIDE, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case WORLD_MENU_RIDE, World_EventScript_Ride
	case WORLD_MENU_ACHIEVEMENTS, World_EventScript_Achievements
	case WORLD_MENU_RECORDS, World_EventScript_Records
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
World_Text_MenuAchievements:
	.string "ACHIEVEMENTS$"
World_Text_MenuRecords:
	.string "RECORDS$"
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
