@ Online contests: START > ONLINE > COMMUNITY > CONTEST.
@ The server is server/community_contest.py; the results are drawn by
@ src/community_contest.c.

	.set CONTEST_WON, 0

Community_EventScript_Contest::
	lockall
	msgbox Community_Text_ContestWhich, MSGBOX_DEFAULT
	dynmultipush Community_Text_ContestCool, 0
	dynmultipush Community_Text_ContestBeauty, 1
	dynmultipush Community_Text_ContestCute, 2
	dynmultipush Community_Text_ContestSmart, 3
	dynmultipush Community_Text_ContestTough, 4
	dynmultistack 0, 0, FALSE, 5, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Community_EventScript_ContestQuit
	copyvar VAR_0x8009, VAR_RESULT
	msgbox Community_Text_ContestWhichMon, MSGBOX_DEFAULT
	closemessage
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, Community_EventScript_Done
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, Community_EventScript_ContestNoEggs
	copyvar VAR_0x8005, VAR_0x8009
	callnative CommunityContest_Enter, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	copyvar VAR_0x8008, VAR_RESULT
	playbgm MUS_CONTEST_RESULTS, FALSE
	callnative CommunityContest_ShowResults, waitstate=1
	goto_if_ne VAR_0x8008, CONTEST_WON, Community_EventScript_ContestNextTime
	callnative CommunityContest_GiveRibbon
	goto_if_eq VAR_RESULT, FALSE, Community_EventScript_ContestWonAgain
	playfanfare MUS_OBTAIN_ITEM
	msgbox Community_Text_ContestRibbon, MSGBOX_DEFAULT
	waitfanfare
	fadedefaultbgm
	releaseall
	end

Community_EventScript_ContestWonAgain:
	msgbox Community_Text_ContestWonAgain, MSGBOX_DEFAULT
	fadedefaultbgm
	releaseall
	end

Community_EventScript_ContestNextTime:
	msgbox Community_Text_ContestNextTime, MSGBOX_DEFAULT
	fadedefaultbgm
	releaseall
	end

Community_EventScript_ContestNoEggs:
	msgbox Community_Text_ContestNoEggs, MSGBOX_DEFAULT
Community_EventScript_ContestQuit:
	releaseall
	end

Community_Text_ContestWhich:
	.string "Which contest will you enter?$"

Community_Text_ContestCool:
	.string "COOLNESS$"
Community_Text_ContestBeauty:
	.string "BEAUTY$"
Community_Text_ContestCute:
	.string "CUTENESS$"
Community_Text_ContestSmart:
	.string "SMARTNESS$"
Community_Text_ContestTough:
	.string "TOUGHNESS$"

Community_Text_ContestWhichMon:
	.string "Which POKéMON will you enter?\n"
	.string "It goes up against other players'.$"

Community_Text_ContestNoEggs:
	.string "An EGG can't enter a contest.$"

Community_Text_ContestRibbon:
	.string "{STR_VAR_1} won the MASTER RANK\n"
	.string "RIBBON!$"

Community_Text_ContestWonAgain:
	.string "{STR_VAR_1} won again!\n"
	.string "It already has this RIBBON.$"

Community_Text_ContestNextTime:
	.string "Better luck next time!\n"
	.string "POKéBLOCKS raise a POKéMON's condition.$"
