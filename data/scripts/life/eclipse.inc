@ Team Eclipse: the postgame chapter. The scientist in the Mossdeep Space
@ Center starts it and gives the reward; the six Eclipse members are in
@ Meteor Falls, on the Mt. Pyre summit and on the Space Center's 2F.

Life_EventScript_Eclipse0::
	setvar VAR_0x8004, 0
	goto Life_EventScript_EclipseTrainer
Life_EventScript_Eclipse1::
	setvar VAR_0x8004, 1
	goto Life_EventScript_EclipseTrainer
Life_EventScript_Eclipse2::
	setvar VAR_0x8004, 2
	goto Life_EventScript_EclipseTrainer
Life_EventScript_Eclipse3::
	setvar VAR_0x8004, 3
	goto Life_EventScript_EclipseTrainer
Life_EventScript_Eclipse4::
	setvar VAR_0x8004, 4
	goto Life_EventScript_EclipseTrainer
Life_EventScript_Eclipse5::
	setvar VAR_0x8004, 5
	goto Life_EventScript_EclipseTrainer

Life_EventScript_EclipseTrainer:
	lock
	faceplayer
	playbgm MUS_ENCOUNTER_MAGMA, FALSE
	callnative LifeEclipse_BufferIntro
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	callnative LifeEclipse_StartBattle
	waitstate
	callnative LifeEclipse_Defeated
	copyvar VAR_0x8005, VAR_RESULT
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	goto_if_eq VAR_0x8005, FALSE, Life_EventScript_EclipseEnd
	goto_if_eq VAR_0x8004, 1, Life_EventScript_EclipseFallsDone
	goto_if_eq VAR_0x8004, 3, Life_EventScript_EclipsePyreDone
	goto_if_eq VAR_0x8004, 5, Life_EventScript_EclipseCenterDone
Life_EventScript_EclipseEnd:
	release
	end

Life_EventScript_EclipseFallsDone:
	msgbox Life_Text_EclipseFallsDone, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipsePyreDone:
	msgbox Life_Text_EclipsePyreDone, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipseCenterDone:
	msgbox Life_Text_EclipseCenterDone, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipseScientist::
	lock
	faceplayer
	callnative LifeEclipse_Scientist
	switch VAR_RESULT
	case LIFE_ECLIPSE_NOT_STARTED, Life_EventScript_EclipseStart
	case LIFE_ECLIPSE_METEOR_FALLS, Life_EventScript_EclipseHintFalls
	case LIFE_ECLIPSE_MT_PYRE, Life_EventScript_EclipseHintPyre
	case LIFE_ECLIPSE_SPACE_CENTER, Life_EventScript_EclipseHintCenter
	case LIFE_ECLIPSE_REWARD, Life_EventScript_EclipseReward
	msgbox Life_Text_EclipseThanks, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipseStart:
	msgbox Life_Text_EclipseStart, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipseHintFalls:
	msgbox Life_Text_EclipseHintFalls, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipseHintPyre:
	msgbox Life_Text_EclipseHintPyre, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipseHintCenter:
	msgbox Life_Text_EclipseHintCenter, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipseReward:
	msgbox Life_Text_EclipseReward, MSGBOX_DEFAULT
	giveitem ITEM_ABILITY_PATCH
	goto_if_eq VAR_RESULT, FALSE, Life_EventScript_EclipseBagFull
	giveitem ITEM_DUSK_BALL, 5
	setvar VAR_LIFE_ECLIPSE, LIFE_ECLIPSE_DONE
	msgbox Life_Text_EclipseThanks, MSGBOX_DEFAULT
	release
	end

Life_EventScript_EclipseBagFull:
	msgbox Life_Text_EclipseBagFull, MSGBOX_DEFAULT
	release
	end

Life_Text_EclipseStart:
	.string "CHAMPION! Thank goodness!\p"
	.string "A gang calling itself TEAM ECLIPSE\n"
	.string "broke in and copied our launch data.\p"
	.string "They want to put a shade in orbit\n"
	.string "and block out the sun over Hoenn!\p"
	.string "They said they needed moon rock\n"
	.string "from METEOR FALLS. Please hurry!$"

Life_Text_EclipseHintFalls:
	.string "TEAM ECLIPSE went to METEOR FALLS,\n"
	.string "off ROUTE 114. Please stop them!$"

Life_Text_EclipseHintPyre:
	.string "Their ADMIN is on the summit of\n"
	.string "MT. PYRE? What could she want there?$"

Life_Text_EclipseHintCenter:
	.string "They're upstairs at the launch\n"
	.string "controls! Their leader is with them!$"

Life_Text_EclipseReward:
	.string "You stopped the launch! The sun\n"
	.string "is safe, thanks to you.\p"
	.string "Please, take these as thanks from\n"
	.string "all of us at the SPACE CENTER.$"

Life_Text_EclipseThanks:
	.string "TEAM ECLIPSE is gone for good.\n"
	.string "Hoenn's sky is clear again!$"

Life_Text_EclipseBagFull:
	.string "Oh, your BAG is full. Come back\n"
	.string "when you've made room!$"

Life_Text_EclipseFallsDone:
	.string "MT. PYRE… The ADMIN must be on\n"
	.string "its summit.$"

Life_Text_EclipsePyreDone:
	.string "The SPACE CENTER in MOSSDEEP! The\n"
	.string "rocket launches tonight!$"

Life_Text_EclipseCenterDone:
	.string "TEAM ECLIPSE fled the SPACE CENTER.\n"
	.string "The scientist downstairs will want\l"
	.string "to hear the news!$"
