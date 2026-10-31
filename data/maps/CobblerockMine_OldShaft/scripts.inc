CobblerockMine_OldShaft_MapScripts::
	.byte 0

CobblerockMine_OldShaft_EventScript_Carbink::
	lock
	faceplayer
	msgbox CobblerockMine_OldShaft_Text_Glitter, MSGBOX_DEFAULT
	setwildbattle SPECIES_CARBINK, 18
	playmoncry SPECIES_CARBINK, CRY_MODE_ENCOUNTER
	msgbox CobblerockMine_OldShaft_Text_Carbink, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, CobblerockMine_OldShaft_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, CobblerockMine_OldShaft_EventScript_Stays
	setflag FLAG_COBBLE_CARBINK_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, CobblerockMine_OldShaft_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox CobblerockMine_OldShaft_Text_Gone, MSGBOX_DEFAULT
CobblerockMine_OldShaft_EventScript_End::
	release
	end

CobblerockMine_OldShaft_EventScript_Stays::
	msgbox CobblerockMine_OldShaft_Text_Stays, MSGBOX_DEFAULT
	release
	end

CobblerockMine_OldShaft_Text_Glitter:
	.string "Something glitters among the rocks…\p"
	.string "It's a POKéMON made of jewels!$"

CobblerockMine_OldShaft_Text_Carbink:
	.string "Kiii-iin!$"

CobblerockMine_OldShaft_Text_Gone:
	.string "CARBINK burrowed back into the rock.$"

CobblerockMine_OldShaft_Text_Stays:
	.string "CARBINK is still glittering quietly\n"
	.string "in the dark.$"

CobblerockMine_OldShaft_EventScript_Item_Nugget::
	finditem ITEM_NUGGET
	end

