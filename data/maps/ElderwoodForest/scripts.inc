ElderwoodForest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ElderwoodForest_OnTransition
	.byte 0

@ Zarude only comes to the grove once the keepers are home.
ElderwoodForest_OnTransition:
	setflag FLAG_HIDE_ELDERWOOD_ZARUDE
	goto_if_set FLAG_ELDERWOOD_ZARUDE_DONE, ElderwoodForest_OnTransitionEnd
	goto_if_unset FLAG_ELDERWOOD_QUEST_DONE, ElderwoodForest_OnTransitionEnd
	clearflag FLAG_HIDE_ELDERWOOD_ZARUDE
ElderwoodForest_OnTransitionEnd:
	end

ElderwoodForest_EventScript_Guide::
	lock
	faceplayer
	msgbox ElderwoodForest_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ElderwoodForest_EventScript_GuideStay
	msgbox ElderwoodForest_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_FORTREE_CITY, 37, 10
	waitstate
	release
	end

ElderwoodForest_EventScript_GuideStay::
	msgbox ElderwoodForest_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_EntranceSign::
	msgbox ElderwoodForest_Text_EntranceSign, MSGBOX_SIGN
	end

ElderwoodForest_EventScript_ShrineSign::
	msgbox ElderwoodForest_Text_ShrineSign, MSGBOX_SIGN
	end

ElderwoodForest_EventScript_GroveStone::
	msgbox ElderwoodForest_Text_GroveStone, MSGBOX_SIGN
	end

@ The three keepers: each one heads home to the Elder Tree when found.
ElderwoodForest_EventScript_Bounsweet::
	lock
	faceplayer
	playmoncry SPECIES_BOUNSWEET, CRY_MODE_NORMAL
	msgbox ElderwoodForest_Text_Bounsweet, MSGBOX_DEFAULT
	waitmoncry
	setflag FLAG_ELDERWOOD_BOUNSWEET_HOME
	goto ElderwoodForest_EventScript_KeeperGoesHome_Bounsweet

ElderwoodForest_EventScript_KeeperGoesHome_Bounsweet::
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ELDERWOOD_BOUNSWEET
	fadescreen FADE_FROM_BLACK
	goto ElderwoodForest_EventScript_KeeperCount

ElderwoodForest_EventScript_Rowlet::
	lock
	faceplayer
	playmoncry SPECIES_ROWLET, CRY_MODE_NORMAL
	msgbox ElderwoodForest_Text_Rowlet, MSGBOX_DEFAULT
	waitmoncry
	setflag FLAG_ELDERWOOD_ROWLET_HOME
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ELDERWOOD_ROWLET
	fadescreen FADE_FROM_BLACK
	goto ElderwoodForest_EventScript_KeeperCount

ElderwoodForest_EventScript_Grookey::
	lock
	faceplayer
	playmoncry SPECIES_GROOKEY, CRY_MODE_NORMAL
	msgbox ElderwoodForest_Text_Grookey, MSGBOX_DEFAULT
	waitmoncry
	setflag FLAG_ELDERWOOD_GROOKEY_HOME
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ELDERWOOD_GROOKEY
	fadescreen FADE_FROM_BLACK
	goto ElderwoodForest_EventScript_KeeperCount

ElderwoodForest_EventScript_KeeperCount::
	goto_if_unset FLAG_ELDERWOOD_BOUNSWEET_HOME, ElderwoodForest_EventScript_KeepersLeft
	goto_if_unset FLAG_ELDERWOOD_ROWLET_HOME, ElderwoodForest_EventScript_KeepersLeft
	goto_if_unset FLAG_ELDERWOOD_GROOKEY_HOME, ElderwoodForest_EventScript_KeepersLeft
	msgbox ElderwoodForest_Text_AllKeepersHome, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_KeepersLeft::
	msgbox ElderwoodForest_Text_KeeperWentHome, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_Zarude::
	lock
	faceplayer
	setwildbattle SPECIES_ZARUDE, 45
	playmoncry SPECIES_ZARUDE, CRY_MODE_ENCOUNTER
	msgbox ElderwoodForest_Text_Zarude, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, ElderwoodForest_EventScript_ZarudeGone
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, ElderwoodForest_EventScript_ZarudeStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, ElderwoodForest_EventScript_ZarudeStays
	setflag FLAG_ELDERWOOD_ZARUDE_DONE
	release
	end

ElderwoodForest_EventScript_ZarudeGone::
	setflag FLAG_ELDERWOOD_ZARUDE_DONE
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ELDERWOOD_ZARUDE
	setflag FLAG_HIDE_ELDERWOOD_ZARUDE
	fadescreen FADE_FROM_BLACK
	msgbox ElderwoodForest_Text_ZarudeFled, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_ZarudeStays::
	msgbox ElderwoodForest_Text_ZarudeWatches, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_Ivy::
	trainerbattle_single TRAINER_ELDERWOOD_IVY, ElderwoodForest_Text_IvyIntro, ElderwoodForest_Text_IvyDefeat
	msgbox ElderwoodForest_Text_IvyPost, MSGBOX_AUTOCLOSE
	end

ElderwoodForest_EventScript_Hal::
	trainerbattle_single TRAINER_ELDERWOOD_HAL, ElderwoodForest_Text_HalIntro, ElderwoodForest_Text_HalDefeat
	msgbox ElderwoodForest_Text_HalPost, MSGBOX_AUTOCLOSE
	end

