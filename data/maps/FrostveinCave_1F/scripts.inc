FrostveinCave_1F_MapScripts::
	.byte 0

FrostveinCave_1F_EventScript_Sailor::
	lock
	faceplayer
	msgbox FrostveinCave_1F_Text_SailorBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FrostveinCave_1F_EventScript_SailorStay
	msgbox FrostveinCave_1F_Text_SailorGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_MOSSDEEP_CITY, 30, 18
	waitstate
	release
	end

FrostveinCave_1F_EventScript_SailorStay::
	msgbox FrostveinCave_1F_Text_SailorStay, MSGBOX_DEFAULT
	release
	end

FrostveinCave_1F_EventScript_Sign::
	msgbox FrostveinCave_1F_Text_Sign, MSGBOX_SIGN
	end

FrostveinCave_1F_EventScript_Researcher::
	lock
	faceplayer
	msgbox FrostveinCave_1F_Text_Researcher, MSGBOX_DEFAULT
	setflag FLAG_FROSTVEIN_RESEARCHER_TALKED
	release
	end

FrostveinCave_1F_EventScript_Glen::
	trainerbattle_single TRAINER_FROSTVEIN_GLEN, FrostveinCave_1F_Text_GlenIntro, FrostveinCave_1F_Text_GlenDefeat
	msgbox FrostveinCave_1F_Text_GlenPost, MSGBOX_AUTOCLOSE
	end

FrostveinCave_1F_EventScript_Bjorn::
	trainerbattle_single TRAINER_FROSTVEIN_BJORN, FrostveinCave_1F_Text_BjornIntro, FrostveinCave_1F_Text_BjornDefeat
	msgbox FrostveinCave_1F_Text_BjornPost, MSGBOX_AUTOCLOSE
	end

FrostveinCave_1F_Text_SailorBack:
	.string "Brr! Ready to sail back to\n"
	.string "MOSSDEEP?$"

FrostveinCave_1F_Text_SailorGo:
	.string "Anchors up!$"

FrostveinCave_1F_Text_SailorStay:
	.string "I'll wait by the boat. Don't freeze\n"
	.string "in there!$"

FrostveinCave_1F_Text_Sign:
	.string "FROSTVEIN CAVE\n"
	.string "Ice floors are slippery. Plan your\l"
	.string "path before you step on!$"

FrostveinCave_1F_Text_Researcher:
	.string "I study the ice here. It never\n"
	.string "melts, not even in summer.\p"
	.string "Once you step on the ice, you slide\n"
	.string "until something stops you. Use the\l"
	.string "frozen rocks as brakes!\p"
	.string "They say a shrine lies at the very\n"
	.string "bottom, guarded by a huge AVALUGG.$"

FrostveinCave_1F_Text_GlenIntro:
	.string "Cold battles keep the mind sharp!$"

FrostveinCave_1F_Text_GlenDefeat:
	.string "Chilled to the bone…$"

FrostveinCave_1F_Text_GlenPost:
	.string "Look before you slide. That's the\n"
	.string "whole secret.$"

FrostveinCave_1F_Text_BjornIntro:
	.string "I slid across that ice field six\n"
	.string "times before I got here!$"

FrostveinCave_1F_Text_BjornDefeat:
	.string "Slipped up!$"

FrostveinCave_1F_Text_BjornPost:
	.string "The ladder goes down to an even\n"
	.string "bigger ice field. Good luck!$"

FrostveinCave_1F_EventScript_Item_IceStone::
	finditem ITEM_ICE_STONE
	end

FrostveinCave_1F_EventScript_Item_FullHeal::
	finditem ITEM_FULL_HEAL
	end

FrostveinCave_1F_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

