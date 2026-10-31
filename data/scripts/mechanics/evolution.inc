@ The evolution sage in a Mauville City house (src/mechanics_evolution.c):
@ Pokémon that only evolve by trading can evolve here after a trial.
Mechanics_EventScript_EvoSage::
	lock
	faceplayer
	goto_if_set FLAG_MECH_MET_EVO_SAGE, Mechanics_EventScript_EvoSageAsk
	setflag FLAG_MECH_MET_EVO_SAGE
	msgbox Mechanics_Text_EvoSageIntro, MSGBOX_DEFAULT
Mechanics_EventScript_EvoSageAsk:
	msgbox Mechanics_Text_EvoSageWhich, MSGBOX_DEFAULT
	special ChoosePartyMon
	lock
	faceplayer
	goto_if_ge VAR_0x8004, PARTY_SIZE, Mechanics_EventScript_EvoSageBye
	callnative Mechanics_CheckEvoQuest
	switch VAR_RESULT
	case EVO_QUEST_NEEDS_ITEM, Mechanics_EventScript_EvoSageItem
	case EVO_QUEST_NEEDS_BOND, Mechanics_EventScript_EvoSageBond
	case EVO_QUEST_READY, Mechanics_EventScript_EvoSageTrial
	msgbox Mechanics_Text_EvoSageNone, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_EvoSageItem:
	msgbox Mechanics_Text_EvoSageItem, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_EvoSageBond:
	msgbox Mechanics_Text_EvoSageBond, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_EvoSageTrial:
	msgbox Mechanics_Text_EvoSageReady, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Mechanics_EventScript_EvoSageBye
	special HealPlayerParty
	callnative Mechanics_SetupEvoTrial
	msgbox Mechanics_Text_EvoSageBegin, MSGBOX_DEFAULT
	closemessage
	callnative Mechanics_StartFacilityBattle
	waitstate
	callnative Mechanics_EvoTrialResult
	special HealPlayerParty
	goto_if_eq VAR_RESULT, FALSE, Mechanics_EventScript_EvoSageFailed
	msgbox Mechanics_Text_EvoSagePassed, MSGBOX_DEFAULT
	closemessage
	callnative Mechanics_EvolveForQuest
	lock
	faceplayer
	msgbox Mechanics_Text_EvoSageAfter, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_EvoSageFailed:
	msgbox Mechanics_Text_EvoSageFailed, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_EvoSageBye:
	msgbox Mechanics_Text_EvoSageBye, MSGBOX_DEFAULT
	release
	end

Mechanics_Text_EvoSageIntro:
	.string "Some POKéMON only change when they\n"
	.string "travel between TRAINERS.\p"
	.string "But a bond can do what a trade does,\n"
	.string "if it's strong enough. I'll show you.$"

Mechanics_Text_EvoSageWhich:
	.string "Which POKéMON shall I look at?$"

Mechanics_Text_EvoSageNone:
	.string "{STR_VAR_1} doesn't need my help.\n"
	.string "It will grow in its own way.$"

Mechanics_Text_EvoSageItem:
	.string "{STR_VAR_1} could become {STR_VAR_3}…\p"
	.string "But first it must hold a\n"
	.string "{STR_VAR_2}. Come back with it.$"

Mechanics_Text_EvoSageBond:
	.string "{STR_VAR_1} could become {STR_VAR_3}…\p"
	.string "But your bond isn't strong enough yet.\n"
	.string "Travel and battle together, then\l"
	.string "come back.$"

Mechanics_Text_EvoSageReady:
	.string "{STR_VAR_1} is ready to become\n"
	.string "{STR_VAR_3}!\p"
	.string "Win a trial battle with {STR_VAR_1} in\n"
	.string "the lead, still standing at the end.\l"
	.string "Shall we?$"

Mechanics_Text_EvoSageBegin:
	.string "Show me your bond!$"

Mechanics_Text_EvoSagePassed:
	.string "Wonderful! Now… watch closely!$"

Mechanics_Text_EvoSageAfter:
	.string "That's the power of a true bond.$"

Mechanics_Text_EvoSageFailed:
	.string "{STR_VAR_1} has to finish the battle\n"
	.string "on its feet. Try again any time.$"

Mechanics_Text_EvoSageBye:
	.string "Come back any time, dear.$"
