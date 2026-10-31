@ Skyrope Canyon guide in Fallarbor Town (tools/areas/canyon.py).
FallarborTown_EventScript_SkyropeGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE05_GET, FallarborTown_EventScript_SkyropeGuideNoBadge
	msgbox FallarborTown_Text_SkyropeGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FallarborTown_EventScript_SkyropeGuideNo
	msgbox FallarborTown_Text_SkyropeGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SKYROPE_CANYON, 32, 57
	waitstate
	release
	end

FallarborTown_EventScript_SkyropeGuideNoBadge::
	msgbox FallarborTown_Text_SkyropeGuideNoBadge, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_SkyropeGuideNo::
	msgbox FallarborTown_Text_SkyropeGuideNo, MSGBOX_DEFAULT
	release
	end

FallarborTown_Text_SkyropeGuideNoBadge:
	.string "Past the ash fields lies SKYROPE\n"
	.string "CANYON, all mesas and rope bridges.\p"
	.string "The winds are strong. Come back with\n"
	.string "NORMAN's badge and I'll take you.$"

FallarborTown_Text_SkyropeGuideAsk:
	.string "That's NORMAN's badge! You'll be\n"
	.string "fine on the bridges.\p"
	.string "Shall I lead you to SKYROPE\n"
	.string "CANYON?$"

FallarborTown_Text_SkyropeGuideGo:
	.string "Hold on to your hat!$"

FallarborTown_Text_SkyropeGuideNo:
	.string "The canyon isn't going anywhere.\n"
	.string "It took a million years to carve!$"
