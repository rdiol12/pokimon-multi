StarfallIsle_EldersHouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, StarfallIsle_EldersHouse_OnTransition
	.byte 0

StarfallIsle_EldersHouse_OnTransition:
	call_if_unset FLAG_STARFALL_SKWOVET_FOUND, StarfallIsle_EldersHouse_EventScript_HideSkwovet
	call_if_set FLAG_STARFALL_SKWOVET_FOUND, StarfallIsle_EldersHouse_EventScript_ShowSkwovet
	end

StarfallIsle_EldersHouse_EventScript_HideSkwovet::
	setflag FLAG_HIDE_STARFALL_SKWOVET_AT_HOME
	return

StarfallIsle_EldersHouse_EventScript_ShowSkwovet::
	clearflag FLAG_HIDE_STARFALL_SKWOVET_AT_HOME
	return

StarfallIsle_EldersHouse_EventScript_Elder::
	lock
	faceplayer
	goto_if_set FLAG_STARFALL_SKWOVET_REWARD, StarfallIsle_EldersHouse_EventScript_AfterReward
	goto_if_set FLAG_STARFALL_SKWOVET_FOUND, StarfallIsle_EldersHouse_EventScript_Reward
	msgbox StarfallIsle_EldersHouse_Text_PleaseFind, MSGBOX_DEFAULT
	release
	end

StarfallIsle_EldersHouse_EventScript_Reward::
	msgbox StarfallIsle_EldersHouse_Text_ThankYou, MSGBOX_DEFAULT
	giveitem ITEM_RARE_CANDY, 3
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_STARFALL_SKWOVET_REWARD
	msgbox StarfallIsle_EldersHouse_Text_AfterReward, MSGBOX_DEFAULT
	release
	end

StarfallIsle_EldersHouse_EventScript_AfterReward::
	msgbox StarfallIsle_EldersHouse_Text_AfterReward, MSGBOX_DEFAULT
	release
	end

StarfallIsle_EldersHouse_EventScript_Skwovet::
	lock
	faceplayer
	playmoncry SPECIES_SKWOVET, CRY_MODE_NORMAL
	msgbox StarfallIsle_EldersHouse_Text_Skwovet, MSGBOX_DEFAULT
	waitmoncry
	release
	end

StarfallIsle_EldersHouse_Text_PleaseFind:
	.string "Oh, a visitor... Welcome to\nSTARFALL ISLE.\pForgive me, I'm worried sick. My\nSKWOVET ran off this morning.\pIt loves the tall grass in the\nmeadow east of the village.\pCould you find it and bring it\nhome?$"

StarfallIsle_EldersHouse_Text_ThankYou:
	.string "My SKWOVET came running home!\nThank you, dear. Thank you!\pPlease take these. They're the\nisland's sweetest treats.$"

StarfallIsle_EldersHouse_Text_AfterReward:
	.string "When I was young, a star fell into\nthe mountain.\pThey say whoever finds it may\nmake a wish...$"

StarfallIsle_EldersHouse_Text_Skwovet:
	.string "SKWOVET: Skwoooo!\nIts cheeks are stuffed with BERRIES.$"
