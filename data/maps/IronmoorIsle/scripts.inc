IronmoorIsle_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, IronmoorIsle_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, IronmoorIsle_OnTransition
	.byte 0

IronmoorIsle_OnLoad:
	call_if_set FLAG_IRONMOOR_POWER_OFF, IronmoorIsle_EventScript_LightsOut
	call_if_set FLAG_IRONMOOR_GATE_OPEN, IronmoorIsle_EventScript_GateUp
	end

IronmoorIsle_OnTransition:
	call_if_set FLAG_IRONMOOR_POWER_OFF, IronmoorIsle_EventScript_BeamsQuiet
	end

IronmoorIsle_EventScript_BeamsQuiet::
	setvar VAR_TEMP_1, 1
	return

IronmoorIsle_EventScript_LightsOut::
	setmetatile 7, 12, 1, FALSE
	setmetatile 8, 12, 1, FALSE
	setmetatile 9, 12, 1, FALSE
	setmetatile 10, 12, 1, FALSE
	setmetatile 11, 12, 1, FALSE
	setmetatile 12, 12, 1, FALSE
	setmetatile 13, 12, 1, FALSE
	setmetatile 14, 12, 1, FALSE
	setmetatile 15, 12, 1, FALSE
	setmetatile 16, 12, 1, FALSE
	setmetatile 17, 12, 1, FALSE
	setmetatile 18, 12, 1, FALSE
	setmetatile 19, 12, 1, FALSE
	setmetatile 20, 12, 1, FALSE
	setmetatile 21, 12, 1, FALSE
	setmetatile 22, 12, 1, FALSE
	setmetatile 23, 12, 1, FALSE
	setmetatile 24, 12, 1, FALSE
	setmetatile 25, 12, 1, FALSE
	setmetatile 26, 12, 1, FALSE
	setmetatile 27, 12, 1, FALSE
	setmetatile 28, 12, 1, FALSE
	setmetatile 29, 12, 1, FALSE
	setmetatile 30, 12, 1, FALSE
	setmetatile 31, 12, 1, FALSE
	setmetatile 32, 12, 1, FALSE
	setmetatile 33, 12, 1, FALSE
	setmetatile 34, 12, 1, FALSE
	setmetatile 35, 12, 1, FALSE
	setmetatile 36, 12, 1, FALSE
	setmetatile 37, 12, 1, FALSE
	setmetatile 38, 12, 1, FALSE
	setmetatile 7, 17, 1, FALSE
	setmetatile 8, 17, 1, FALSE
	setmetatile 9, 17, 1, FALSE
	setmetatile 10, 17, 1, FALSE
	setmetatile 11, 17, 1, FALSE
	setmetatile 12, 17, 1, FALSE
	setmetatile 13, 17, 1, FALSE
	setmetatile 14, 17, 1, FALSE
	setmetatile 15, 17, 1, FALSE
	setmetatile 16, 17, 1, FALSE
	setmetatile 17, 17, 1, FALSE
	setmetatile 18, 17, 1, FALSE
	setmetatile 19, 17, 1, FALSE
	setmetatile 20, 17, 1, FALSE
	setmetatile 21, 17, 1, FALSE
	setmetatile 22, 17, 1, FALSE
	setmetatile 23, 17, 1, FALSE
	setmetatile 24, 17, 1, FALSE
	setmetatile 25, 17, 1, FALSE
	setmetatile 26, 17, 1, FALSE
	setmetatile 27, 17, 1, FALSE
	setmetatile 28, 17, 1, FALSE
	setmetatile 29, 17, 1, FALSE
	setmetatile 30, 17, 1, FALSE
	setmetatile 31, 17, 1, FALSE
	setmetatile 32, 17, 1, FALSE
	setmetatile 33, 17, 1, FALSE
	setmetatile 34, 17, 1, FALSE
	setmetatile 35, 17, 1, FALSE
	setmetatile 36, 17, 1, FALSE
	setmetatile 37, 17, 1, FALSE
	setmetatile 38, 17, 1, FALSE
	setmetatile 8, 7, 541, TRUE
	setmetatile 9, 7, 542, TRUE
	setmetatile 8, 8, 543, TRUE
	setmetatile 9, 8, 544, TRUE
	setmetatile 8, 9, 545, TRUE
	setmetatile 9, 9, 546, TRUE
	setmetatile 8, 10, 547, TRUE
	setmetatile 9, 10, 548, TRUE
	setmetatile 36, 7, 541, TRUE
	setmetatile 37, 7, 542, TRUE
	setmetatile 36, 8, 543, TRUE
	setmetatile 37, 8, 544, TRUE
	setmetatile 36, 9, 545, TRUE
	setmetatile 37, 9, 546, TRUE
	setmetatile 36, 10, 547, TRUE
	setmetatile 37, 10, 548, TRUE
	setmetatile 35, 19, 554, TRUE
	setmetatile 36, 19, 555, TRUE
	setmetatile 35, 20, 556, TRUE
	setmetatile 36, 20, 557, TRUE
	return

