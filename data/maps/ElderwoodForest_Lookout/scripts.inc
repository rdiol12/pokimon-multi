ElderwoodForest_Lookout_MapScripts::
	.byte 0

ElderwoodForest_Lookout_EventScript_Ranger::
	lock
	faceplayer
	goto_if_set FLAG_ELDERWOOD_LOOKOUT_GIFT, ElderwoodForest_Lookout_EventScript_RangerAfter
	msgbox ElderwoodForest_Lookout_Text_Ranger, MSGBOX_DEFAULT
	giveitem ITEM_BIG_ROOT
	goto_if_eq VAR_RESULT, FALSE, ElderwoodForest_Lookout_EventScript_End
	setflag FLAG_ELDERWOOD_LOOKOUT_GIFT
ElderwoodForest_Lookout_EventScript_RangerAfter::
	msgbox ElderwoodForest_Lookout_Text_RangerAfter, MSGBOX_DEFAULT
ElderwoodForest_Lookout_EventScript_End::
	release
	end

ElderwoodForest_Lookout_EventScript_Girl::
	msgbox ElderwoodForest_Lookout_Text_Girl, MSGBOX_NPC
	end

ElderwoodForest_Lookout_Text_Ranger:
	.string "Welcome to the ranger lookout!\n"
	.string "From up here you can see the whole\l"
	.string "canopy.\p"
	.string "Here, a souvenir. Roots like this\n"
	.string "hold the whole forest together.$"

ElderwoodForest_Lookout_Text_RangerAfter:
	.string "Mushroom hunters say the best ones\n"
	.string "grow in the far corners, where the\l"
	.string "light hardly reaches.$"

ElderwoodForest_Lookout_Text_Girl:
	.string "I counted the treehouses: four!\n"
	.string "But only two doors open. The\l"
	.string "others are full of BOUNSWEET nests.$"
