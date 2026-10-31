CobblerockMine_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, CobblerockMine_B1F_OnLoad
	.byte 0

CobblerockMine_B1F_OnLoad:
	call_if_set FLAG_COBBLE_ORE_2, CobblerockMine_B1F_EventScript_Mined2
	call_if_set FLAG_COBBLE_ORE_3, CobblerockMine_B1F_EventScript_Mined3
	end

CobblerockMine_B1F_EventScript_Mined2::
	setmetatile 8, 4, 938, TRUE
	return

CobblerockMine_B1F_EventScript_Ore2::
	lockall
	goto_if_set FLAG_COBBLE_ORE_2, CobblerockMine_B1F_EventScript_OreDone
	msgbox Cobblerock_Text_OreAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CobblerockMine_B1F_EventScript_OreLeave
	playse SE_M_ROCK_THROW
	setmetatile 8, 4, 938, TRUE
	special DrawWholeMapView
	setflag FLAG_COBBLE_ORE_2
	msgbox Cobblerock_Text_OreGot, MSGBOX_DEFAULT
	releaseall
	end

CobblerockMine_B1F_EventScript_Mined3::
	setmetatile 36, 27, 938, TRUE
	return

CobblerockMine_B1F_EventScript_Ore3::
	lockall
	goto_if_set FLAG_COBBLE_ORE_3, CobblerockMine_B1F_EventScript_OreDone
	msgbox Cobblerock_Text_OreAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CobblerockMine_B1F_EventScript_OreLeave
	playse SE_M_ROCK_THROW
	setmetatile 36, 27, 938, TRUE
	special DrawWholeMapView
	setflag FLAG_COBBLE_ORE_3
	msgbox Cobblerock_Text_OreGot, MSGBOX_DEFAULT
	releaseall
	end

CobblerockMine_B1F_EventScript_OreDone::
	msgbox Cobblerock_Text_OreDone, MSGBOX_DEFAULT
	releaseall
	end

CobblerockMine_B1F_EventScript_OreLeave::
	releaseall
	end


CobblerockMine_B1F_EventScript_Doug::
	trainerbattle_single TRAINER_COBBLE_DOUG, CobblerockMine_B1F_Text_DougIntro, CobblerockMine_B1F_Text_DougDefeat
	msgbox CobblerockMine_B1F_Text_DougPost, MSGBOX_AUTOCLOSE
	end

CobblerockMine_B1F_Text_DougIntro:
	.string "Deep down here, only the tough\n"
	.string "survive! Show me you're tough!$"

CobblerockMine_B1F_Text_DougDefeat:
	.string "You're tougher than granite!$"

CobblerockMine_B1F_Text_DougPost:
	.string "I heard there's a vein in the far\n"
	.string "west pocket. Mind the dead end.$"


CobblerockMine_B1F_EventScript_Todd::
	trainerbattle_single TRAINER_COBBLE_TODD, CobblerockMine_B1F_Text_ToddIntro, CobblerockMine_B1F_Text_ToddDefeat
	msgbox CobblerockMine_B1F_Text_ToddPost, MSGBOX_AUTOCLOSE
	end

CobblerockMine_B1F_Text_ToddIntro:
	.string "These tunnels are older than the\n"
	.string "town! Who knows what's buried here?$"

CobblerockMine_B1F_Text_ToddDefeat:
	.string "I dug up a loss.$"

CobblerockMine_B1F_Text_ToddPost:
	.string "The ancients dug here first. Their\n"
	.string "old shaft is sealed up, outside.$"

CobblerockMine_B1F_EventScript_Item_Everstone::
	finditem ITEM_EVERSTONE
	end

CobblerockMine_B1F_EventScript_Item_TmDig::
	finditem ITEM_TM_DIG
	end

CobblerockMine_B1F_EventScript_Item_SuperPotion::
	finditem ITEM_SUPER_POTION
	end

