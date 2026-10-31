VerdantRiver_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, VerdantRiver_OnLoad
	.byte 0

VerdantRiver_OnLoad:
	call_if_set FLAG_VERDANT_SLUICE, VerdantRiver_EventScript_LeverOnSluice
	call_if_set FLAG_VERDANT_SLUICE, VerdantRiver_EventScript_GorgeCalm
	call_if_set FLAG_VERDANT_WHIRL, VerdantRiver_EventScript_LeverOnWhirl
	call_if_set FLAG_VERDANT_WHIRL, VerdantRiver_EventScript_WhirlOpen
	end

VerdantRiver_EventScript_GorgeCalm::
	setmetatile 26, 31, 368, FALSE
	setmetatile 26, 32, 368, FALSE
	setmetatile 26, 33, 368, FALSE
	setmetatile 26, 34, 368, FALSE
	setmetatile 26, 35, 368, FALSE
	setmetatile 26, 36, 368, FALSE
	setmetatile 26, 37, 368, FALSE
	setmetatile 26, 38, 368, FALSE
	setmetatile 26, 39, 368, FALSE
	setmetatile 26, 40, 368, FALSE
	setmetatile 26, 41, 368, FALSE
	setmetatile 26, 42, 368, FALSE
	setmetatile 27, 31, 368, FALSE
	setmetatile 27, 32, 368, FALSE
	setmetatile 27, 33, 368, FALSE
	setmetatile 27, 34, 368, FALSE
	setmetatile 27, 35, 368, FALSE
	setmetatile 27, 36, 368, FALSE
	setmetatile 27, 37, 368, FALSE
	setmetatile 27, 38, 368, FALSE
	setmetatile 27, 39, 368, FALSE
	setmetatile 27, 40, 368, FALSE
	setmetatile 27, 41, 368, FALSE
	setmetatile 27, 42, 368, FALSE
	setmetatile 28, 31, 368, FALSE
	setmetatile 28, 32, 368, FALSE
	setmetatile 28, 33, 368, FALSE
	setmetatile 28, 34, 368, FALSE
	setmetatile 28, 35, 368, FALSE
	setmetatile 28, 36, 368, FALSE
	setmetatile 28, 37, 368, FALSE
	setmetatile 28, 38, 368, FALSE
	setmetatile 28, 39, 368, FALSE
	setmetatile 28, 40, 368, FALSE
	setmetatile 28, 41, 368, FALSE
	setmetatile 28, 42, 368, FALSE
	setmetatile 29, 31, 368, FALSE
	setmetatile 29, 32, 368, FALSE
	setmetatile 29, 33, 368, FALSE
	setmetatile 29, 34, 368, FALSE
	setmetatile 29, 35, 368, FALSE
	setmetatile 29, 36, 368, FALSE
	setmetatile 29, 37, 368, FALSE
	setmetatile 29, 38, 368, FALSE
	setmetatile 29, 39, 368, FALSE
	setmetatile 29, 40, 368, FALSE
	setmetatile 29, 41, 368, FALSE
	setmetatile 29, 42, 368, FALSE
	setmetatile 30, 31, 368, FALSE
	setmetatile 30, 32, 368, FALSE
	setmetatile 30, 33, 368, FALSE
	setmetatile 30, 34, 368, FALSE
	setmetatile 30, 35, 368, FALSE
	setmetatile 30, 36, 368, FALSE
	setmetatile 30, 37, 368, FALSE
	setmetatile 30, 38, 368, FALSE
	setmetatile 30, 39, 368, FALSE
	setmetatile 30, 40, 368, FALSE
	setmetatile 30, 41, 368, FALSE
	setmetatile 30, 42, 368, FALSE
	return

VerdantRiver_EventScript_WhirlOpen::
	setmetatile 28, 24, 758, FALSE
	return

VerdantRiver_EventScript_LeverOnSluice::
	setmetatile 15, 29, 514, TRUE
	setmetatile 15, 30, 522, TRUE
	return

