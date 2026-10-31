MetroMauville_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, MetroMauville_OnLoad
	.byte 0

MetroMauville_OnLoad:
	call_if_set FLAG_METRO_OLD_LINE_OPEN, MetroMauville_EventScript_OpenShutter
	end

MetroMauville_EventScript_OpenShutter::
	setmetatile 34, 9, 855, FALSE
	setmetatile 35, 9, 855, FALSE
	setmetatile 34, 10, 855, FALSE
	setmetatile 35, 10, 855, FALSE
	return

MetroMauville_EventScript_Train::
	lockall
	message Metro_Text_WhereTo
	waitmessage
	dynmultipush Metro_Text_Slateport, 0
	dynmultipush Metro_Text_Verdanturf, 1
	dynmultipush Metro_Text_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, NULL
	closemessage
	goto_if_eq VAR_RESULT, 0, Metro_EventScript_RideToSlateport
	goto_if_eq VAR_RESULT, 1, Metro_EventScript_RideToVerdanturf
	releaseall
	end

MetroMauville_EventScript_Stamp::
	lockall
	goto_if_set FLAG_METRO_STAMP_1, Metro_EventScript_Stamped
	setflag FLAG_METRO_STAMP_1
	playse SE_BANG
	msgbox MetroMauville_Text_Stamp, MSGBOX_DEFAULT
	releaseall
	end

MetroMauville_EventScript_Board::
	msgbox MetroMauville_Text_Board, MSGBOX_SIGN
	end

MetroMauville_Text_Stamp:
	.string "{PLAYER} stamped the rally card!\n"
	.string "It's the MAUVILLE STATION stamp.$"

MetroMauville_Text_Board:
	.string "HOENN METRO\n"
	.string "MAUVILLE STATION$"

MetroMauville_EventScript_Master::
	lock
	faceplayer
	goto_if_set FLAG_METRO_RALLY_DONE, MetroMauville_EventScript_MasterDone
	goto_if_set FLAG_METRO_STAMP_4, MetroMauville_EventScript_MasterReward
	goto_if_set FLAG_METRO_OLD_LINE_OPEN, MetroMauville_EventScript_MasterOldLine
	goto_if_unset FLAG_METRO_STAMP_1, MetroMauville_EventScript_MasterAsk
	goto_if_unset FLAG_METRO_STAMP_2, MetroMauville_EventScript_MasterAsk
	goto_if_unset FLAG_METRO_STAMP_3, MetroMauville_EventScript_MasterAsk
	msgbox MetroMauville_Text_MasterThree, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_METRO_OLD_LINE_OPEN
	playse SE_TRUCK_DOOR
	call MetroMauville_EventScript_OpenShutter
	special DrawWholeMapView
	waitse
	msgbox MetroMauville_Text_MasterOldLine, MSGBOX_DEFAULT
	release
	end

MetroMauville_EventScript_MasterAsk::
	msgbox MetroMauville_Text_MasterAsk, MSGBOX_DEFAULT
	release
	end

MetroMauville_EventScript_MasterOldLine::
	msgbox MetroMauville_Text_MasterOldLine, MSGBOX_DEFAULT
	release
	end

MetroMauville_EventScript_MasterReward::
	msgbox MetroMauville_Text_MasterFour, MSGBOX_DEFAULT
	givemon SPECIES_KLINK, 25
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, MetroMauville_EventScript_MasterFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox MetroMauville_Text_GotKlink, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_METRO_RALLY_DONE
	msgbox MetroMauville_Text_MasterDone, MSGBOX_DEFAULT
	release
	end

MetroMauville_EventScript_MasterFull::
	msgbox MetroMauville_Text_MasterFull, MSGBOX_DEFAULT
	release
	end

MetroMauville_EventScript_MasterDone::
	msgbox MetroMauville_Text_MasterDone, MSGBOX_DEFAULT
	release
	end

MetroMauville_EventScript_Commuter::
	msgbox MetroMauville_Text_Commuter, MSGBOX_NPC
	end

MetroMauville_EventScript_Kid::
	msgbox MetroMauville_Text_Kid, MSGBOX_NPC
	end

MetroMauville_Text_MasterAsk:
	.string "I'm the station master here at\n"
	.string "MAUVILLE CENTRAL.\p"
	.string "Fancy a stamp rally? Every station\n"
	.string "has a stamp stand on its platform:\l"
	.string "MAUVILLE, SLATEPORT and VERDANTURF.\p"
	.string "Fill your card and come and see me.\n"
	.string "There's something special in it.$"

MetroMauville_Text_MasterThree:
	.string "All three stamps! Well done.\p"
	.string "Here's the special part. There used\n"
	.string "to be a fourth station, on the old\l"
	.string "line. It closed long ago.\p"
	.string "Its stamp stand is still down there.\n"
	.string "I'll open the shutter for you.$"

MetroMauville_Text_MasterOldLine:
	.string "The old line runs east, past the\n"
	.string "shutter. Watch your step, and…\l"
	.string "they say the last station isn't\l"
	.string "as empty as it should be.$"

MetroMauville_Text_MasterFour:
	.string "The GHOST STATION's stamp! I\n"
	.string "haven't seen that one in years.\p"
	.string "A full card deserves a prize. This\n"
	.string "KLINK lives in the signal box. It\l"
	.string "wants to see the world.$"

MetroMauville_Text_GotKlink:
	.string "{PLAYER} received a KLINK!$"

MetroMauville_Text_MasterFull:
	.string "Your party's full. Come back when\n"
	.string "you have room for KLINK.$"

MetroMauville_Text_MasterDone:
	.string "Thanks for riding the HOENN METRO!$"

MetroMauville_Text_Commuter:
	.string "The METRO gets me to SLATEPORT\n"
	.string "before my coffee's cold.$"

MetroMauville_Text_Kid:
	.string "I've got two stamps! I just need\n"
	.string "VERDANTURF.$"

