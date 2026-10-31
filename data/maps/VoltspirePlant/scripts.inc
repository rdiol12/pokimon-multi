VoltspirePlant_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, VoltspirePlant_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, VoltspirePlant_OnTransition
	.byte 0

VoltspirePlant_OnLoad:
	call VoltspirePlant_EventScript_Refresh
	call_if_unset FLAG_VOLT_POWER_ON, VoltspirePlant_EventScript_GeneratorOff
	end

@ the Rotom hides in one of the five appliances
VoltspirePlant_OnTransition:
	random 5
	copyvar VAR_TEMP_1, VAR_RESULT
	end

VoltspirePlant_EventScript_GeneratorOff::
	setmetatile 18, 4, 752, TRUE
	setmetatile 19, 4, 753, TRUE
	setmetatile 20, 4, 754, TRUE
	setmetatile 21, 4, 755, TRUE
	setmetatile 18, 5, 756, TRUE
	setmetatile 19, 5, 757, TRUE
	setmetatile 20, 5, 758, TRUE
	setmetatile 21, 5, 759, TRUE
	return

VoltspirePlant_EventScript_Refresh::
	call VoltspirePlant_EventScript_Door0
	call VoltspirePlant_EventScript_Door1
	call VoltspirePlant_EventScript_Door2
	call VoltspirePlant_EventScript_Door3
	call VoltspirePlant_EventScript_Panels
	return

VoltspirePlant_EventScript_Door0::
	goto_if_unset FLAG_VOLT_BREAKER_A, VoltspirePlant_EventScript_Door0Shut
	setmetatile 14, 22, 617, TRUE
	setmetatile 14, 23, 625, TRUE
	setmetatile 14, 24, 621, FALSE
	setmetatile 14, 25, 641, FALSE
	return

VoltspirePlant_EventScript_Door0Shut::
	setmetatile 14, 22, 694, TRUE
	setmetatile 14, 23, 702, TRUE
	setmetatile 14, 24, 710, TRUE
	setmetatile 14, 25, 718, TRUE
	return

VoltspirePlant_EventScript_Door1::
	goto_if_set FLAG_VOLT_BREAKER_A, VoltspirePlant_EventScript_Door1Shut
	setmetatile 29, 22, 617, TRUE
	setmetatile 29, 23, 625, TRUE
	setmetatile 29, 24, 621, FALSE
	setmetatile 29, 25, 641, FALSE
	return

VoltspirePlant_EventScript_Door1Shut::
	setmetatile 29, 22, 694, TRUE
	setmetatile 29, 23, 702, TRUE
	setmetatile 29, 24, 710, TRUE
	setmetatile 29, 25, 718, TRUE
	return

VoltspirePlant_EventScript_Door2::
	goto_if_set FLAG_VOLT_BREAKER_B, VoltspirePlant_EventScript_Door2Shut
	goto_if_unset FLAG_VOLT_BREAKER_C, VoltspirePlant_EventScript_Door2Shut
	setmetatile 14, 5, 617, TRUE
	setmetatile 14, 6, 625, TRUE
	setmetatile 14, 7, 621, FALSE
	setmetatile 14, 8, 641, FALSE
	return

VoltspirePlant_EventScript_Door2Shut::
	setmetatile 14, 5, 695, TRUE
	setmetatile 14, 6, 703, TRUE
	setmetatile 14, 7, 711, TRUE
	setmetatile 14, 8, 719, TRUE
	return

VoltspirePlant_EventScript_Door3::
	goto_if_unset FLAG_VOLT_BREAKER_B, VoltspirePlant_EventScript_Door3Shut
	goto_if_set FLAG_VOLT_BREAKER_C, VoltspirePlant_EventScript_Door3Shut
	setmetatile 29, 5, 617, TRUE
	setmetatile 29, 6, 625, TRUE
	setmetatile 29, 7, 621, FALSE
	setmetatile 29, 8, 641, FALSE
	return

VoltspirePlant_EventScript_Door3Shut::
	setmetatile 29, 5, 695, TRUE
	setmetatile 29, 6, 703, TRUE
	setmetatile 29, 7, 711, TRUE
	setmetatile 29, 8, 719, TRUE
	return

