@ The move tutor who stands in every Pokémon Center (src/mechanics.c).
@ Reuses the move relearner's menu: level-up moves, Egg Moves, TM moves and
@ tutor moves, free of charge.
Mechanics_EventScript_MoveTutor::
	lockall
	faceplayer
	goto_if_set FLAG_MECH_MET_TUTOR, Mechanics_EventScript_MoveTutorAgain
	setflag FLAG_MECH_MET_TUTOR
	msgbox Mechanics_Text_MoveTutorIntro, MSGBOX_DEFAULT
	message Mechanics_Text_MoveTutorAgain
	waitmessage
	goto Common_EventScript_MoveRelearnerDynMultiChoice
	end

Mechanics_EventScript_MoveTutorAgain:
	message Mechanics_Text_MoveTutorAgain
	waitmessage
	goto Common_EventScript_MoveRelearnerDynMultiChoice
	end

Mechanics_Text_MoveTutorIntro:
	.string "I'm a MOVE TUTOR. The POKéMON\n"
	.string "LEAGUE posts one of us in every\l"
	.string "POKéMON CENTER.\p"
	.string "I can help a POKéMON remember a\n"
	.string "move it forgot, or teach it an\l"
	.string "EGG MOVE its parents could have\l"
	.string "passed on.\p"
	.string "No HEART SCALES needed. Training\n"
	.string "should be fun, not a chore!$"

Mechanics_Text_MoveTutorAgain:
	.string "Which kind of move shall we\n"
	.string "work on today?$"
