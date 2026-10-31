@ Verdant River canoe guide on Route 119 (tools/areas/jungle.py).
Route119_EventScript_VerdantGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE05_GET, Route119_EventScript_VerdantNoBadge
	msgbox Route119_Text_VerdantAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route119_EventScript_VerdantNo
	msgbox Route119_Text_VerdantGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_VERDANT_RIVER, 10, 60
	waitstate
	release
	end

Route119_EventScript_VerdantNoBadge::
	msgbox Route119_Text_VerdantNoBadge, MSGBOX_DEFAULT
	release
	end

Route119_EventScript_VerdantNo::
	msgbox Route119_Text_VerdantNo, MSGBOX_DEFAULT
	release
	end

Route119_Text_VerdantNoBadge:
	.string "I paddle travellers to the VERDANT\n"
	.string "RIVER, deep in the jungle.\p"
	.string "You'll need to SURF up there. Come\n"
	.string "back once you have five badges.$"

Route119_Text_VerdantAsk:
	.string "Five badges! The VERDANT RIVER\n"
	.string "village could use a trainer who\l"
	.string "can SURF.\p"
	.string "Shall I paddle you there?$"

Route119_Text_VerdantGo:
	.string "Keep your hands inside the canoe!$"

Route119_Text_VerdantNo:
	.string "The river will wait.$"
