Gearholt_Gym_MapScripts::
	.byte 0

Gearholt_Gym_EventScript_Leader::
	goto_if_set FLAG_SOL_BADGE_2, Gearholt_Gym_EventScript_Rematch
	trainerbattle_single TRAINER_SOL_LEADER_BRANT, Gearholt_Gym_EventScript_Text_Intro, Gearholt_Gym_EventScript_Text_Defeat, Gearholt_Gym_EventScript_Won, NO_MUSIC
	release
	end

Gearholt_Gym_EventScript_Won::
	message Gearholt_Gym_EventScript_Text_GotBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_SOL_BADGE_2
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Gearholt_Gym_EventScript_Text_BadgeInfo, MSGBOX_DEFAULT
	giveitem ITEM_TM_STEEL_WING
	msgbox Gearholt_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Gearholt_Gym_EventScript_Rematch::
	lock
	faceplayer
	msgbox Gearholt_Gym_EventScript_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Gearholt_Gym_EventScript_RematchNo
	cleartrainerflag TRAINER_SOL_LEADER_BRANT
	trainerbattle_single TRAINER_SOL_LEADER_BRANT, Gearholt_Gym_EventScript_Text_RematchIntro, Gearholt_Gym_EventScript_Text_Defeat, Gearholt_Gym_EventScript_RematchWon, NO_MUSIC
	release
	end

Gearholt_Gym_EventScript_RematchWon::
	msgbox Gearholt_Gym_EventScript_Text_RematchDone, MSGBOX_DEFAULT
	release
	end

Gearholt_Gym_EventScript_RematchNo::
	msgbox Gearholt_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Gearholt_Gym_EventScript_Guide::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Gearholt_Gym_EventScript_Text_Guide, MSGBOX_DEFAULT
	release
	end

Gearholt_Gym_EventScript_Text_Intro:
	.string "BRANT's the name. I forge steel\n"
	.string "and trainers.\l"
	.string "Let's see if you hold your shape\l"
	.string "under the hammer!$"

Gearholt_Gym_EventScript_Text_Defeat:
	.string "Well fought! You've earned this.$"

Gearholt_Gym_EventScript_Text_GotBadge:
	.string "{PLAYER} received the\n"
	.string "ANVIL BADGE from BRANT!$"

Gearholt_Gym_EventScript_Text_BadgeInfo:
	.string "That makes {STR_VAR_1} SOLMARA badges.\n"
	.string "Every leader you face from now\l"
	.string "on will battle a little harder.\p"
	.string "And take this, too!$"

Gearholt_Gym_EventScript_Text_After:
	.string "Come back with your friends\n"
	.string "any time. A rematch keeps us\l"
	.string "all sharp!$"

Gearholt_Gym_EventScript_Text_RematchAsk:
	.string "Back for more? My team has grown\n"
	.string "along with your badges. Battle?$"

Gearholt_Gym_EventScript_Text_RematchIntro:
	.string "Then show me how much stronger\n"
	.string "you've become!$"

Gearholt_Gym_EventScript_Text_RematchDone:
	.string "Even better than last time!$"

Gearholt_Gym_EventScript_Text_Guide:
	.string "Yo, challenger! This is the GEARHOLT GYM.\p"
	.string "You have {STR_VAR_1} SOLMARA badges.\n"
	.string "The leader's team grows with\l"
	.string "every badge you hold.\p"
	.string "Lots of room in here, so bring\n"
	.string "your friends along!$"

Gearholt_Gym_EventScript_Worker::
	trainerbattle_single TRAINER_SOL_GYM_BRANT_WORKER, Gearholt_Gym_EventScript_Worker_Intro, Gearholt_Gym_EventScript_Worker_Defeat
	msgbox Gearholt_Gym_EventScript_Worker_Post, MSGBOX_AUTOCLOSE
	end

Gearholt_Gym_EventScript_Worker_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Gearholt_Gym_EventScript_Worker_Defeat:
	.string "Go on, then!$"

Gearholt_Gym_EventScript_Worker_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

Gearholt_Gym_EventScript_Engineer::
	trainerbattle_single TRAINER_SOL_GYM_BRANT_ENGINEER, Gearholt_Gym_EventScript_Engineer_Intro, Gearholt_Gym_EventScript_Engineer_Defeat
	msgbox Gearholt_Gym_EventScript_Engineer_Post, MSGBOX_AUTOCLOSE
	end

Gearholt_Gym_EventScript_Engineer_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Gearholt_Gym_EventScript_Engineer_Defeat:
	.string "Go on, then!$"

Gearholt_Gym_EventScript_Engineer_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

