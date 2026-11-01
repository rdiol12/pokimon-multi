Silkwood_Gym_MapScripts::
	.byte 0

Silkwood_Gym_EventScript_Leader::
	goto_if_set FLAG_SOL_BADGE_4, Silkwood_Gym_EventScript_Rematch
	trainerbattle_single TRAINER_SOL_LEADER_THISTLE, Silkwood_Gym_EventScript_Text_Intro, Silkwood_Gym_EventScript_Text_Defeat, Silkwood_Gym_EventScript_Won, NO_MUSIC
	release
	end

Silkwood_Gym_EventScript_Won::
	message Silkwood_Gym_EventScript_Text_GotBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_SOL_BADGE_4
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Silkwood_Gym_EventScript_Text_BadgeInfo, MSGBOX_DEFAULT
	giveitem ITEM_TM_GIGA_DRAIN
	msgbox Silkwood_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Silkwood_Gym_EventScript_Rematch::
	lock
	faceplayer
	msgbox Silkwood_Gym_EventScript_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Silkwood_Gym_EventScript_RematchNo
	cleartrainerflag TRAINER_SOL_LEADER_THISTLE
	trainerbattle_single TRAINER_SOL_LEADER_THISTLE, Silkwood_Gym_EventScript_Text_RematchIntro, Silkwood_Gym_EventScript_Text_Defeat, Silkwood_Gym_EventScript_RematchWon, NO_MUSIC
	release
	end

Silkwood_Gym_EventScript_RematchWon::
	msgbox Silkwood_Gym_EventScript_Text_RematchDone, MSGBOX_DEFAULT
	release
	end

Silkwood_Gym_EventScript_RematchNo::
	msgbox Silkwood_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Silkwood_Gym_EventScript_Guide::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Silkwood_Gym_EventScript_Text_Guide, MSGBOX_DEFAULT
	release
	end

Silkwood_Gym_EventScript_Text_Intro:
	.string "Welcome to the hive! I'm\n"
	.string "THISTLE.\l"
	.string "Bugs work together. Let's see\l"
	.string "how your team works together!$"

Silkwood_Gym_EventScript_Text_Defeat:
	.string "Well fought! You've earned this.$"

Silkwood_Gym_EventScript_Text_GotBadge:
	.string "{PLAYER} received the\n"
	.string "HIVE BADGE from THISTLE!$"

Silkwood_Gym_EventScript_Text_BadgeInfo:
	.string "That makes {STR_VAR_1} SOLMARA badges.\n"
	.string "Every leader you face from now\l"
	.string "on will battle a little harder.\p"
	.string "And take this, too!$"

Silkwood_Gym_EventScript_Text_After:
	.string "Come back with your friends\n"
	.string "any time. A rematch keeps us\l"
	.string "all sharp!$"

Silkwood_Gym_EventScript_Text_RematchAsk:
	.string "Back for more? My team has grown\n"
	.string "along with your badges. Battle?$"

Silkwood_Gym_EventScript_Text_RematchIntro:
	.string "Then show me how much stronger\n"
	.string "you've become!$"

Silkwood_Gym_EventScript_Text_RematchDone:
	.string "Even better than last time!$"

Silkwood_Gym_EventScript_Text_Guide:
	.string "Yo, challenger! This is the SILKWOOD GYM.\p"
	.string "You have {STR_VAR_1} SOLMARA badges.\n"
	.string "The leader's team grows with\l"
	.string "every badge you hold.\p"
	.string "Lots of room in here, so bring\n"
	.string "your friends along!$"

Silkwood_Gym_EventScript_Bugcatcher::
	trainerbattle_single TRAINER_SOL_GYM_THISTLE_BUGCATCHER, Silkwood_Gym_EventScript_Bugcatcher_Intro, Silkwood_Gym_EventScript_Bugcatcher_Defeat
	msgbox Silkwood_Gym_EventScript_Bugcatcher_Post, MSGBOX_AUTOCLOSE
	end

Silkwood_Gym_EventScript_Bugcatcher_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Silkwood_Gym_EventScript_Bugcatcher_Defeat:
	.string "Go on, then!$"

Silkwood_Gym_EventScript_Bugcatcher_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

Silkwood_Gym_EventScript_Maniac::
	trainerbattle_single TRAINER_SOL_GYM_THISTLE_MANIAC, Silkwood_Gym_EventScript_Maniac_Intro, Silkwood_Gym_EventScript_Maniac_Defeat
	msgbox Silkwood_Gym_EventScript_Maniac_Post, MSGBOX_AUTOCLOSE
	end

Silkwood_Gym_EventScript_Maniac_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Silkwood_Gym_EventScript_Maniac_Defeat:
	.string "Go on, then!$"

Silkwood_Gym_EventScript_Maniac_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

