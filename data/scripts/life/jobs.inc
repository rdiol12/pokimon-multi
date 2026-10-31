@ Pokémon jobs: the job agent in the Mauville Pokémon Center, and R > JOBS.

Life_EventScript_JobAgent::
	lock
	faceplayer
Life_EventScript_JobCollect:
	callnative LifeJobs_Collect
	goto_if_eq VAR_RESULT, 0, Life_EventScript_JobMenu
	playfanfare MUS_LEVEL_UP
	msgbox Life_Text_JobBack, MSGBOX_DEFAULT
	waitfanfare
	call_if_eq VAR_0x8006, TRUE, Life_EventScript_JobGreat
	call_if_ne VAR_0x8007, 0, Life_EventScript_JobGrew
	call_if_eq VAR_RESULT, 2, Life_EventScript_JobToPC
	goto Life_EventScript_JobCollect

Life_EventScript_JobGreat:
	msgbox Life_Text_JobGreat, MSGBOX_DEFAULT
	return

Life_EventScript_JobGrew:
	buffernumberstring STR_VAR_2, VAR_0x8007
	msgbox Life_Text_JobGrew, MSGBOX_DEFAULT
	return

Life_EventScript_JobToPC:
	msgbox Life_Text_JobToPC, MSGBOX_DEFAULT
	return

Life_EventScript_JobMenu:
	message Life_Text_JobWelcome
	waitmessage
	dynmultipush Life_Text_JobSend, 0
	dynmultipush Life_Text_JobSee, 1
	dynmultipush Life_Text_JobOptBye, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Life_EventScript_JobPick
	case 1, Life_EventScript_JobSee
	goto Life_EventScript_JobBye

Life_EventScript_JobSee:
	closemessage
	callnative LifeJobs_ShowList, waitstate=1
	goto Life_EventScript_JobMenu

Life_EventScript_JobPick:
	callnative LifeJobs_HasFreeSlot
	goto_if_eq VAR_RESULT, FALSE, Life_EventScript_JobFull
	message Life_Text_JobWhich
	waitmessage
	dynmultipush Life_Text_JobBerryFarm, LIFE_JOB_BERRY_FARM
	dynmultipush Life_Text_JobDelivery, LIFE_JOB_DELIVERY
	dynmultipush Life_Text_JobLifeguard, LIFE_JOB_LIFEGUARD
	dynmultipush Life_Text_JobPowerPlant, LIFE_JOB_POWER_PLANT
	dynmultipush Life_Text_JobCaveSurvey, LIFE_JOB_CAVE_SURVEY
	dynmultipush Life_Text_JobNightWatch, LIFE_JOB_NIGHT_WATCH
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Life_EventScript_JobMenu
	copyvar VAR_0x8005, VAR_RESULT
	callnative LifeJobs_Describe
	msgbox Life_Text_JobAbout, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Life_EventScript_JobMenu
	special ChoosePartyMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, Life_EventScript_JobMenu
	callnative LifeJobs_Send
	goto_if_eq VAR_RESULT, LIFE_JOB_EGG, Life_EventScript_JobEgg
	goto_if_eq VAR_RESULT, LIFE_JOB_LAST_MON, Life_EventScript_JobLastMon
	playse SE_EXIT
	msgbox Life_Text_JobSent, MSGBOX_DEFAULT
	release
	end

Life_EventScript_JobFull:
	msgbox Life_Text_JobFull, MSGBOX_DEFAULT
	goto Life_EventScript_JobMenu

Life_EventScript_JobEgg:
	msgbox Life_Text_JobEgg, MSGBOX_DEFAULT
	goto Life_EventScript_JobMenu

Life_EventScript_JobLastMon:
	msgbox Life_Text_JobLastMon, MSGBOX_DEFAULT
	goto Life_EventScript_JobMenu

Life_EventScript_JobBye:
	msgbox Life_Text_JobBye, MSGBOX_DEFAULT
	release
	end

Life_EventScript_JobList::
	callnative LifeJobs_ShowList, waitstate=1
	releaseall
	end

Life_Text_MenuJobs:
	.string "JOBS$"

Life_Text_JobWelcome:
	.string "POKéMON JOB OFFICE! Send a Pokémon\n"
	.string "out to work and it'll earn its keep!$"

Life_Text_JobSend:
	.string "SEND ON A JOB$"
Life_Text_JobSee:
	.string "SEE JOBS$"
Life_Text_JobOptBye:
	.string "BYE$"

Life_Text_JobWhich:
	.string "Which job? Longer jobs pay more.$"

Life_Text_JobBerryFarm:
	.string "BERRY FARM (1h)$"
Life_Text_JobDelivery:
	.string "DELIVERY (2h)$"
Life_Text_JobLifeguard:
	.string "LIFEGUARD (2h)$"
Life_Text_JobPowerPlant:
	.string "POWER PLANT (4h)$"
Life_Text_JobCaveSurvey:
	.string "CAVE SURVEY (4h)$"
Life_Text_JobNightWatch:
	.string "NIGHT WATCH (8h)$"

Life_Text_JobAbout:
	.string "{STR_VAR_1} takes {STR_VAR_2} hour(s).\p"
	.string "Best for {STR_VAR_3}\n"
	.string "types. Send one of your Pokémon?$"

Life_Text_JobSent:
	.string "{STR_VAR_1} set off for\n"
	.string "{STR_VAR_2}!\p"
	.string "Come back in {STR_VAR_3} hour(s).$"

Life_Text_JobFull:
	.string "Three of your Pokémon are already\n"
	.string "out working. Let them finish first!$"

Life_Text_JobEgg:
	.string "An EGG can't work, silly!$"

Life_Text_JobLastMon:
	.string "That's your only Pokémon that can\n"
	.string "battle! Keep it with you.$"

Life_Text_JobBye:
	.string "Hard work makes strong Pokémon!$"

Life_Text_JobBack:
	.string "{STR_VAR_1} is back from\n"
	.string "{STR_VAR_2}!\p"
	.string "It brought back\n"
	.string "{STR_VAR_3}!$"

Life_Text_JobGreat:
	.string "It was a natural at that job, so\n"
	.string "it earned double!$"

Life_Text_JobGrew:
	.string "{STR_VAR_1} grew to Lv. {STR_VAR_2}!$"

Life_Text_JobToPC:
	.string "Your party was full, so it went\n"
	.string "to the PC.$"
