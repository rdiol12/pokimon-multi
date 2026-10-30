StarfallIsle_MapScripts::
	.byte 0

StarfallIsle_EventScript_Sailor::
	lock
	faceplayer
	msgbox StarfallIsle_Text_SailBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, StarfallIsle_EventScript_SailorStay
	msgbox StarfallIsle_Text_AllAboard, MSGBOX_DEFAULT
	closemessage
	warp MAP_LILYCOVE_CITY, 13, 34
	waitstate
	release
	end

StarfallIsle_EventScript_SailorStay::
	msgbox StarfallIsle_Text_EnjoyIsland, MSGBOX_DEFAULT
	release
	end

StarfallIsle_EventScript_LostSkwovet::
	lock
	faceplayer
	playmoncry SPECIES_SKWOVET, CRY_MODE_NORMAL
	msgbox StarfallIsle_Text_SkwovetFound, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	setflag FLAG_STARFALL_SKWOVET_FOUND
	removeobject LOCALID_STARFALL_SKWOVET
	fadescreen FADE_FROM_BLACK
	msgbox StarfallIsle_Text_SkwovetRanHome, MSGBOX_DEFAULT
	release
	end

StarfallIsle_EventScript_Dax::
	trainerbattle_single TRAINER_STARFALL_DAX, StarfallIsle_Text_DaxIntro, StarfallIsle_Text_DaxDefeat
	msgbox StarfallIsle_Text_DaxPostBattle, MSGBOX_AUTOCLOSE
	end

StarfallIsle_EventScript_Grant::
	trainerbattle_single TRAINER_STARFALL_GRANT, StarfallIsle_Text_GrantIntro, StarfallIsle_Text_GrantDefeat
	msgbox StarfallIsle_Text_GrantPostBattle, MSGBOX_AUTOCLOSE
	end

StarfallIsle_EventScript_Wade::
	trainerbattle_single TRAINER_STARFALL_WADE, StarfallIsle_Text_WadeIntro, StarfallIsle_Text_WadeDefeat
	msgbox StarfallIsle_Text_WadePostBattle, MSGBOX_AUTOCLOSE
	end

StarfallIsle_EventScript_CaveExplorer::
	msgbox StarfallIsle_Text_CaveExplorer, MSGBOX_NPC
	end

StarfallIsle_EventScript_Gardener::
	msgbox StarfallIsle_Text_Gardener, MSGBOX_NPC
	end

StarfallIsle_EventScript_LittleGirl::
	msgbox StarfallIsle_Text_LittleGirl, MSGBOX_NPC
	end

StarfallIsle_EventScript_Wingull::
	lock
	faceplayer
	playmoncry SPECIES_WINGULL, CRY_MODE_NORMAL
	msgbox StarfallIsle_Text_Wingull, MSGBOX_DEFAULT
	waitmoncry
	release
	end

StarfallIsle_EventScript_PondBoy::
	msgbox StarfallIsle_Text_PondBoy, MSGBOX_NPC
	end

StarfallIsle_EventScript_ItemBigNugget::
	finditem ITEM_BIG_NUGGET
	end

StarfallIsle_EventScript_DockSign::
	msgbox StarfallIsle_Text_DockSign, MSGBOX_SIGN
	end

StarfallIsle_EventScript_CaveSign::
	msgbox StarfallIsle_Text_CaveSign, MSGBOX_SIGN
	end

StarfallIsle_EventScript_CarvedTree::
	msgbox StarfallIsle_Text_CarvedTree, MSGBOX_SIGN
	end

StarfallIsle_Text_SailBack:
	.string "Ahoy! Ready to sail back to\nLILYCOVE CITY?$"

StarfallIsle_Text_AllAboard:
	.string "All aboard! Hold on to your hat!$"

StarfallIsle_Text_EnjoyIsland:
	.string "Take your time. The island's full of\nsecrets, they say.$"

StarfallIsle_Text_SkwovetFound:
	.string "SKWOVET: Skwo! Skwovet!\pIt's the ELDER's lost SKWOVET!\nIt looks happy to see you.$"

StarfallIsle_Text_SkwovetRanHome:
	.string "SKWOVET scampered off toward\nthe village!$"

StarfallIsle_Text_DaxIntro:
	.string "This island's POKéMON come from all\nover the world. So do mine!$"

StarfallIsle_Text_DaxDefeat:
	.string "Whoa, you're the real deal.$"

StarfallIsle_Text_DaxPostBattle:
	.string "There's a rare POKéMON that only\nshows up in this meadow's tall grass.\pI've seen it once. Big, round, and\nalways sleeping...$"

StarfallIsle_Text_GrantIntro:
	.string "Halt! This path leads to the cave.\nProve you're tough enough!$"

StarfallIsle_Text_GrantDefeat:
	.string "Rock solid... and I still crumbled.$"

StarfallIsle_Text_GrantPostBattle:
	.string "The cave inside the mountain is a\nmaze. Keep your wits about you.$"

StarfallIsle_Text_WadeIntro:
	.string "Shh! You'll scare the fish!\nNow you have to battle me!$"

StarfallIsle_Text_WadeDefeat:
	.string "Got reeled in myself, huh.$"

StarfallIsle_Text_WadePostBattle:
	.string "With a SUPER ROD, this pond hides\nDRATINI. I've only ever seen one.$"

StarfallIsle_Text_CaveExplorer:
	.string "That cave goes deep into the\nmountain.\pSome nights a light glows inside,\nlike a star that fell to earth...$"

StarfallIsle_Text_Gardener:
	.string "I grow BERRIES for the whole village.\pThe ELDER has been so sad since her\nSKWOVET ran off. Have you seen it?$"

StarfallIsle_Text_DockSign:
	.string "STARFALL ISLE\n“Where wishing stars fall to earth.”$"

StarfallIsle_Text_CaveSign:
	.string "STARFALL CAVE\nExplorers have lost their way inside.$"

StarfallIsle_Text_CarvedTree:
	.string "Two names are carved into this tree:\nRON and CLAUDE.\pUnder them, in tiny letters:\n“Thanks for playing!”$"

StarfallIsle_Text_LittleGirl:
	.string "Every night I make a wish on the\n"
	.string "shooting stars over the mountain!\p"
	.string "Grandma says one of them landed\n"
	.string "inside it a long time ago.$"

StarfallIsle_Text_Wingull:
	.string "WINGULL: Pihyoh!\n"
	.string "It's begging for a snack.$"

StarfallIsle_Text_PondBoy:
	.string "I saw a SKWOVET dash into the tall\n"
	.string "grass up on that ledge.\p"
	.string "You can't climb up from here.\n"
	.string "Go around through the village!$"

StarfallIsle_EventScript_Boatman::
	lock
	faceplayer
	msgbox StarfallIsle_Text_BoatmanOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, StarfallIsle_EventScript_BoatmanNo
	msgbox StarfallIsle_Text_BoatmanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_AMBERWOOD_HOLLOW, 19, 30
	waitstate
	release
	end

StarfallIsle_EventScript_BoatmanNo::
	msgbox StarfallIsle_Text_BoatmanNo, MSGBOX_DEFAULT
	release
	end

StarfallIsle_Text_BoatmanOffer:
	.string "My rowboat goes to AMBERWOOD\nHOLLOW, a lake in an autumn forest.\pNobody's mapped it but me.\nWant to row over?$"

StarfallIsle_Text_BoatmanGo:
	.string "Hop in! Mind the leaves.$"

StarfallIsle_Text_BoatmanNo:
	.string "Suit yourself. The hollow will wait.$"
