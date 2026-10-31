@ The league ferry in Slateport City (tools/areas/laurel.py).
SlateportCity_EventScript_LeagueFerry::
	lock
	faceplayer
	msgbox SlateportCity_Text_LeagueFerryAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SlateportCity_EventScript_LeagueFerryNo
	msgbox SlateportCity_Text_LeagueFerryGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_LAUREL_CITY, 25, 47
	waitstate
	release
	end

SlateportCity_EventScript_LeagueFerryNo::
	msgbox SlateportCity_Text_LeagueFerryNo, MSGBOX_DEFAULT
	release
	end

SlateportCity_Text_LeagueFerryAsk:
	.string "This is the league ferry to LAUREL\n"
	.string "CITY, home of the ROVER LEAGUE!\p"
	.string "Want to go watch, or compete?$"

SlateportCity_Text_LeagueFerryGo:
	.string "Off we go! Next stop, LAUREL CITY!$"

SlateportCity_Text_LeagueFerryNo:
	.string "The ferry runs every day. Come\n"
	.string "back any time!$"
