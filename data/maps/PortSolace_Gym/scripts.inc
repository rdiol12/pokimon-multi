PortSolace_Gym_MapScripts::
	.byte 0

PortSolace_Gym_EventScript_Leader::
	goto_if_set FLAG_SOL_BADGE_1, PortSolace_Gym_EventScript_Rematch
	trainerbattle_single TRAINER_SOL_LEADER_MARINA, PortSolace_Gym_EventScript_Text_Intro, PortSolace_Gym_EventScript_Text_Defeat, PortSolace_Gym_EventScript_Won, NO_MUSIC
	release
	end

PortSolace_Gym_EventScript_Won::
	message PortSolace_Gym_EventScript_Text_GotBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_SOL_BADGE_1
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox PortSolace_Gym_EventScript_Text_BadgeInfo, MSGBOX_DEFAULT
	giveitem ITEM_TM_WATER_PULSE
	msgbox PortSolace_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

PortSolace_Gym_EventScript_Rematch::
	lock
	faceplayer
	msgbox PortSolace_Gym_EventScript_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PortSolace_Gym_EventScript_RematchNo
	cleartrainerflag TRAINER_SOL_LEADER_MARINA
	trainerbattle_single TRAINER_SOL_LEADER_MARINA, PortSolace_Gym_EventScript_Text_RematchIntro, PortSolace_Gym_EventScript_Text_Defeat, PortSolace_Gym_EventScript_RematchWon, NO_MUSIC
	release
	end

PortSolace_Gym_EventScript_RematchWon::
	msgbox PortSolace_Gym_EventScript_Text_RematchDone, MSGBOX_DEFAULT
	release
	end

PortSolace_Gym_EventScript_RematchNo::
	msgbox PortSolace_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

PortSolace_Gym_EventScript_Guide::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox PortSolace_Gym_EventScript_Text_Guide, MSGBOX_DEFAULT
	release
	end

PortSolace_Gym_EventScript_Text_Intro:
	.string "I am MARINA, keeper of PORT SOLACE.\n"
	.string "Every trainer from HOENN washes\l"
	.string "up here first. Let's see what\l"
	.string "the tide brought in!$"

PortSolace_Gym_EventScript_Text_Defeat:
	.string "Well fought! You've earned this.$"

PortSolace_Gym_EventScript_Text_GotBadge:
	.string "{PLAYER} received the\n"
	.string "TIDE BADGE from MARINA!$"

PortSolace_Gym_EventScript_Text_BadgeInfo:
	.string "That makes {STR_VAR_1} SOLMARA badges.\n"
	.string "Every leader you face from now\l"
	.string "on will battle a little harder.\p"
	.string "And take this, too!$"

PortSolace_Gym_EventScript_Text_After:
	.string "Come back with your friends\n"
	.string "any time. A rematch keeps us\l"
	.string "all sharp!$"

PortSolace_Gym_EventScript_Text_RematchAsk:
	.string "Back for more? My team has grown\n"
	.string "along with your badges. Battle?$"

PortSolace_Gym_EventScript_Text_RematchIntro:
	.string "Then show me how much stronger\n"
	.string "you've become!$"

PortSolace_Gym_EventScript_Text_RematchDone:
	.string "Even better than last time!$"

PortSolace_Gym_EventScript_Text_Guide:
	.string "Yo, challenger! This is the PORT SOLACE GYM.\p"
	.string "You have {STR_VAR_1} SOLMARA badges.\n"
	.string "The leader's team grows with\l"
	.string "every badge you hold.\p"
	.string "Lots of room in here, so bring\n"
	.string "your friends along!$"

PortSolace_Gym_EventScript_Swimmer::
	trainerbattle_single TRAINER_SOL_GYM_MARINA_SWIMMER, PortSolace_Gym_EventScript_Swimmer_Intro, PortSolace_Gym_EventScript_Swimmer_Defeat
	msgbox PortSolace_Gym_EventScript_Swimmer_Post, MSGBOX_AUTOCLOSE
	end

PortSolace_Gym_EventScript_Swimmer_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

PortSolace_Gym_EventScript_Swimmer_Defeat:
	.string "Go on, then!$"

PortSolace_Gym_EventScript_Swimmer_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

PortSolace_Gym_EventScript_Tuber::
	trainerbattle_single TRAINER_SOL_GYM_MARINA_TUBER, PortSolace_Gym_EventScript_Tuber_Intro, PortSolace_Gym_EventScript_Tuber_Defeat
	msgbox PortSolace_Gym_EventScript_Tuber_Post, MSGBOX_AUTOCLOSE
	end

PortSolace_Gym_EventScript_Tuber_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

PortSolace_Gym_EventScript_Tuber_Defeat:
	.string "Go on, then!$"

PortSolace_Gym_EventScript_Tuber_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