VoltspirePlant_EventScript_Panels::
	call_if_set FLAG_VOLT_BREAKER_A, VoltspirePlant_EventScript_PanelAOn
	call_if_unset FLAG_VOLT_BREAKER_A, VoltspirePlant_EventScript_PanelAOff
	call_if_set FLAG_VOLT_BREAKER_B, VoltspirePlant_EventScript_PanelBOn
	call_if_unset FLAG_VOLT_BREAKER_B, VoltspirePlant_EventScript_PanelBOff
	call_if_set FLAG_VOLT_BREAKER_C, VoltspirePlant_EventScript_PanelCOn
	call_if_unset FLAG_VOLT_BREAKER_C, VoltspirePlant_EventScript_PanelCOff
	return

VoltspirePlant_EventScript_PanelAOn::
	setmetatile 19, 18, 762, TRUE
	setmetatile 19, 19, 763, TRUE
	return

VoltspirePlant_EventScript_PanelAOff::
	setmetatile 19, 18, 760, TRUE
	setmetatile 19, 19, 761, TRUE
	return

VoltspirePlant_EventScript_BreakerA::
	lockall
	msgbox VoltspirePlant_Text_BreakerA, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VoltspirePlant_EventScript_BreakerLeave
	playse SE_SWITCH
	goto_if_set FLAG_VOLT_BREAKER_A, VoltspirePlant_EventScript_BreakerADown
	setflag FLAG_VOLT_BREAKER_A
	goto VoltspirePlant_EventScript_BreakerFlipped

VoltspirePlant_EventScript_BreakerADown::
	clearflag FLAG_VOLT_BREAKER_A
	goto VoltspirePlant_EventScript_BreakerFlipped

VoltspirePlant_EventScript_PanelBOn::
	setmetatile 6, 2, 766, TRUE
	setmetatile 6, 3, 767, TRUE
	return

VoltspirePlant_EventScript_PanelBOff::
	setmetatile 6, 2, 764, TRUE
	setmetatile 6, 3, 765, TRUE
	return

VoltspirePlant_EventScript_BreakerB::
	lockall
	msgbox VoltspirePlant_Text_BreakerB, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VoltspirePlant_EventScript_BreakerLeave
	playse SE_SWITCH
	goto_if_set FLAG_VOLT_BREAKER_B, VoltspirePlant_EventScript_BreakerBDown
	setflag FLAG_VOLT_BREAKER_B
	goto VoltspirePlant_EventScript_BreakerFlipped

VoltspirePlant_EventScript_BreakerBDown::
	clearflag FLAG_VOLT_BREAKER_B
	goto VoltspirePlant_EventScript_BreakerFlipped

VoltspirePlant_EventScript_PanelCOn::
	setmetatile 37, 2, 770, TRUE
	setmetatile 37, 3, 771, TRUE
	return

VoltspirePlant_EventScript_PanelCOff::
	setmetatile 37, 2, 768, TRUE
	setmetatile 37, 3, 769, TRUE
	return

VoltspirePlant_EventScript_BreakerC::
	lockall
	msgbox VoltspirePlant_Text_BreakerC, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VoltspirePlant_EventScript_BreakerLeave
	playse SE_SWITCH
	goto_if_set FLAG_VOLT_BREAKER_C, VoltspirePlant_EventScript_BreakerCDown
	setflag FLAG_VOLT_BREAKER_C
	goto VoltspirePlant_EventScript_BreakerFlipped

VoltspirePlant_EventScript_BreakerCDown::
	clearflag FLAG_VOLT_BREAKER_C
	goto VoltspirePlant_EventScript_BreakerFlipped

VoltspirePlant_EventScript_BreakerFlipped::
	call VoltspirePlant_EventScript_Refresh
	special DrawWholeMapView
	waitse
	playse SE_THUNDERSTORM_STOP
	msgbox VoltspirePlant_Text_BreakerFlipped, MSGBOX_DEFAULT
	releaseall
	end

VoltspirePlant_EventScript_BreakerLeave::
	releaseall
	end


@ ---- the generator
VoltspirePlant_EventScript_Generator::
	lockall
	goto_if_set FLAG_VOLT_POWER_ON, VoltspirePlant_EventScript_GeneratorHums
	msgbox VoltspirePlant_Text_GeneratorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VoltspirePlant_EventScript_BreakerLeave
	setflag FLAG_VOLT_POWER_ON
	playse SE_THUNDER
	setmetatile 18, 4, 728, TRUE
	setmetatile 19, 4, 729, TRUE
	setmetatile 20, 4, 730, TRUE
	setmetatile 21, 4, 731, TRUE
	setmetatile 18, 5, 736, TRUE
	setmetatile 19, 5, 737, TRUE
	setmetatile 20, 5, 738, TRUE
	setmetatile 21, 5, 739, TRUE
	special DrawWholeMapView
	waitse
	msgbox VoltspirePlant_Text_GeneratorOn, MSGBOX_DEFAULT
	releaseall
	end

