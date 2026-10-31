MirageDunes_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, MirageDunes_OnLoad
	.byte 0

MirageDunes_OnLoad:
	call_if_set FLAG_MIRAGE_1, MirageDunes_EventScript_Gone1
	call_if_set FLAG_MIRAGE_2, MirageDunes_EventScript_Gone2
	call_if_set FLAG_MIRAGE_3, MirageDunes_EventScript_Gone3
	goto_if_unset FLAG_MIRAGE_1, MirageDunes_EventScript_OnLoadEnd
	goto_if_unset FLAG_MIRAGE_2, MirageDunes_EventScript_OnLoadEnd
	goto_if_unset FLAG_MIRAGE_3, MirageDunes_EventScript_OnLoadEnd
	call MirageDunes_EventScript_HazeGone
MirageDunes_EventScript_OnLoadEnd:
	end

MirageDunes_EventScript_Gone1::
	setmetatile 7, 27, 593, FALSE
	setmetatile 8, 27, 593, FALSE
	setmetatile 9, 27, 593, FALSE
	setmetatile 7, 28, 593, FALSE
	setmetatile 8, 28, 593, FALSE
	setmetatile 9, 28, 593, FALSE
	setmetatile 4, 24, 593, FALSE
	setmetatile 5, 24, 593, FALSE
	setmetatile 4, 25, 593, FALSE
	setmetatile 5, 25, 593, FALSE
	setmetatile 4, 26, 593, FALSE
	setmetatile 5, 26, 593, FALSE
	setmetatile 10, 24, 593, FALSE
	setmetatile 11, 24, 593, FALSE
	setmetatile 10, 25, 593, FALSE
	setmetatile 11, 25, 593, FALSE
	setmetatile 10, 26, 593, FALSE
	setmetatile 11, 26, 593, FALSE
	return

MirageDunes_EventScript_Gone2::
	setmetatile 46, 30, 593, FALSE
	setmetatile 47, 30, 593, FALSE
	setmetatile 48, 30, 593, FALSE
	setmetatile 46, 31, 593, FALSE
	setmetatile 47, 31, 593, FALSE
	setmetatile 48, 31, 593, FALSE
	setmetatile 43, 27, 593, FALSE
	setmetatile 44, 27, 593, FALSE
	setmetatile 43, 28, 593, FALSE
	setmetatile 44, 28, 593, FALSE
	setmetatile 43, 29, 593, FALSE
	setmetatile 44, 29, 593, FALSE
	setmetatile 49, 27, 593, FALSE
	setmetatile 50, 27, 593, FALSE
	setmetatile 49, 28, 593, FALSE
	setmetatile 50, 28, 593, FALSE
	setmetatile 49, 29, 593, FALSE
	setmetatile 50, 29, 593, FALSE
	return

MirageDunes_EventScript_Gone3::
	setmetatile 36, 11, 593, FALSE
	setmetatile 37, 11, 593, FALSE
	setmetatile 38, 11, 593, FALSE
	setmetatile 36, 12, 593, FALSE
	setmetatile 37, 12, 593, FALSE
	setmetatile 38, 12, 593, FALSE
	setmetatile 33, 8, 593, FALSE
	setmetatile 34, 8, 593, FALSE
	setmetatile 33, 9, 593, FALSE
	setmetatile 34, 9, 593, FALSE
	setmetatile 33, 10, 593, FALSE
	setmetatile 34, 10, 593, FALSE
	setmetatile 39, 8, 593, FALSE
	setmetatile 40, 8, 593, FALSE
	setmetatile 39, 9, 593, FALSE
	setmetatile 40, 9, 593, FALSE
	setmetatile 39, 10, 593, FALSE
	setmetatile 40, 10, 593, FALSE
	return

MirageDunes_EventScript_HazeGone::
	setmetatile 25, 5, 593, FALSE
	setmetatile 26, 5, 593, FALSE
	setmetatile 27, 5, 593, FALSE
	setmetatile 28, 5, 593, FALSE
	setmetatile 29, 5, 593, FALSE
	setmetatile 30, 5, 593, FALSE
	return

