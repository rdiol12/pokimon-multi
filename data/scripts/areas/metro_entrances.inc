@ Hoenn Metro attendants in the cities (tools/areas/metro.py).

MauvilleCity_EventScript_MetroAttendant::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE03_GET, Metro_EventScript_AttendantClosed
	msgbox Metro_Text_AttendantAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Metro_EventScript_AttendantNo
	closemessage
	warp MAP_METRO_MAUVILLE, 3, 2
	waitstate
	release
	end

SlateportCity_EventScript_MetroAttendant::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE03_GET, Metro_EventScript_AttendantClosed
	msgbox Metro_Text_AttendantAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Metro_EventScript_AttendantNo
	closemessage
	warp MAP_METRO_SLATEPORT, 3, 2
	waitstate
	release
	end

VerdanturfTown_EventScript_MetroAttendant::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE03_GET, Metro_EventScript_AttendantClosed
	msgbox Metro_Text_AttendantAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Metro_EventScript_AttendantNo
	closemessage
	warp MAP_METRO_VERDANTURF, 3, 2
	waitstate
	release
	end

Metro_EventScript_AttendantClosed::
	msgbox Metro_Text_AttendantClosed, MSGBOX_DEFAULT
	release
	end

Metro_EventScript_AttendantNo::
	msgbox Metro_Text_AttendantNo, MSGBOX_DEFAULT
	release
	end

Metro_Text_AttendantClosed:
	.string "The HOENN METRO's still being\n"
	.string "built. WATTSON's checking the\l"
	.string "power lines.\p"
	.string "Come back when you've beaten him\n"
	.string "at the MAUVILLE GYM.$"

Metro_Text_AttendantAsk:
	.string "Welcome to the HOENN METRO! Trains\n"
	.string "to MAUVILLE, SLATEPORT and\l"
	.string "VERDANTURF. Rides are free!\p"
	.string "Go down to the platform?$"

Metro_Text_AttendantNo:
	.string "Have a nice day!$"
