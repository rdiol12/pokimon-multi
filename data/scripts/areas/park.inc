@ Wonderwild Park guide in Slateport City (tools/areas/park.py).
SlateportCity_EventScript_ParkGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE02_GET, SlateportCity_EventScript_ParkNoBadge
	msgbox SlateportCity_Text_ParkAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SlateportCity_EventScript_ParkNo
	msgbox SlateportCity_Text_ParkGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_WONDERWILD_PARK, 27, 44
	waitstate
	release
	end

SlateportCity_EventScript_ParkNoBadge::
	msgbox SlateportCity_Text_ParkNoBadge, MSGBOX_DEFAULT
	release
	end

SlateportCity_EventScript_ParkNo::
	msgbox SlateportCity_Text_ParkNo, MSGBOX_DEFAULT
	release
	end

SlateportCity_Text_ParkNoBadge:
	.string "The tram to WONDERWILD PARK is for\n"
	.string "trainers with two badges.\p"
	.string "The rides are a little wild for\n"
	.string "beginners!$"

SlateportCity_Text_ParkAsk:
	.string "Two badges! Fancy a trip to\n"
	.string "WONDERWILD PARK? Roller coasters,\l"
	.string "teacups, a hedge maze…\p"
	.string "Hop on the tram?$"

SlateportCity_Text_ParkGo:
	.string "Have fun!$"

SlateportCity_Text_ParkNo:
	.string "The tram runs all day.$"
