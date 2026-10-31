@ Prism Caverns guide in Meteor Falls (tools/areas/crystal.py).
MeteorFalls_1F_1R_EventScript_PrismGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE05_GET, MeteorFalls_1F_1R_EventScript_PrismNoBadge
	msgbox MeteorFalls_1F_1R_Text_PrismAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MeteorFalls_1F_1R_EventScript_PrismNo
	msgbox MeteorFalls_1F_1R_Text_PrismGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_PRISM_CAVERNS, 28, 41
	waitstate
	release
	end

MeteorFalls_1F_1R_EventScript_PrismNoBadge::
	msgbox MeteorFalls_1F_1R_Text_PrismNoBadge, MSGBOX_DEFAULT
	release
	end

MeteorFalls_1F_1R_EventScript_PrismNo::
	msgbox MeteorFalls_1F_1R_Text_PrismNo, MSGBOX_DEFAULT
	release
	end

MeteorFalls_1F_1R_Text_PrismNoBadge:
	.string "I'm with the GEOLOGY TEAM. We found\n"
	.string "a crystal cave behind the falls!\p"
	.string "It's deep and full of strong\n"
	.string "POKéMON. I can only take trainers\l"
	.string "with five badges.$"

MeteorFalls_1F_1R_Text_PrismAsk:
	.string "Five badges! You could help us.\n"
	.string "There's a crystal cave behind the\l"
	.string "falls: the PRISM CAVERNS.\p"
	.string "Our team is camped inside. Shall I\n"
	.string "show you the way?$"

MeteorFalls_1F_1R_Text_PrismGo:
	.string "Watch your step. The crystals are\n"
	.string "sharp!$"

MeteorFalls_1F_1R_Text_PrismNo:
	.string "I'll be here, taking samples.$"
