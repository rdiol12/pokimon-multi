ElderwoodForest_ElderHouse_MapScripts::
	.byte 0

ElderwoodForest_ElderHouse_EventScript_Elder::
	lock
	faceplayer
	goto_if_set FLAG_ELDERWOOD_QUEST_DONE, ElderwoodForest_ElderHouse_EventScript_ElderAfter
	goto_if_unset FLAG_ELDERWOOD_BOUNSWEET_HOME, ElderwoodForest_ElderHouse_EventScript_ElderAsk
	goto_if_unset FLAG_ELDERWOOD_ROWLET_HOME, ElderwoodForest_ElderHouse_EventScript_ElderAsk
	goto_if_unset FLAG_ELDERWOOD_GROOKEY_HOME, ElderwoodForest_ElderHouse_EventScript_ElderAsk
	msgbox ElderwoodForest_ElderHouse_Text_Thanks, MSGBOX_DEFAULT
	giveitem ITEM_MIRACLE_SEED
	giveitem ITEM_SITRUS_BERRY, 5
	giveitem ITEM_TM_GIGA_DRAIN
	setflag FLAG_ELDERWOOD_QUEST_DONE
	msgbox ElderwoodForest_ElderHouse_Text_KingComes, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_ElderHouse_EventScript_ElderAsk::
	msgbox ElderwoodForest_ElderHouse_Text_Ask, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_ElderHouse_EventScript_ElderAfter::
	msgbox ElderwoodForest_ElderHouse_Text_After, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_ElderHouse_EventScript_Bounsweet::
	lock
	faceplayer
	playmoncry SPECIES_BOUNSWEET, CRY_MODE_NORMAL
	msgbox ElderwoodForest_ElderHouse_Text_Bounsweet, MSGBOX_DEFAULT
	waitmoncry
	release
	end

ElderwoodForest_ElderHouse_Text_Ask:
	.string "I am the ELDER of this wood.\p"
	.string "Three keepers tend the ELDER TREE:\n"
	.string "BOUNSWEET, ROWLET and GROOKEY.\p"
	.string "A storm scattered them across the\n"
	.string "forest. The tree droops without\l"
	.string "them.\p"
	.string "Please find them. Look in the tall\n"
	.string "grass, up on the walkways, and by\l"
	.string "the old shrine.$"

ElderwoodForest_ElderHouse_Text_Thanks:
	.string "All three keepers are home!\n"
	.string "The ELDER TREE is green again.\p"
	.string "Take these, with the forest's\n"
	.string "thanks.$"

ElderwoodForest_ElderHouse_Text_KingComes:
	.string "Listen… the drumming has stopped.\p"
	.string "When the keepers are home, the\n"
	.string "king of the jungle visits the old\l"
	.string "stump in the west woods.$"

ElderwoodForest_ElderHouse_Text_After:
	.string "The forest is at peace, thanks\n"
	.string "to you, {PLAYER}.$"

ElderwoodForest_ElderHouse_Text_Bounsweet:
	.string "BOUNSWEET: Bou-bou!$"