VerdantRiver_EventScript_LeverSluice::
	lockall
	goto_if_set FLAG_VERDANT_SLUICE, VerdantRiver_EventScript_LeverSluiceDone
	msgbox VerdantRiver_Text_SluiceAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VerdantRiver_EventScript_LeverLeave
	playse SE_PIN
	call VerdantRiver_EventScript_LeverOnSluice
	special DrawWholeMapView
	delay 30
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_M_WATERFALL
	call VerdantRiver_EventScript_GorgeCalm
	special DrawWholeMapView
	delay 40
	setflag FLAG_VERDANT_SLUICE
	msgbox VerdantRiver_Text_SluiceDone, MSGBOX_DEFAULT
	releaseall
	end

VerdantRiver_EventScript_LeverSluiceDone::
	msgbox VerdantRiver_Text_SluiceAlready, MSGBOX_DEFAULT
	releaseall
	end

VerdantRiver_EventScript_LeverOnWhirl::
	setmetatile 8, 15, 514, TRUE
	setmetatile 8, 16, 522, TRUE
	return

VerdantRiver_EventScript_LeverWhirl::
	lockall
	goto_if_set FLAG_VERDANT_WHIRL, VerdantRiver_EventScript_LeverWhirlDone
	msgbox VerdantRiver_Text_WhirlAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VerdantRiver_EventScript_LeverLeave
	playse SE_PIN
	call VerdantRiver_EventScript_LeverOnWhirl
	special DrawWholeMapView
	delay 30
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_M_WATERFALL
	call VerdantRiver_EventScript_WhirlOpen
	special DrawWholeMapView
	delay 40
	setflag FLAG_VERDANT_WHIRL
	msgbox VerdantRiver_Text_WhirlDone, MSGBOX_DEFAULT
	releaseall
	end

VerdantRiver_EventScript_LeverWhirlDone::
	msgbox VerdantRiver_Text_WhirlAlready, MSGBOX_DEFAULT
	releaseall
	end

VerdantRiver_EventScript_LeverLeave::
	releaseall
	end

VerdantRiver_EventScript_Guide::
	lock
	faceplayer
	msgbox VerdantRiver_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VerdantRiver_EventScript_GuideStay
	msgbox VerdantRiver_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_ROUTE119, 13, 132
	waitstate
	release
	end

VerdantRiver_EventScript_GuideStay::
	msgbox VerdantRiver_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

VerdantRiver_EventScript_Sign::
	msgbox VerdantRiver_Text_Sign, MSGBOX_SIGN
	end

VerdantRiver_EventScript_Elder::
	lock
	faceplayer
	goto_if_set FLAG_VERDANT_FROAKIE, VerdantRiver_EventScript_ElderDone
	goto_if_set FLAG_VERDANT_TEMPLE_REACHED, VerdantRiver_EventScript_ElderReward
	goto_if_set FLAG_VERDANT_SLUICE, VerdantRiver_EventScript_ElderSluice
	goto_if_set FLAG_VERDANT_ELDER_TALKED, VerdantRiver_EventScript_ElderAgain
	setflag FLAG_VERDANT_ELDER_TALKED
	msgbox VerdantRiver_Text_Elder, MSGBOX_DEFAULT
	release
	end

VerdantRiver_EventScript_ElderAgain::
	msgbox VerdantRiver_Text_ElderAgain, MSGBOX_DEFAULT
	release
	end

VerdantRiver_EventScript_ElderSluice::
	msgbox VerdantRiver_Text_ElderSluice, MSGBOX_DEFAULT
	release
	end

VerdantRiver_EventScript_ElderReward::
	msgbox VerdantRiver_Text_ElderThanks, MSGBOX_DEFAULT
	givemon SPECIES_FROAKIE, 30
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, VerdantRiver_EventScript_ElderFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox VerdantRiver_Text_GotFroakie, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_VERDANT_FROAKIE
	msgbox VerdantRiver_Text_ElderDone, MSGBOX_DEFAULT
	release
	end

VerdantRiver_EventScript_ElderFull::
	msgbox VerdantRiver_Text_ElderFull, MSGBOX_DEFAULT
	release
	end

VerdantRiver_EventScript_ElderDone::
	msgbox VerdantRiver_Text_ElderDone, MSGBOX_DEFAULT
	release
	end

