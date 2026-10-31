@ Starter signature moves (src/mechanics_signature.c): Birch's aide sends
@ the player to an old statue of their starter's final form; beating its
@ guardian earns the starter's own move.
Mechanics_EventScript_LabAide::
	lock
	faceplayer
	goto_if_set FLAG_MECH_SIG_TAUGHT, Mechanics_EventScript_AideDone
	callnative Mechanics_FindStarter
	goto_if_eq VAR_RESULT, STARTER_LINE_NONE, Mechanics_EventScript_AideNoStarter
	goto_if_set FLAG_MECH_SIG_BEATEN, Mechanics_EventScript_AideTeach
	goto_if_set FLAG_MECH_SIG_STARTED, Mechanics_EventScript_AideReminder
	goto_if_unset FLAG_BADGE03_GET, Mechanics_EventScript_AideTooEarly
	msgbox Mechanics_Text_AideStory, MSGBOX_DEFAULT
	setflag FLAG_MECH_SIG_STARTED
	msgbox Mechanics_Text_AideGo, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_AideReminder:
	msgbox Mechanics_Text_AideGo, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_AideTooEarly:
	msgbox Mechanics_Text_AideTooEarly, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_AideNoStarter:
	msgbox Mechanics_Text_AideNoStarter, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_AideTeach:
	msgbox Mechanics_Text_AideBeaten, MSGBOX_DEFAULT
	switch VAR_RESULT
	case STARTER_LINE_TREECKO, Mechanics_EventScript_AideTeachTreecko
	case STARTER_LINE_TORCHIC, Mechanics_EventScript_AideTeachTorchic
	setvar VAR_0x8005, MOVE_MIRE_CRASH
	goto Mechanics_EventScript_AideTeachMove
Mechanics_EventScript_AideTeachTreecko:
	setvar VAR_0x8005, MOVE_CANOPY_EDGE
	goto Mechanics_EventScript_AideTeachMove
Mechanics_EventScript_AideTeachTorchic:
	setvar VAR_0x8005, MOVE_BLAZE_RUSH
Mechanics_EventScript_AideTeachMove:
	msgbox Mechanics_Text_AideWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveTutor
	lock
	faceplayer
	goto_if_eq VAR_RESULT, FALSE, Mechanics_EventScript_AideLater
	setflag FLAG_MECH_SIG_TAUGHT
	msgbox Mechanics_Text_AideTaught, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_AideLater:
	msgbox Mechanics_Text_AideLater, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_AideDone:
	msgbox Mechanics_Text_AideDone, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_StatueTreecko::
	setvar VAR_0x8004, STARTER_LINE_TREECKO
	goto Mechanics_EventScript_Statue

Mechanics_EventScript_StatueTorchic::
	setvar VAR_0x8004, STARTER_LINE_TORCHIC
	goto Mechanics_EventScript_Statue

Mechanics_EventScript_StatueMudkip::
	setvar VAR_0x8004, STARTER_LINE_MUDKIP
Mechanics_EventScript_Statue:
	lockall
	callnative Mechanics_SetupGuardian
	msgbox Mechanics_Text_Statue, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Mechanics_EventScript_StatueEnd
	msgbox Mechanics_Text_StatueWakes, MSGBOX_DEFAULT
	closemessage
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, Mechanics_EventScript_StatueStill
	setflag FLAG_MECH_SIG_BEATEN
	msgbox Mechanics_Text_StatueBeaten, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_EventScript_StatueStill:
	msgbox Mechanics_Text_StatueStill, MSGBOX_DEFAULT
Mechanics_EventScript_StatueEnd:
	releaseall
	end

Mechanics_Text_AideTooEarly:
	.string "PROF. BIRCH has me reading old field\n"
	.string "notes about the HOENN starters.\p"
	.string "Come back once you have three GYM\n"
	.string "BADGES. I may have found something!$"

Mechanics_Text_AideNoStarter:
	.string "These notes are about the POKéMON\n"
	.string "PROF. BIRCH gives new TRAINERS.\p"
	.string "Bring yours along and I'll tell you\n"
	.string "what I found!$"

Mechanics_Text_AideStory:
	.string "Oh, {STR_VAR_1}! Perfect timing.\p"
	.string "These old field notes say that long\n"
	.string "ago, POKéMON like {STR_VAR_1} had a\l"
	.string "move of their very own.\p"
	.string "People built statues where they\n"
	.string "trained. A spirit still guards each\l"
	.string "one, waiting for a worthy partner.$"

Mechanics_Text_AideGo:
	.string "The notes put {STR_VAR_1}'s statue in\n"
	.string "{STR_VAR_3}.\p"
	.string "Bring {STR_VAR_1} there and see what\n"
	.string "happens. Then come tell me!$"

Mechanics_Text_AideBeaten:
	.string "You beat the guardian? Then it's\n"
	.string "true!\p"
	.string "The notes say the move is called\n"
	.string "{STR_VAR_2}. Let's teach it!$"

Mechanics_Text_AideWhichMon:
	.string "Which POKéMON should learn it?$"

Mechanics_Text_AideTaught:
	.string "Amazing… A move nobody has seen in\n"
	.string "hundreds of years! PROF. BIRCH will\l"
	.string "be so excited!$"

Mechanics_Text_AideLater:
	.string "Not now? Come back any time.$"

Mechanics_Text_AideDone:
	.string "I'm writing up everything we learned\n"
	.string "about that move. Thank you!$"

Mechanics_Text_Statue:
	.string "An old stone statue of a {STR_VAR_2}.\n"
	.string "It's worn smooth by the years.$"

Mechanics_Text_StatueWakes:
	.string "{STR_VAR_1} stepped up to the statue…\p"
	.string "The statue's eyes are glowing!\n"
	.string "Its guardian appeared!$"

Mechanics_Text_StatueBeaten:
	.string "The glow faded. The statue seems\n"
	.string "to be at peace.\p"
	.string "{STR_VAR_1} looks stronger somehow.\n"
	.string "Better tell PROF. BIRCH's aide!$"

Mechanics_Text_StatueStill:
	.string "The statue's eyes are still glowing.\n"
	.string "The guardian is waiting.$"
