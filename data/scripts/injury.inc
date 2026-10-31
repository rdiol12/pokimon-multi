@ Pokémon injuries (src/injury.c).

@ Runs on the first step after an injury got worse, a part was lost or a scar appeared.
Injury_EventScript_Notify::
	lockall
	playse SE_NOT_EFFECTIVE
	message gStringVar4
	waitmessage
	waitbuttonpress
	closemessage
	releaseall
	end

@ Pokémon Center clinic. Called by the nurse after healing the party.
Injury_EventScript_Clinic::
	callnative Injury_ScriptPartyHasInjury
	goto_if_eq VAR_RESULT, FALSE, Injury_EventScript_ClinicEnd
	showmoneybox 0, 0
	setvar VAR_0x8005, 0
Injury_EventScript_ClinicLoop::
	callnative Injury_ClinicFindNext
	goto_if_eq VAR_RESULT, FALSE, Injury_EventScript_ClinicDone
	msgbox gStringVar4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Injury_EventScript_ClinicNext
	callnative Injury_ClinicTreat
	goto_if_eq VAR_RESULT, FALSE, Injury_EventScript_ClinicNoMoney
	playse SE_SHOP
	message gStringVar4
	waitmessage
	updatemoneybox
	waitbuttonpress
	goto Injury_EventScript_ClinicNext
Injury_EventScript_ClinicNoMoney::
	msgbox gStringVar4, MSGBOX_DEFAULT
Injury_EventScript_ClinicNext::
	addvar VAR_0x8005, 1
	goto Injury_EventScript_ClinicLoop
Injury_EventScript_ClinicDone::
	hidemoneybox
Injury_EventScript_ClinicEnd::
	return
