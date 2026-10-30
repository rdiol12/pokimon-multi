AmberwoodHollow_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, AmberwoodHollow_OnLoad
	.byte 0

AmberwoodHollow_OnLoad:
	call_if_set FLAG_AMBERWOOD_LANTERN_1, AmberwoodHollow_EventScript_SetLantern1
	call_if_set FLAG_AMBERWOOD_LANTERN_2, AmberwoodHollow_EventScript_SetLantern2
	call_if_set FLAG_AMBERWOOD_LANTERN_3, AmberwoodHollow_EventScript_SetLantern3
	call AmberwoodHollow_EventScript_TryOpenShrine
	end

AmberwoodHollow_EventScript_SetLantern1::
	setmetatile 11, 11, 0x289, FALSE
	setmetatile 11, 12, 0x28A, TRUE
	return

AmberwoodHollow_EventScript_SetLantern2::
	setmetatile 33, 8, 0x28B, FALSE
	setmetatile 33, 9, 0x28C, TRUE
	return

AmberwoodHollow_EventScript_SetLantern3::
	setmetatile 13, 21, 0x28D, FALSE
	setmetatile 13, 22, 0x28A, TRUE
	return

AmberwoodHollow_EventScript_TryOpenShrine::
	goto_if_unset FLAG_AMBERWOOD_LANTERN_1, AmberwoodHollow_EventScript_Return
	goto_if_unset FLAG_AMBERWOOD_LANTERN_2, AmberwoodHollow_EventScript_Return
	goto_if_unset FLAG_AMBERWOOD_LANTERN_3, AmberwoodHollow_EventScript_Return
	setmetatile 19, 9, 0x231, FALSE
	setmetatile 20, 9, 0x231, FALSE
	return

AmberwoodHollow_EventScript_Return::
	return

AmberwoodHollow_EventScript_Lantern1::
	lockall
	goto_if_set FLAG_AMBERWOOD_LANTERN_1, AmberwoodHollow_EventScript_LanternAlreadyLit
	msgbox AmberwoodHollow_Text_LanternUnlit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, AmberwoodHollow_EventScript_ReleaseAll
	setflag FLAG_AMBERWOOD_LANTERN_1
	playse SE_M_EMBER
	call AmberwoodHollow_EventScript_SetLantern1
	special DrawWholeMapView
	waitse
	msgbox AmberwoodHollow_Text_LanternNowLit, MSGBOX_DEFAULT
	goto AmberwoodHollow_EventScript_CheckAllLit

AmberwoodHollow_EventScript_Lantern2::
	lockall
	goto_if_set FLAG_AMBERWOOD_LANTERN_2, AmberwoodHollow_EventScript_LanternAlreadyLit
	msgbox AmberwoodHollow_Text_LanternUnlit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, AmberwoodHollow_EventScript_ReleaseAll
	setflag FLAG_AMBERWOOD_LANTERN_2
	playse SE_M_EMBER
	call AmberwoodHollow_EventScript_SetLantern2
	special DrawWholeMapView
	waitse
	msgbox AmberwoodHollow_Text_LanternNowLit, MSGBOX_DEFAULT
	goto AmberwoodHollow_EventScript_CheckAllLit

AmberwoodHollow_EventScript_Lantern3::
	lockall
	goto_if_set FLAG_AMBERWOOD_LANTERN_3, AmberwoodHollow_EventScript_LanternAlreadyLit
	msgbox AmberwoodHollow_Text_LanternUnlit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, AmberwoodHollow_EventScript_ReleaseAll
	setflag FLAG_AMBERWOOD_LANTERN_3
	playse SE_M_EMBER
	call AmberwoodHollow_EventScript_SetLantern3
	special DrawWholeMapView
	waitse
	msgbox AmberwoodHollow_Text_LanternNowLit, MSGBOX_DEFAULT
	goto AmberwoodHollow_EventScript_CheckAllLit

AmberwoodHollow_EventScript_LanternAlreadyLit::
	msgbox AmberwoodHollow_Text_LanternBurning, MSGBOX_DEFAULT
	releaseall
	end

