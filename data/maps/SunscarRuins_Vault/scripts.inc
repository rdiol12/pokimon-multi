SunscarRuins_Vault_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SunscarRuins_Vault_OnTransition
	.byte 0

SunscarRuins_Vault_OnTransition:
	call_if_set FLAG_SUNSCAR_SPIRITOMB_DONE, SunscarRuins_Vault_EventScript_Hide
	end

SunscarRuins_Vault_EventScript_Hide::
	setflag FLAG_HIDE_SUNSCAR_SPIRITOMB
	return

SunscarRuins_Vault_EventScript_Spiritomb::
	lock
	faceplayer
	msgbox SunscarRuins_Vault_Text_Spirits, MSGBOX_DEFAULT
	setwildbattle SPECIES_SPIRITOMB, 40
	playmoncry SPECIES_SPIRITOMB, CRY_MODE_ENCOUNTER
	msgbox SunscarRuins_Vault_Text_Spiritomb, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, SunscarRuins_Vault_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, SunscarRuins_Vault_EventScript_Stays
	setflag FLAG_SUNSCAR_SPIRITOMB_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, SunscarRuins_Vault_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_SUNSCAR_SPIRITOMB
	setflag FLAG_HIDE_SUNSCAR_SPIRITOMB
	fadescreen FADE_FROM_BLACK
	msgbox SunscarRuins_Vault_Text_Faded, MSGBOX_DEFAULT
SunscarRuins_Vault_EventScript_End::
	release
	end

SunscarRuins_Vault_EventScript_Stays::
	msgbox SunscarRuins_Vault_Text_Stays, MSGBOX_DEFAULT
	release
	end

SunscarRuins_Vault_Text_Spirits:
	.string "Dozens of faint voices whisper from\n"
	.string "the old stone…$"

SunscarRuins_Vault_Text_Spiritomb:
	.string "Kuhehehe!$"

SunscarRuins_Vault_Text_Faded:
	.string "SPIRITOMB faded back into the\n"
	.string "stone.$"

SunscarRuins_Vault_Text_Stays:
	.string "The voices are still whispering.$"

SunscarRuins_Vault_EventScript_Item_OddKeystone::
	finditem ITEM_ODD_KEYSTONE
	end

SunscarRuins_Vault_EventScript_Item_DuskBall::
	finditem ITEM_DUSK_BALL
	end

SunscarRuins_Vault_EventScript_Item_RelicGold::
	finditem ITEM_RELIC_GOLD
	end