VoltspirePlant_EventScript_GeneratorHums::
	msgbox VoltspirePlant_Text_GeneratorHums, MSGBOX_DEFAULT
	releaseall
	end

@ ---- the Rotom in the break room
VoltspirePlant_EventScript_Appliance0::
	lockall
	goto_if_set FLAG_VOLT_ROTOM_DONE, VoltspirePlant_EventScript_Appliance0Quiet
	goto_if_eq VAR_TEMP_1, 0, VoltspirePlant_EventScript_RotomFound
	msgbox VoltspirePlant_Text_Appliance0, MSGBOX_DEFAULT
	goto VoltspirePlant_EventScript_RotomHops

VoltspirePlant_EventScript_Appliance0Quiet::
	msgbox VoltspirePlant_Text_Appliance0Quiet, MSGBOX_DEFAULT
	releaseall
	end

VoltspirePlant_EventScript_Appliance1::
	lockall
	goto_if_set FLAG_VOLT_ROTOM_DONE, VoltspirePlant_EventScript_Appliance1Quiet
	goto_if_eq VAR_TEMP_1, 1, VoltspirePlant_EventScript_RotomFound
	msgbox VoltspirePlant_Text_Appliance1, MSGBOX_DEFAULT
	goto VoltspirePlant_EventScript_RotomHops

VoltspirePlant_EventScript_Appliance1Quiet::
	msgbox VoltspirePlant_Text_Appliance1Quiet, MSGBOX_DEFAULT
	releaseall
	end

VoltspirePlant_EventScript_Appliance2::
	lockall
	goto_if_set FLAG_VOLT_ROTOM_DONE, VoltspirePlant_EventScript_Appliance2Quiet
	goto_if_eq VAR_TEMP_1, 2, VoltspirePlant_EventScript_RotomFound
	msgbox VoltspirePlant_Text_Appliance2, MSGBOX_DEFAULT
	goto VoltspirePlant_EventScript_RotomHops

VoltspirePlant_EventScript_Appliance2Quiet::
	msgbox VoltspirePlant_Text_Appliance2Quiet, MSGBOX_DEFAULT
	releaseall
	end

VoltspirePlant_EventScript_Appliance3::
	lockall
	goto_if_set FLAG_VOLT_ROTOM_DONE, VoltspirePlant_EventScript_Appliance3Quiet
	goto_if_eq VAR_TEMP_1, 3, VoltspirePlant_EventScript_RotomFound
	msgbox VoltspirePlant_Text_Appliance3, MSGBOX_DEFAULT
	goto VoltspirePlant_EventScript_RotomHops

VoltspirePlant_EventScript_Appliance3Quiet::
	msgbox VoltspirePlant_Text_Appliance3Quiet, MSGBOX_DEFAULT
	releaseall
	end

VoltspirePlant_EventScript_Appliance4::
	lockall
	goto_if_set FLAG_VOLT_ROTOM_DONE, VoltspirePlant_EventScript_Appliance4Quiet
	goto_if_eq VAR_TEMP_1, 4, VoltspirePlant_EventScript_RotomFound
	msgbox VoltspirePlant_Text_Appliance4, MSGBOX_DEFAULT
	goto VoltspirePlant_EventScript_RotomHops

VoltspirePlant_EventScript_Appliance4Quiet::
	msgbox VoltspirePlant_Text_Appliance4Quiet, MSGBOX_DEFAULT
	releaseall
	end


VoltspirePlant_EventScript_RotomHops::
	playse SE_M_THUNDERBOLT2
	msgbox VoltspirePlant_Text_RotomHops, MSGBOX_DEFAULT
	random 5
	copyvar VAR_TEMP_1, VAR_RESULT
	releaseall
	end

