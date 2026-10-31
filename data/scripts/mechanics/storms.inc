@ The postgame storms (src/mechanics_storms.c): a researcher in the Weather
@ Institute sends the player to calm a drought on Route 114 and a thunderstorm
@ on Route 118 by beating Groudon's and Kyogre's echoes.
Mechanics_EventScript_StormResearcher::
	lock
	faceplayer
	goto_if_set FLAG_MECH_STORM_REWARD, Mechanics_EventScript_StormDone
	callnative Mechanics_CountCalmedStorms
	goto_if_eq VAR_RESULT, STORM_COUNT, Mechanics_EventScript_StormReward
	goto_if_eq VAR_RESULT, 1, Mechanics_EventScript_StormHalfway
	msgbox Mechanics_Text_StormStory, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_StormHalfway:
	msgbox Mechanics_Text_StormHalfway, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_StormReward:
	msgbox Mechanics_Text_StormThanks, MSGBOX_DEFAULT
	giveitem ITEM_SMOOTH_ROCK
	giveitem ITEM_ICY_ROCK
	setflag FLAG_MECH_STORM_REWARD
	msgbox Mechanics_Text_StormRocks, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_StormDone:
	msgbox Mechanics_Text_StormDone, MSGBOX_DEFAULT
	release
	end

Mechanics_EventScript_EchoGroudon::
	setvar VAR_0x8004, STORM_GROUDON
	goto Mechanics_EventScript_Echo

Mechanics_EventScript_EchoKyogre::
	setvar VAR_0x8004, STORM_KYOGRE
Mechanics_EventScript_Echo:
	lockall
	callnative Mechanics_SetupStormEcho
	playmoncry VAR_0x8006, CRY_MODE_ENCOUNTER
	msgbox gStringVar4, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	dowildbattle
	callnative Mechanics_EndStormEcho
	goto_if_eq VAR_RESULT, FALSE, Mechanics_EventScript_EchoStill
	removeobject LOCALID_MECH_STORM_ECHO
	resetweather
	doweather
	msgbox Mechanics_Text_EchoFaded, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	releaseall
	end

Mechanics_EventScript_EchoStill:
	msgbox Mechanics_Text_EchoStill, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_Text_StormStory:
	.string "I study the weather of all HOENN.\n"
	.string "Something's wrong out there.\p"
	.string "ROUTE 114 is baking in a drought,\n"
	.string "and ROUTE 118 is in a thunderstorm.\p"
	.string "It feels like GROUDON and KYOGRE…\n"
	.string "Their power is still in the air!\p"
	.string "Echoes of them must be keeping the\n"
	.string "storms going. Could you calm them?$"

Mechanics_Text_StormHalfway:
	.string "One storm is calm! The readings\n"
	.string "look so much better.\p"
	.string "Now the other one. Check ROUTE 114\n"
	.string "and ROUTE 118!$"

Mechanics_Text_StormThanks:
	.string "The drought and the thunderstorm\n"
	.string "are both gone! You did it!\p"
	.string "Please take these. We use them to\n"
	.string "study the weather.$"

Mechanics_Text_StormRocks:
	.string "A POKéMON holding one of those\n"
	.string "makes its weather last longer.$"

Mechanics_Text_StormDone:
	.string "HOENN's weather is calm again.\n"
	.string "GROUDON and KYOGRE are at rest.$"

Mechanics_Text_EchoFaded:
	.string "The echo faded away…\n"
	.string "The sky is clearing up!\p"
	.string "Something was left where it stood.$"

Mechanics_Text_EchoStill:
	.string "The echo is still here, and the\n"
	.string "storm rages on…$"
