MetroSlateport_MapScripts::
	.byte 0

MetroSlateport_EventScript_Train::
	lockall
	message Metro_Text_WhereTo
	waitmessage
	dynmultipush Metro_Text_Mauville, 0
	dynmultipush Metro_Text_Verdanturf, 1
	dynmultipush Metro_Text_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, NULL
	closemessage
	goto_if_eq VAR_RESULT, 0, Metro_EventScript_RideToMauville
	goto_if_eq VAR_RESULT, 1, Metro_EventScript_RideToVerdanturf
	releaseall
	end

MetroSlateport_EventScript_Stamp::
	lockall
	goto_if_set FLAG_METRO_STAMP_2, Metro_EventScript_Stamped
	setflag FLAG_METRO_STAMP_2
	playse SE_BANG
	msgbox MetroSlateport_Text_Stamp, MSGBOX_DEFAULT
	releaseall
	end

MetroSlateport_EventScript_Board::
	msgbox MetroSlateport_Text_Board, MSGBOX_SIGN
	end

MetroSlateport_Text_Stamp:
	.string "{PLAYER} stamped the rally card!\n"
	.string "It's the SLATEPORT STATION stamp.$"

MetroSlateport_Text_Board:
	.string "HOENN METRO\n"
	.string "SLATEPORT STATION$"

MetroSlateport_EventScript_Sailor::
	msgbox MetroSlateport_Text_Sailor, MSGBOX_NPC
	end

MetroSlateport_EventScript_Woman::
	msgbox MetroSlateport_Text_Woman, MSGBOX_NPC
	end

MetroSlateport_EventScript_Strum::
	trainerbattle_single TRAINER_METRO_STRUM, MetroSlateport_Text_StrumIntro, MetroSlateport_Text_StrumDefeat
	msgbox MetroSlateport_Text_StrumPost, MSGBOX_AUTOCLOSE
	end

MetroSlateport_Text_Sailor:
	.string "Ten minutes from the harbour to\n"
	.string "MAUVILLE. The sea was never this\l"
	.string "quick!$"

MetroSlateport_Text_Woman:
	.string "The stamp stand's by the platform.\n"
	.string "The kids love it.$"

MetroSlateport_Text_StrumIntro:
	.string "The acoustics down here are\n"
	.string "electric! Battle me to the beat!$"

MetroSlateport_Text_StrumDefeat:
	.string "You broke a string…$"

MetroSlateport_Text_StrumPost:
	.string "Busking pays better on the METRO.\n"
	.string "Everyone's waiting for a train.$"

