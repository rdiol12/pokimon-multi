@ Mt. Frostcrown guide in Lilycove City (tools/areas/frostcrown.py).
LilycoveCity_EventScript_FrostcrownGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE06_GET, LilycoveCity_EventScript_FrostcrownNoBadge
	msgbox LilycoveCity_Text_FrostcrownAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LilycoveCity_EventScript_FrostcrownNo
	msgbox LilycoveCity_Text_FrostcrownGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_MT_FROSTCROWN, 22, 85
	waitstate
	release
	end

LilycoveCity_EventScript_FrostcrownNoBadge::
	msgbox LilycoveCity_Text_FrostcrownNoBadge, MSGBOX_DEFAULT
	release
	end

LilycoveCity_EventScript_FrostcrownNo::
	msgbox LilycoveCity_Text_FrostcrownNo, MSGBOX_DEFAULT
	release
	end

LilycoveCity_Text_FrostcrownNoBadge:
	.string "I'm a ranger on MT. FROSTCROWN, the\n"
	.string "snowy peak north of here.\p"
	.string "The trail's dangerous after the\n"
	.string "blizzard. I'll only take trainers\l"
	.string "with six badges.$"

LilycoveCity_Text_FrostcrownAsk:
	.string "Six badges! Just the trainer we\n"
	.string "need. Climbers went missing on\l"
	.string "MT. FROSTCROWN in the blizzard.\p"
	.string "Will you come up and help look?$"

LilycoveCity_Text_FrostcrownGo:
	.string "Wrap up warm!$"

LilycoveCity_Text_FrostcrownNo:
	.string "If you change your mind, I'll be\n"
	.string "right here.$"
