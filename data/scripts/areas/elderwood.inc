@ Elderwood Forest guide in Fortree City (tools/areas/elderwood.py).
FortreeCity_EventScript_ElderwoodGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE06_GET, FortreeCity_EventScript_ElderwoodGuideNoBadge
	msgbox FortreeCity_Text_ElderwoodGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_EventScript_ElderwoodGuideNo
	msgbox FortreeCity_Text_ElderwoodGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_ELDERWOOD_FOREST, 27, 49
	waitstate
	release
	end

FortreeCity_EventScript_ElderwoodGuideNoBadge::
	msgbox FortreeCity_Text_ElderwoodGuideNoBadge, MSGBOX_DEFAULT
	release
	end

FortreeCity_EventScript_ElderwoodGuideNo::
	msgbox FortreeCity_Text_ElderwoodGuideNo, MSGBOX_DEFAULT
	release
	end

FortreeCity_Text_ElderwoodGuideNoBadge:
	.string "East of here lies ELDERWOOD, the\n"
	.string "oldest forest in HOENN.\p"
	.string "The canopy paths are dangerous.\n"
	.string "Come back with WINONA's badge and\l"
	.string "I'll take you there.$"

FortreeCity_Text_ElderwoodGuideAsk:
	.string "That's WINONA's badge! Then you can\n"
	.string "handle the canopy.\p"
	.string "Shall I lead you to ELDERWOOD\n"
	.string "FOREST?$"

FortreeCity_Text_ElderwoodGuideGo:
	.string "Stay close. The trail is hidden\n"
	.string "under the vines!$"

FortreeCity_Text_ElderwoodGuideNo:
	.string "The forest has waited a thousand\n"
	.string "years. It can wait for you.$"
