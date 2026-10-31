@ Ember Springs guide in Lavaridge Town (tools/areas/embersprings.py).
LavaridgeTown_EventScript_EmberSpringsGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE04_GET, LavaridgeTown_EventScript_EmberSpringsNoBadge
	msgbox LavaridgeTown_Text_EmberSpringsAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LavaridgeTown_EventScript_EmberSpringsNo
	closemessage
	warp MAP_EMBER_SPRINGS, 22, 38
	waitstate
	release
	end

LavaridgeTown_EventScript_EmberSpringsNoBadge::
	msgbox LavaridgeTown_Text_EmberSpringsNoBadge, MSGBOX_DEFAULT
	release
	end

LavaridgeTown_EventScript_EmberSpringsNo::
	msgbox LavaridgeTown_Text_EmberSpringsNo, MSGBOX_DEFAULT
	release
	end

LavaridgeTown_Text_EmberSpringsNoBadge:
	.string "Up the mountain is EMBER SPRINGS,\n"
	.string "a town of hot springs.\p"
	.string "The trail is steep. Beat FLANNERY\n"
	.string "first, then I'll show you the way.$"

LavaridgeTown_Text_EmberSpringsAsk:
	.string "You beat FLANNERY! Want me to show\n"
	.string "you the trail up to EMBER SPRINGS?$"

LavaridgeTown_Text_EmberSpringsNo:
	.string "The springs aren't going anywhere!$"
