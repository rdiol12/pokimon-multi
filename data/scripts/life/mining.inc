@ Underground mining: the digger in Rusturf Tunnel, and the glittering
@ rocks in Granite Cave (1F, B1F), Rusturf Tunnel, Fiery Path, Meteor Falls
@ and Victory Road.

Life_EventScript_DigSpot0::
	setvar VAR_0x8005, 0
	goto Life_EventScript_DigSpot
Life_EventScript_DigSpot1::
	setvar VAR_0x8005, 1
	goto Life_EventScript_DigSpot
Life_EventScript_DigSpot2::
	setvar VAR_0x8005, 2
	goto Life_EventScript_DigSpot
Life_EventScript_DigSpot3::
	setvar VAR_0x8005, 3
	goto Life_EventScript_DigSpot
Life_EventScript_DigSpot4::
	setvar VAR_0x8005, 4
	goto Life_EventScript_DigSpot
Life_EventScript_DigSpot5::
	setvar VAR_0x8005, 5
	goto Life_EventScript_DigSpot

Life_EventScript_DigSpot:
	lockall
	callnative LifeMining_CheckSpot
	goto_if_eq VAR_RESULT, LIFE_DIG_NO_KIT, Life_EventScript_DigNoKit
	goto_if_eq VAR_RESULT, LIFE_DIG_DONE_TODAY, Life_EventScript_DigDone
	msgbox Life_Text_DigAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Life_EventScript_DigEnd
	closemessage
	playse SE_M_DIG
	fadescreen FADE_TO_BLACK
	callnative LifeMining_Start
	waitstate
	goto_if_eq VAR_RESULT, 0, Life_EventScript_DigEnd
	msgbox Life_Text_DigAfter, MSGBOX_DEFAULT
Life_EventScript_DigEnd:
	releaseall
	end

Life_EventScript_DigNoKit:
	msgbox Life_Text_DigNoKit, MSGBOX_DEFAULT
	releaseall
	end

Life_EventScript_DigDone:
	msgbox Life_Text_DigDone, MSGBOX_DEFAULT
	releaseall
	end

Life_EventScript_Digger::
	lock
	faceplayer
	callnative LifeMining_GiveKit
	goto_if_eq VAR_RESULT, TRUE, Life_EventScript_DiggerAgain
	msgbox Life_Text_DiggerIntro, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Life_Text_DiggerGot
	waitmessage
	waitfanfare
	msgbox Life_Text_DiggerHow, MSGBOX_DEFAULT
	release
	end

Life_EventScript_DiggerAgain:
	msgbox Life_Text_DiggerAgain, MSGBOX_DEFAULT
	release
	end

Life_Text_DigNoKit:
	.string "Something glitters inside this rock.\p"
	.string "You'd need proper digging tools to\n"
	.string "get at it.$"

Life_Text_DigDone:
	.string "You already dug here today. The rock\n"
	.string "needs time to settle.$"

Life_Text_DigAsk:
	.string "Something glitters inside this rock.\n"
	.string "Dig into it?$"

Life_Text_DigAfter:
	.string "The finds went into your BAG.$"

Life_Text_DiggerIntro:
	.string "Hoo! You've got the look of someone\n"
	.string "who likes poking around in caves.\p"
	.string "There's treasure under all of Hoenn:\n"
	.string "shards, evolution stones, fossils…\p"
	.string "I'm too old to dig it all up myself.\n"
	.string "Here, take my spare tools!$"

Life_Text_DiggerGot:
	.string "{PLAYER} received the DIGGING TOOLS!$"

Life_Text_DiggerHow:
	.string "Look for rocks with a glint in them,\n"
	.string "in caves all over Hoenn.\p"
	.string "The pick digs deep, the hammer digs\n"
	.string "wide but shakes the whole wall.\p"
	.string "Dig too much and it caves in, so be\n"
	.string "smart about it! Each rock refills\l"
	.string "overnight.$"

Life_Text_DiggerAgain:
	.string "Found anything good? You've dug up\n"
	.string "{STR_VAR_1} treasures so far!$"
