@ The Haven Reserve (src/reserve.c, docs/reserve.md): the guard at the
@ Verdanturf gate, the warden who takes Pokémon in, the roaming Pokémon,
@ the guards and the notice board.

@ --- Verdanturf Town ----------------------------------------------------------

VerdanturfTown_EventScript_ReserveGuard::
	lock
	faceplayer
	msgbox Reserve_Text_GateGuard, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VerdanturfTown_EventScript_ReserveGuardNo
	msgbox Reserve_Text_GateGuardCheck, MSGBOX_DEFAULT
	closemessage
	callnative Reserve_CheckOnline
	goto_if_eq VAR_RESULT, FALSE, VerdanturfTown_EventScript_ReserveGo
	callnative Reserve_FetchList, waitstate=1
VerdanturfTown_EventScript_ReserveGo:
	callnative Reserve_BuildList
	warp MAP_HAVEN_RESERVE, 17, 28
	waitstate
	release
	end

VerdanturfTown_EventScript_ReserveGuardNo:
	msgbox Reserve_Text_GateGuardNo, MSGBOX_DEFAULT
	release
	end

@ --- The way out -----------------------------------------------------------------

HavenReserve_EventScript_Leave::
	lockall
	warp MAP_VERDANTURF_TOWN, 14, 10
	waitstate
	releaseall
	end

@ --- The warden: leave a Pokémon -------------------------------------------------

HavenReserve_EventScript_Warden::
	lock
	faceplayer
	msgbox Reserve_Text_Warden, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HavenReserve_EventScript_WardenNo
	msgbox Reserve_Text_WardenWhich, MSGBOX_DEFAULT
	closemessage
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, HavenReserve_EventScript_WardenNo
	callnative Reserve_CanLeave
	goto_if_eq VAR_RESULT, RESERVE_LEAVE_EGG, HavenReserve_EventScript_WardenEgg
	goto_if_eq VAR_RESULT, RESERVE_LEAVE_LAST, HavenReserve_EventScript_WardenLast
	msgbox Reserve_Text_WardenSure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HavenReserve_EventScript_WardenNo
	callnative Reserve_CheckOnline
	goto_if_eq VAR_RESULT, FALSE, HavenReserve_EventScript_WardenOffline
	callnative Reserve_LeaveOnline, waitstate=1
	goto_if_eq VAR_RESULT, RESERVE_FULL, HavenReserve_EventScript_WardenFull
	goto_if_ne VAR_RESULT, COMMUNITY_OK, HavenReserve_EventScript_WardenOffline
	callnative Reserve_ShowLeftOnline
	callnative Reserve_FinishLeave
	goto HavenReserve_EventScript_WardenLeft

HavenReserve_EventScript_WardenOffline:
	callnative Reserve_LeaveOffline
	goto_if_eq VAR_RESULT, RESERVE_LEAVE_LOCAL_FULL, HavenReserve_EventScript_WardenLocalFull
	callnative Reserve_FinishLeave
	msgbox Reserve_Text_WardenOfflineNote, MSGBOX_DEFAULT
HavenReserve_EventScript_WardenLeft:
	msgbox Reserve_Text_WardenLeft, MSGBOX_DEFAULT
	closemessage
	goto_if_eq VAR_0x8006, 0, HavenReserve_EventScript_WardenEnd
	addobject VAR_0x8006
HavenReserve_EventScript_WardenEnd:
	release
	end

HavenReserve_EventScript_WardenNo:
	msgbox Reserve_Text_WardenNo, MSGBOX_DEFAULT
	release
	end

HavenReserve_EventScript_WardenEgg:
	msgbox Reserve_Text_WardenEgg, MSGBOX_DEFAULT
	release
	end

HavenReserve_EventScript_WardenLast:
	msgbox Reserve_Text_WardenLast, MSGBOX_DEFAULT
	release
	end

HavenReserve_EventScript_WardenFull:
	msgbox Reserve_Text_WardenFull, MSGBOX_DEFAULT
	release
	end

HavenReserve_EventScript_WardenLocalFull:
	msgbox Reserve_Text_WardenLocalFull, MSGBOX_DEFAULT
	release
	end

@ --- The roaming Pokémon ---------------------------------------------------------

HavenReserve_EventScript_Roamer1::
	setvar VAR_0x8004, 0
	goto HavenReserve_EventScript_Roamer
HavenReserve_EventScript_Roamer2::
	setvar VAR_0x8004, 1
	goto HavenReserve_EventScript_Roamer
HavenReserve_EventScript_Roamer3::
	setvar VAR_0x8004, 2
	goto HavenReserve_EventScript_Roamer
HavenReserve_EventScript_Roamer4::
	setvar VAR_0x8004, 3
	goto HavenReserve_EventScript_Roamer
HavenReserve_EventScript_Roamer5::
	setvar VAR_0x8004, 4
	goto HavenReserve_EventScript_Roamer
HavenReserve_EventScript_Roamer6::
	setvar VAR_0x8004, 5
	goto HavenReserve_EventScript_Roamer