IronmoorIsle_EventScript_GateUp::
	setmetatile 21, 22, 527, TRUE
	setmetatile 22, 22, 528, FALSE
	setmetatile 23, 22, 529, TRUE
	setmetatile 21, 23, 530, TRUE
	setmetatile 22, 23, 531, FALSE
	setmetatile 23, 23, 532, TRUE
	return

IronmoorIsle_EventScript_Spotted::
	lockall
	playse SE_PIN
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	msgbox IronmoorIsle_Text_Spotted, MSGBOX_DEFAULT
	closemessage
	playse SE_WARP_IN
	warpsilent MAP_IRONMOOR_ISLE, 8, 21
	waitstate
	releaseall
	end

IronmoorIsle_EventScript_Generator::
	lockall
	goto_if_set FLAG_IRONMOOR_POWER_OFF, IronmoorIsle_EventScript_GeneratorOff
	msgbox IronmoorIsle_Text_GeneratorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IronmoorIsle_EventScript_Leave
	playse SE_PIN
	delay 20
	playse SE_BANG
	call IronmoorIsle_EventScript_LightsOut
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_POWER_OFF
	setvar VAR_TEMP_1, 1
	delay 30
	msgbox IronmoorIsle_Text_GeneratorDone, MSGBOX_DEFAULT
	releaseall
	end

IronmoorIsle_EventScript_GeneratorOff::
	msgbox IronmoorIsle_Text_GeneratorIsOff, MSGBOX_DEFAULT
	releaseall
	end

IronmoorIsle_EventScript_Leave::
	releaseall
	end

IronmoorIsle_EventScript_GateInside::
	lockall
	goto_if_set FLAG_IRONMOOR_GATE_OPEN, IronmoorIsle_EventScript_Leave
	msgbox IronmoorIsle_Text_GateBolt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IronmoorIsle_EventScript_Leave
	playse SE_BANG
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 6
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_DOOR
	call IronmoorIsle_EventScript_GateUp
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_GATE_OPEN
	msgbox IronmoorIsle_Text_GateUp, MSGBOX_DEFAULT
	releaseall
	end

IronmoorIsle_EventScript_GateOutside::
	goto_if_set FLAG_IRONMOOR_GATE_OPEN, IronmoorIsle_EventScript_Leave
	msgbox IronmoorIsle_Text_GateShut, MSGBOX_SIGN
	end

IronmoorIsle_EventScript_Boatman::
	lock
	faceplayer
	msgbox IronmoorIsle_Text_BoatmanBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IronmoorIsle_EventScript_BoatmanStay
	msgbox IronmoorIsle_Text_BoatmanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_ROUTE109, 17, 22
	waitstate
	release
	end

IronmoorIsle_EventScript_BoatmanStay::
	msgbox IronmoorIsle_Text_BoatmanStay, MSGBOX_DEFAULT
	release
	end

IronmoorIsle_EventScript_Sign::
	msgbox IronmoorIsle_Text_Sign, MSGBOX_SIGN
	end

IronmoorIsle_EventScript_Duskull::
	lock
	faceplayer
	playmoncry SPECIES_DUSKULL, CRY_MODE_NORMAL
	msgbox IronmoorIsle_Text_Duskull, MSGBOX_DEFAULT
	waitmoncry
	release
	end

