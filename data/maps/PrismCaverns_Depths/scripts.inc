PrismCaverns_Depths_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PrismCaverns_Depths_OnLoad
	.byte 0

PrismCaverns_Depths_OnLoad:
	call_if_set FLAG_PRISM_EMERALD, PrismCaverns_Depths_EventScript_LitGreen
	call_if_set FLAG_PRISM_TOPAZ, PrismCaverns_Depths_EventScript_LitGold
	call_if_set FLAG_PRISM_SEAL_OPEN, PrismCaverns_Depths_EventScript_SealOpen
	call_if_set FLAG_PRISM_RUBY, PrismCaverns_Depths_EventScript_WakeRed
	call_if_set FLAG_PRISM_SAPPHIRE, PrismCaverns_Depths_EventScript_WakeBlue
	call_if_set FLAG_PRISM_EMERALD, PrismCaverns_Depths_EventScript_WakeGreen
	call_if_set FLAG_PRISM_TOPAZ, PrismCaverns_Depths_EventScript_WakeGold
	end

PrismCaverns_Depths_EventScript_WakeRed::
	setmetatile 18, 32, 969, FALSE
	setmetatile 7, 34, 969, FALSE
	setmetatile 38, 31, 969, FALSE
	setmetatile 41, 10, 969, FALSE
	setmetatile 3, 8, 969, FALSE
	setmetatile 8, 22, 969, FALSE
	return

PrismCaverns_Depths_EventScript_WakeBlue::
	setmetatile 30, 32, 971, FALSE
	setmetatile 41, 34, 971, FALSE
	setmetatile 10, 31, 971, FALSE
	setmetatile 7, 10, 971, FALSE
	return

PrismCaverns_Depths_EventScript_WakeGreen::
	setmetatile 11, 21, 973, FALSE
	setmetatile 37, 22, 973, FALSE
	return

PrismCaverns_Depths_EventScript_WakeGold::
	setmetatile 44, 22, 975, FALSE
	setmetatile 20, 36, 975, FALSE
	return


PrismCaverns_Depths_EventScript_SealOpen::
	setmetatile 23, 24, 933, FALSE
	setmetatile 24, 24, 933, FALSE
	setmetatile 25, 24, 933, FALSE
	setmetatile 23, 25, 933, FALSE
	setmetatile 24, 25, 933, FALSE
	setmetatile 25, 25, 933, FALSE
	return

PrismCaverns_Depths_EventScript_LitGreen::
	setmetatile 5, 18, 963, TRUE
	setmetatile 5, 19, 964, TRUE
	return

PrismCaverns_Depths_EventScript_ResGreen::
	lockall
	goto_if_set FLAG_PRISM_EMERALD, PrismCaverns_Depths_EventScript_ResAlreadyLit
	msgbox PrismCaverns_Text_ResTouch, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PrismCaverns_Depths_EventScript_ResLeave
	playse SE_M_HEAL_BELL
	call PrismCaverns_Depths_EventScript_LitGreen
	special DrawWholeMapView
	waitse
	setflag FLAG_PRISM_EMERALD
	msgbox PrismCaverns_Text_ResGreen, MSGBOX_DEFAULT
	call PrismCaverns_Depths_EventScript_WakeGreen
	playse SE_GLASS_FLUTE
	special DrawWholeMapView
	waitse
	msgbox PrismCaverns_Text_PadsWakeGreen, MSGBOX_DEFAULT
	goto_if_unset FLAG_PRISM_EMERALD, PrismCaverns_Depths_EventScript_ResDone
	goto_if_unset FLAG_PRISM_TOPAZ, PrismCaverns_Depths_EventScript_ResDone
	goto_if_unset FLAG_PRISM_RUBY, PrismCaverns_Depths_EventScript_ResDone
	goto_if_unset FLAG_PRISM_SAPPHIRE, PrismCaverns_Depths_EventScript_ResDone
	call PrismCaverns_Depths_EventScript_AllFour
	releaseall
	end


PrismCaverns_Depths_EventScript_LitGold::
	setmetatile 43, 18, 967, TRUE
	setmetatile 43, 19, 968, TRUE
	return

PrismCaverns_Depths_EventScript_ResGold::
	lockall
	goto_if_set FLAG_PRISM_TOPAZ, PrismCaverns_Depths_EventScript_ResAlreadyLit
	msgbox PrismCaverns_Text_ResTouch, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PrismCaverns_Depths_EventScript_ResLeave
	playse SE_M_HEAL_BELL
	call PrismCaverns_Depths_EventScript_LitGold
	special DrawWholeMapView
	waitse
	setflag FLAG_PRISM_TOPAZ
	msgbox PrismCaverns_Text_ResGold, MSGBOX_DEFAULT
	call PrismCaverns_Depths_EventScript_WakeGold
	playse SE_GLASS_FLUTE
	special DrawWholeMapView
	waitse
	msgbox PrismCaverns_Text_PadsWakeGold, MSGBOX_DEFAULT
	goto_if_unset FLAG_PRISM_EMERALD, PrismCaverns_Depths_EventScript_ResDone
	goto_if_unset FLAG_PRISM_TOPAZ, PrismCaverns_Depths_EventScript_ResDone
	goto_if_unset FLAG_PRISM_RUBY, PrismCaverns_Depths_EventScript_ResDone
	goto_if_unset FLAG_PRISM_SAPPHIRE, PrismCaverns_Depths_EventScript_ResDone
	call PrismCaverns_Depths_EventScript_AllFour
	releaseall
	end