HavenReserve_EventScript_Roamer7::
	setvar VAR_0x8004, 6
	goto HavenReserve_EventScript_Roamer
HavenReserve_EventScript_Roamer8::
	setvar VAR_0x8004, 7
HavenReserve_EventScript_Roamer:
	lock
	faceplayer
	callnative Reserve_DescribeRoamer
	goto_if_eq VAR_RESULT, RESERVE_KIND_NONE, HavenReserve_EventScript_RoamerEnd
	waitmoncry
	switch VAR_RESULT
	case RESERVE_KIND_STRAY, HavenReserve_EventScript_RoamerStray
	case RESERVE_KIND_MINE, HavenReserve_EventScript_RoamerMine
	case RESERVE_KIND_LOCAL, HavenReserve_EventScript_RoamerMine
	msgbox Reserve_Text_RoamerShared, MSGBOX_DEFAULT
	goto HavenReserve_EventScript_RoamerAsk

HavenReserve_EventScript_RoamerStray:
	msgbox Reserve_Text_RoamerStray, MSGBOX_DEFAULT
	goto HavenReserve_EventScript_RoamerAsk

HavenReserve_EventScript_RoamerMine:
	msgbox Reserve_Text_RoamerMine, MSGBOX_DEFAULT
HavenReserve_EventScript_RoamerAsk:
	msgbox Reserve_Text_RoamerChallenge, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HavenReserve_EventScript_RoamerEnd
	callnative Reserve_IsShared
	goto_if_eq VAR_RESULT, FALSE, HavenReserve_EventScript_RoamerBattle
	callnative Reserve_Claim, waitstate=1
	goto_if_eq VAR_RESULT, RESERVE_GONE, HavenReserve_EventScript_RoamerGone
	goto_if_ne VAR_RESULT, COMMUNITY_OK, HavenReserve_EventScript_RoamerNoServer
HavenReserve_EventScript_RoamerBattle:
	closemessage
	callnative Reserve_StartBattle, waitstate=1
	specialvar VAR_0x8005, GetBattleOutcome
	callnative Reserve_AfterBattle
	copyvar VAR_0x8009, VAR_RESULT
	callnative Reserve_IsShared
	goto_if_eq VAR_RESULT, FALSE, HavenReserve_EventScript_RoamerAfter
	callnative Reserve_ReportResult, waitstate=1
HavenReserve_EventScript_RoamerAfter:
	goto_if_eq VAR_0x8009, RESERVE_END_STILL_HERE, HavenReserve_EventScript_RoamerEnd
	removeobject VAR_0x8006
	goto_if_eq VAR_0x8009, RESERVE_END_CAUGHT, HavenReserve_EventScript_RoamerCaught
	msgbox Reserve_Text_RoamerKnockedOut, MSGBOX_DEFAULT
	goto HavenReserve_EventScript_RoamerEnd

HavenReserve_EventScript_RoamerCaught:
	msgbox Reserve_Text_RoamerCaught, MSGBOX_DEFAULT
HavenReserve_EventScript_RoamerEnd:
	release
	end

HavenReserve_EventScript_RoamerGone:
	msgbox Reserve_Text_RoamerGone, MSGBOX_DEFAULT
	closemessage
	callnative Reserve_ForgetRoamer
	removeobject VAR_0x8006
	release
	end

HavenReserve_EventScript_RoamerNoServer:
	msgbox Reserve_Text_RoamerNoServer, MSGBOX_DEFAULT
	release
	end

@ --- Guards, visitors and signs --------------------------------------------------

HavenReserve_EventScript_GuardWest::
	msgbox Reserve_Text_GuardWest, MSGBOX_NPC
	end

HavenReserve_EventScript_GuardEast::
	msgbox Reserve_Text_GuardEast, MSGBOX_NPC
	end

HavenReserve_EventScript_Patrol::
	msgbox Reserve_Text_Patrol, MSGBOX_NPC
	end

HavenReserve_EventScript_Visitor::
	msgbox Reserve_Text_Visitor, MSGBOX_NPC
	end

HavenReserve_EventScript_Sign::
	msgbox Reserve_Text_Sign, MSGBOX_SIGN
	end

HavenReserve_EventScript_Board::
	lockall
	msgbox Reserve_Text_Board, MSGBOX_DEFAULT
	callnative Reserve_CheckOnline
	goto_if_eq VAR_RESULT, FALSE, HavenReserve_EventScript_BoardShow
	callnative Reserve_FetchNews, waitstate=1
HavenReserve_EventScript_BoardShow:
	callnative Reserve_ShowBoard, waitstate=1
	releaseall
	end

@ --- The lodge -------------------------------------------------------------------

HavenReserve_EventScript_Nurse::
	lock
	faceplayer
	msgbox Reserve_Text_Nurse, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox Reserve_Text_NurseDone, MSGBOX_DEFAULT
	release
	end

HavenReserve_EventScript_HeadGuard::
	msgbox Reserve_Text_HeadGuard, MSGBOX_NPC
	end

