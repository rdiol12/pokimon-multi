@ The living world (docs/alive.md, src/alive*.c).

@ A townsperson remarks on the player first, then says what they always say.
Alive_EventScript_Remark::
	lock
	faceplayer
	callnative AliveTalk_BufferRemark
	msgbox gStringVar4, MSGBOX_DEFAULT
	release
	callnative Alive_ResumeTalk
	end

Alive_EventScript_ShopClosed::
	msgbox Alive_Text_ShopClosed, MSGBOX_DEFAULT
	releaseall
	end

Alive_Text_ShopClosed:
	.string "Sorry, we're closed for the night.\n"
	.string "We open again at 6 in the morning!$"

@ A resident town Pokémon.
Alive_EventScript_TownMon::
	lock
	faceplayer
	callnative AliveTown_BufferMonText
	playmoncry VAR_0x8004, CRY_MODE_NORMAL
	msgbox gStringVar4, MSGBOX_DEFAULT
	waitmoncry
	goto_if_eq VAR_RESULT, ITEM_NONE, Alive_EventScript_TownMonEnd
	giveitem VAR_RESULT
Alive_EventScript_TownMonEnd:
	release
	end

@ The town crier hands out the paper.
Alive_EventScript_Crier::
	lock
	faceplayer
	msgbox Alive_Text_Crier, MSGBOX_DEFAULT
	callnative AliveNews_ShowPaper, waitstate=1
	release
	end

Alive_Text_Crier:
	.string "Extra! Extra! Read all about it!\n"
	.string "Here, take a paper!$"

@ TVs show the news (data/scripts/tv.inc).
Alive_EventScript_TvNews::
	msgbox gStringVar4, MSGBOX_DEFAULT
	special TurnOffTVScreen
	releaseall
	end

@ The foreman of a town project takes donations.
Alive_EventScript_Foreman::
	lock
	faceplayer
	callnative AliveProjects_Refresh, waitstate=1
	callnative AliveProjects_BufferStatus
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, 1, Alive_EventScript_ForemanGift
	goto_if_eq VAR_RESULT, 2, Alive_EventScript_ForemanEnd
	msgbox Alive_Text_ForemanAsk, MSGBOX_DEFAULT
	showmoneybox 0, 0
	dynmultichoice 18, 6, FALSE, 4, 0, DYN_MULTICHOICE_CB_NONE, Alive_Text_Give1000, Alive_Text_Give5000, Alive_Text_Give20000, Alive_Text_NotNow
	switch VAR_RESULT
	case 0, Alive_EventScript_Give1000
	case 1, Alive_EventScript_Give5000
	case 2, Alive_EventScript_Give20000
	hidemoneybox
	msgbox Alive_Text_ForemanLater, MSGBOX_DEFAULT
	goto Alive_EventScript_ForemanEnd

Alive_EventScript_Give1000:
	setvar VAR_0x8004, 1000
	goto Alive_EventScript_Donate
Alive_EventScript_Give5000:
	setvar VAR_0x8004, 5000
	goto Alive_EventScript_Donate
Alive_EventScript_Give20000:
	setvar VAR_0x8004, 20000
Alive_EventScript_Donate:
	setvar VAR_0x8005, 0
	callnative AliveProjects_Donate, waitstate=1
	callnative AliveProjects_AfterDonate
	updatemoneybox
	goto_if_eq VAR_RESULT, 0, Alive_EventScript_DonatePoor
	goto_if_eq VAR_RESULT, 3, Alive_EventScript_DonateRefused
	playse SE_SHOP
	waitse
	goto_if_eq VAR_RESULT, 2, Alive_EventScript_DonateFinished
	hidemoneybox
	msgbox Alive_Text_DonateThanks, MSGBOX_DEFAULT
	goto Alive_EventScript_ForemanEnd

Alive_EventScript_DonatePoor:
	hidemoneybox
	msgbox Alive_Text_DonatePoor, MSGBOX_DEFAULT
	goto Alive_EventScript_ForemanEnd

Alive_EventScript_DonateRefused:
	hidemoneybox
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Alive_EventScript_ForemanEnd