PrismCaverns_Depths_EventScript_ResDone::
	releaseall
	end

PrismCaverns_Depths_EventScript_ResAlreadyLit::
	msgbox PrismCaverns_Text_ResAlreadyLit, MSGBOX_DEFAULT
	releaseall
	end

PrismCaverns_Depths_EventScript_ResLeave::
	releaseall
	end

PrismCaverns_Depths_EventScript_AllFour::
	delay 30
	msgbox PrismCaverns_Depths_Text_AllFour, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 3
	setvar VAR_0x8006, 10
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_M_BRICK_BREAK
	setflag FLAG_PRISM_SEAL_OPEN
	call PrismCaverns_Depths_EventScript_SealOpen
	special DrawWholeMapView
	waitse
	msgbox PrismCaverns_Depths_Text_SealBroke, MSGBOX_DEFAULT
	return

PrismCaverns_Depths_EventScript_Seal::
	goto_if_set FLAG_PRISM_SEAL_OPEN, PrismCaverns_Depths_EventScript_SealGone
	msgbox PrismCaverns_Depths_Text_Seal, MSGBOX_SIGN
	end

PrismCaverns_Depths_EventScript_SealGone::
	end

PrismCaverns_Depths_EventScript_Diancie::
	lock
	faceplayer
	msgbox PrismCaverns_Depths_Text_Sparkle, MSGBOX_DEFAULT
	setwildbattle SPECIES_DIANCIE, 40
	playmoncry SPECIES_DIANCIE, CRY_MODE_ENCOUNTER
	msgbox PrismCaverns_Depths_Text_Diancie, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, PrismCaverns_Depths_EventScript_DiancieStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, PrismCaverns_Depths_EventScript_DiancieStays
	setflag FLAG_PRISM_DIANCIE_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, PrismCaverns_Depths_EventScript_DiancieEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox PrismCaverns_Depths_Text_DiancieGone, MSGBOX_DEFAULT
PrismCaverns_Depths_EventScript_DiancieEnd::
	release
	end

PrismCaverns_Depths_EventScript_DiancieStays::
	msgbox PrismCaverns_Depths_Text_DiancieStays, MSGBOX_DEFAULT
	release
	end

PrismCaverns_Depths_EventScript_Tessa::
	trainerbattle_single TRAINER_PRISM_TESSA, PrismCaverns_Depths_Text_TessaIntro, PrismCaverns_Depths_Text_TessaDefeat
	msgbox PrismCaverns_Depths_Text_TessaPost, MSGBOX_AUTOCLOSE
	end

PrismCaverns_Depths_Text_TessaIntro:
	.string "You found the way down here? Then\n"
	.string "you can handle the cold, too!$"

PrismCaverns_Depths_Text_TessaDefeat:
	.string "Frozen solid…$"

PrismCaverns_Depths_Text_TessaPost:
	.string "The red plate on this island goes\n"
	.string "somewhere new. I haven't dared.$"


PrismCaverns_Depths_EventScript_Bruno::
	trainerbattle_single TRAINER_PRISM_BRUNO, PrismCaverns_Depths_Text_BrunoIntro, PrismCaverns_Depths_Text_BrunoDefeat
	msgbox PrismCaverns_Depths_Text_BrunoPost, MSGBOX_AUTOCLOSE
	end

PrismCaverns_Depths_Text_BrunoIntro:
	.string "End of the line, kid! This island\n"
	.string "goes nowhere. But I do battle!$"

PrismCaverns_Depths_Text_BrunoDefeat:
	.string "Nowhere, indeed.$"

PrismCaverns_Depths_Text_BrunoPost:
	.string "Dead end, this. Take the other\n"
	.string "plate from the landing.$"


PrismCaverns_Depths_Text_Seal:
	.string "A wall of pink crystal blocks the\n"
	.string "passage. Four hollows are cut into\l"
	.string "it, each the size of a fist.\p"
	.string "Light shines in the first two…\n"
	.string "but two are still dark.$"

PrismCaverns_Depths_Text_AllFour:
	.string "Ruby, sapphire, emerald, topaz…\n"
	.string "All four crystals sing as one!$"

PrismCaverns_Depths_Text_SealBroke:
	.string "Somewhere nearby, a wall of crystal\n"
	.string "shattered!$"

PrismCaverns_Depths_Text_Sparkle:
	.string "A small POKéMON sits before the\n"
	.string "great heart crystal, glittering\l"
	.string "like a jewel.$"

PrismCaverns_Depths_Text_Diancie:
	.string "Dian!$"

PrismCaverns_Depths_Text_DiancieGone:
	.string "DIANCIE vanished in a flash of pink\n"
	.string "light…$"

PrismCaverns_Depths_Text_DiancieStays:
	.string "DIANCIE is still sitting by the\n"
	.string "heart crystal, sparkling.$"

PrismCaverns_Depths_EventScript_Item_TmReflect::
	finditem ITEM_TM_REFLECT
	end

PrismCaverns_Depths_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

PrismCaverns_Depths_EventScript_Item_ShinyStone::
	finditem ITEM_SHINY_STONE
	end

PrismCaverns_Depths_EventScript_Item_RareCandy::
	finditem ITEM_RARE_CANDY
	end

PrismCaverns_Depths_EventScript_Item_FullRestore::
	finditem ITEM_FULL_RESTORE
	end

