Rosemead_Gym_MapScripts::
	.byte 0

Rosemead_Gym_EventScript_Leader::
	goto_if_set FLAG_SOL_BADGE_5, Rosemead_Gym_EventScript_Rematch
	trainerbattle_single TRAINER_SOL_LEADER_BELLE, Rosemead_Gym_EventScript_Text_Intro, Rosemead_Gym_EventScript_Text_Defeat, Rosemead_Gym_EventScript_Won, NO_MUSIC
	release
	end

Rosemead_Gym_EventScript_Won::
	message Rosemead_Gym_EventScript_Text_GotBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_SOL_BADGE_5
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Rosemead_Gym_EventScript_Text_BadgeInfo, MSGBOX_DEFAULT
	giveitem ITEM_TM_CALM_MIND
	msgbox Rosemead_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Rosemead_Gym_EventScript_Rematch::
	lock
	faceplayer
	msgbox Rosemead_Gym_EventScript_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Rosemead_Gym_EventScript_RematchNo
	cleartrainerflag TRAINER_SOL_LEADER_BELLE
	trainerbattle_single TRAINER_SOL_LEADER_BELLE, Rosemead_Gym_EventScript_Text_RematchIntro, Rosemead_Gym_EventScript_Text_Defeat, Rosemead_Gym_EventScript_RematchWon, NO_MUSIC
	release
	end

Rosemead_Gym_EventScript_RematchWon::
	msgbox Rosemead_Gym_EventScript_Text_RematchDone, MSGBOX_DEFAULT
	release
	end

Rosemead_Gym_EventScript_RematchNo::
	msgbox Rosemead_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Rosemead_Gym_EventScript_Guide::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Rosemead_Gym_EventScript_Text_Guide, MSGBOX_DEFAULT
	release
	end

Rosemead_Gym_EventScript_Text_Intro:
	.string "Oh, a visitor! I'm BELLE.\n"
	.string "Pretty things can be strong too.\l"
	.string "Let me show you!$"

Rosemead_Gym_EventScript_Text_Defeat:
	.string "Well fought! You've earned this.$"

Rosemead_Gym_EventScript_Text_GotBadge:
	.string "{PLAYER} received the\n"
	.string "BLOOM BADGE from BELLE!$"

Rosemead_Gym_EventScript_Text_BadgeInfo:
	.string "That makes {STR_VAR_1} SOLMARA badges.\n"
	.string "Every leader you face from now\l"
	.string "on will battle a little harder.\p"
	.string "And take this, too!$"

Rosemead_Gym_EventScript_Text_After:
	.string "Come back with your friends\n"
	.string "any time. A rematch keeps us\l"
	.string "all sharp!$"

Rosemead_Gym_EventScript_Text_RematchAsk:
	.string "Back for more? My team has grown\n"
	.string "along with your badges. Battle?$"

Rosemead_Gym_EventScript_Text_RematchIntro:
	.string "Then show me how much stronger\n"
	.string "you've become!$"

Rosemead_Gym_EventScript_Text_RematchDone:
	.string "Even better than last time!$"

Rosemead_Gym_EventScript_Text_Guide:
	.string "Yo, challenger! This is the ROSEMEAD GYM.\p"
	.string "You have {STR_VAR_1} SOLMARA badges.\n"
	.string "The leader's team grows with\l"
	.string "every badge you hold.\p"
	.string "Lots of room in here, so bring\n"
	.string "your friends along!$"

Rosemead_Gym_EventScript_Lady::
	trainerbattle_single TRAINER_SOL_GYM_BELLE_LADY, Rosemead_Gym_EventScript_Lady_Intro, Rosemead_Gym_EventScript_Lady_Defeat
	msgbox Rosemead_Gym_EventScript_Lady_Post, MSGBOX_AUTOCLOSE
	end

Rosemead_Gym_EventScript_Lady_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Rosemead_Gym_EventScript_Lady_Defeat:
	.string "Go on, then!$"

Rosemead_Gym_EventScript_Lady_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

Rosemead_Gym_EventScript_Girl::
	trainerbattle_single TRAINER_SOL_GYM_BELLE_GIRL, Rosemead_Gym_EventScript_Girl_Intro, Rosemead_Gym_EventScript_Girl_Defeat
	msgbox Rosemead_Gym_EventScript_Girl_Post, MSGBOX_AUTOCLOSE
	end

Rosemead_Gym_EventScript_Girl_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Rosemead_Gym_EventScript_Girl_Defeat:
	.string "Go on, then!$"

Rosemead_Gym_EventScript_Girl_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

