MirageOasis_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, MirageOasis_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, MirageOasis_OnTransition
	.byte 0

MirageOasis_OnLoad:
	call_if_set FLAG_MIRAGE_SPRING_DONE, MirageOasis_EventScript_Refill
	call_if_set FLAG_MIRAGE_SPRING_DONE, MirageOasis_EventScript_WellFull
	end

MirageOasis_OnTransition:
	setflag FLAG_HIDE_MIRAGE_LANDORUS
	goto_if_unset FLAG_MIRAGE_SPRING_DONE, MirageOasis_EventScript_TransitionEnd
	goto_if_set FLAG_MIRAGE_LANDORUS_DONE, MirageOasis_EventScript_TransitionEnd
	clearflag FLAG_HIDE_MIRAGE_LANDORUS
MirageOasis_EventScript_TransitionEnd:
	end

MirageOasis_EventScript_Refill::
	setmetatile 21, 13, 200, FALSE
	setmetatile 22, 13, 201, FALSE
	setmetatile 23, 13, 201, FALSE
	setmetatile 24, 13, 201, FALSE
	setmetatile 25, 13, 201, FALSE
	setmetatile 26, 13, 202, FALSE
	setmetatile 19, 14, 200, FALSE
	setmetatile 20, 14, 201, FALSE
	setmetatile 21, 14, 209, FALSE
	setmetatile 22, 14, 209, FALSE
	setmetatile 23, 14, 209, FALSE
	setmetatile 24, 14, 209, FALSE
	setmetatile 25, 14, 209, FALSE
	setmetatile 26, 14, 209, FALSE
	setmetatile 27, 14, 201, FALSE
	setmetatile 28, 14, 202, FALSE
	setmetatile 18, 15, 200, FALSE
	setmetatile 19, 15, 209, FALSE
	setmetatile 20, 15, 209, FALSE
	setmetatile 21, 15, 209, FALSE
	setmetatile 22, 15, 209, FALSE
	setmetatile 23, 15, 209, FALSE
	setmetatile 24, 15, 209, FALSE
	setmetatile 25, 15, 209, FALSE
	setmetatile 26, 15, 209, FALSE
	setmetatile 27, 15, 209, FALSE
	setmetatile 28, 15, 209, FALSE
	setmetatile 29, 15, 202, FALSE
	setmetatile 18, 16, 208, FALSE
	setmetatile 19, 16, 209, FALSE
	setmetatile 20, 16, 209, FALSE
	setmetatile 21, 16, 209, FALSE
	setmetatile 22, 16, 209, FALSE
	setmetatile 23, 16, 209, FALSE
	setmetatile 24, 16, 209, FALSE
	setmetatile 25, 16, 209, FALSE
	setmetatile 26, 16, 209, FALSE
	setmetatile 27, 16, 209, FALSE
	setmetatile 28, 16, 209, FALSE
	setmetatile 29, 16, 210, FALSE
	setmetatile 18, 17, 208, FALSE
	setmetatile 19, 17, 209, FALSE
	setmetatile 20, 17, 209, FALSE
	setmetatile 21, 17, 209, FALSE
	setmetatile 22, 17, 209, FALSE
	setmetatile 23, 17, 209, FALSE
	setmetatile 24, 17, 209, FALSE
	setmetatile 25, 17, 209, FALSE
	setmetatile 26, 17, 209, FALSE
	setmetatile 27, 17, 209, FALSE
	setmetatile 28, 17, 209, FALSE
	setmetatile 29, 17, 210, FALSE
	setmetatile 18, 18, 216, FALSE
	setmetatile 19, 18, 209, FALSE
	setmetatile 20, 18, 209, FALSE
	setmetatile 21, 18, 209, FALSE
	setmetatile 22, 18, 209, FALSE
	setmetatile 23, 18, 209, FALSE
	setmetatile 24, 18, 209, FALSE
	setmetatile 25, 18, 209, FALSE
	setmetatile 26, 18, 209, FALSE
	setmetatile 27, 18, 209, FALSE
	setmetatile 28, 18, 209, FALSE
	setmetatile 29, 18, 218, FALSE
	setmetatile 19, 19, 216, FALSE
	setmetatile 20, 19, 217, FALSE
	setmetatile 21, 19, 209, FALSE
	setmetatile 22, 19, 209, FALSE
	setmetatile 23, 19, 209, FALSE
	setmetatile 24, 19, 209, FALSE
	setmetatile 25, 19, 209, FALSE
	setmetatile 26, 19, 209, FALSE
	setmetatile 27, 19, 217, FALSE
	setmetatile 28, 19, 218, FALSE
	setmetatile 21, 20, 216, FALSE
	setmetatile 22, 20, 217, FALSE
	setmetatile 23, 20, 217, FALSE
	setmetatile 24, 20, 217, FALSE
	setmetatile 25, 20, 217, FALSE
	setmetatile 26, 20, 218, FALSE
	return

MirageOasis_EventScript_WellFull::
	setmetatile 25, 25, 577, TRUE
	setmetatile 26, 25, 578, TRUE
	setmetatile 25, 26, 579, TRUE
	setmetatile 26, 26, 580, TRUE
	return

