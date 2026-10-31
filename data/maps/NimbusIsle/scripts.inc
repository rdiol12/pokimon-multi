NimbusIsle_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, NimbusIsle_OnLoad
	.byte 0

NimbusIsle_OnLoad:
	call_if_set FLAG_NIMBUS_CHIMES_DONE, NimbusIsle_EventScript_SummitWind
	end

NimbusIsle_EventScript_SummitWind::
	setmetatile 41, 9, 674, FALSE
	setmetatile 41, 10, 674, FALSE
	setmetatile 41, 11, 674, FALSE
	setmetatile 41, 12, 674, FALSE
	setmetatile 41, 13, 674, FALSE
	return

NimbusIsle_EventScript_Chime1::
	lock
	playse SE_BALL_TRAY_EXIT
	msgbox NimbusIsle_Text_Chime, MSGBOX_DEFAULT
	removeobject VAR_LAST_TALKED
	setflag FLAG_NIMBUS_CHIME_1
	release
	end

NimbusIsle_EventScript_Chime2::
	lock
	playse SE_BALL_TRAY_EXIT
	msgbox NimbusIsle_Text_Chime, MSGBOX_DEFAULT
	removeobject VAR_LAST_TALKED
	setflag FLAG_NIMBUS_CHIME_2
	release
	end

NimbusIsle_EventScript_Chime3::
	lock
	playse SE_BALL_TRAY_EXIT
	msgbox NimbusIsle_Text_Chime, MSGBOX_DEFAULT
	removeobject VAR_LAST_TALKED
	setflag FLAG_NIMBUS_CHIME_3
	release
	end

NimbusIsle_EventScript_Tornadus::
	lock
	faceplayer
	msgbox NimbusIsle_Text_Gale, MSGBOX_DEFAULT
	setwildbattle SPECIES_TORNADUS, 50
	playmoncry SPECIES_TORNADUS, CRY_MODE_ENCOUNTER
	msgbox NimbusIsle_Text_Tornadus, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, NimbusIsle_EventScript_TornadusStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, NimbusIsle_EventScript_TornadusStays
	setflag FLAG_NIMBUS_TORNADUS_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, NimbusIsle_EventScript_TornadusEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox NimbusIsle_Text_TornadusGone, MSGBOX_DEFAULT
NimbusIsle_EventScript_TornadusEnd::
	release
	end

NimbusIsle_EventScript_TornadusStays::
	msgbox NimbusIsle_Text_TornadusStays, MSGBOX_DEFAULT
	release
	end

NimbusIsle_EventScript_Swablu::
	lock
	faceplayer
	playmoncry SPECIES_SWABLU, CRY_MODE_NORMAL
	msgbox NimbusIsle_Text_Swablu, MSGBOX_DEFAULT
	waitmoncry
	release
	end

NimbusIsle_EventScript_Corvin::
	trainerbattle_single TRAINER_NIMBUS_CORVIN, NimbusIsle_Text_CorvinIntro, NimbusIsle_Text_CorvinDefeat
	msgbox NimbusIsle_Text_CorvinPost, MSGBOX_AUTOCLOSE
	end

NimbusIsle_EventScript_Draco::
	trainerbattle_single TRAINER_NIMBUS_DRACO, NimbusIsle_Text_DracoIntro, NimbusIsle_Text_DracoDefeat
	msgbox NimbusIsle_Text_DracoPost, MSGBOX_AUTOCLOSE
	end

NimbusIsle_EventScript_Aurora::
	trainerbattle_single TRAINER_NIMBUS_AURORA, NimbusIsle_Text_AuroraIntro, NimbusIsle_Text_AuroraDefeat
	msgbox NimbusIsle_Text_AuroraPost, MSGBOX_AUTOCLOSE
	end

NimbusIsle_EventScript_Zephyr::
	trainerbattle_single TRAINER_NIMBUS_ZEPHYR, NimbusIsle_Text_ZephyrIntro, NimbusIsle_Text_ZephyrDefeat
	msgbox NimbusIsle_Text_ZephyrPost, MSGBOX_AUTOCLOSE
	end

NimbusIsle_Text_Chime:
	.string "Ting-a-ling…\n"
	.string "{PLAYER} found a WIND CHIME!$"

NimbusIsle_Text_Gale:
	.string "A fierce wind howls around the\n"
	.string "shrine… and takes shape!$"

NimbusIsle_Text_Tornadus:
	.string "Fwoooosh!$"

NimbusIsle_Text_TornadusGone:
	.string "TORNADUS whirled away into the\n"
	.string "clouds.$"

NimbusIsle_Text_TornadusStays:
	.string "TORNADUS still circles the shrine,\n"
	.string "howling.$"

NimbusIsle_Text_Swablu:
	.string "SWABLU fluffed up its cotton wings.$"

NimbusIsle_Text_CorvinIntro:
	.string "Up here, my birds fly higher than\n"
	.string "anywhere! Can you keep up?$"

NimbusIsle_Text_CorvinDefeat:
	.string "Shot down…$"

NimbusIsle_Text_CorvinPost:
	.string "The wind streams only blow one way.\n"
	.string "East out, west back.$"

NimbusIsle_Text_DracoIntro:
	.string "Dragons rule the skies! Prepare\n"
	.string "yourself!$"

NimbusIsle_Text_DracoDefeat:
	.string "My dragons… grounded!$"

NimbusIsle_Text_DracoPost:
	.string "I saw something shiny blow onto\n"
	.string "this island in the storm.$"

NimbusIsle_Text_AuroraIntro:
	.string "Isn't the view stunning? Almost as\n"
	.string "stunning as my team!$"

NimbusIsle_Text_AuroraDefeat:
	.string "Stunned, indeed.$"

NimbusIsle_Text_AuroraPost:
	.string "On a clear day, you can see all of\n"
	.string "HOENN from these islands.$"

NimbusIsle_Text_ZephyrIntro:
	.string "I sense a great wind above us,\n"
	.string "waiting to be woken…$"

NimbusIsle_Text_ZephyrDefeat:
	.string "My senses… scattered.$"

NimbusIsle_Text_ZephyrPost:
	.string "When the CHIME TOWER sings, the\n"
	.string "wind will rise to the shrine.$"

NimbusIsle_EventScript_Item_SharpBeak::
	finditem ITEM_SHARP_BEAK
	end

NimbusIsle_EventScript_Item_WideLens::
	finditem ITEM_WIDE_LENS
	end

NimbusIsle_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

NimbusIsle_EventScript_Item_TmSteelWing::
	finditem ITEM_TM_STEEL_WING
	end

NimbusIsle_EventScript_Item_PpMax::
	finditem ITEM_PP_MAX
	end

NimbusIsle_EventScript_Item_CometShard::
	finditem ITEM_COMET_SHARD
	end