Alive_EventScript_DonateFinished:
	hidemoneybox
	msgbox Alive_Text_DonateFinished, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	callnative AliveProjects_BuildNow
	playfanfare MUS_OBTAIN_BADGE
	fadescreen FADE_FROM_BLACK
	waitfanfare
	callnative AliveProjects_BufferStatus
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, 1, Alive_EventScript_ForemanGift
	goto Alive_EventScript_ForemanEnd

Alive_EventScript_ForemanGift:
	msgbox Alive_Text_ForemanGift, MSGBOX_DEFAULT
	giveitem VAR_0x8004
Alive_EventScript_ForemanEnd:
	release
	end

Alive_Text_Give1000:
	.string "¥1,000$"
Alive_Text_Give5000:
	.string "¥5,000$"
Alive_Text_Give20000:
	.string "¥20,000$"
Alive_Text_NotNow:
	.string "Not now$"

Alive_Text_ForemanAsk:
	.string "Would you chip in?$"

Alive_Text_ForemanLater:
	.string "No worries. Come back any time!$"

Alive_Text_DonateThanks:
	.string "Thank you kindly!\n"
	.string "Every bit gets us closer.$"

Alive_Text_DonatePoor:
	.string "Oh, you don't have that much on you.\n"
	.string "Don't worry about it!$"

Alive_Text_DonateFinished:
	.string "That's it! We've got enough!\n"
	.string "All right, everyone, let's build!$"

Alive_Text_ForemanGift:
	.string "Here, take this. A thank-you from\n"
	.string "the whole town. Come by every day!$"

@ --- Surprise events -----------------------------------------------------

Alive_EventScript_LostChild::
	lock
	faceplayer
	callnative AliveEvents_ChildState
	goto_if_eq VAR_RESULT, 1, Alive_EventScript_ChildFollowing
	msgbox Alive_Text_LostChild, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Alive_EventScript_ChildAlone
	callnative AliveEvents_ChildFollow
	msgbox Alive_Text_ChildThanks, MSGBOX_DEFAULT
	release
	end

Alive_EventScript_ChildAlone:
	msgbox Alive_Text_ChildAlone, MSGBOX_DEFAULT
	release
	end

Alive_EventScript_ChildFollowing:
	msgbox Alive_Text_ChildFollowing, MSGBOX_DEFAULT
	release
	end

Alive_EventScript_ChildHome::
	lockall
	playse SE_PIN
	msgbox Alive_Text_ChildHome, MSGBOX_DEFAULT
	callnative AliveEvents_ChildReward
	msgbox Alive_Text_ChildReward, MSGBOX_DEFAULT
	playse SE_SHOP
	giveitem VAR_0x8004
	releaseall
	end

Alive_Text_LostChild:
	.string "Sniff… I wandered off to catch\n"
	.string "a POKéMON and now I'm lost…\p"
	.string "Can you take me to a town?$"

Alive_Text_ChildThanks:
	.string "Really? Thank you! I'll stay\n"
	.string "right behind you!$"

Alive_Text_ChildAlone:
	.string "Okay… I'll wait here…$"

Alive_Text_ChildFollowing:
	.string "Are we nearly there?$"

Alive_Text_ChildHome:
	.string "Mom! Dad! I'm back!\p"
	.string "MOTHER: Oh, thank goodness!\n"
	.string "You brought our child home!$"

Alive_Text_ChildReward:
	.string "Please, take this. It's the least\n"
	.string "we can do. (¥{STR_VAR_2} received.)$"

Alive_EventScript_Stuck::
	lock
	faceplayer
	callnative AliveEvents_BufferStuck
	goto_if_eq VAR_RESULT, 1, Alive_EventScript_StuckDone
	msgbox Alive_Text_Stuck, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Alive_EventScript_StuckNo
	msgbox Alive_Text_StuckCoax, MSGBOX_DEFAULT
	callnative AliveEvents_RescueStuck
	delay 30
	msgbox Alive_Text_StuckThanks, MSGBOX_DEFAULT
	giveitem VAR_0x8004
	release
	end