MirageOasis_EventScript_Sign::
	msgbox MirageOasis_Text_Sign, MSGBOX_SIGN
	end

MirageOasis_EventScript_Well::
	lockall
	goto_if_set FLAG_MIRAGE_SPRING_DONE, MirageOasis_EventScript_WellFlowing
	goto_if_set FLAG_MIRAGE_SPRING_STONE, MirageOasis_EventScript_PlaceStone
	msgbox MirageOasis_Text_WellDry, MSGBOX_DEFAULT
	releaseall
	end

MirageOasis_EventScript_WellFlowing::
	msgbox MirageOasis_Text_WellFlowing, MSGBOX_DEFAULT
	releaseall
	end

MirageOasis_EventScript_PlaceStone::
	msgbox MirageOasis_Text_PlaceStoneAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MirageOasis_EventScript_WellLeave
	msgbox MirageOasis_Text_PlaceStone, MSGBOX_DEFAULT
	closemessage
	playse SE_M_WATERFALL
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	call MirageOasis_EventScript_WellFull
	special DrawWholeMapView
	delay 30
	call MirageOasis_EventScript_Refill
	special DrawWholeMapView
	delay 40
	setflag FLAG_MIRAGE_SPRING_DONE
	msgbox MirageOasis_Text_SpringFlows, MSGBOX_DEFAULT
	releaseall
	end

MirageOasis_EventScript_WellLeave::
	releaseall
	end

MirageOasis_EventScript_Elder::
	lock
	faceplayer
	goto_if_set FLAG_MIRAGE_LARVESTA, MirageOasis_EventScript_ElderDone
	goto_if_set FLAG_MIRAGE_SPRING_DONE, MirageOasis_EventScript_ElderReward
	goto_if_set FLAG_MIRAGE_SPRING_STONE, MirageOasis_EventScript_ElderStone
	goto_if_set FLAG_MIRAGE_ELDER_TALKED, MirageOasis_EventScript_ElderAgain
	setflag FLAG_MIRAGE_ELDER_TALKED
	msgbox MirageOasis_Text_Elder, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_ElderAgain::
	msgbox MirageOasis_Text_ElderAgain, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_ElderStone::
	msgbox MirageOasis_Text_ElderStone, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_ElderReward::
	msgbox MirageOasis_Text_ElderThanks, MSGBOX_DEFAULT
	givemon SPECIES_LARVESTA, 45
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, MirageOasis_EventScript_ElderFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox MirageOasis_Text_GotLarvesta, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_MIRAGE_LARVESTA
	msgbox MirageOasis_Text_ElderDone, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_ElderFull::
	msgbox MirageOasis_Text_ElderFull, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_ElderDone::
	msgbox MirageOasis_Text_ElderDone, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_Merchant::
	lock
	faceplayer
	message MirageOasis_Text_Merchant
	waitmessage
	pokemart MirageOasis_Mart
	msgbox MirageOasis_Text_MerchantBye, MSGBOX_DEFAULT
	release
	end

	.align 2
MirageOasis_Mart:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_SOFT_SAND
	.2byte ITEM_SMOOTH_ROCK
	.2byte ITEM_HEAT_ROCK
	.2byte ITEM_NONE
	release
	end

MirageOasis_EventScript_FruitSeller::
	lock
	faceplayer
	message MirageOasis_Text_Fruit
	waitmessage
	pokemart MirageOasis_FruitMart
	msgbox MirageOasis_Text_MerchantBye, MSGBOX_DEFAULT
	release
	end

	.align 2
MirageOasis_FruitMart:
	.2byte ITEM_ORAN_BERRY
	.2byte ITEM_SITRUS_BERRY
	.2byte ITEM_LUM_BERRY
	.2byte ITEM_LEPPA_BERRY
	.2byte ITEM_NONE
	release
	end

MirageOasis_EventScript_Fortune::
	lock
	faceplayer
	goto_if_set FLAG_MIRAGE_SPRING_STONE, MirageOasis_EventScript_FortuneDone
	msgbox MirageOasis_Text_Fortune, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_FortuneDone::
	msgbox MirageOasis_Text_FortuneDone, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_Girl::
	lock
	faceplayer
	goto_if_set FLAG_MIRAGE_SPRING_DONE, MirageOasis_EventScript_GirlHappy
	msgbox MirageOasis_Text_Girl, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_GirlHappy::
	msgbox MirageOasis_Text_GirlHappy, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_Traveller::
	msgbox MirageOasis_Text_Traveller, MSGBOX_NPC
	end

MirageOasis_EventScript_Landorus::
	lock
	faceplayer
	msgbox MirageOasis_Text_LandorusSees, MSGBOX_DEFAULT
	setwildbattle SPECIES_LANDORUS, 55
	playmoncry SPECIES_LANDORUS, CRY_MODE_ENCOUNTER
	msgbox MirageOasis_Text_Landorus, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, MirageOasis_EventScript_LandorusStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, MirageOasis_EventScript_LandorusStays
	setflag FLAG_MIRAGE_LANDORUS_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, MirageOasis_EventScript_LandorusEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox MirageOasis_Text_LandorusGone, MSGBOX_DEFAULT
