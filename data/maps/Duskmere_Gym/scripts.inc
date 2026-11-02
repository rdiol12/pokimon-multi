Duskmere_Gym_MapScripts::
	.byte 0

Duskmere_Gym_EventScript_Leader::
	goto_if_set FLAG_SOL_BADGE_6, Duskmere_Gym_EventScript_Rematch
	trainerbattle_single TRAINER_SOL_LEADER_MORWYN, Duskmere_Gym_EventScript_Text_Intro, Duskmere_Gym_EventScript_Text_Defeat, Duskmere_Gym_EventScript_Won, NO_MUSIC
	release
	end

Duskmere_Gym_EventScript_Won::
	message Duskmere_Gym_EventScript_Text_GotBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_SOL_BADGE_6
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Duskmere_Gym_EventScript_Text_BadgeInfo, MSGBOX_DEFAULT
	giveitem ITEM_TM_SHADOW_BALL
	msgbox Duskmere_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Duskmere_Gym_EventScript_Rematch::
	lock
	faceplayer
	msgbox Duskmere_Gym_EventScript_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Duskmere_Gym_EventScript_RematchNo
	cleartrainerflag TRAINER_SOL_LEADER_MORWYN
	trainerbattle_single TRAINER_SOL_LEADER_MORWYN, Duskmere_Gym_EventScript_Text_RematchIntro, Duskmere_Gym_EventScript_Text_Defeat, Duskmere_Gym_EventScript_RematchWon, NO_MUSIC
	release
	end

Duskmere_Gym_EventScript_RematchWon::
	msgbox Duskmere_Gym_EventScript_Text_RematchDone, MSGBOX_DEFAULT
	release
	end

Duskmere_Gym_EventScript_RematchNo::
	msgbox Duskmere_Gym_EventScript_Text_After, MSGBOX_DEFAULT
	release
	end

Duskmere_Gym_EventScript_Guide::
	lock
	faceplayer
	callnative Solmara_GetBadgeCount
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Duskmere_Gym_EventScript_Text_Guide, MSGBOX_DEFAULT
	release
	end

Duskmere_Gym_EventScript_Text_Intro:
	.string "The mist told me you were coming.\n"
	.string "I am MORWYN.\l"
	.string "Let us see if your spirit is as\l"
	.string "strong as your team...$"

Duskmere_Gym_EventScript_Text_Defeat:
	.string "Well fought! You've earned this.$"

Duskmere_Gym_EventScript_Text_GotBadge:
	.string "{PLAYER} received the\n"
	.string "MIST BADGE from MORWYN!$"

Duskmere_Gym_EventScript_Text_BadgeInfo:
	.string "That makes {STR_VAR_1} SOLMARA badges.\n"
	.string "Every leader you face from now\l"
	.string "on will battle a little harder.\p"
	.string "And take this, too!$"

Duskmere_Gym_EventScript_Text_After:
	.string "Come back with your friends\n"
	.string "any time. A rematch keeps us\l"
	.string "all sharp!$"

Duskmere_Gym_EventScript_Text_RematchAsk:
	.string "Back for more? My team has grown\n"
	.string "along with your badges. Battle?$"

Duskmere_Gym_EventScript_Text_RematchIntro:
	.string "Then show me how much stronger\n"
	.string "you've become!$"

Duskmere_Gym_EventScript_Text_RematchDone:
	.string "Even better than last time!$"

Duskmere_Gym_EventScript_Text_Guide:
	.string "Yo, challenger! This is the DUSKMERE GYM.\p"
	.string "You have {STR_VAR_1} SOLMARA badges.\n"
	.string "The leader's team grows with\l"
	.string "every badge you hold.\p"
	.string "Lots of room in here, so bring\n"
	.string "your friends along!$"

Duskmere_Gym_EventScript_Hex::
	trainerbattle_single TRAINER_SOL_GYM_MORWYN_HEX, Duskmere_Gym_EventScript_Hex_Intro, Duskmere_Gym_EventScript_Hex_Defeat
	msgbox Duskmere_Gym_EventScript_Hex_Post, MSGBOX_AUTOCLOSE
	end

Duskmere_Gym_EventScript_Hex_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Duskmere_Gym_EventScript_Hex_Defeat:
	.string "Go on, then!$"

Duskmere_Gym_EventScript_Hex_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

Duskmere_Gym_EventScript_Psychic::
	trainerbattle_single TRAINER_SOL_GYM_MORWYN_PSYCHIC, Duskmere_Gym_EventScript_Psychic_Intro, Duskmere_Gym_EventScript_Psychic_Defeat
	msgbox Duskmere_Gym_EventScript_Psychic_Post, MSGBOX_AUTOCLOSE
	end

Duskmere_Gym_EventScript_Psychic_Intro:
	.string "Think you can reach the leader?\n"
	.string "Get past me first!$"

Duskmere_Gym_EventScript_Psychic_Defeat:
	.string "Go on, then!$"

Duskmere_Gym_EventScript_Psychic_Post:
	.string "The leader is just ahead. Good\n"
	.string "luck!$"

