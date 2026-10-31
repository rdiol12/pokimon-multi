@ Hoenn Metro: riding the train, and the stamp rally (tools/areas/metro.py).
Metro_EventScript_RideToMauville::
	call Metro_EventScript_Depart
	warp MAP_METRO_MAUVILLE, 13, 12
	waitstate
	releaseall
	end

Metro_EventScript_RideToSlateport::
	call Metro_EventScript_Depart
	warp MAP_METRO_SLATEPORT, 13, 12
	waitstate
	releaseall
	end

Metro_EventScript_RideToVerdanturf::
	call Metro_EventScript_Depart
	warp MAP_METRO_VERDANTURF, 13, 12
	waitstate
	releaseall
	end

Metro_EventScript_Depart::
	msgbox Metro_Text_AllAboard, MSGBOX_DEFAULT
	closemessage
	playse SE_TRUCK_DOOR
	waitse
	playse SE_TRUCK_MOVE
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 12
	setvar VAR_0x8007, 4
	special ShakeCamera
	waitstate
	fadescreen FADE_TO_BLACK
	delay 60
	playse SE_TRUCK_STOP
	waitse
	return

Metro_EventScript_Stamped::
	msgbox Metro_Text_Stamped, MSGBOX_DEFAULT
	releaseall
	end

Metro_Text_WhereTo:
	.string "The train's doors are open.\n"
	.string "Where would you like to go?$"

Metro_Text_Mauville:
	.string "MAUVILLE$"

Metro_Text_Slateport:
	.string "SLATEPORT$"

Metro_Text_Verdanturf:
	.string "VERDANTURF$"

Metro_Text_Cancel:
	.string "CANCEL$"

Metro_Text_AllAboard:
	.string "All aboard! Mind the doors!$"

Metro_Text_Stamped:
	.string "This stamp is already on your card.$"
