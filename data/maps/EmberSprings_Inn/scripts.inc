EmberSprings_Inn_MapScripts::
	.byte 0

EmberSprings_Inn_EventScript_Innkeeper::
	lock
	faceplayer
	msgbox EmberSprings_Inn_Text_Rest, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EmberSprings_Inn_EventScript_NoRest
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	msgbox EmberSprings_Inn_Text_Rested, MSGBOX_DEFAULT
EmberSprings_Inn_EventScript_NoRest::
	release
	end

EmberSprings_Inn_EventScript_Espurr::
	lock
	faceplayer
	playmoncry SPECIES_ESPURR, CRY_MODE_NORMAL
	msgbox EmberSprings_Inn_Text_Espurr, MSGBOX_DEFAULT
	waitmoncry
	release
	end

EmberSprings_Inn_Text_Rest:
	.string "Welcome to the EMBER INN!\n"
	.string "Would you like to rest a while?$"

EmberSprings_Inn_Text_Rested:
	.string "Good morning! Your POKéMON are\n"
	.string "full of energy.$"

EmberSprings_Inn_Text_Espurr:
	.string "ESPURR: Mew?$"
