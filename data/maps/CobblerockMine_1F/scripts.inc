CobblerockMine_1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, CobblerockMine_1F_OnLoad
	.byte 0

CobblerockMine_1F_OnLoad:
	call_if_set FLAG_COBBLE_ORE_1, CobblerockMine_1F_EventScript_Mined1
	end

CobblerockMine_1F_EventScript_Mined1::
	setmetatile 3, 19, 938, TRUE
	return

CobblerockMine_1F_EventScript_Ore1::
	lockall
	goto_if_set FLAG_COBBLE_ORE_1, CobblerockMine_1F_EventScript_OreDone
	msgbox Cobblerock_Text_OreAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CobblerockMine_1F_EventScript_OreLeave
	playse SE_M_ROCK_THROW
	setmetatile 3, 19, 938, TRUE
	special DrawWholeMapView
	setflag FLAG_COBBLE_ORE_1
	msgbox Cobblerock_Text_OreGot, MSGBOX_DEFAULT
	releaseall
	end

CobblerockMine_1F_EventScript_OreDone::
	msgbox Cobblerock_Text_OreDone, MSGBOX_DEFAULT
	releaseall
	end

CobblerockMine_1F_EventScript_OreLeave::
	releaseall
	end


CobblerockMine_1F_EventScript_Miner::
	msgbox CobblerockMine_1F_Text_Miner, MSGBOX_NPC
	end

CobblerockMine_1F_EventScript_Grant::
	trainerbattle_single TRAINER_COBBLE_GRANT, CobblerockMine_1F_Text_GrantIntro, CobblerockMine_1F_Text_GrantDefeat
	msgbox CobblerockMine_1F_Text_GrantPost, MSGBOX_AUTOCLOSE
	end

CobblerockMine_1F_Text_GrantIntro:
	.string "You're no miner! But you look like\n"
	.string "you could dig up a battle.$"

CobblerockMine_1F_Text_GrantDefeat:
	.string "Rock bottom…$"

CobblerockMine_1F_Text_GrantPost:
	.string "The ladder down is in the room\n"
	.string "at the end of the north drift.$"


CobblerockMine_1F_Text_Miner:
	.string "Most of the good veins on this floor\n"
	.string "are dug out. Try the west drift,\l"
	.string "past the old cart.$"

CobblerockMine_1F_EventScript_Item_EscapeRope::
	finditem ITEM_ESCAPE_ROPE
	end

CobblerockMine_1F_EventScript_Item_HardStone::
	finditem ITEM_HARD_STONE
	end

