Drakespire_Gym_MapScripts::
	.byte 0

Drakespire_Gym_EventScript_Leader::
	goto_if_set FLAG_SOL_BADGE_8, Drakespire_Gym_EventScript_Rematch
	trainerbattle_single TRAINER_SOL_LEADER_VALEN, Drakespire_Gym_EventScript_Text_Intro, Drakespire_Gym_EventScript_Text_Defeat, Drakespire_Gym_EventScript_Won, NO_MUSIC
	release
	end

Drakespire_Gym_EventScript_Won::
	message Drakespire_Gym_EventScript_Text_GotBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_SOL_BADGE_8
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Drakespire_Gym_EventScript_Text_BadgeInfo, MSGBOX_DEFAULT
	giveitem ITEM_TM_DRAGON_CLAW
	msgbox Drakespire_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Drakespire_Gym_EventScript_Rematch::
	lock
	faceplayer
	msgbox Drakespire_Gym_EventScript_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Drakespire_Gym_EventScript_RematchNo
	cleartrainerflag TRAINER_SOL_LEADER_VALEN
	trainerbattle_single TRAINER_SOL_LEADER_VALEN, Drakespire_Gym_EventScript_Text_RematchIntro, Drakespire_Gym_EventScript_Text_Defeat, Drakespire_Gym_EventScript_RematchWon, NO_MUSIC
	release
	end

Drakespire_Gym_EventScript_RematchWon::
	msgbox Drakespire_Gym_EventScript_Text_RematchDone, MSGBOX_DEFAULT
	release
	end

Drakespire_Gym_EventScript_RematchNo::
	msgbox Drakespire_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Drakespire_Gym_EventScript_Guide::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Drakespire_Gym_EventScript_Text_Guide, MSGBOX_DEFAULT
	release
	end

Drakespire_Gym_EventScript_Text_Intro:
	.string "I am VALEN. Dragons have nested on\n"
	.string "this spire for a thousand years.\l"
	.string "Face their fury!$"

Drakespire_Gym_EventScript_Text_Defeat:
	.string "Well fought! You've earned this.$"

Drakespire_Gym_EventScript_Text_GotBadge:
	.string "{PLAYER} received the\n"
	.string "WYRM BADGE from VALEN!$"

Drakespire_Gym_EventScript_Text_BadgeInfo:
	.string "That makes {STR_VAR_1} SOLMARA badges.\n"
	.string "Every leader you face from now\l"
	.string "on will battle a little harder.\p"
	.string "And take this, too!$"

Drakespire_Gym_EventScript_Text_After:
	.string "Come back with your friends\n"
	.string "any time. A rematch keeps us\l"
	.string "all sharp!$"

Drakespire_Gym_EventScript_Text_RematchAsk:
	.string "Back for more? My team has grown\n"
	.string "along with your badges. Battle?$"

Drakespire_Gym_EventScript_Text_RematchIntro:
	.string "Then show me how much stronger\n"
	.string "you've become!$"

Drakespire_Gym_EventScript_Text_RematchDone:
	.string "Even better than last time!$"

Drakespire_Gym_EventScript_Text_Guide:
	.string "Yo, challenger! This is the DRAKESPIRE GYM.\p"
	.string "You have {STR_VAR_1} SOLMARA badges.\n"
	.string "The leader's team grows with\l"
	.string "every badge you hold.\p"
	.string "Lots of room in here, so bring\n"
	.string "your friends along!$"

Drakespire_Gym_EventScript_Tamer::
	trainerbattle_single TRAINER_SOL_GYM_VALEN_TAMER, Drakespire_Gym_EventScript_Tamer_Intro, Drakespire_Gym_EventScript_Tamer_Defeat
	msgbox Drakespire_Gym_EventScript_Tamer_Post, MSGBOX_AUTOCLOSE
	end

Drakespire_Gym_EventScript_Tamer_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Drakespire_Gym_EventScript_Tamer_Defeat:
	.string "Go on, then!$"

Drakespire_Gym_EventScript_Tamer_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

Drakespire_Gym_EventScript_Coolm::
	trainerbattle_single TRAINER_SOL_GYM_VALEN_COOLM, Drakespire_Gym_EventScript_Coolm_Intro, Drakespire_Gym_EventScript_Coolm_Defeat
	msgbox Drakespire_Gym_EventScript_Coolm_Post, MSGBOX_AUTOCLOSE
	end

Drakespire_Gym_EventScript_Coolm_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Drakespire_Gym_EventScript_Coolm_Defeat:
	.string "Go on, then!$"

Drakespire_Gym_EventScript_Coolm_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

