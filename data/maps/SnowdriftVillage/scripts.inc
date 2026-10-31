SnowdriftVillage_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SnowdriftVillage_OnLoad
	.byte 0

SnowdriftVillage_OnLoad:
	call_if_set FLAG_FROST_RESCUE_DONE, SnowdriftVillage_EventScript_ClearDrift
	end

SnowdriftVillage_EventScript_ClearDrift::
	setmetatile 26, 8, 577, FALSE
	setmetatile 27, 8, 577, FALSE
	return

SnowdriftVillage_EventScript_Digger::
	lock
	faceplayer
	goto_if_set FLAG_FROST_RESCUE_DONE, SnowdriftVillage_EventScript_DiggerDone
	msgbox SnowdriftVillage_Text_Digger, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_EventScript_DiggerDone::
	msgbox SnowdriftVillage_Text_DiggerDone, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_EventScript_Articuno::
	lock
	faceplayer
	msgbox SnowdriftVillage_Text_Chill, MSGBOX_DEFAULT
	setwildbattle SPECIES_ARTICUNO, 45
	playmoncry SPECIES_ARTICUNO, CRY_MODE_ENCOUNTER
	msgbox SnowdriftVillage_Text_Articuno, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, SnowdriftVillage_EventScript_ArticunoStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, SnowdriftVillage_EventScript_ArticunoStays
	setflag FLAG_FROST_ARTICUNO_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, SnowdriftVillage_EventScript_ArticunoEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox SnowdriftVillage_Text_ArticunoGone, MSGBOX_DEFAULT
SnowdriftVillage_EventScript_ArticunoEnd::
	release
	end

SnowdriftVillage_EventScript_ArticunoStays::
	msgbox SnowdriftVillage_Text_ArticunoStays, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_EventScript_SledKid::
	msgbox SnowdriftVillage_Text_SledKid, MSGBOX_NPC
	end

SnowdriftVillage_EventScript_Woman::
	msgbox SnowdriftVillage_Text_Woman, MSGBOX_NPC
	end

SnowdriftVillage_EventScript_Cubchoo::
	lock
	faceplayer
	playmoncry SPECIES_CUBCHOO, CRY_MODE_NORMAL
	msgbox SnowdriftVillage_Text_Cubchoo, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SnowdriftVillage_EventScript_Delibird::
	lock
	faceplayer
	playmoncry SPECIES_DELIBIRD, CRY_MODE_NORMAL
	msgbox SnowdriftVillage_Text_Delibird, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SnowdriftVillage_EventScript_Sign::
	msgbox SnowdriftVillage_Text_Sign, MSGBOX_SIGN
	end

SnowdriftVillage_EventScript_SledSign::
	msgbox SnowdriftVillage_Text_SledSign, MSGBOX_SIGN
	end

SnowdriftVillage_Text_Digger:
	.string "The blizzard buried the summit path\n"
	.string "under a mountain of snow.\p"
	.string "MOUNTAIN RESCUE has every shovel\n"
	.string "out looking for the lost climbers.\l"
	.string "Once they're safe, we'll dig.$"

SnowdriftVillage_Text_DiggerDone:
	.string "All dug out! The summit path is\n"
	.string "open. Mind the wind up there.$"

SnowdriftVillage_Text_Chill:
	.string "The air turns bitterly cold…$"

SnowdriftVillage_Text_Articuno:
	.string "Kyaaaah!$"

SnowdriftVillage_Text_ArticunoGone:
	.string "ARTICUNO flew off into the snow.$"

SnowdriftVillage_Text_ArticunoStays:
	.string "ARTICUNO is still perched by the\n"
	.string "cairn.$"

SnowdriftVillage_Text_SledKid:
	.string "Wanna race? Step on the run and\n"
	.string "whoosh! All the way to the bottom\l"
	.string "of the mountain!$"

SnowdriftVillage_Text_Woman:
	.string "Welcome to SNOWDRIFT VILLAGE!\n"
	.string "Warm up by the fire.$"

SnowdriftVillage_Text_Cubchoo:
	.string "CUBCHOO sniffled. Its nose is\n"
	.string "always running!$"

SnowdriftVillage_Text_Delibird:
	.string "DELIBIRD is guarding a sack of\n"
	.string "something. Presents, maybe?$"

SnowdriftVillage_Text_Sign:
	.string "SNOWDRIFT VILLAGE\n"
	.string "Warm hearts on a cold mountain.$"

SnowdriftVillage_Text_SledSign:
	.string "SLED RUN\n"
	.string "To the ranger's camp. One way only!$"

SnowdriftVillage_EventScript_Item_IceStone::
	finditem ITEM_ICE_STONE
	end

