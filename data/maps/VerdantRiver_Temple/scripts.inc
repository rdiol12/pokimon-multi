VerdantRiver_Temple_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VerdantRiver_Temple_OnTransition
	.byte 0

VerdantRiver_Temple_OnTransition:
	setflag FLAG_VERDANT_TEMPLE_REACHED
	end

VerdantRiver_Temple_EventScript_Keldeo::
	lock
	faceplayer
	msgbox VerdantRiver_Temple_Text_Sees, MSGBOX_DEFAULT
	setwildbattle SPECIES_KELDEO, 40
	playmoncry SPECIES_KELDEO, CRY_MODE_ENCOUNTER
	msgbox VerdantRiver_Temple_Text_Keldeo, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, VerdantRiver_Temple_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, VerdantRiver_Temple_EventScript_Stays
	setflag FLAG_VERDANT_KELDEO_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, VerdantRiver_Temple_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox VerdantRiver_Temple_Text_Gone, MSGBOX_DEFAULT
VerdantRiver_Temple_EventScript_End::
	release
	end

VerdantRiver_Temple_EventScript_Stays::
	msgbox VerdantRiver_Temple_Text_Stays, MSGBOX_DEFAULT
	release
	end

VerdantRiver_Temple_Text_Sees:
	.string "A young POKéMON stands proudly in\n"
	.string "the temple, its horn shining like\l"
	.string "a blade of water.$"

VerdantRiver_Temple_Text_Keldeo:
	.string "Kyuuun!$"

VerdantRiver_Temple_Text_Gone:
	.string "KELDEO bounded away across the lake,\n"
	.string "running on the water.$"

VerdantRiver_Temple_Text_Stays:
	.string "KELDEO is still standing guard in\n"
	.string "the temple.$"

VerdantRiver_Temple_EventScript_Item_TmCalmMind::
	finditem ITEM_TM_CALM_MIND
	end

VerdantRiver_Temple_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

