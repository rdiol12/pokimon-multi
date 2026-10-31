@ Bonding: R > BOND, pick a party Pokémon, then pet it or feed it a berry.

Life_EventScript_Bond::
	msgbox Life_Text_BondWho, MSGBOX_DEFAULT
	closemessage
	special ChoosePartyMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, Life_EventScript_BondDone
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, Life_EventScript_BondEgg
	fadescreen FADE_TO_BLACK
	callnative LifeBond_Open
	waitstate
Life_EventScript_BondDone:
	releaseall
	end

Life_EventScript_BondEgg:
	msgbox Life_Text_BondEgg, MSGBOX_DEFAULT
	releaseall
	end

Life_Text_MenuBond:
	.string "BOND$"

Life_Text_BondWho:
	.string "Which Pokémon do you want to spend\n"
	.string "some time with?$"

Life_Text_BondEgg:
	.string "The EGG is warm. It seems happy\n"
	.string "just being close to you.$"
