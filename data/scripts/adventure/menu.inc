@ R > ADVENTURE: the fourth-batch features that have a screen of their own.

Adventure_EventScript_Menu::
	dynmultipush Adventure_Text_MenuTrainerLevel, ADVENTURE_MENU_TRAINER_LEVEL
	dynmultipush Adventure_Text_MenuOutfits, ADVENTURE_MENU_OUTFITS
	dynmultipush Adventure_Text_MenuDailyQuests, ADVENTURE_MENU_DAILY_QUESTS
	dynmultipush Adventure_Text_MenuCrafting, ADVENTURE_MENU_CRAFTING
	dynmultipush Adventure_Text_MenuTreasureMap, ADVENTURE_MENU_TREASURE_MAP
	dynmultipush Adventure_Text_MenuClose, ADVENTURE_MENU_CLOSE
	dynmultistack 0, 0, FALSE, 6, FALSE, ADVENTURE_MENU_TRAINER_LEVEL, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case ADVENTURE_MENU_TRAINER_LEVEL, Adventure_EventScript_TrainerLevel
	case ADVENTURE_MENU_OUTFITS, Adventure_EventScript_Outfits
	case ADVENTURE_MENU_DAILY_QUESTS, Adventure_EventScript_DailyQuests
	case ADVENTURE_MENU_CRAFTING, Adventure_EventScript_Crafting
	case ADVENTURE_MENU_TREASURE_MAP, Adventure_EventScript_TreasureMap
Adventure_EventScript_MenuDone::
	releaseall
	end

Adventure_EventScript_TrainerLevel:
	callnative TrainerLevel_ShowCard, waitstate=1
	releaseall
	end

Adventure_EventScript_Outfits:
	callnative TrainerLevel_ShowOutfitMenu, waitstate=1
	goto_if_eq VAR_RESULT, FALSE, Adventure_EventScript_NoOutfits
	releaseall
	end

Adventure_EventScript_DailyQuests:
	callnative DailyQuest_ShowList, waitstate=1
	releaseall
	end

Adventure_EventScript_NoOutfits:
	msgbox Adventure_Text_NoOutfits, MSGBOX_DEFAULT
	releaseall
	end

Adventure_Text_MenuTrainerLevel:
	.string "TRAINER LEVEL$"
Adventure_Text_MenuOutfits:
	.string "OUTFITS$"
Adventure_Text_MenuDailyQuests:
	.string "DAILY QUESTS$"
Adventure_Text_MenuCrafting:
	.string "CRAFTING$"
Adventure_Text_MenuTreasureMap:
	.string "TREASURE MAP$"
Adventure_Text_MenuClose:
	.string "CLOSE$"

Adventure_Text_NoOutfits:
	.string "Outfits are made for BRENDAN and MAY.\n"
	.string "Pick one of them in START > STYLE.$"
