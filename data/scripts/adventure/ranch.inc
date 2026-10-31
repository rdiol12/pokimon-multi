@ The ranch hand next to the Route 117 Day Care takes the player to the
@ Hoenn Ranch (data/maps/AdventureRanch).

Adventure_EventScript_RanchHand::
	lock
	faceplayer
	msgbox Adventure_Text_RanchHand, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Adventure_EventScript_RanchHandNo
	msgbox Adventure_Text_RanchHandGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_ADVENTURE_RANCH, 14, 15
	waitstate
	release
	end

Adventure_EventScript_RanchHandNo:
	msgbox Adventure_Text_RanchHandNo, MSGBOX_DEFAULT
	release
	end

Adventure_Text_RanchHand:
	.string "Behind the DAY CARE is the HOENN\n"
	.string "RANCH, where POKéMON roam free.\p"
	.string "Want me to show you the way?$"

Adventure_Text_RanchHandGo:
	.string "Follow me, it's just through here!$"

Adventure_Text_RanchHandNo:
	.string "The ranch is always open. Come by\n"
	.string "any time!$"