AmberwoodHollow_EventScript_ReleaseAll::
	releaseall
	end

AmberwoodHollow_EventScript_CheckAllLit::
	goto_if_unset FLAG_AMBERWOOD_LANTERN_1, AmberwoodHollow_EventScript_SomeLeft
	goto_if_unset FLAG_AMBERWOOD_LANTERN_2, AmberwoodHollow_EventScript_SomeLeft
	goto_if_unset FLAG_AMBERWOOD_LANTERN_3, AmberwoodHollow_EventScript_SomeLeft
	closemessage
	delay 20
	playse SE_M_DETECT
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	setmetatile 19, 9, 0x231, FALSE
	setmetatile 20, 9, 0x231, FALSE
	special DrawWholeMapView
	msgbox AmberwoodHollow_Text_ShrineOpens, MSGBOX_DEFAULT
	releaseall
	end

AmberwoodHollow_EventScript_SomeLeft::
	msgbox AmberwoodHollow_Text_SomeLeft, MSGBOX_DEFAULT
	releaseall
	end

AmberwoodHollow_EventScript_Barrier::
	msgbox AmberwoodHollow_Text_Barrier, MSGBOX_SIGN
	end

AmberwoodHollow_EventScript_Celebi::
	lock
	faceplayer
	msgbox AmberwoodHollow_Text_CelebiAppears, MSGBOX_DEFAULT
	closemessage
	waitse
	playmoncry SPECIES_CELEBI, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_CELEBI, 50
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	dowildbattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, AmberwoodHollow_EventScript_CelebiDefeated
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, AmberwoodHollow_EventScript_CelebiRan
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, AmberwoodHollow_EventScript_CelebiRan
	setflag FLAG_AMBERWOOD_CELEBI_DONE
	release
	end

AmberwoodHollow_EventScript_CelebiDefeated::
	setflag FLAG_AMBERWOOD_CELEBI_DONE
	goto Common_EventScript_RemoveStaticPokemon
	end

AmberwoodHollow_EventScript_CelebiRan::
	setvar VAR_0x8004, SPECIES_CELEBI
	goto Common_EventScript_LegendaryFlewAway
	end

AmberwoodHollow_EventScript_Boatman::
	lock
	faceplayer
	msgbox AmberwoodHollow_Text_BoatmanBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, AmberwoodHollow_EventScript_BoatmanStay
	closemessage
	warp MAP_STARFALL_ISLE, 28, 51
	waitstate
	release
	end

AmberwoodHollow_EventScript_BoatmanStay::
	msgbox AmberwoodHollow_Text_BoatmanStay, MSGBOX_DEFAULT
	release
	end

AmberwoodHollow_EventScript_Camper::
	lock
	faceplayer
	goto_if_set FLAG_AMBERWOOD_CAMPER_REWARD, AmberwoodHollow_EventScript_CamperDone
	goto_if_set FLAG_AMBERWOOD_CELEBI_DONE, AmberwoodHollow_EventScript_CamperReward
	msgbox AmberwoodHollow_Text_CamperHint, MSGBOX_DEFAULT
	release
	end

AmberwoodHollow_EventScript_CamperReward::
	msgbox AmberwoodHollow_Text_CamperThanks, MSGBOX_DEFAULT
	giveitem ITEM_LEAF_STONE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_AMBERWOOD_CAMPER_REWARD
	release
	end

AmberwoodHollow_EventScript_CamperDone::
	msgbox AmberwoodHollow_Text_CamperDone, MSGBOX_DEFAULT
	release
	end

AmberwoodHollow_EventScript_Hazel::
	trainerbattle_single TRAINER_AMBERWOOD_HAZEL, AmberwoodHollow_Text_HazelIntro, AmberwoodHollow_Text_HazelDefeat
	msgbox AmberwoodHollow_Text_HazelPostBattle, MSGBOX_AUTOCLOSE
	end

