Hollowmere_Inn_MapScripts::
	.byte 0

Hollowmere_Inn_EventScript_Keeper::
	lock
	faceplayer
	msgbox Hollowmere_Inn_Text_Rest, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Hollowmere_Inn_EventScript_No
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox Hollowmere_Inn_Text_Rested, MSGBOX_DEFAULT
	release
	end

Hollowmere_Inn_EventScript_No::
	msgbox Hollowmere_Inn_Text_No, MSGBOX_DEFAULT
	release
	end

Hollowmere_Inn_Text_Rest:
	.string "Welcome to the last inn in\n"
	.string "HOLLOWMERE. Will you rest a while?$"

Hollowmere_Inn_Text_Rested:
	.string "You slept soundly. Odd, the bed\n"
	.string "was cold the whole time…$"

Hollowmere_Inn_Text_No:
	.string "The bed will be here. It always is.$"
