@ Ghost trainers: another player's team, met on a route. See
@ src/life_ghosts.c.

Life_EventScript_Ghost::
	lock
	faceplayer
	callnative LifeGhost_BufferIntro
	playse SE_M_CONFUSE_RAY
	msgbox Life_Text_GhostIntro, MSGBOX_DEFAULT
	msgbox Life_Text_GhostAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Life_EventScript_GhostLeave
	msgbox Life_Text_GhostGo, MSGBOX_DEFAULT
	closemessage
	callnative LifeGhost_StartBattle, waitstate=1
	callnative LifeGhost_BufferIntro
	msgbox Life_Text_GhostAfter, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers FADE_TO_WHITE
	removeobject LOCALID_LIFE_GHOST
	fadescreenswapbuffers FADE_FROM_WHITE
	release
	end

Life_EventScript_GhostLeave:
	msgbox Life_Text_GhostFades, MSGBOX_DEFAULT
	closemessage
	playse SE_M_CONFUSE_RAY
	fadescreenswapbuffers FADE_TO_WHITE
	callnative LifeGhost_Dismiss
	fadescreenswapbuffers FADE_FROM_WHITE
	release
	end

Life_Text_GhostIntro:
	.string "A ghostly trainer shimmers into view…\p"
	.string "It's the team of {STR_VAR_1}, a trainer\n"
	.string "somewhere out there in the world!\p"
	.string "This ghost team has won {STR_VAR_2} and\n"
	.string "lost {STR_VAR_3} battles.$"

Life_Text_GhostAsk:
	.string "Battle {STR_VAR_1}'s ghost team?$"

Life_Text_GhostGo:
	.string "The ghost raises a hand and its\n"
	.string "POKéMON step forward!$"

Life_Text_GhostAfter:
	.string "{STR_VAR_1}'s ghost team bows and fades\n"
	.string "away. {STR_VAR_1} will hear about this!$"

Life_Text_GhostFades:
	.string "The ghost nods and fades away…$"