Alive_EventScript_StuckNo:
	msgbox Alive_Text_StuckNo, MSGBOX_DEFAULT
	release
	end

Alive_EventScript_StuckDone:
	msgbox Alive_Text_StuckDone, MSGBOX_DEFAULT
	release
	end

Alive_Text_Stuck:
	.string "My {STR_VAR_1} climbed up there and\n"
	.string "won't come down!\p"
	.string "Could your POKéMON coax it down?$"

Alive_Text_StuckCoax:
	.string "{STR_VAR_2} called out gently…$"

Alive_Text_StuckThanks:
	.string "It came down! Oh, thank you!\n"
	.string "Here, please have this.$"

Alive_Text_StuckNo:
	.string "Oh dear, oh dear…$"

Alive_Text_StuckDone:
	.string "Thanks again! It won't do that\n"
	.string "again… I hope.$"

Alive_EventScript_Crate::
Alive_EventScript_Dropped::
	lock
	callnative AliveEvents_OpenCrate
	playse SE_BALL_OPEN
	giveitem VAR_0x8004
	release
	end

Alive_EventScript_HurtMon::
	lock
	callnative AliveEvents_BufferHurt
	playmoncry VAR_0x8004, CRY_MODE_WEAK
	waitmoncry
	goto_if_eq VAR_RESULT, FALSE, Alive_EventScript_HurtNoItem
	bufferitemname STR_VAR_2, VAR_0x8005
	msgbox Alive_Text_HurtAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Alive_EventScript_HurtEnd
	callnative AliveEvents_HealHurt
	waitmoncry
	goto_if_eq VAR_RESULT, 1, Alive_EventScript_HurtWantsToJoin
	msgbox Alive_Text_HurtThanks, MSGBOX_DEFAULT
	callnative AliveEvents_HurtLeaves
	giveitem VAR_0x8004
	release
	end

Alive_EventScript_HurtWantsToJoin:
	msgbox Alive_Text_HurtJoin, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Alive_EventScript_HurtGoesHome
	callnative AliveEvents_HurtJoins
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, Alive_EventScript_HurtNoRoom
	playfanfare MUS_OBTAIN_ITEM
	msgbox Alive_Text_HurtJoined, MSGBOX_DEFAULT
	waitfanfare
	release
	end

Alive_EventScript_HurtNoRoom:
	msgbox Alive_Text_HurtNoRoom, MSGBOX_DEFAULT
Alive_EventScript_HurtGoesHome:
	callnative AliveEvents_HurtLeaves
	msgbox Alive_Text_HurtLeft, MSGBOX_DEFAULT
	release
	end

Alive_EventScript_HurtNoItem:
	msgbox Alive_Text_HurtNoItem, MSGBOX_DEFAULT
Alive_EventScript_HurtEnd:
	release
	end

Alive_Text_HurtAsk:
	.string "The wild {STR_VAR_1} is hurt and\n"
	.string "can't get up.\p"
	.string "Give it your {STR_VAR_2}?$"

Alive_Text_HurtThanks:
	.string "{STR_VAR_1} looks much better!\p"
	.string "It left something behind as it\n"
	.string "ran off into the grass.$"

Alive_Text_HurtJoin:
	.string "{STR_VAR_1} looks much better!\p"
	.string "It won't leave your side…\n"
	.string "Take {STR_VAR_1} with you?$"

Alive_Text_HurtJoined:
	.string "{STR_VAR_1} joined you!$"

Alive_Text_HurtNoRoom:
	.string "There's no room for it…$"

Alive_Text_HurtLeft:
	.string "{STR_VAR_1} ran off happily.$"

Alive_Text_HurtNoItem:
	.string "The wild {STR_VAR_1} is hurt…\p"
	.string "A POTION or a BERRY would help it.$"

@ --- Weather -------------------------------------------------------------

Alive_EventScript_PondFind::
	lock
	callnative AliveWeather_TakeFind
	msgbox Alive_Text_PondFind, MSGBOX_DEFAULT
	giveitem VAR_0x8004
	release
	end

Alive_Text_PondFind:
	.string "Something was lying on the bottom\n"
	.string "of the dried-up pond!$"