AmberwoodHollow_EventScript_Picnicker::
	msgbox AmberwoodHollow_Text_Picnicker, MSGBOX_NPC
	end

AmberwoodHollow_EventScript_Deerling::
	lock
	faceplayer
	playmoncry SPECIES_DEERLING, CRY_MODE_NORMAL
	msgbox AmberwoodHollow_Text_Deerling, MSGBOX_DEFAULT
	waitmoncry
	release
	end

AmberwoodHollow_EventScript_ItemClearing::
	finditem ITEM_ABILITY_PATCH
	end

AmberwoodHollow_EventScript_DockSign::
	msgbox AmberwoodHollow_Text_DockSign, MSGBOX_SIGN
	end

AmberwoodHollow_EventScript_ShrineSign::
	msgbox AmberwoodHollow_Text_ShrineSign, MSGBOX_SIGN
	end

AmberwoodHollow_EventScript_Carving::
	msgbox AmberwoodHollow_Text_Carving, MSGBOX_SIGN
	end

AmberwoodHollow_Text_LanternUnlit:
	.string "An old stone lantern. The wick\nis dry and dusty.\pLight it?$"

AmberwoodHollow_Text_LanternNowLit:
	.string "The lantern flickers to life with a\nwarm, golden glow.$"

AmberwoodHollow_Text_LanternBurning:
	.string "The lantern is burning warmly.$"

AmberwoodHollow_Text_SomeLeft:
	.string "Somewhere in the hollow, another\nlantern is still dark...$"

AmberwoodHollow_Text_ShrineOpens:
	.string "The ground rumbled!\pThe crystals sealing the shrine\nhave melted away!$"

AmberwoodHollow_Text_Barrier:
	.string "Glittering crystals block the way\ninto the shrine.\pThey shimmer faintly, as if waiting\nfor light...$"

AmberwoodHollow_Text_CelebiAppears:
	.string "A small POKéMON is drifting above\nthe shrine's stones...\pIt seems to have come here from\nanother time!$"

AmberwoodHollow_Text_BoatmanBack:
	.string "Sail back to STARFALL ISLE?$"

AmberwoodHollow_Text_BoatmanStay:
	.string "The leaves are lovely this time of\nyear. Enjoy the hollow!$"

AmberwoodHollow_Text_CamperHint:
	.string "Welcome to AMBERWOOD HOLLOW!\nI've camped here all autumn.\pThere are three old lanterns\naround the hollow.\pLegend says if all three burn at\nonce, the shrine will open.\pOne is near my tent... I haven't\nfound the others yet!$"

AmberwoodHollow_Text_CamperThanks:
	.string "You opened the shrine AND met the\nPOKéMON inside?! Incredible!\pHere, a little thank-you from my\ncamping kit.$"

AmberwoodHollow_Text_CamperDone:
	.string "I'll write a song about the night\nthe lanterns were lit.$"

AmberwoodHollow_Text_HazelIntro:
	.string "The spirits of the hollow whisper\nabout you... Let me see why!$"

AmberwoodHollow_Text_HazelDefeat:
	.string "The spirits were right about you...$"

AmberwoodHollow_Text_HazelPostBattle:
	.string "The lantern in the meadow... it\nlooks over the red grass.\pAnd one more burns by the lake,\nif someone lights it.$"

AmberwoodHollow_Text_Picnicker:
	.string "Everything here looks different\nfrom the rest of HOENN.\pThe leaves, the stones, even the\nwater feels new!$"

AmberwoodHollow_Text_Deerling:
	.string "DEERLING: Bwuh!\nIt's nibbling on autumn leaves.$"

AmberwoodHollow_Text_DockSign:
	.string "AMBERWOOD HOLLOW\n“Where every leaf turns to gold.”$"

AmberwoodHollow_Text_ShrineSign:
	.string "OLD SHRINE\nLight the three lanterns, and the\lway shall open.$"

AmberwoodHollow_Text_Carving:
	.string "Words are carved into the stone:\p“This hollow was drawn by hand,\nleaf by leaf, for RON.”$"