ElderwoodForest_EventScript_Fern::
	trainerbattle_single TRAINER_ELDERWOOD_FERN, ElderwoodForest_Text_FernIntro, ElderwoodForest_Text_FernDefeat
	msgbox ElderwoodForest_Text_FernPost, MSGBOX_AUTOCLOSE
	end

ElderwoodForest_EventScript_Fisher::
	msgbox ElderwoodForest_Text_Fisher, MSGBOX_NPC
	end

ElderwoodForest_EventScript_Botanist::
	msgbox ElderwoodForest_Text_Botanist, MSGBOX_NPC
	end

ElderwoodForest_EventScript_Kid::
	msgbox ElderwoodForest_Text_Kid, MSGBOX_NPC
	end

ElderwoodForest_Text_GuideBack:
	.string "The canopy paths can make a person\n"
	.string "dizzy. Shall I lead you back\l"
	.string "to FORTREE CITY?$"

ElderwoodForest_Text_GuideLead:
	.string "Follow the vines, and mind your step!$"

ElderwoodForest_Text_GuideStay:
	.string "Take your time. ELDERWOOD has been\n"
	.string "growing for a thousand years.$"

ElderwoodForest_Text_EntranceSign:
	.string "ELDERWOOD FOREST\n"
	.string "“Walk softly. The trees remember.”$"

ElderwoodForest_Text_ShrineSign:
	.string "A mossy stone shrine.\n"
	.string "“Here rest the keepers of the\l"
	.string "ELDER TREE: the sweet, the swift,\l"
	.string "and the drummer.”$"

ElderwoodForest_Text_GroveStone:
	.string "Words are carved into the stump.\p"
	.string "“When the three keepers sleep\n"
	.string "at home, the king of the jungle\l"
	.string "comes to listen.”$"

ElderwoodForest_Text_Bounsweet:
	.string "BOUNSWEET: Bou! Bou!\p"
	.string "It smells wonderfully sweet.\n"
	.string "It looks like it got lost in the\l"
	.string "tall grass.$"

ElderwoodForest_Text_Rowlet:
	.string "ROWLET: Hoo-hoo!\p"
	.string "It was dozing on the walkway.\n"
	.string "It blinks, then flutters off\l"
	.string "toward the ELDER TREE.$"

ElderwoodForest_Text_Grookey:
	.string "GROOKEY: Grook! Grook!\p"
	.string "It drums its stick on the shrine,\n"
	.string "then scampers off home.$"

ElderwoodForest_Text_KeeperWentHome:
	.string "The keeper went home to the\n"
	.string "ELDER TREE.$"

ElderwoodForest_Text_AllKeepersHome:
	.string "That was the last keeper!\n"
	.string "The whole forest seems to sigh\l"
	.string "with relief.\p"
	.string "You should tell the ELDER in the\n"
	.string "treehouse.$"

ElderwoodForest_Text_Zarude:
	.string "Uuh-rah!$"

ElderwoodForest_Text_ZarudeFled:
	.string "ZARUDE swung away into the canopy.$"

ElderwoodForest_Text_ZarudeWatches:
	.string "ZARUDE is watching you from the\n"
	.string "branches.$"

ElderwoodForest_Text_IvyIntro:
	.string "Rangers patrol the canopy so no one\n"
	.string "falls. Show me you're sure-footed!$"

ElderwoodForest_Text_IvyDefeat:
	.string "Steady hands, steady team!$"

ElderwoodForest_Text_IvyPost:
	.string "There's a stump in the west woods\n"
	.string "behind a thin tree. A cut would\l"
	.string "open the way.$"

ElderwoodForest_Text_HalIntro:
	.string "The biggest bugs in HOENN live\n"
	.string "right here. Want to see?$"

ElderwoodForest_Text_HalDefeat:
	.string "Bzzzt! Out-bugged!$"

ElderwoodForest_Text_HalPost:
	.string "I've seen a YANMEGA fly so fast\n"
	.string "it left a gust behind.$"

ElderwoodForest_Text_FernIntro:
	.string "Breathe in. Can you smell the\n"
	.string "flowers? My team grew up here.$"

ElderwoodForest_Text_FernDefeat:
	.string "Such a fresh breeze!$"

ElderwoodForest_Text_FernPost:
	.string "COMFEY likes to braid the flowers\n"
	.string "by the pond into garlands.$"

ElderwoodForest_Text_Fisher:
	.string "This pond never dries up, not even\n"
	.string "in summer. The roots drink deep.$"

ElderwoodForest_Text_Botanist:
	.string "I'm cataloguing every plant in\n"
	.string "ELDERWOOD. I'm on number 4,012.\p"
	.string "Some of these mushrooms only grow\n"
	.string "where nobody walks…$"

ElderwoodForest_Text_Kid:
	.string "Grandma says the forest has a king\n"
	.string "who swings on vines! Have you\l"
	.string "seen him?$"

ElderwoodForest_EventScript_Item_MiracleSeed::
	finditem ITEM_MIRACLE_SEED
	end

ElderwoodForest_EventScript_Item_LeafStone::
	finditem ITEM_LEAF_STONE
	end

ElderwoodForest_EventScript_Item_Revive::
	finditem ITEM_REVIVE
	end

ElderwoodForest_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