IronmoorIsle_EventScript_Garrick::
	trainerbattle_single TRAINER_IRONMOOR_GARRICK, IronmoorIsle_Text_GarrickIntro, IronmoorIsle_Text_GarrickDefeat
	msgbox IronmoorIsle_Text_GarrickPost, MSGBOX_AUTOCLOSE
	end

IronmoorIsle_Text_GarrickIntro:
	.string "Ironmoor was a fortress before it was\n"
	.string "a prison. I'm here for its history!$"

IronmoorIsle_Text_GarrickDefeat:
	.string "History repeats… my losses.$"

IronmoorIsle_Text_GarrickPost:
	.string "The gate's rusted shut. The wall fell\n"
	.string "in by the graveyard, though.$"


IronmoorIsle_EventScript_Kaito::
	trainerbattle_single TRAINER_IRONMOOR_KAITO, IronmoorIsle_Text_KaitoIntro, IronmoorIsle_Text_KaitoDefeat
	msgbox IronmoorIsle_Text_KaitoPost, MSGBOX_AUTOCLOSE
	end

IronmoorIsle_Text_KaitoIntro:
	.string "I train among the graves. Nothing\n"
	.string "scares a ninja!$"

IronmoorIsle_Text_KaitoDefeat:
	.string "Eek! I mean… well fought.$"

IronmoorIsle_Text_KaitoPost:
	.string "The searchlights in the yard still\n"
	.string "work. Nobody knows who runs them.$"


IronmoorIsle_EventScript_Morwen::
	trainerbattle_single TRAINER_IRONMOOR_MORWEN, IronmoorIsle_Text_MorwenIntro, IronmoorIsle_Text_MorwenDefeat
	msgbox IronmoorIsle_Text_MorwenPost, MSGBOX_AUTOCLOSE
	end

IronmoorIsle_Text_MorwenIntro:
	.string "The guards never left, you know.\n"
	.string "They still watch the yard…$"

IronmoorIsle_Text_MorwenDefeat:
	.string "The spirits laugh at me…$"

IronmoorIsle_Text_MorwenPost:
	.string "The lights run off the generator in\n"
	.string "the corner of the yard.$"


IronmoorIsle_Text_Spotted:
	.string "A searchlight swept over you!\p"
	.string "Ghostly whistles shriek across the\n"
	.string "yard, and cold hands drag you back\l"
	.string "to the breach…$"

IronmoorIsle_Text_GeneratorAsk:
	.string "An old generator, still humming.\n"
	.string "Cables run up to both watchtowers.\p"
	.string "Pull the switch?$"

IronmoorIsle_Text_GeneratorDone:
	.string "The generator shuddered to a stop.\p"
	.string "The searchlights went dark!$"

IronmoorIsle_Text_GeneratorIsOff:
	.string "The generator is silent.$"

IronmoorIsle_Text_GateBolt:
	.string "The gate's bolt is rusted in place,\n"
	.string "but from this side you could knock\l"
	.string "it loose.\p"
	.string "Strike the bolt?$"

IronmoorIsle_Text_GateUp:
	.string "The bolt gave way, and the portcullis\n"
	.string "groaned up into the arch!$"

IronmoorIsle_Text_GateShut:
	.string "The gate is rusted shut.$"

IronmoorIsle_Text_BoatmanBack:
	.string "Seen enough ghosts? I'll row you\n"
	.string "back to ROUTE 109.$"

IronmoorIsle_Text_BoatmanGo:
	.string "Mind the fog.$"

IronmoorIsle_Text_BoatmanStay:
	.string "I'll be here. I don't go ashore.$"

IronmoorIsle_Text_Sign:
	.string "IRONMOOR PRISON\n"
	.string "Closed by order of the HOENN courts.$"

IronmoorIsle_Text_Duskull:
	.string "DUSKULL: Duuu…\p"
	.string "It's drifting along the yard like a\n"
	.string "guard on patrol.$"

IronmoorIsle_EventScript_Item_SpellTag::
	finditem ITEM_SPELL_TAG
	end

IronmoorIsle_EventScript_Item_DuskBall::
	finditem ITEM_DUSK_BALL
	end

IronmoorIsle_EventScript_Item_FullHeal::
	finditem ITEM_FULL_HEAL
	end

