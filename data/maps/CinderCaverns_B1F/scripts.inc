CinderCaverns_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, CinderCaverns_B1F_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, CinderCaverns_B1F_OnTransition
	.byte 0

@ After the League, the volcanologist steps aside from the Molten Heart.
CinderCaverns_B1F_OnTransition:
	goto_if_unset FLAG_SYS_GAME_CLEAR, CinderCaverns_B1F_OnTransitionEnd
	setobjectxyperm LOCALID_CINDER_GUARD, 22, 3
	setobjectmovementtype LOCALID_CINDER_GUARD, MOVEMENT_TYPE_FACE_RIGHT
CinderCaverns_B1F_OnTransitionEnd:
	end

@ Cooled stretches of the causeway stay cool.
CinderCaverns_B1F_OnLoad:
	call_if_set FLAG_CINDER_VALVE_WEST, CinderCaverns_B1F_EventScript_CoolWest
	call_if_set FLAG_CINDER_VALVE_EAST, CinderCaverns_B1F_EventScript_CoolEast
	call_if_set FLAG_CINDER_VALVE_SOUTH, CinderCaverns_B1F_EventScript_CoolSouth
	end

CinderCaverns_B1F_EventScript_ValveDone::
	msgbox CinderCaverns_B1F_Text_ValveDone, MSGBOX_DEFAULT
CinderCaverns_B1F_EventScript_ValveLeave::
	releaseall
	end

CinderCaverns_B1F_EventScript_ValveWest::
	lockall
	goto_if_set FLAG_CINDER_VALVE_WEST, CinderCaverns_B1F_EventScript_ValveDone
	msgbox CinderCaverns_B1F_Text_Valve, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinderCaverns_B1F_EventScript_ValveLeave
	playse SE_PIN
	setflag FLAG_CINDER_VALVE_WEST
	call CinderCaverns_B1F_EventScript_CoolWest
	special DrawWholeMapView
	playse SE_M_MIST
	waitse
	msgbox CinderCaverns_B1F_Text_Cooled, MSGBOX_DEFAULT
	releaseall
	end

CinderCaverns_B1F_EventScript_CoolWest::
	setmetatile 23, 5, 776, FALSE
	setmetatile 24, 5, 776, FALSE
	setmetatile 23, 6, 776, FALSE
	setmetatile 24, 6, 776, FALSE
	setmetatile 23, 7, 776, FALSE
	setmetatile 24, 7, 776, FALSE
	setmetatile 23, 8, 776, FALSE
	setmetatile 24, 8, 776, FALSE
	setmetatile 23, 9, 776, FALSE
	setmetatile 24, 9, 776, FALSE
	setmetatile 23, 10, 776, FALSE
	setmetatile 24, 10, 776, FALSE
	setmetatile 23, 11, 776, FALSE
	setmetatile 24, 11, 776, FALSE
	setmetatile 23, 12, 776, FALSE
	setmetatile 24, 12, 776, FALSE
	setmetatile 23, 13, 776, FALSE
	setmetatile 24, 13, 776, FALSE
	setmetatile 23, 14, 776, FALSE
	setmetatile 24, 14, 776, FALSE
	return

CinderCaverns_B1F_EventScript_ValveEast::
	lockall
	goto_if_set FLAG_CINDER_VALVE_EAST, CinderCaverns_B1F_EventScript_ValveDone
	msgbox CinderCaverns_B1F_Text_Valve, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinderCaverns_B1F_EventScript_ValveLeave
	playse SE_PIN
	setflag FLAG_CINDER_VALVE_EAST
	call CinderCaverns_B1F_EventScript_CoolEast
	special DrawWholeMapView
	playse SE_M_MIST
	waitse
	msgbox CinderCaverns_B1F_Text_Cooled, MSGBOX_DEFAULT
	releaseall
	end

CinderCaverns_B1F_EventScript_CoolEast::
	setmetatile 23, 15, 776, FALSE
	setmetatile 24, 15, 776, FALSE
	setmetatile 23, 16, 776, FALSE
	setmetatile 24, 16, 776, FALSE
	setmetatile 23, 17, 776, FALSE
	setmetatile 24, 17, 776, FALSE
	setmetatile 23, 18, 776, FALSE
	setmetatile 24, 18, 776, FALSE
	setmetatile 23, 19, 776, FALSE
	setmetatile 24, 19, 776, FALSE
	setmetatile 23, 20, 776, FALSE
	setmetatile 24, 20, 776, FALSE
	setmetatile 23, 21, 776, FALSE
	setmetatile 24, 21, 776, FALSE
	setmetatile 23, 22, 776, FALSE
	setmetatile 24, 22, 776, FALSE
	return

