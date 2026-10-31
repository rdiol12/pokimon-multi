NimbusCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, NimbusCity_OnLoad
	.byte 0

NimbusCity_OnLoad:
	call_if_set FLAG_NIMBUS_CHIMES_DONE, NimbusCity_EventScript_TowerRings
	end

NimbusCity_EventScript_TowerRings::
	setmetatile 18, 3, 720, TRUE
	setmetatile 19, 3, 721, TRUE
	setmetatile 20, 3, 722, TRUE
	setmetatile 18, 4, 723, TRUE
	setmetatile 19, 4, 724, TRUE
	setmetatile 20, 4, 725, TRUE
	setmetatile 18, 5, 726, TRUE
	setmetatile 19, 5, 727, TRUE
	setmetatile 20, 5, 728, TRUE
	return

NimbusCity_EventScript_Pilot::
	lock
	faceplayer
	msgbox NimbusCity_Text_PilotBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NimbusCity_EventScript_PilotStay
	msgbox NimbusCity_Text_PilotGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_FORTREE_CITY, 3, 11
	waitstate
	release
	end

NimbusCity_EventScript_PilotStay::
	msgbox NimbusCity_Text_PilotStay, MSGBOX_DEFAULT
	release
	end

NimbusCity_EventScript_Keeper::
	lock
	faceplayer
	goto_if_set FLAG_NIMBUS_CHIMES_DONE, NimbusCity_EventScript_KeeperDone
	goto_if_unset FLAG_NIMBUS_CHIME_1, NimbusCity_EventScript_KeeperAsk
	goto_if_unset FLAG_NIMBUS_CHIME_2, NimbusCity_EventScript_KeeperAsk
	goto_if_unset FLAG_NIMBUS_CHIME_3, NimbusCity_EventScript_KeeperAsk
	msgbox NimbusCity_Text_KeeperThanks, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_NIMBUS_CHIMES_DONE
	call NimbusCity_EventScript_TowerRings
	special DrawWholeMapView
	playse SE_BALL_TRAY_EXIT
	waitse
	playse SE_BALL_TRAY_EXIT
	waitse
	msgbox NimbusCity_Text_TowerSings, MSGBOX_DEFAULT
	givemon SPECIES_CASTFORM, 40
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, NimbusCity_EventScript_KeeperFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox NimbusCity_Text_GotCastform, MSGBOX_DEFAULT
	waitfanfare
	msgbox NimbusCity_Text_KeeperWind, MSGBOX_DEFAULT
	release
	end

NimbusCity_EventScript_KeeperFull::
	msgbox NimbusCity_Text_KeeperWind, MSGBOX_DEFAULT
	release
	end

NimbusCity_EventScript_KeeperAsk::
	msgbox NimbusCity_Text_KeeperAsk, MSGBOX_DEFAULT
	release
	end

NimbusCity_EventScript_KeeperDone::
	msgbox NimbusCity_Text_KeeperWind, MSGBOX_DEFAULT
	release
	end

NimbusCity_EventScript_Altaria::
	lock
	faceplayer
	playmoncry SPECIES_ALTARIA, CRY_MODE_NORMAL
	msgbox NimbusCity_Text_Altaria, MSGBOX_DEFAULT
	waitmoncry
	release
	end

NimbusCity_EventScript_Swablu::
	lock
	faceplayer
	playmoncry SPECIES_SWABLU, CRY_MODE_NORMAL
	msgbox NimbusCity_Text_Swablu, MSGBOX_DEFAULT
	waitmoncry
	release
	end

NimbusCity_EventScript_Girl::
	msgbox NimbusCity_Text_Girl, MSGBOX_NPC
	end

NimbusCity_EventScript_Gentleman::
	msgbox NimbusCity_Text_Gentleman, MSGBOX_NPC
	end

NimbusCity_EventScript_Sign::
	msgbox NimbusCity_Text_Sign, MSGBOX_SIGN
	end

NimbusCity_Text_PilotBack:
	.string "Ready to float back down to\n"
	.string "FORTREE CITY?$"

NimbusCity_Text_PilotGo:
	.string "Hold on tight!$"

NimbusCity_Text_PilotStay:
	.string "Enjoy the view. The balloon will\n"
	.string "be right here.$"

NimbusCity_Text_KeeperAsk:
	.string "I keep the CHIME TOWER. Its song\n"
	.string "calls the winds of the isles.\p"
	.string "But last night's storm blew all\n"
	.string "three WIND CHIMES away, onto the\l"
	.string "islands past the rainbow bridge.\p"
	.string "Without their song, the wind up to\n"
	.string "the summit won't blow. Please,\l"
	.string "would you find them?$"

NimbusCity_Text_KeeperThanks:
	.string "All three chimes! Let me hang them\n"
	.string "back in the belfry…$"

NimbusCity_Text_TowerSings:
	.string "The CHIME TOWER is singing!\p"
	.string "Thank you. Please take this CASTFORM.\n"
	.string "It was born in the tower's wind.$"

NimbusCity_Text_GotCastform:
	.string "{PLAYER} received a CASTFORM!$"

NimbusCity_Text_KeeperWind:
	.string "Hear that? The summit wind is\n"
	.string "blowing again. Ride it from the\l"
	.string "eastern island, up to the shrine.$"

NimbusCity_Text_Altaria:
	.string "ALTARIA is humming along with the\n"
	.string "breeze.$"

NimbusCity_Text_Swablu:
	.string "SWABLU is trying to hide in a\n"
	.string "cloud. It blends right in!$"

NimbusCity_Text_Girl:
	.string "Don't walk off the edge of the\n"
	.string "cloud! Grown-ups always say that.$"

NimbusCity_Text_Gentleman:
	.string "The rainbow bridge leads to the\n"
	.string "isles. The winds there blow one\l"
	.string "way only, so mind where you step.$"

NimbusCity_Text_Sign:
	.string "NIMBUS CITY\n"
	.string "The town above the clouds.$"

