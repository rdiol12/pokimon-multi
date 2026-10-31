@ The Hollowmere ferryman in Lilycove City (tools/areas/ghosttown.py). He
@ only rows out at night, when the town appears.
LilycoveCity_EventScript_HollowmereFerryman::
	lock
	faceplayer
	gettimeofday
	goto_if_ne VAR_0x8000, TIME_NIGHT, LilycoveCity_EventScript_HollowmereDay
	goto_if_unset FLAG_BADGE07_GET, LilycoveCity_EventScript_HollowmereNoBadge
	msgbox LilycoveCity_Text_HollowmereAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LilycoveCity_EventScript_HollowmereNo
	msgbox LilycoveCity_Text_HollowmereGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_HOLLOWMERE, 22, 37
	waitstate
	release
	end

LilycoveCity_EventScript_HollowmereDay::
	msgbox LilycoveCity_Text_HollowmereDay, MSGBOX_DEFAULT
	release
	end

LilycoveCity_EventScript_HollowmereNoBadge::
	msgbox LilycoveCity_Text_HollowmereNoBadge, MSGBOX_DEFAULT
	release
	end

LilycoveCity_EventScript_HollowmereNo::
	msgbox LilycoveCity_Text_HollowmereNo, MSGBOX_DEFAULT
	release
	end

LilycoveCity_Text_HollowmereDay:
	.string "Out past MT. PYRE there's a town\n"
	.string "called HOLLOWMERE.\p"
	.string "You can only find it at night. Come\n"
	.string "back after dark and I'll row you.$"

LilycoveCity_Text_HollowmereNoBadge:
	.string "The mist is thick tonight. Only\n"
	.string "those with the MIND BADGE may cross\l"
	.string "to HOLLOWMERE.$"

LilycoveCity_Text_HollowmereAsk:
	.string "The mist is up and the lamps are\n"
	.string "out. Shall I row you to HOLLOWMERE?$"

LilycoveCity_Text_HollowmereGo:
	.string "Sit still, and don't trail your\n"
	.string "hand in the water…$"

LilycoveCity_Text_HollowmereNo:
	.string "HOLLOWMERE will be there tomorrow\n"
	.string "night. And the next.$"