VerdantRiver_EventScript_Kid::
	msgbox VerdantRiver_Text_Kid, MSGBOX_NPC
	end

VerdantRiver_EventScript_Builder::
	msgbox VerdantRiver_Text_Builder, MSGBOX_NPC
	end

VerdantRiver_EventScript_Lotad::
	lock
	faceplayer
	playmoncry SPECIES_LOTAD, CRY_MODE_NORMAL
	msgbox VerdantRiver_Text_Lotad, MSGBOX_DEFAULT
	waitmoncry
	release
	end

VerdantRiver_EventScript_Reed::
	trainerbattle_single TRAINER_VERDANT_REED, VerdantRiver_Text_ReedIntro, VerdantRiver_Text_ReedDefeat
	msgbox VerdantRiver_Text_ReedPost, MSGBOX_AUTOCLOSE
	end

VerdantRiver_Text_ReedIntro:
	.string "The fish bite best where the river\n"
	.string "meets the lagoon. And so do I!$"

VerdantRiver_Text_ReedDefeat:
	.string "Off the hook…$"

VerdantRiver_Text_ReedPost:
	.string "Ride the current east of the lagoon.\n"
	.string "Nobody fishes the pool behind it.$"


VerdantRiver_EventScript_Kobe::
	trainerbattle_single TRAINER_VERDANT_KOBE, VerdantRiver_Text_KobeIntro, VerdantRiver_Text_KobeDefeat
	msgbox VerdantRiver_Text_KobePost, MSGBOX_AUTOCLOSE
	end

VerdantRiver_Text_KobeIntro:
	.string "The jungle's full of bugs, and I've\n"
	.string "caught the best of them!$"

VerdantRiver_Text_KobeDefeat:
	.string "Squashed…$"

VerdantRiver_Text_KobePost:
	.string "HERACROSS love the sap of the trees\n"
	.string "by the river.$"


VerdantRiver_EventScript_Liana::
	trainerbattle_single TRAINER_VERDANT_LIANA, VerdantRiver_Text_LianaIntro, VerdantRiver_Text_LianaDefeat
	msgbox VerdantRiver_Text_LianaPost, MSGBOX_AUTOCLOSE
	end

VerdantRiver_Text_LianaIntro:
	.string "From up here I watch the birds fly\n"
	.string "over the gorge. Want to see mine?$"

VerdantRiver_Text_LianaDefeat:
	.string "Grounded!$"

VerdantRiver_Text_LianaPost:
	.string "The old sluice lever is up ahead.\n"
	.string "Nobody's touched it in years.$"


VerdantRiver_EventScript_Maya::
	trainerbattle_single TRAINER_VERDANT_MAYA, VerdantRiver_Text_MayaIntro, VerdantRiver_Text_MayaDefeat
	msgbox VerdantRiver_Text_MayaPost, MSGBOX_AUTOCLOSE
	end

VerdantRiver_Text_MayaIntro:
	.string "Swimming in the lagoon is the best!\n"
	.string "Battling in it is even better!$"

VerdantRiver_Text_MayaDefeat:
	.string "I'm all washed up.$"

VerdantRiver_Text_MayaPost:
	.string "The currents carry you, so don't\n"
	.string "fight them. Just ride along.$"


VerdantRiver_EventScript_Rowan::
	trainerbattle_single TRAINER_VERDANT_ROWAN, VerdantRiver_Text_RowanIntro, VerdantRiver_Text_RowanDefeat
	msgbox VerdantRiver_Text_RowanPost, MSGBOX_AUTOCLOSE
	end

VerdantRiver_Text_RowanIntro:
	.string "I swim laps round the lake. Don't get\n"
	.string "caught in the whirl like I did!$"

VerdantRiver_Text_RowanDefeat:
	.string "Swept away…$"

VerdantRiver_Text_RowanPost:
	.string "The whirl always throws you out to\n"
	.string "the west. Look for a lever there.$"


VerdantRiver_Text_SluiceAsk:
	.string "An old sluice lever. Below the\n"
	.string "lookout, the gorge roars with\l"
	.string "rapids.\p"
	.string "Pull the lever?$"

