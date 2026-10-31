@ Mirebough Marsh guide in Fortree City (tools/areas/swamp.py).
FortreeCity_EventScript_MireboughGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE06_GET, FortreeCity_EventScript_MireboughGuideNoBadge
	msgbox FortreeCity_Text_MireboughGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_EventScript_MireboughGuideNo
	msgbox FortreeCity_Text_MireboughGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_MIREBOUGH_MARSH, 28, 45
	waitstate
	release
	end

FortreeCity_EventScript_MireboughGuideNoBadge::
	msgbox FortreeCity_Text_MireboughGuideNoBadge, MSGBOX_DEFAULT
	release
	end

FortreeCity_EventScript_MireboughGuideNo::
	msgbox FortreeCity_Text_MireboughGuideNo, MSGBOX_DEFAULT
	release
	end

FortreeCity_Text_MireboughGuideNoBadge:
	.string "West of here, where it always rains,\n"
	.string "is MIREBOUGH MARSH.\p"
	.string "It's easy to get lost. Come back\n"
	.string "with WINONA's badge.$"

FortreeCity_Text_MireboughGuideAsk:
	.string "WINONA's badge! Then you won't get\n"
	.string "lost.\p"
	.string "Shall I take you to MIREBOUGH\n"
	.string "MARSH?$"

FortreeCity_Text_MireboughGuideGo:
	.string "Bring an umbrella!$"

FortreeCity_Text_MireboughGuideNo:
	.string "The rain will keep. It never stops.$"