@ --- Text ------------------------------------------------------------------------

Reserve_Text_GateGuard:
	.string "West of town is the HAVEN RESERVE.\p"
	.string "Trainers leave POKéMON they can't\n"
	.string "keep there, and we guard them.\p"
	.string "Anyone may go in and battle them.\n"
	.string "Win, and you may catch one!\p"
	.string "Shall I take you to the gate?$"

Reserve_Text_GateGuardCheck:
	.string "Let me radio in and see who's\n"
	.string "roaming the meadow today…$"

Reserve_Text_GateGuardNo:
	.string "The reserve is open day and night.\n"
	.string "Come by any time.$"

Reserve_Text_Warden:
	.string "I'm the warden of HAVEN RESERVE.\p"
	.string "If there's a POKéMON you can't keep,\n"
	.string "it can live here, free and safe.\p"
	.string "Trainers from all over HOENN come to\n"
	.string "meet them. Some may catch it.\p"
	.string "Would you like to leave one here?$"

Reserve_Text_WardenWhich:
	.string "Which POKéMON will you leave?$"

Reserve_Text_WardenSure:
	.string "You'll leave {STR_VAR_1} here for good?\p"
	.string "Other trainers may battle it, or\n"
	.string "catch it and take it home.$"

Reserve_Text_WardenLeft:
	.string "{STR_VAR_1} ran off into the meadow!\p"
	.string "Don't worry. We'll look after it.$"

Reserve_Text_WardenOfflineNote:
	.string "Our radio to the other reserves is\n"
	.string "down, so it'll stay in this meadow.$"

Reserve_Text_WardenNo:
	.string "The meadow is always open, should\n"
	.string "you change your mind.$"

Reserve_Text_WardenEgg:
	.string "An EGG? It needs someone to hatch\n"
	.string "it. Please keep it with you.$"

Reserve_Text_WardenLast:
	.string "That's the only POKéMON you have\n"
	.string "that can battle! Keep it with you.$"

Reserve_Text_WardenFull:
	.string "I'm sorry, the reserve is full.\n"
	.string "Try again after some are caught.$"

Reserve_Text_WardenLocalFull:
	.string "The meadow here is full for now.\n"
	.string "Come back once some are caught.$"

Reserve_Text_RoamerShared:
	.string "It's a {STR_VAR_1}, Lv. {STR_VAR_2}.\n"
	.string "{STR_VAR_3} left it here.$"

Reserve_Text_RoamerStray:
	.string "It's a stray {STR_VAR_1}, Lv. {STR_VAR_2}.\n"
	.string "It wandered in from the wild.$"

Reserve_Text_RoamerMine:
	.string "It's the {STR_VAR_1} you left here.\n"
	.string "It seems to remember you!$"

Reserve_Text_RoamerChallenge:
	.string "Challenge it? Win, and you may catch\n"
	.string "it. If it faints, it leaves too.$"

Reserve_Text_RoamerCaught:
	.string "{STR_VAR_1} is leaving the reserve\n"
	.string "with you. Take good care of it!$"

Reserve_Text_RoamerKnockedOut:
	.string "{STR_VAR_1} fainted. The guards carried\n"
	.string "it out of the reserve to recover.$"

Reserve_Text_RoamerGone:
	.string "Oh! {STR_VAR_1} dashed off. Another\n"
	.string "trainer must have reached it first.$"

Reserve_Text_RoamerNoServer:
	.string "{STR_VAR_1} won't come near you.\n"
	.string "Try again in a moment.$"

Reserve_Text_GuardWest:
	.string "Nobody harms a POKéMON in here.\n"
	.string "Fair battles only, trainer!$"

Reserve_Text_GuardEast:
	.string "We guard the gate day and night.\p"
	.string "Every POKéMON left here gets a\n"
	.string "home, until a trainer wins it.$"

Reserve_Text_Patrol:
	.string "I walk the fences. No poachers\n"
	.string "get into HAVEN on my watch!$"

Reserve_Text_Visitor:
	.string "I come here to visit the POKéMON\n"
	.string "people left behind.\p"
	.string "Some of them are really strong.\n"
	.string "Their old trainers raised them well!$"

Reserve_Text_Sign:
	.string "HAVEN RESERVE\n"
	.string "A safe home for every POKéMON.$"

Reserve_Text_Board:
	.string "It's the reserve's notice board.$"

Reserve_Text_Nurse:
	.string "You've been battling in the meadow?\n"
	.string "Let me look after your POKéMON.$"

Reserve_Text_NurseDone:
	.string "There! Good as new. Good luck out\n"
	.string "there.$"

Reserve_Text_HeadGuard:
	.string "I'm the head guard.\p"
	.string "The reserve's POKéMON are left by\n"
	.string "trainers from every corner of HOENN.\p"
	.string "Even the wandering trainers you meet\n"
	.string "on the routes drop theirs off here,\l"
	.string "and come back to catch new ones.\p"
	.string "Catch one, and it's gone for every-\n"
	.string "one. So don't wait too long!$"