MirageDunes_EventScript_Mirage1::
	goto_if_set FLAG_MIRAGE_1, MirageDunes_EventScript_MirageNone
	lockall
	playse SE_M_MIST
	call MirageDunes_EventScript_Gone1
	special DrawWholeMapView
	waitse
	setflag FLAG_MIRAGE_1
	msgbox MirageDunes_Text_Mirage, MSGBOX_DEFAULT
	goto_if_unset FLAG_MIRAGE_1, MirageDunes_EventScript_MirageEnd1
	goto_if_unset FLAG_MIRAGE_2, MirageDunes_EventScript_MirageEnd1
	goto_if_unset FLAG_MIRAGE_3, MirageDunes_EventScript_MirageEnd1
	delay 30
	playse SE_M_GUST
	call MirageDunes_EventScript_HazeGone
	special DrawWholeMapView
	delay 40
	msgbox MirageDunes_Text_HazeLifts, MSGBOX_DEFAULT
MirageDunes_EventScript_MirageEnd1::
	releaseall
	end

MirageDunes_EventScript_Mirage2::
	goto_if_set FLAG_MIRAGE_2, MirageDunes_EventScript_MirageNone
	lockall
	playse SE_M_MIST
	call MirageDunes_EventScript_Gone2
	special DrawWholeMapView
	waitse
	setflag FLAG_MIRAGE_2
	msgbox MirageDunes_Text_Mirage, MSGBOX_DEFAULT
	goto_if_unset FLAG_MIRAGE_1, MirageDunes_EventScript_MirageEnd2
	goto_if_unset FLAG_MIRAGE_2, MirageDunes_EventScript_MirageEnd2
	goto_if_unset FLAG_MIRAGE_3, MirageDunes_EventScript_MirageEnd2
	delay 30
	playse SE_M_GUST
	call MirageDunes_EventScript_HazeGone
	special DrawWholeMapView
	delay 40
	msgbox MirageDunes_Text_HazeLifts, MSGBOX_DEFAULT
MirageDunes_EventScript_MirageEnd2::
	releaseall
	end

MirageDunes_EventScript_Mirage3::
	goto_if_set FLAG_MIRAGE_3, MirageDunes_EventScript_MirageNone
	lockall
	playse SE_M_MIST
	call MirageDunes_EventScript_Gone3
	special DrawWholeMapView
	waitse
	setflag FLAG_MIRAGE_3
	msgbox MirageDunes_Text_Mirage, MSGBOX_DEFAULT
	goto_if_unset FLAG_MIRAGE_1, MirageDunes_EventScript_MirageEnd3
	goto_if_unset FLAG_MIRAGE_2, MirageDunes_EventScript_MirageEnd3
	goto_if_unset FLAG_MIRAGE_3, MirageDunes_EventScript_MirageEnd3
	delay 30
	playse SE_M_GUST
	call MirageDunes_EventScript_HazeGone
	special DrawWholeMapView
	delay 40
	msgbox MirageDunes_Text_HazeLifts, MSGBOX_DEFAULT
MirageDunes_EventScript_MirageEnd3::
	releaseall
	end

MirageDunes_EventScript_MirageNone::
	end

MirageDunes_EventScript_Guide::
	lock
	faceplayer
	msgbox MirageDunes_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MirageDunes_EventScript_GuideStay
	msgbox MirageDunes_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_ROUTE111, 20, 106
	waitstate
	release
	end

MirageDunes_EventScript_GuideStay::
	msgbox MirageDunes_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

MirageDunes_EventScript_Sign::
	msgbox MirageDunes_Text_Sign, MSGBOX_SIGN
	end

MirageDunes_EventScript_Camerupt::
	lock
	faceplayer
	playmoncry SPECIES_CAMERUPT, CRY_MODE_NORMAL
	msgbox MirageDunes_Text_Camerupt, MSGBOX_DEFAULT
	waitmoncry
	release
	end

