@ Nimbus Isles balloon pilot in Fortree City (tools/areas/nimbus.py).
FortreeCity_EventScript_NimbusPilot::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE07_GET, FortreeCity_EventScript_NimbusNoBadge
	msgbox FortreeCity_Text_NimbusAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_EventScript_NimbusNo
	msgbox FortreeCity_Text_NimbusGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_NIMBUS_CITY, 20, 27
	waitstate
	release
	end

FortreeCity_EventScript_NimbusNoBadge::
	msgbox FortreeCity_Text_NimbusNoBadge, MSGBOX_DEFAULT
	release
	end

FortreeCity_EventScript_NimbusNo::
	msgbox FortreeCity_Text_NimbusNo, MSGBOX_DEFAULT
	release
	end

FortreeCity_Text_NimbusNoBadge:
	.string "My balloon flies up to NIMBUS CITY,\n"
	.string "the town above the clouds.\p"
	.string "The winds up there are rough,\n"
	.string "though. Come back when you've\l"
	.string "earned seven badges.$"

FortreeCity_Text_NimbusAsk:
	.string "Seven badges! You can handle the\n"
	.string "winds of the NIMBUS ISLES.\p"
	.string "Shall we fly up to NIMBUS CITY?$"

FortreeCity_Text_NimbusGo:
	.string "Up, up and away!$"

FortreeCity_Text_NimbusNo:
	.string "The balloon's ready when you are.$"