MirageOasis_EventScript_LandorusEnd::
	release
	end

MirageOasis_EventScript_LandorusStays::
	msgbox MirageOasis_Text_LandorusStays, MSGBOX_DEFAULT
	release
	end

MirageOasis_EventScript_Omar::
	trainerbattle_single TRAINER_MIRAGE_OMAR, MirageOasis_Text_OmarIntro, MirageOasis_Text_OmarDefeat
	msgbox MirageOasis_Text_OmarPost, MSGBOX_AUTOCLOSE
	end

MirageOasis_Text_OmarIntro:
	.string "I guard the path to the mesa. Only\n"
	.string "the strong may look upon the sands\l"
	.string "from up here!$"

MirageOasis_Text_OmarDefeat:
	.string "You are strong indeed.$"

MirageOasis_Text_OmarPost:
	.string "From the mesa you can see the whole\n"
	.string "desert. And sometimes… a guardian.$"


MirageOasis_Text_Sign:
	.string "MIRAGE OASIS\n"
	.string "The true water of the dunes.$"

MirageOasis_Text_WellDry:
	.string "The well is bone dry. A round hollow\n"
	.string "is carved in the stone at the bottom,\l"
	.string "as if something should sit there.$"

MirageOasis_Text_WellFlowing:
	.string "Cool, clear spring water fills the\n"
	.string "well to the brim.$"

MirageOasis_Text_PlaceStoneAsk:
	.string "A round hollow is carved at the\n"
	.string "bottom of the dry well.\p"
	.string "Put the SPRING STONE in it?$"

MirageOasis_Text_PlaceStone:
	.string "{PLAYER} lowered the SPRING STONE\n"
	.string "into the well…$"

MirageOasis_Text_SpringFlows:
	.string "Water gushed up from the stone!\n"
	.string "The spring is flowing again!$"

MirageOasis_Text_Elder:
	.string "Welcome, traveller. You saw through\n"
	.string "the mirages to find us. Few can.\p"
	.string "But our oasis is dying. The spring\n"
	.string "has dried and the pond is cracked mud.\p"
	.string "Long ago the SPRING STONE fed our\n"
	.string "well. Thieves took it, and the sand\l"
	.string "swallowed them and their court.\p"
	.string "Old tales say the sand swallows you\n"
	.string "where three stones stand. Will you\l"
	.string "bring the stone home?$"

MirageOasis_Text_ElderAgain:
	.string "Where three stones stand in the dunes,\n"
	.string "the sand will swallow you. Below lies\l"
	.string "the court, and the SPRING STONE.$"

MirageOasis_Text_ElderStone:
	.string "The SPRING STONE! You found it!\n"
	.string "Quickly, place it in the well!$"

MirageOasis_Text_ElderThanks:
	.string "Listen to it… the spring sings again!\n"
	.string "The pond is full for the first time\l"
	.string "in years.\p"
	.string "Take this LARVESTA, child of the sun.\n"
	.string "It hatched by the well as the water\l"
	.string "rose.$"

MirageOasis_Text_GotLarvesta:
	.string "{PLAYER} received a LARVESTA!$"

MirageOasis_Text_ElderFull:
	.string "Your party is full. Come back for\n"
	.string "LARVESTA when there's room.$"

MirageOasis_Text_ElderDone:
	.string "Something came down to rest on the\n"
	.string "mesa when the water rose. The\l"
	.string "guardian of the land, they say.$"

MirageOasis_Text_Merchant:
	.string "Welcome to the bazaar! Goods from\n"
	.string "every corner of the dunes!$"

MirageOasis_Text_Fruit:
	.string "Fresh berries from the oasis\n"
	.string "gardens!$"

MirageOasis_Text_MerchantBye:
	.string "Come again, traveller!$"

MirageOasis_Text_Fortune:
	.string "I see… sand. Falling sand. And three\n"
	.string "stones in a ring, near the north\l"
	.string "pass, west of the mirage palms.\p"
	.string "That will be one POKé DOLLAR.\n"
	.string "…Just kidding.$"

MirageOasis_Text_FortuneDone:
	.string "The stars told me you'd find it.$"

MirageOasis_Text_Girl:
	.string "The pond used to be full of water.\n"
	.string "Now it's just cracks. I miss\l"
	.string "splashing!$"

MirageOasis_Text_GirlHappy:
	.string "The pond is back! Splash splash!$"

MirageOasis_Text_Traveller:
	.string "I followed the caravan here. The\n"
	.string "inn in the blue tent lets you rest\l"
	.string "for free!$"

MirageOasis_Text_LandorusSees:
	.string "A great POKéMON rests on the mesa,\n"
	.string "gazing over the restored oasis.$"

MirageOasis_Text_Landorus:
	.string "Graaoooh!$"

MirageOasis_Text_LandorusGone:
	.string "LANDORUS soared away over the dunes.$"

MirageOasis_Text_LandorusStays:
	.string "LANDORUS is still watching over the\n"
	.string "oasis from the mesa.$"

MirageOasis_EventScript_Item_PpMax::
	finditem ITEM_PP_MAX
	end