MirageDunes_EventScript_Trader::
	lock
	faceplayer
	goto_if_set FLAG_MIRAGE_SPRING_DONE, MirageDunes_EventScript_TraderDone
	msgbox MirageDunes_Text_Trader, MSGBOX_DEFAULT
	release
	end

MirageDunes_EventScript_TraderDone::
	msgbox MirageDunes_Text_TraderDone, MSGBOX_DEFAULT
	release
	end

MirageDunes_EventScript_Rashid::
	trainerbattle_single TRAINER_MIRAGE_RASHID, MirageDunes_Text_RashidIntro, MirageDunes_Text_RashidDefeat
	msgbox MirageDunes_Text_RashidPost, MSGBOX_AUTOCLOSE
	end

MirageDunes_Text_RashidIntro:
	.string "Lost in the dunes? Me too! Might\n"
	.string "as well have a battle while we're here.$"

MirageDunes_Text_RashidDefeat:
	.string "Sand in my boots…$"

MirageDunes_Text_RashidPost:
	.string "Those pools out there? Don't trust\n"
	.string "them. Walk right in and see.$"


MirageDunes_EventScript_Zahra::
	trainerbattle_single TRAINER_MIRAGE_ZAHRA, MirageDunes_Text_ZahraIntro, MirageDunes_Text_ZahraDefeat
	msgbox MirageDunes_Text_ZahraPost, MSGBOX_AUTOCLOSE
	end

MirageDunes_Text_ZahraIntro:
	.string "The heat plays tricks on the eyes.\n"
	.string "My POKéMON don't play tricks!$"

MirageDunes_Text_ZahraDefeat:
	.string "Seeing things…$"

MirageDunes_Text_ZahraPost:
	.string "Once you see through every mirage,\n"
	.string "the haze in the north pass lifts.$"


MirageDunes_EventScript_Dunstan::
	trainerbattle_single TRAINER_MIRAGE_DUNSTAN, MirageDunes_Text_DunstanIntro, MirageDunes_Text_DunstanDefeat
	msgbox MirageDunes_Text_DunstanPost, MSGBOX_AUTOCLOSE
	end

MirageDunes_Text_DunstanIntro:
	.string "A real oasis lies beyond the haze.\n"
	.string "I've come to find its ruins!$"

MirageDunes_Text_DunstanDefeat:
	.string "Buried, like the old court.$"

MirageDunes_Text_DunstanPost:
	.string "They say a whole court sank beneath\n"
	.string "these dunes. The sand swallows you\l"
	.string "where three stones stand.$"


MirageDunes_Text_Mirage:
	.string "The oasis wavered in the heat…\n"
	.string "and melted away into the sand!\p"
	.string "It was only a mirage.$"

MirageDunes_Text_HazeLifts:
	.string "A hot wind swept across the dunes.\p"
	.string "To the north, the haze has lifted.\n"
	.string "A gate stands in the pass!$"

MirageDunes_Text_GuideBack:
	.string "I'm the caravan's scout. The dunes\n"
	.string "are no place to get lost.\p"
	.string "Head back to ROUTE 111?$"

MirageDunes_Text_GuideLead:
	.string "Follow my footprints!$"

MirageDunes_Text_GuideStay:
	.string "Keep the sun on your left and you'll\n"
	.string "find your way. Mostly.$"

MirageDunes_Text_Sign:
	.string "MIRAGE DUNES\p"
	.string "Beware the sinking sand.\n"
	.string "Trust no water you cannot touch.$"

MirageDunes_Text_Camerupt:
	.string "CAMERUPT: Grooomph…\p"
	.string "It's carrying the caravan's packs.\n"
	.string "It looks very thirsty.$"

MirageDunes_Text_Trader:
	.string "We came to trade at the oasis, but\n"
	.string "every pool we find vanishes when\l"
	.string "we get close!\p"
	.string "The real one must be north, past\n"
	.string "that shimmering haze.$"

MirageDunes_Text_TraderDone:
	.string "The spring is flowing again! Our\n"
	.string "CAMERUPT finally had a long drink.$"

MirageDunes_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

MirageDunes_EventScript_Item_MaxElixir::
	finditem ITEM_MAX_ELIXIR
	end