VerdantRiver_Text_SluiceDone:
	.string "The sluice gate creaked shut!\p"
	.string "The rapids in the gorge calmed.\n"
	.string "You can surf up to the lake now!$"

VerdantRiver_Text_SluiceAlready:
	.string "The lever is pulled down. The gorge\n"
	.string "flows gently.$"

VerdantRiver_Text_WhirlAsk:
	.string "Another old lever, half buried in\n"
	.string "the sand. Out in the lake, the whirl\l"
	.string "spins round the temple island.\p"
	.string "Pull the lever?$"

VerdantRiver_Text_WhirlDone:
	.string "Somewhere under the lake, a gate\n"
	.string "groaned open!\p"
	.string "The whirl now turns in toward the\n"
	.string "temple island's dock.$"

VerdantRiver_Text_WhirlAlready:
	.string "The lever is pulled down. The whirl\n"
	.string "turns in toward the island.$"

VerdantRiver_Text_GuideBack:
	.string "I paddle travellers up and down the\n"
	.string "river. Back to ROUTE 119?$"

VerdantRiver_Text_GuideLead:
	.string "Hop in the canoe!$"

VerdantRiver_Text_GuideStay:
	.string "Enjoy the river!$"

VerdantRiver_Text_Sign:
	.string "VERDANT RIVER VILLAGE\n"
	.string "Where the river meets the sea of trees.$"

VerdantRiver_Text_Elder:
	.string "Welcome to our village, traveller.\p"
	.string "Up the gorge lies a lake, and on its\n"
	.string "island stands the river temple. Its\l"
	.string "guardian keeps the water clean.\p"
	.string "But years ago the old sluice stuck\n"
	.string "open. Rapids fill the gorge and a\l"
	.string "whirl rings the island. No one has\l"
	.string "reached the temple since.\p"
	.string "The sluice lever is on the lookout,\n"
	.string "up the path behind my hut.$"

VerdantRiver_Text_ElderAgain:
	.string "The sluice lever is on the lookout,\n"
	.string "up the path behind my hut. Pull it\l"
	.string "and the gorge will calm.$"

VerdantRiver_Text_ElderSluice:
	.string "The gorge is calm! You can surf up\n"
	.string "to the lake now.\p"
	.string "But beware the whirl round the\n"
	.string "island. They say another lever once\l"
	.string "turned it.$"

VerdantRiver_Text_ElderThanks:
	.string "You reached the temple! The guardian\n"
	.string "has woken, I can feel it. The river\l"
	.string "runs clearer already.\p"
	.string "Please take this FROAKIE. It grew up\n"
	.string "in our lagoon, and it wants to see\l"
	.string "the world.$"

VerdantRiver_Text_GotFroakie:
	.string "{PLAYER} received a FROAKIE!$"

VerdantRiver_Text_ElderFull:
	.string "Your party is full. Come back for\n"
	.string "FROAKIE when there's room.$"

VerdantRiver_Text_ElderDone:
	.string "The river sings again. Thank you,\n"
	.string "traveller.$"

VerdantRiver_Text_Kid:
	.string "I tried to swim up the gorge once.\n"
	.string "The rapids pushed me right back down!$"

VerdantRiver_Text_Builder:
	.string "I'm building canoes. That one's\n"
	.string "nearly done. It just needs a hole\l"
	.string "in the right place. Or not.$"

VerdantRiver_Text_Lotad:
	.string "LOTAD: Lo-lo!\p"
	.string "It's floating happily in the hidden\n"
	.string "lagoon.$"

VerdantRiver_EventScript_Item_NetBall::
	finditem ITEM_NET_BALL
	end

VerdantRiver_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

VerdantRiver_EventScript_Item_LeafStone::
	finditem ITEM_LEAF_STONE
	end

VerdantRiver_EventScript_Item_MysticWater::
	finditem ITEM_MYSTIC_WATER
	end

VerdantRiver_EventScript_Item_WaterStone::
	finditem ITEM_WATER_STONE
	end

VerdantRiver_EventScript_Item_DiveBall::
	finditem ITEM_DIVE_BALL
	end

VerdantRiver_EventScript_Item_PpUp::
	finditem ITEM_PP_UP
	end

