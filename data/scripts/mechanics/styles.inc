@ The battle style master in the Fallarbor Battle Tent
@ (src/mechanics_styles.c): a practice battle using AGILE and STRONG
@ styles, then the player may use them in every battle.
Mechanics_EventScript_StyleMaster::
	lockall
	faceplayer
	goto_if_set FLAG_MECH_BATTLE_STYLES, Mechanics_EventScript_StyleMasterAgain
	msgbox Mechanics_Text_StyleIntro, MSGBOX_DEFAULT
	msgbox Mechanics_Text_StyleExplain, MSGBOX_DEFAULT
	msgbox Mechanics_Text_StyleReady, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Mechanics_EventScript_StyleBye
	call Mechanics_EventScript_StyleBattle
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, Mechanics_EventScript_StyleLearned
	msgbox Mechanics_Text_StyleLost, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_EventScript_StyleLearned:
	setflag FLAG_MECH_BATTLE_STYLES
	msgbox Mechanics_Text_StyleWon, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Mechanics_Text_StyleLearned
	waitfanfare
	waitmessage
	waitbuttonpress
	msgbox Mechanics_Text_StyleReminder, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_EventScript_StyleMasterAgain:
	msgbox Mechanics_Text_StyleAgain, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Mechanics_EventScript_StyleBye
	call Mechanics_EventScript_StyleBattle
	msgbox Mechanics_Text_StyleAfterPractice, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_EventScript_StyleBattle:
	special HealPlayerParty
	callnative Mechanics_SetupStylePractice
	msgbox Mechanics_Text_StyleBegin, MSGBOX_DEFAULT
	closemessage
	callnative Mechanics_StartFacilityBattle
	waitstate
	callnative Mechanics_EndStylePractice
	special HealPlayerParty
	return

Mechanics_EventScript_StyleBye:
	msgbox Mechanics_Text_StyleBye, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_Text_StyleIntro:
	.string "Hm! You have the eyes of a TRAINER\n"
	.string "who wants more from every move.\p"
	.string "I teach BATTLE STYLES. A POKéMON can\n"
	.string "put its move in one of two styles.$"

Mechanics_Text_StyleExplain:
	.string "AGILE style: the move is lighter,\n"
	.string "but it goes before other moves of\l"
	.string "the same priority.\p"
	.string "STRONG style: the move hits harder,\n"
	.string "but it goes after other moves of\l"
	.string "the same priority.\p"
	.string "Either style uses 1 more PP.\n"
	.string "Press SELECT when you pick a move.$"

Mechanics_Text_StyleReady:
	.string "Show me in a practice battle, two\n"
	.string "POKéMON each. Ready?$"

Mechanics_Text_StyleBegin:
	.string "My student here knows styles too.\n"
	.string "Try SELECT on the move menu!$"

Mechanics_Text_StyleLost:
	.string "Not yet! Speed and power both have\n"
	.string "their time. Come back and try again.$"

Mechanics_Text_StyleWon:
	.string "Splendid! You read the battle and\n"
	.string "picked your moment.$"

Mechanics_Text_StyleLearned:
	.string "{PLAYER} learned BATTLE STYLES!$"

Mechanics_Text_StyleReminder:
	.string "Use them in any battle. Press SELECT\n"
	.string "on the move menu to switch between\l"
	.string "normal, AGILE and STRONG.\p"
	.string "Only link battles forbid them, so\n"
	.string "everyone plays fair online.$"

Mechanics_Text_StyleAgain:
	.string "Want another practice battle?$"

Mechanics_Text_StyleAfterPractice:
	.string "Good! Keep your styles sharp.$"

Mechanics_Text_StyleBye:
	.string "A style is only as good as its timing.$"
