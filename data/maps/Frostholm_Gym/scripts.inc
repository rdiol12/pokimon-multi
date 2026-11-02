Frostholm_Gym_MapScripts::
	.byte 0

Frostholm_Gym_EventScript_Leader::
	goto_if_set FLAG_SOL_BADGE_7, Frostholm_Gym_EventScript_Rematch
	trainerbattle_single TRAINER_SOL_LEADER_SIGRID, Frostholm_Gym_EventScript_Text_Intro, Frostholm_Gym_EventScript_Text_Defeat, Frostholm_Gym_EventScript_Won, NO_MUSIC
	release
	end

Frostholm_Gym_EventScript_Won::
	message Frostholm_Gym_EventScript_Text_GotBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_SOL_BADGE_7
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Frostholm_Gym_EventScript_Text_BadgeInfo, MSGBOX_DEFAULT
	giveitem ITEM_TM_ICE_BEAM
	msgbox Frostholm_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Frostholm_Gym_EventScript_Rematch::
	lock
	faceplayer
	msgbox Frostholm_Gym_EventScript_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Frostholm_Gym_EventScript_RematchNo
	cleartrainerflag TRAINER_SOL_LEADER_SIGRID
	trainerbattle_single TRAINER_SOL_LEADER_SIGRID, Frostholm_Gym_EventScript_Text_RematchIntro, Frostholm_Gym_EventScript_Text_Defeat, Frostholm_Gym_EventScript_RematchWon, NO_MUSIC
	release
	end

Frostholm_Gym_EventScript_RematchWon::
	msgbox Frostholm_Gym_EventScript_Text_RematchDone, MSGBOX_DEFAULT
	release
	end

Frostholm_Gym_EventScript_RematchNo::
	msgbox Frostholm_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Frostholm_Gym_EventScript_Guide::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Frostholm_Gym_EventScript_Text_Guide, MSGBOX_DEFAULT
	release
	end

Frostholm_Gym_EventScript_Text_Intro:
	.string "I am SIGRID. In the north, the\n"
	.string "cold teaches patience.\l"
	.string "Your patience ends here!$"

Frostholm_Gym_EventScript_Text_Defeat:
	.string "Well fought! You've earned this.$"

Frostholm_Gym_EventScript_Text_GotBadge:
	.string "{PLAYER} received the\n"
	.string "RIME BADGE from SIGRID!$"

Frostholm_Gym_EventScript_Text_BadgeInfo:
	.string "That makes {STR_VAR_1} SOLMARA badges.\n"
	.string "Every leader you face from now\l"
	.string "on will battle a little harder.\p"
	.string "And take this, too!$"

Frostholm_Gym_EventScript_Text_After:
	.string "Come back with your friends\n"
	.string "any time. A rematch keeps us\l"
	.string "all sharp!$"

Frostholm_Gym_EventScript_Text_RematchAsk:
	.string "Back for more? My team has grown\n"
	.string "along with your badges. Battle?$"

Frostholm_Gym_EventScript_Text_RematchIntro:
	.string "Then show me how much stronger\n"
	.string "you've become!$"

Frostholm_Gym_EventScript_Text_RematchDone:
	.string "Even better than last time!$"

Frostholm_Gym_EventScript_Text_Guide:
	.string "Yo, challenger! This is the FROSTHOLM GYM.\p"
	.string "You have {STR_VAR_1} SOLMARA badges.\n"
	.string "The leader's team grows with\l"
	.string "every badge you hold.\p"
	.string "Lots of room in here, so bring\n"
	.string "your friends along!$"

Frostholm_Gym_EventScript_Skier::
	trainerbattle_single TRAINER_SOL_GYM_SIGRID_SKIER, Frostholm_Gym_EventScript_Skier_Intro, Frostholm_Gym_EventScript_Skier_Defeat
	msgbox Frostholm_Gym_EventScript_Skier_Post, MSGBOX_AUTOCLOSE
	end

Frostholm_Gym_EventScript_Skier_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Frostholm_Gym_EventScript_Skier_Defeat:
	.string "Go on, then!$"

Frostholm_Gym_EventScript_Skier_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

Frostholm_Gym_EventScript_Boarder::
	trainerbattle_single TRAINER_SOL_GYM_SIGRID_BOARDER, Frostholm_Gym_EventScript_Boarder_Intro, Frostholm_Gym_EventScript_Boarder_Defeat
	msgbox Frostholm_Gym_EventScript_Boarder_Post, MSGBOX_AUTOCLOSE
	end

Frostholm_Gym_EventScript_Boarder_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Frostholm_Gym_EventScript_Boarder_Defeat:
	.string "Go on, then!$"

Frostholm_Gym_EventScript_Boarder_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

