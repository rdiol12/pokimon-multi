SnowdriftVillage_WestCabin_MapScripts::
	.byte 0

SnowdriftVillage_WestCabin_EventScript_Granny::
	msgbox SnowdriftVillage_WestCabin_Text_Granny, MSGBOX_NPC
	end

SnowdriftVillage_WestCabin_EventScript_Snom::
	lock
	faceplayer
	playmoncry SPECIES_SNOM, CRY_MODE_NORMAL
	msgbox SnowdriftVillage_WestCabin_Text_Snom, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SnowdriftVillage_WestCabin_Text_Granny:
	.string "When I was a girl, we sledded to\n"
	.string "the bottom of the mountain and\l"
	.string "walked all the way back up!\p"
	.string "Young folk today have it easy.$"

SnowdriftVillage_WestCabin_Text_Snom:
	.string "SNOM is nibbling on an icicle.$"
