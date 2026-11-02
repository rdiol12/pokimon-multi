Ashcombe_Gym_MapScripts::
	.byte 0

Ashcombe_Gym_EventScript_Leader::
	goto_if_set FLAG_SOL_BADGE_3, Ashcombe_Gym_EventScript_Rematch
	trainerbattle_single TRAINER_SOL_LEADER_CINDRA, Ashcombe_Gym_EventScript_Text_Intro, Ashcombe_Gym_EventScript_Text_Defeat, Ashcombe_Gym_EventScript_Won, NO_MUSIC
	release
	end

Ashcombe_Gym_EventScript_Won::
	message Ashcombe_Gym_EventScript_Text_GotBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_SOL_BADGE_3
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Ashcombe_Gym_EventScript_Text_BadgeInfo, MSGBOX_DEFAULT
	giveitem ITEM_TM_FLAMETHROWER
	msgbox Ashcombe_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Ashcombe_Gym_EventScript_Rematch::
	lock
	faceplayer
	msgbox Ashcombe_Gym_EventScript_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Ashcombe_Gym_EventScript_RematchNo
	cleartrainerflag TRAINER_SOL_LEADER_CINDRA
	trainerbattle_single TRAINER_SOL_LEADER_CINDRA, Ashcombe_Gym_EventScript_Text_RematchIntro, Ashcombe_Gym_EventScript_Text_Defeat, Ashcombe_Gym_EventScript_RematchWon, NO_MUSIC
	release
	end

Ashcombe_Gym_EventScript_RematchWon::
	msgbox Ashcombe_Gym_EventScript_Text_RematchDone, MSGBOX_DEFAULT
	release
	end

Ashcombe_Gym_EventScript_RematchNo::
	msgbox Ashcombe_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Ashcombe_Gym_EventScript_Guide::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Ashcombe_Gym_EventScript_Text_Guide, MSGBOX_DEFAULT
	release
	end

Ashcombe_Gym_EventScript_Text_Intro:
	.string "I'm CINDRA! The springs keep this\n"
	.string "town warm, and I keep it hot!\l"
	.string "Show me your fighting spirit!$"

Ashcombe_Gym_EventScript_Text_Defeat:
	.string "Well fought! You've earned this.$"

Ashcombe_Gym_EventScript_Text_GotBadge:
	.string "{PLAYER} received the\n"
	.string "CINDER BADGE from CINDRA!$"

Ashcombe_Gym_EventScript_Text_BadgeInfo:
	.string "That makes {STR_VAR_1} SOLMARA badges.\n"
	.string "Every leader you face from now\l"
	.string "on will battle a little harder.\p"
	.string "And take this, too!$"

Ashcombe_Gym_EventScript_Text_After:
	.string "Come back with your friends\n"
	.string "any time. A rematch keeps us\l"
	.string "all sharp!$"

Ashcombe_Gym_EventScript_Text_RematchAsk:
	.string "Back for more? My team has grown\n"
	.string "along with your badges. Battle?$"

Ashcombe_Gym_EventScript_Text_RematchIntro:
	.string "Then show me how much stronger\n"
	.string "you've become!$"

Ashcombe_Gym_EventScript_Text_RematchDone:
	.string "Even better than last time!$"

Ashcombe_Gym_EventScript_Text_Guide:
	.string "Yo, challenger! This is the ASHCOMBE GYM.\p"
	.string "You have {STR_VAR_1} SOLMARA badges.\n"
	.string "The leader's team grows with\l"
	.string "every badge you hold.\p"
	.string "Lots of room in here, so bring\n"
	.string "your friends along!$"

Ashcombe_Gym_EventScript_Kindler::
	trainerbattle_single TRAINER_SOL_GYM_CINDRA_KINDLER, Ashcombe_Gym_EventScript_Kindler_Intro, Ashcombe_Gym_EventScript_Kindler_Defeat
	msgbox Ashcombe_Gym_EventScript_Kindler_Post, MSGBOX_AUTOCLOSE
	end

Ashcombe_Gym_EventScript_Kindler_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Ashcombe_Gym_EventScript_Kindler_Defeat:
	.string "Go on, then!$"

Ashcombe_Gym_EventScript_Kindler_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

Ashcombe_Gym_EventScript_Cool::
	trainerbattle_single TRAINER_SOL_GYM_CINDRA_COOL, Ashcombe_Gym_EventScript_Cool_Intro, Ashcombe_Gym_EventScript_Cool_Defeat
	msgbox Ashcombe_Gym_EventScript_Cool_Post, MSGBOX_AUTOCLOSE
	end

Ashcombe_Gym_EventScript_Cool_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Ashcombe_Gym_EventScript_Cool_Defeat:
	.string "Go on, then!$"

Ashcombe_Gym_EventScript_Cool_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

