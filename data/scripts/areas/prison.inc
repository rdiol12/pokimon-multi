@ Ironmoor Prison boatman on Route 109 (tools/areas/prison.py).
Route109_EventScript_IronmoorBoatman::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE04_GET, Route109_EventScript_IronmoorNoBadge
	msgbox Route109_Text_IronmoorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route109_EventScript_IronmoorNo
	msgbox Route109_Text_IronmoorGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_IRONMOOR_ISLE, 22, 45
	waitstate
	release
	end

Route109_EventScript_IronmoorNoBadge::
	msgbox Route109_Text_IronmoorNoBadge, MSGBOX_DEFAULT
	release
	end

Route109_EventScript_IronmoorNo::
	msgbox Route109_Text_IronmoorNo, MSGBOX_DEFAULT
	release
	end

Route109_Text_IronmoorNoBadge:
	.string "See that island in the fog? IRONMOOR\n"
	.string "PRISON. Empty for fifty years, but\l"
	.string "the lights still burn at night.\p"
	.string "I only row out trainers who've\n"
	.string "beaten four GYM LEADERS.$"

Route109_Text_IronmoorAsk:
	.string "Four badges, eh? Brave enough for\n"
	.string "IRONMOOR PRISON?\p"
	.string "I'll row you out, but I won't\n"
	.string "step ashore.$"

Route109_Text_IronmoorGo:
	.string "Hold on tight.$"

Route109_Text_IronmoorNo:
	.string "Can't say I blame you.$"
