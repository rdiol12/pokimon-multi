Hollowmere_Manor_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Hollowmere_Manor_2F_OnTransition
	.byte 0

Hollowmere_Manor_2F_OnTransition:
	call_if_set FLAG_HOLLOW_MIMIKYU_DONE, Hollowmere_Manor_2F_EventScript_Hide
	end

Hollowmere_Manor_2F_EventScript_Hide::
	setflag FLAG_HIDE_HOLLOW_MIMIKYU
	return

Hollowmere_Manor_2F_EventScript_Mimikyu::
	lock
	faceplayer
	msgbox Hollowmere_Manor_2F_Text_Doll, MSGBOX_DEFAULT
	setwildbattle SPECIES_MIMIKYU, 40
	playmoncry SPECIES_MIMIKYU, CRY_MODE_ENCOUNTER
	msgbox Hollowmere_Manor_2F_Text_Mimikyu, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, Hollowmere_Manor_2F_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, Hollowmere_Manor_2F_EventScript_Stays
	setflag FLAG_HOLLOW_MIMIKYU_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Hollowmere_Manor_2F_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_HOLLOW_MIMIKYU
	setflag FLAG_HIDE_HOLLOW_MIMIKYU
	fadescreen FADE_FROM_BLACK
	msgbox Hollowmere_Manor_2F_Text_Gone, MSGBOX_DEFAULT
Hollowmere_Manor_2F_EventScript_End::
	release
	end

Hollowmere_Manor_2F_EventScript_Stays::
	msgbox Hollowmere_Manor_2F_Text_Stays, MSGBOX_DEFAULT
	release
	end

Hollowmere_Manor_2F_EventScript_Elvira::
	trainerbattle_single TRAINER_HOLLOW_ELVIRA, Hollowmere_Manor_2F_Text_ElviraIntro, Hollowmere_Manor_2F_Text_ElviraDefeat
	msgbox Hollowmere_Manor_2F_Text_ElviraPost, MSGBOX_AUTOCLOSE
	end

Hollowmere_Manor_2F_EventScript_ReaperCloth::
	finditem ITEM_REAPER_CLOTH
	end

Hollowmere_Manor_2F_EventScript_Case::
	msgbox Hollowmere_Manor_2F_Text_Case, MSGBOX_SIGN
	end

Hollowmere_Manor_2F_Text_Doll:
	.string "A doll in a ragged PIKACHU costume\n"
	.string "sits alone… It just moved!$"

Hollowmere_Manor_2F_Text_Mimikyu:
	.string "Mimikyu!$"

Hollowmere_Manor_2F_Text_Gone:
	.string "MIMIKYU slipped away into the\n"
	.string "shadows.$"

Hollowmere_Manor_2F_Text_Stays:
	.string "The doll is sitting very still\n"
	.string "again.$"

Hollowmere_Manor_2F_Text_Case:
	.string "A glass case full of old dolls.\n"
	.string "One of the spaces is empty.$"

Hollowmere_Manor_2F_Text_ElviraIntro:
	.string "You woke the master's dolls! Now\n"
	.string "you must play with us!$"

Hollowmere_Manor_2F_Text_ElviraDefeat:
	.string "Playtime is over…$"

Hollowmere_Manor_2F_Text_ElviraPost:
	.string "The master was lonely. That's why\n"
	.string "he kept so many dolls.$"
