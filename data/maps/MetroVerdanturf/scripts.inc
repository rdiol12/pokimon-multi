MetroVerdanturf_MapScripts::
	.byte 0

MetroVerdanturf_EventScript_Train::
	lockall
	message Metro_Text_WhereTo
	waitmessage
	dynmultipush Metro_Text_Mauville, 0
	dynmultipush Metro_Text_Slateport, 1
	dynmultipush Metro_Text_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, NULL
	closemessage
	goto_if_eq VAR_RESULT, 0, Metro_EventScript_RideToMauville
	goto_if_eq VAR_RESULT, 1, Metro_EventScript_RideToSlateport
	releaseall
	end

MetroVerdanturf_EventScript_Stamp::
	lockall
	goto_if_set FLAG_METRO_STAMP_3, Metro_EventScript_Stamped
	setflag FLAG_METRO_STAMP_3
	playse SE_BANG
	msgbox MetroVerdanturf_Text_Stamp, MSGBOX_DEFAULT
	releaseall
	end

MetroVerdanturf_EventScript_Board::
	msgbox MetroVerdanturf_Text_Board, MSGBOX_SIGN
	end

MetroVerdanturf_Text_Stamp:
	.string "{PLAYER} stamped the rally card!\n"
	.string "It's the VERDANTURF STATION stamp.$"

MetroVerdanturf_Text_Board:
	.string "HOENN METRO\n"
	.string "VERDANTURF STATION$"

MetroVerdanturf_EventScript_Granny::
	msgbox MetroVerdanturf_Text_Granny, MSGBOX_NPC
	end

MetroVerdanturf_EventScript_Skitty::
	lock
	faceplayer
	playmoncry SPECIES_SKITTY, CRY_MODE_NORMAL
	msgbox MetroVerdanturf_Text_Skitty, MSGBOX_DEFAULT
	waitmoncry
	release
	end

MetroVerdanturf_Text_Granny:
	.string "The air's so clean in VERDANTURF,\n"
	.string "they built the station deep so the\l"
	.string "trains wouldn't spoil it.$"

MetroVerdanturf_Text_Skitty:
	.string "SKITTY is chasing its tail around\n"
	.string "the bench.$"

