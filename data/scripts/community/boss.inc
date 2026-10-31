@ The server boss: START > ONLINE > COMMUNITY > BOSS.
@ The server is server/community_boss.py; the window and battle are in
@ src/community_boss.c.

	.set BOSS_ACTIVE, 2
	.set BOSS_REWARD, 3

Community_EventScript_Boss::
	lockall
	setvar VAR_0x8004, COMMUNITY_REQ_BOSS_STATUS
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	goto_if_eq VAR_RESULT, BOSS_REWARD, Community_EventScript_BossReward
	callnative CommunityBoss_ShowStatus, waitstate=1
	goto_if_eq VAR_RESULT, FALSE, Community_EventScript_Done
	callnative CommunityBoss_SetUpBattle
	dowildbattle
	callnative CommunityBoss_ReportDamage, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	callnative Community_BufferRows
	msgbox Community_Text_BossDamage, MSGBOX_DEFAULT
	releaseall
	end

Community_EventScript_BossReward:
	callnative CommunityBoss_ShowStatus, waitstate=1
	setvar VAR_0x8004, COMMUNITY_REQ_BOSS_CLAIM
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	msgbox Community_Text_BossThanks, MSGBOX_DEFAULT
	setvar VAR_0x8004, 0
	callnative Community_GetRespArg
	copyvar VAR_0x8006, VAR_RESULT
	setvar VAR_0x8004, 1
	callnative Community_GetRespArg
	copyvar VAR_0x8007, VAR_RESULT
	giveitem VAR_0x8006, VAR_0x8007
	setvar VAR_0x8004, 2
	callnative Community_GetRespArg
	goto_if_eq VAR_RESULT, 0, Community_EventScript_Done
	copyvar VAR_0x8006, VAR_RESULT
	setvar VAR_0x8004, 3
	callnative Community_GetRespArg
	copyvar VAR_0x8007, VAR_RESULT
	giveitem VAR_0x8006, VAR_0x8007
	releaseall
	end

@ Shown to everyone online when the boss falls (src/community_boss.c).
Community_EventScript_BossFell::
	lockall
	playfanfare MUS_OBTAIN_B_POINTS
	msgbox Community_Text_BossFell, MSGBOX_DEFAULT
	waitfanfare
	releaseall
	end

Community_Text_BossDamage:
	.string "{STR_VAR_1}\n"
	.string "{STR_VAR_2}$"

Community_Text_BossThanks:
	.string "Thanks for fighting the boss!\n"
	.string "Here's your share of the reward.$"

Community_Text_BossFell:
	.string "SERVER BOSS: {STR_VAR_1} brought down\n"
	.string "the giant {STR_VAR_2}!\p"
	.string "Everyone who fought it can claim\n"
	.string "a reward in COMMUNITY > BOSS.$"