VoltspirePlant_EventScript_RotomFound::
	msgbox VoltspirePlant_Text_RotomFound, MSGBOX_DEFAULT
	setwildbattle SPECIES_ROTOM, 25
	playmoncry SPECIES_ROTOM, CRY_MODE_ENCOUNTER
	msgbox VoltspirePlant_Text_Rotom, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, VoltspirePlant_EventScript_RotomStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, VoltspirePlant_EventScript_RotomStays
	setflag FLAG_VOLT_ROTOM_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, VoltspirePlant_EventScript_RotomEnd
	msgbox VoltspirePlant_Text_RotomGone, MSGBOX_DEFAULT
VoltspirePlant_EventScript_RotomEnd::
	releaseall
	end

VoltspirePlant_EventScript_RotomStays::
	msgbox VoltspirePlant_Text_RotomStays, MSGBOX_DEFAULT
	random 5
	copyvar VAR_TEMP_1, VAR_RESULT
	releaseall
	end

@ ---- people
VoltspirePlant_EventScript_Chief::
	lock
	faceplayer
	goto_if_set FLAG_VOLT_REWARD, VoltspirePlant_EventScript_ChiefDone
	goto_if_set FLAG_VOLT_POWER_ON, VoltspirePlant_EventScript_ChiefThanks
	msgbox VoltspirePlant_Text_ChiefAsk, MSGBOX_DEFAULT
	release
	end

VoltspirePlant_EventScript_ChiefThanks::
	msgbox VoltspirePlant_Text_ChiefThanks, MSGBOX_DEFAULT
	giveitem ITEM_TM_SHOCK_WAVE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_VOLT_REWARD
	msgbox VoltspirePlant_Text_ChiefAfter, MSGBOX_DEFAULT
	release
	end

VoltspirePlant_EventScript_ChiefDone::
	msgbox VoltspirePlant_Text_ChiefDone, MSGBOX_DEFAULT
	release
	end

VoltspirePlant_EventScript_Worker::
	msgbox VoltspirePlant_Text_Worker, MSGBOX_NPC
	end

VoltspirePlant_EventScript_WorkerF::
	msgbox VoltspirePlant_Text_WorkerF, MSGBOX_NPC
	end

VoltspirePlant_EventScript_Zapp::
	trainerbattle_single TRAINER_VOLT_ZAPP, VoltspirePlant_Text_ZappIntro, VoltspirePlant_Text_ZappDefeat
	msgbox VoltspirePlant_Text_ZappPost, MSGBOX_AUTOCLOSE
	end

VoltspirePlant_EventScript_Ohm::
	trainerbattle_single TRAINER_VOLT_OHM, VoltspirePlant_Text_OhmIntro, VoltspirePlant_Text_OhmDefeat
	msgbox VoltspirePlant_Text_OhmPost, MSGBOX_AUTOCLOSE
	end

VoltspirePlant_EventScript_Ampere::
	trainerbattle_single TRAINER_VOLT_AMPERE, VoltspirePlant_Text_AmpereIntro, VoltspirePlant_Text_AmpereDefeat
	msgbox VoltspirePlant_Text_AmperePost, MSGBOX_AUTOCLOSE
	end

VoltspirePlant_Text_BreakerA:
	.string "A breaker panel with a YELLOW band.\n"
	.string "Flip the lever?$"

VoltspirePlant_Text_BreakerB:
	.string "A breaker panel with a BLUE band.\n"
	.string "Flip the lever?$"

VoltspirePlant_Text_BreakerC:
	.string "A breaker panel with a RED band.\n"
	.string "Flip the lever?$"

VoltspirePlant_Text_BreakerFlipped:
	.string "Clunk! Somewhere in the plant, the\n"
	.string "barriers hummed and shifted.$"

VoltspirePlant_Text_GeneratorAsk:
	.string "The main generator is dead quiet.\n"
	.string "Someone has pulled every plug.\p"
	.string "Plug it back in and restart it?$"

VoltspirePlant_Text_GeneratorOn:
	.string "Whirrr… VRMMMM!\p"
	.string "The generator roared back to life!\n"
	.string "The whole plant lights up!$"

VoltspirePlant_Text_GeneratorHums:
	.string "The generator hums steadily. All of\n"
	.string "MAUVILLE has power again.$"

VoltspirePlant_Text_RotomHops:
	.string "Bzzt! Something zipped out and\n"
	.string "into another appliance!$"

VoltspirePlant_Text_RotomFound:
	.string "Something in there is giggling…\n"
	.string "A POKéMON made of electricity!$"

VoltspirePlant_Text_Rotom:
	.string "Rotom-rotom!$"

