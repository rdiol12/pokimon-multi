IronmoorPrison_Solitary_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, IronmoorPrison_Solitary_OnLoad
	.byte 0

IronmoorPrison_Solitary_OnLoad:
	call_if_set FLAG_IRONMOOR_SPIRITOMB_DONE, IronmoorPrison_Solitary_EventScript_Cracked
	end

IronmoorPrison_Solitary_EventScript_Cracked::
	setmetatile 21, 10, 616, TRUE
	return

IronmoorPrison_Solitary_EventScript_Keystone::
	lockall
	goto_if_set FLAG_IRONMOOR_SPIRITOMB_DONE, IronmoorPrison_Solitary_EventScript_KeystoneEmpty
	goto_if_unset FLAG_IRONMOOR_SPIRIT_1, IronmoorPrison_Solitary_EventScript_KeystoneCold
	goto_if_unset FLAG_IRONMOOR_SPIRIT_2, IronmoorPrison_Solitary_EventScript_KeystoneCold
	goto_if_unset FLAG_IRONMOOR_SPIRIT_3, IronmoorPrison_Solitary_EventScript_KeystoneCold
	goto_if_unset FLAG_IRONMOOR_SPIRIT_4, IronmoorPrison_Solitary_EventScript_KeystoneCold
	msgbox IronmoorPrison_Solitary_Text_KeystoneWakes, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 10
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	call IronmoorPrison_Solitary_EventScript_Cracked
	special DrawWholeMapView
	setwildbattle SPECIES_SPIRITOMB, 32
	playmoncry SPECIES_SPIRITOMB, CRY_MODE_ENCOUNTER
	msgbox IronmoorPrison_Solitary_Text_Spiritomb, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, IronmoorPrison_Solitary_EventScript_Lingers
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, IronmoorPrison_Solitary_EventScript_Lingers
	setflag FLAG_IRONMOOR_SPIRITOMB_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, IronmoorPrison_Solitary_EventScript_End
	msgbox IronmoorPrison_Solitary_Text_Gone, MSGBOX_DEFAULT
IronmoorPrison_Solitary_EventScript_End::
	releaseall
	end

IronmoorPrison_Solitary_EventScript_Lingers::
	msgbox IronmoorPrison_Solitary_Text_Lingers, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_Solitary_EventScript_KeystoneCold::
	msgbox IronmoorPrison_Solitary_Text_KeystoneCold, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_Solitary_EventScript_KeystoneEmpty::
	msgbox IronmoorPrison_Solitary_Text_KeystoneEmpty, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_Solitary_EventScript_Selma::
	trainerbattle_single TRAINER_IRONMOOR_SELMA, IronmoorPrison_Solitary_Text_SelmaIntro, IronmoorPrison_Solitary_Text_SelmaDefeat
	msgbox IronmoorPrison_Solitary_Text_SelmaPost, MSGBOX_AUTOCLOSE
	end

IronmoorPrison_Solitary_Text_SelmaIntro:
	.string "Shh… Can you hear it? The stone at\n"
	.string "the end of the hall is whispering.$"

IronmoorPrison_Solitary_Text_SelmaDefeat:
	.string "The whispers stopped…$"

IronmoorPrison_Solitary_Text_SelmaPost:
	.string "The stone wants its spirits back.\n"
	.string "They're bound up in the cells above.$"


IronmoorPrison_Solitary_Text_KeystoneCold:
	.string "An odd stone sits on a plinth, cold\n"
	.string "and silent. A face seems to stare\l"
	.string "out of it.\p"
	.string "Something is missing from it…$"

IronmoorPrison_Solitary_Text_KeystoneWakes:
	.string "Blue flames swirled down the hall\n"
	.string "and poured into the stone!\p"
	.string "The keystone is cracking!$"

IronmoorPrison_Solitary_Text_Spiritomb:
	.string "Hehehe…$"

IronmoorPrison_Solitary_Text_Gone:
	.string "SPIRITOMB scattered into the dark,\n"
	.string "laughing.$"

IronmoorPrison_Solitary_Text_Lingers:
	.string "Spirits still swirl round the cracked\n"
	.string "keystone…$"

IronmoorPrison_Solitary_Text_KeystoneEmpty:
	.string "The keystone is split in two. It's\n"
	.string "quiet now.$"

IronmoorPrison_Solitary_EventScript_Item_TmShadowBall::
	finditem ITEM_TM_SHADOW_BALL
	end

