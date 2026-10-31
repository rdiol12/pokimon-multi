Abyssia_Temple_MapScripts::
	.byte 0

Abyssia_Temple_EventScript_Relicanth::
	lock
	faceplayer
	playmoncry SPECIES_RELICANTH, CRY_MODE_ENCOUNTER
	msgbox Abyssia_Temple_Text_Relicanth, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_RELICANTH, 48
	dowildbattle
	setflag FLAG_HIDE_ABYSS_RELICANTH
	setflag FLAG_ABYSS_RELICANTH_DONE
	removeobject VAR_LAST_TALKED
	release
	end

Abyssia_Temple_Text_Relicanth:
	.string "An ancient fish rests before the\n"
	.string "altar. It opens one old eye…$"

Abyssia_Temple_EventScript_Item_HeartScale::
	finditem ITEM_HEART_SCALE
	end

