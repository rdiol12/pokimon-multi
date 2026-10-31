@ The rules referee in the Slateport Battle Tent (src/mechanics_battle.c):
@ battles against Battle Frontier trainers under special rule sets.
Mechanics_EventScript_Referee::
	lockall
	faceplayer
	goto_if_set FLAG_MECH_MET_REFEREE, Mechanics_EventScript_RefereeMenu
	setflag FLAG_MECH_MET_REFEREE
	msgbox Mechanics_Text_RefereeIntro, MSGBOX_DEFAULT
Mechanics_EventScript_RefereeMenu:
	message Mechanics_Text_RefereeWhich
	waitmessage
	callnative Mechanics_PushRuleRows
	dynmultipush Mechanics_Text_RefereeNotNow, 0
	dynmultistack 0, 0, FALSE, 6, FALSE, RULESET_INVERSE, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, Mechanics_EventScript_RefereeBye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Mechanics_EventScript_RefereeBye
	copyvar VAR_0x8005, VAR_RESULT
	callnative Mechanics_GetRuleHelp
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Mechanics_Text_RefereeReady, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Mechanics_EventScript_RefereeMenu
	special HealPlayerParty
	callnative Mechanics_SetupRulesBattle
	msgbox Mechanics_Text_RefereeOpponent, MSGBOX_DEFAULT
	closemessage
	callnative Mechanics_StartFacilityBattle
	waitstate
	callnative Mechanics_RulesAfterBattle
	special HealPlayerParty
	goto_if_eq VAR_RESULT, FALSE, Mechanics_EventScript_RefereeLost
	msgbox Mechanics_Text_RefereeWon, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8006, FALSE, Mechanics_EventScript_RefereeMenu
	msgbox Mechanics_Text_RefereeAllWon, MSGBOX_DEFAULT
	giveitem ITEM_ABILITY_PATCH
	msgbox Mechanics_Text_RefereeAbilityPatch, MSGBOX_DEFAULT
	goto Mechanics_EventScript_RefereeMenu

Mechanics_EventScript_RefereeLost:
	msgbox Mechanics_Text_RefereeLost, MSGBOX_DEFAULT
	goto Mechanics_EventScript_RefereeMenu

Mechanics_EventScript_RefereeBye:
	msgbox Mechanics_Text_RefereeBye, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_Text_RefereeIntro:
	.string "Welcome to the RULES HALL corner!\n"
	.string "I referee battles with special rules.\p"
	.string "Win under every rule set, and the\n"
	.string "BATTLE TENT has a prize for you.\p"
	.string "Your POKéMON are healed before and\n"
	.string "after, and nobody gains EXP.$"

Mechanics_Text_RefereeWhich:
	.string "Which rules will you battle under?$"

Mechanics_Text_RefereeNotNow:
	.string "Not now$"

Mechanics_Text_RefereeReady:
	.string "Ready to battle under these rules?$"

Mechanics_Text_RefereeOpponent:
	.string "Your opponent is {STR_VAR_1}!\n"
	.string "{STR_VAR_2} rules… Begin!$"

Mechanics_Text_RefereeWon:
	.string "The winner is {PLAYER}!\n"
	.string "You've mastered {STR_VAR_2} rules.$"

Mechanics_Text_RefereeAllWon:
	.string "You've won under every rule set!\n"
	.string "Here's your prize.$"

Mechanics_Text_RefereeAbilityPatch:
	.string "An ABILITY PATCH switches a POKéMON\n"
	.string "to its hidden ABILITY. Use it well!$"

Mechanics_Text_RefereeLost:
	.string "That's the match! Rules like these\n"
	.string "take practice. Try again anytime.$"

Mechanics_Text_RefereeBye:
	.string "The rules will be waiting!$"