CinderCaverns_B1F_EventScript_ValveSouth::
	lockall
	goto_if_set FLAG_CINDER_VALVE_SOUTH, CinderCaverns_B1F_EventScript_ValveDone
	msgbox CinderCaverns_B1F_Text_Valve, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinderCaverns_B1F_EventScript_ValveLeave
	playse SE_PIN
	setflag FLAG_CINDER_VALVE_SOUTH
	call CinderCaverns_B1F_EventScript_CoolSouth
	special DrawWholeMapView
	playse SE_M_MIST
	waitse
	msgbox CinderCaverns_B1F_Text_Cooled, MSGBOX_DEFAULT
	releaseall
	end

CinderCaverns_B1F_EventScript_CoolSouth::
	setmetatile 23, 23, 776, FALSE
	setmetatile 24, 23, 776, FALSE
	setmetatile 23, 24, 776, FALSE
	setmetatile 24, 24, 776, FALSE
	setmetatile 23, 25, 776, FALSE
	setmetatile 24, 25, 776, FALSE
	setmetatile 23, 26, 776, FALSE
	setmetatile 24, 26, 776, FALSE
	setmetatile 23, 27, 776, FALSE
	setmetatile 24, 27, 776, FALSE
	setmetatile 23, 28, 776, FALSE
	setmetatile 24, 28, 776, FALSE
	setmetatile 23, 29, 776, FALSE
	setmetatile 24, 29, 776, FALSE
	setmetatile 23, 30, 776, FALSE
	setmetatile 24, 30, 776, FALSE
	setmetatile 23, 31, 776, FALSE
	setmetatile 24, 31, 776, FALSE
	return

CinderCaverns_B1F_EventScript_Bruno::
	trainerbattle_single TRAINER_CINDER_BRUNO, CinderCaverns_B1F_Text_BrunoIntro, CinderCaverns_B1F_Text_BrunoDefeat
	msgbox CinderCaverns_B1F_Text_BrunoPost, MSGBOX_AUTOCLOSE
	end

CinderCaverns_B1F_EventScript_Tess::
	trainerbattle_single TRAINER_CINDER_TESS, CinderCaverns_B1F_Text_TessIntro, CinderCaverns_B1F_Text_TessDefeat
	msgbox CinderCaverns_B1F_Text_TessPost, MSGBOX_AUTOCLOSE
	end

@ The Molten Heart is only for Champions.
CinderCaverns_B1F_EventScript_Guard::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, CinderCaverns_B1F_EventScript_GuardOk
	msgbox CinderCaverns_B1F_Text_GuardNo, MSGBOX_DEFAULT
	release
	end

CinderCaverns_B1F_EventScript_GuardOk::
	msgbox CinderCaverns_B1F_Text_GuardOk, MSGBOX_DEFAULT
	release
	end

CinderCaverns_B1F_Text_Valve:
	.string "A rusty old valve with a MAGMA\n"
	.string "symbol. Turn it?$"

CinderCaverns_B1F_Text_Cooled:
	.string "Steam roars up from the lake!\n"
	.string "Part of the lava has cooled into\l"
	.string "solid rock.$"

CinderCaverns_B1F_Text_ValveDone:
	.string "The valve is turned all the way.$"

CinderCaverns_B1F_Text_BrunoIntro:
	.string "I came to study the lava rocks.\n"
	.string "And to battle, of course!$"

CinderCaverns_B1F_Text_BrunoDefeat:
	.string "You're a hot one!$"

CinderCaverns_B1F_Text_BrunoPost:
	.string "Those valves are TEAM MAGMA's work.\n"
	.string "At least they're useful for once.$"

CinderCaverns_B1F_Text_TessIntro:
	.string "Only the toughest trainers make it\n"
	.string "this deep. Are you one of them?$"

CinderCaverns_B1F_Text_TessDefeat:
	.string "You really are.$"

CinderCaverns_B1F_Text_TessPost:
	.string "There's a valve in the far south-west\n"
	.string "too. Easy to miss!$"

CinderCaverns_B1F_Text_GuardNo:
	.string "Behind me is the MOLTEN HEART. Its\n"
	.string "heat could melt a lesser trainer.\p"
	.string "Come back once you're CHAMPION.$"

CinderCaverns_B1F_Text_GuardOk:
	.string "CHAMPION, you may enter. Something\n"
	.string "is breathing in there… slowly.$"

CinderCaverns_B1F_EventScript_Item_Magmarizer::
	finditem ITEM_MAGMARIZER
	end

CinderCaverns_B1F_EventScript_Item_FullRestore::
	finditem ITEM_FULL_RESTORE
	end

CinderCaverns_B1F_EventScript_Item_TmFlamethrower::
	finditem ITEM_TM_FLAMETHROWER
	end

