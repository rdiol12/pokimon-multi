CinderCaverns_Heart_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CinderCaverns_Heart_OnTransition
	.byte 0

CinderCaverns_Heart_OnTransition:
	call_if_set FLAG_CINDER_HEATRAN_DONE, CinderCaverns_Heart_EventScript_Hide
	end

CinderCaverns_Heart_EventScript_Hide::
	setflag FLAG_HIDE_CINDER_HEATRAN
	return

CinderCaverns_Heart_EventScript_Heatran::
	lock
	faceplayer
	setwildbattle SPECIES_HEATRAN, 50
	playmoncry SPECIES_HEATRAN, CRY_MODE_ENCOUNTER
	msgbox CinderCaverns_Heart_Text_Heatran, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, CinderCaverns_Heart_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, CinderCaverns_Heart_EventScript_Stays
	setflag FLAG_CINDER_HEATRAN_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, CinderCaverns_Heart_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_CINDER_HEATRAN
	setflag FLAG_HIDE_CINDER_HEATRAN
	fadescreen FADE_FROM_BLACK
	msgbox CinderCaverns_Heart_Text_Sank, MSGBOX_DEFAULT
CinderCaverns_Heart_EventScript_End::
	release
	end

CinderCaverns_Heart_EventScript_Stays::
	msgbox CinderCaverns_Heart_Text_Stays, MSGBOX_DEFAULT
	release
	end

CinderCaverns_Heart_Text_Heatran:
	.string "Gyuuoooh!$"

CinderCaverns_Heart_Text_Sank:
	.string "HEATRAN sank back into the magma.$"

CinderCaverns_Heart_Text_Stays:
	.string "HEATRAN is glaring from its island.$"

