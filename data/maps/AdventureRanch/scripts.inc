@ The Hoenn Ranch behind the Route 117 Day Care (src/ranch.c). The
@ Pokémon in the RANCH box live here.

AdventureRanch_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, AdventureRanch_OnTransition
	.byte 0

AdventureRanch_OnTransition:
	callnative Ranch_SetupMap
	end

AdventureRanch_EventScript_Rancher::
	lock
	faceplayer
	callnative Ranch_Collect
	switch VAR_RESULT
	case RANCH_TALK_FIRST, AdventureRanch_EventScript_FirstVisit
	case RANCH_TALK_EMPTY, AdventureRanch_EventScript_Empty
	case RANCH_TALK_NOTHING_YET, AdventureRanch_EventScript_NothingYet
	msgbox AdventureRanch_Text_Haul, MSGBOX_DEFAULT
	callnative Ranch_ShowHaul, waitstate=1
	msgbox AdventureRanch_Text_ComeAgain, MSGBOX_DEFAULT
	release
	end

AdventureRanch_EventScript_FirstVisit:
	msgbox AdventureRanch_Text_FirstVisit, MSGBOX_DEFAULT
	release
	end

AdventureRanch_EventScript_Empty:
	msgbox AdventureRanch_Text_Empty, MSGBOX_DEFAULT
	release
	end

AdventureRanch_EventScript_NothingYet:
	msgbox AdventureRanch_Text_NothingYet, MSGBOX_DEFAULT
	release
	end

AdventureRanch_EventScript_Mon1::
	setvar VAR_0x8004, 0
	goto AdventureRanch_EventScript_Mon
AdventureRanch_EventScript_Mon2::
	setvar VAR_0x8004, 1
	goto AdventureRanch_EventScript_Mon
AdventureRanch_EventScript_Mon3::
	setvar VAR_0x8004, 2
	goto AdventureRanch_EventScript_Mon
AdventureRanch_EventScript_Mon4::
	setvar VAR_0x8004, 3
	goto AdventureRanch_EventScript_Mon
AdventureRanch_EventScript_Mon5::
	setvar VAR_0x8004, 4
	goto AdventureRanch_EventScript_Mon
AdventureRanch_EventScript_Mon6::
	setvar VAR_0x8004, 5
AdventureRanch_EventScript_Mon:
	lock
	faceplayer
	callnative Ranch_TalkToMon
	msgbox AdventureRanch_Text_Mon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

AdventureRanch_EventScript_Sign::
	msgbox AdventureRanch_Text_Sign, MSGBOX_SIGN
	end

AdventureRanch_EventScript_Leave::
	lockall
	warp MAP_ROUTE117, 55, 7
	waitstate
	releaseall
	end

AdventureRanch_Text_FirstVisit:
	.string "Welcome to the HOENN RANCH!\p"
	.string "The DAY CARE raises POKéMON. Here,\n"
	.string "we let them roam free and forage.\p"
	.string "Put POKéMON in the last box of your\n"
	.string "PC. I've named it RANCH for you.\p"
	.string "They'll live here and find things\n"
	.string "while you're out: BERRIES, APRICORNS,\l"
	.string "SHARDS… whatever they're good at.\p"
	.string "Every few hours, come and see me.\n"
	.string "I'll hold on to a day's worth.$"

AdventureRanch_Text_Empty:
	.string "The RANCH box in your PC is empty.\p"
	.string "Put some POKéMON in it and they'll\n"
	.string "come and live here!$"

AdventureRanch_Text_NothingYet:
	.string "Your POKéMON are still out looking.\n"
	.string "Come back in a few hours!$"

AdventureRanch_Text_Haul:
	.string "Welcome back! Your POKéMON found\n"
	.string "{STR_VAR_1} things while you were away.$"

AdventureRanch_Text_ComeAgain:
	.string "It's all in your BAG. They'll keep\n"
	.string "looking while you're gone!$"

AdventureRanch_Text_Mon:
	.string "{STR_VAR_1} {STR_VAR_2}$"

AdventureRanch_Text_Sign:
	.string "HOENN RANCH\n"
	.string "POKéMON in the RANCH box live here.$"