VoltspirePlant_Text_RotomGone:
	.string "ROTOM zipped off into the wiring.\n"
	.string "The appliances are quiet now.$"

VoltspirePlant_Text_RotomStays:
	.string "ROTOM giggled and dove into\n"
	.string "another appliance!$"

VoltspirePlant_Text_Appliance0:
	.string "The TV flickers on by itself… and\n"
	.string "snaps off again.$"

VoltspirePlant_Text_Appliance0Quiet:
	.string "It's an old TV. The picture is fuzzy.$"

VoltspirePlant_Text_Appliance1:
	.string "The fridge door rattles. A cold draft\n"
	.string "and a spark escape from inside!$"

VoltspirePlant_Text_Appliance1Quiet:
	.string "It's a fridge full of SODA POP.$"

VoltspirePlant_Text_Appliance2:
	.string "The washing machine starts spinning\n"
	.string "all on its own… then stops dead.$"

VoltspirePlant_Text_Appliance2Quiet:
	.string "It's a washing machine. Someone's\n"
	.string "uniform is in the wash.$"

VoltspirePlant_Text_Appliance3:
	.string "The fan whirs to full speed for a\n"
	.string "moment, then winds down.$"

VoltspirePlant_Text_Appliance3Quiet:
	.string "It's an electric fan. It's off.$"

VoltspirePlant_Text_Appliance4:
	.string "The lawn mower coughs, sputters\n"
	.string "and goes quiet.$"

VoltspirePlant_Text_Appliance4Quiet:
	.string "It's a lawn mower. Why is it here?$"

VoltspirePlant_Text_ChiefAsk:
	.string "You're the trainer WATTSON sent? Good.\p"
	.string "Something got into the wiring and\n"
	.string "tripped the main generator. Now the\l"
	.string "barriers between the halls are\l"
	.string "jammed every which way.\p"
	.string "Three breakers, yellow, blue and\n"
	.string "red, move the barriers. Find a way\l"
	.string "to the generator room up north!$"

VoltspirePlant_Text_ChiefThanks:
	.string "The lights are back! You did it!\p"
	.string "The workers say a ROTOM was behind\n"
	.string "it all. Ha! Here, this is for you.$"

VoltspirePlant_Text_ChiefAfter:
	.string "SHOCK WAVE never misses. A fitting\n"
	.string "move for a trainer this sharp!$"

VoltspirePlant_Text_ChiefDone:
	.string "MAUVILLE runs on this plant. Thanks\n"
	.string "to you, the lights stay on.$"

VoltspirePlant_Text_Worker:
	.string "The yellow breaker swaps the two\n"
	.string "lobby doors. The others… I forget!$"

VoltspirePlant_Text_WorkerF:
	.string "Our break room appliances keep\n"
	.string "turning on by themselves. Spooky!$"

VoltspirePlant_Text_ZappIntro:
	.string "This hall's got the best acoustics!\n"
	.string "Let's make some noise!$"

VoltspirePlant_Text_ZappDefeat:
	.string "You blew my fuse!$"

VoltspirePlant_Text_ZappPost:
	.string "The blue breaker is in this hall.\n"
	.string "Try it, see what happens.$"

VoltspirePlant_Text_OhmIntro:
	.string "I come here to see rare electric\n"
	.string "POKéMON. Are yours rare?$"

VoltspirePlant_Text_OhmDefeat:
	.string "Shocking!$"

VoltspirePlant_Text_OhmPost:
	.string "I heard giggling in the break room.\n"
	.string "Something's hiding in there!$"

VoltspirePlant_Text_AmpereIntro:
	.string "You found the way through? Then\n"
	.string "you're worth a real battle!$"

VoltspirePlant_Text_AmpereDefeat:
	.string "Overloaded!$"

VoltspirePlant_Text_AmperePost:
	.string "Go on, the generator's right there.\n"
	.string "Plug it in and give it a kick!$"

VoltspirePlant_EventScript_Item_Magnet::
	finditem ITEM_MAGNET
	end

VoltspirePlant_EventScript_Item_Electirizer::
	finditem ITEM_ELECTIRIZER
	end

VoltspirePlant_EventScript_Item_ThunderStone::
	finditem ITEM_THUNDER_STONE
	end

VoltspirePlant_EventScript_Item_SuperRepel::
	finditem ITEM_SUPER_REPEL
	end

VoltspirePlant_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

