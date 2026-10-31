GullwingPoint_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, GullwingPoint_OnLoad
	.byte 0

GullwingPoint_OnLoad:
	call_if_set FLAG_GULL_MIRROR_1, GullwingPoint_OnLoadMirror1
	call_if_set FLAG_GULL_MIRROR_2, GullwingPoint_OnLoadMirror2
	call_if_set FLAG_GULL_MIRROR_3, GullwingPoint_OnLoadMirror3
	call_if_set FLAG_GULL_MIRROR_4, GullwingPoint_OnLoadMirror4
	call_if_set FLAG_GULL_BEAM_SOLVED, GullwingPoint_ShowBeam
	end

GullwingPoint_OnLoadMirror1:
	setmetatile 41, 15, 802, TRUE
	return

GullwingPoint_OnLoadMirror2:
	setmetatile 33, 15, 802, TRUE
	return

GullwingPoint_OnLoadMirror3:
	setmetatile 33, 22, 801, TRUE
	return

GullwingPoint_OnLoadMirror4:
	setmetatile 37, 22, 801, TRUE
	return

GullwingPoint_ShowBeam:
	setmetatile 41, 11, 804, FALSE
	setmetatile 41, 12, 804, FALSE
	setmetatile 41, 13, 804, FALSE
	setmetatile 41, 14, 804, FALSE
	setmetatile 34, 15, 803, FALSE
	setmetatile 35, 15, 803, FALSE
	setmetatile 36, 15, 803, FALSE
	setmetatile 37, 15, 803, FALSE
	setmetatile 38, 15, 803, FALSE
	setmetatile 39, 15, 803, FALSE
	setmetatile 40, 15, 803, FALSE
	setmetatile 33, 16, 804, FALSE
	setmetatile 33, 17, 804, FALSE
	setmetatile 33, 18, 804, FALSE
	setmetatile 33, 19, 804, FALSE
	setmetatile 33, 20, 804, FALSE
	setmetatile 33, 21, 804, FALSE
	setmetatile 34, 22, 803, FALSE
	setmetatile 35, 22, 803, FALSE
	setmetatile 36, 22, 803, FALSE
	setmetatile 37, 23, 804, FALSE
	setmetatile 37, 24, 804, FALSE
	setmetatile 37, 25, 804, FALSE
	setmetatile 37, 26, 804, FALSE
	setmetatile 37, 27, 804, FALSE
	setmetatile 37, 28, 804, FALSE
	setmetatile 37, 29, 804, FALSE
	setmetatile 37, 30, 804, FALSE
	setmetatile 27, 35, 865, FALSE
	return

GullwingPoint_EventScript_Boatman::
	lock
	faceplayer
	msgbox GullwingPoint_Text_BoatmanBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GullwingPoint_EventScript_BoatmanStay
	msgbox GullwingPoint_Text_BoatmanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_DEWFORD_TOWN, 5, 12
	waitstate
	release
	end

GullwingPoint_EventScript_BoatmanStay::
	msgbox GullwingPoint_Text_BoatmanStay, MSGBOX_DEFAULT
	release
	end

GullwingPoint_EventScript_Sign::
	msgbox GullwingPoint_Text_Sign, MSGBOX_SIGN
	end

GullwingPoint_EventScript_Plaque::
	msgbox GullwingPoint_Text_Plaque, MSGBOX_SIGN
	end

@ ---- the keeper and the mirror puzzle
GullwingPoint_EventScript_Keeper::
	lock
	faceplayer
	goto_if_set FLAG_GULL_CAPTAIN_BEATEN, GullwingPoint_EventScript_KeeperThanks
	goto_if_set FLAG_GULL_BEAM_SOLVED, GullwingPoint_EventScript_KeeperGo
	setflag FLAG_GULL_KEEPER_TALKED
	msgbox GullwingPoint_Text_KeeperAsk, MSGBOX_DEFAULT
	release
	end

GullwingPoint_EventScript_KeeperGo::
	msgbox GullwingPoint_Text_KeeperGo, MSGBOX_DEFAULT
	release
	end

GullwingPoint_EventScript_KeeperThanks::
	msgbox GullwingPoint_Text_KeeperThanks, MSGBOX_DEFAULT
	release
	end

GullwingPoint_EventScript_Mirror1::
	lockall
	goto_if_set FLAG_GULL_BEAM_SOLVED, GullwingPoint_EventScript_MirrorDone
	msgbox GullwingPoint_Text_MirrorTurn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GullwingPoint_EventScript_MirrorLeave
	playse SE_CLICK
	goto_if_set FLAG_GULL_MIRROR_1, GullwingPoint_EventScript_Mirror1Back
	setflag FLAG_GULL_MIRROR_1
	setmetatile 41, 15, 802, TRUE
	special DrawWholeMapView
	goto GullwingPoint_EventScript_CheckBeam

GullwingPoint_EventScript_Mirror1Back::
	clearflag FLAG_GULL_MIRROR_1
	setmetatile 41, 15, 801, TRUE
	special DrawWholeMapView
	releaseall
	end

GullwingPoint_EventScript_Mirror2::
	lockall
	goto_if_set FLAG_GULL_BEAM_SOLVED, GullwingPoint_EventScript_MirrorDone
	msgbox GullwingPoint_Text_MirrorTurn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GullwingPoint_EventScript_MirrorLeave
	playse SE_CLICK
	goto_if_set FLAG_GULL_MIRROR_2, GullwingPoint_EventScript_Mirror2Back
	setflag FLAG_GULL_MIRROR_2
	setmetatile 33, 15, 802, TRUE
	special DrawWholeMapView
	goto GullwingPoint_EventScript_CheckBeam

GullwingPoint_EventScript_Mirror2Back::
	clearflag FLAG_GULL_MIRROR_2
	setmetatile 33, 15, 801, TRUE
	special DrawWholeMapView
	releaseall
	end

GullwingPoint_EventScript_Mirror3::
	lockall
	goto_if_set FLAG_GULL_BEAM_SOLVED, GullwingPoint_EventScript_MirrorDone
	msgbox GullwingPoint_Text_MirrorTurn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GullwingPoint_EventScript_MirrorLeave
	playse SE_CLICK
	goto_if_set FLAG_GULL_MIRROR_3, GullwingPoint_EventScript_Mirror3Back
	setflag FLAG_GULL_MIRROR_3
	setmetatile 33, 22, 801, TRUE
	special DrawWholeMapView
	goto GullwingPoint_EventScript_CheckBeam

GullwingPoint_EventScript_Mirror3Back::
	clearflag FLAG_GULL_MIRROR_3
	setmetatile 33, 22, 802, TRUE
	special DrawWholeMapView
	releaseall
	end

GullwingPoint_EventScript_Mirror4::
	lockall
	goto_if_set FLAG_GULL_BEAM_SOLVED, GullwingPoint_EventScript_MirrorDone
	msgbox GullwingPoint_Text_MirrorTurn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GullwingPoint_EventScript_MirrorLeave
	playse SE_CLICK
	goto_if_set FLAG_GULL_MIRROR_4, GullwingPoint_EventScript_Mirror4Back
	setflag FLAG_GULL_MIRROR_4
	setmetatile 37, 22, 801, TRUE
	special DrawWholeMapView
	goto GullwingPoint_EventScript_CheckBeam

GullwingPoint_EventScript_Mirror4Back::
	clearflag FLAG_GULL_MIRROR_4
	setmetatile 37, 22, 802, TRUE
	special DrawWholeMapView
	releaseall
	end

GullwingPoint_EventScript_MirrorDone::
	msgbox GullwingPoint_Text_MirrorDone, MSGBOX_DEFAULT
	releaseall
	end

GullwingPoint_EventScript_MirrorLeave::
	releaseall
	end

GullwingPoint_EventScript_CheckBeam::
	goto_if_unset FLAG_GULL_MIRROR_1, GullwingPoint_EventScript_BeamNo
	goto_if_unset FLAG_GULL_MIRROR_2, GullwingPoint_EventScript_BeamNo
	goto_if_unset FLAG_GULL_MIRROR_3, GullwingPoint_EventScript_BeamNo
	goto_if_unset FLAG_GULL_MIRROR_4, GullwingPoint_EventScript_BeamNo
	setflag FLAG_GULL_BEAM_SOLVED
	playse SE_M_REFLECT
	call GullwingPoint_ShowBeam
	special DrawWholeMapView
	waitse
	msgbox GullwingPoint_Text_BeamHits, MSGBOX_DEFAULT
	playse SE_M_SURF
	waitse
	msgbox GullwingPoint_Text_Gangplank, MSGBOX_DEFAULT
	releaseall
	end

GullwingPoint_EventScript_BeamNo::
	releaseall
	end

@ ---- people and Pokémon
GullwingPoint_EventScript_Fisher::
	msgbox GullwingPoint_Text_Fisher, MSGBOX_NPC
	end

GullwingPoint_EventScript_Woman::
	msgbox GullwingPoint_Text_Woman, MSGBOX_NPC
	end

GullwingPoint_EventScript_Boy::
	msgbox GullwingPoint_Text_Boy, MSGBOX_NPC
	end

GullwingPoint_EventScript_Pelipper::
	lock
	faceplayer
	playmoncry SPECIES_PELIPPER, CRY_MODE_NORMAL
	msgbox GullwingPoint_Text_Pelipper, MSGBOX_DEFAULT
	waitmoncry
	release
	end

GullwingPoint_EventScript_Wingull::
	lock
	faceplayer
	playmoncry SPECIES_WINGULL, CRY_MODE_NORMAL
	msgbox GullwingPoint_Text_Wingull, MSGBOX_DEFAULT
	waitmoncry
	release
	end

GullwingPoint_EventScript_Skerry::
	trainerbattle_single TRAINER_GULL_SKERRY, GullwingPoint_Text_SkerryIntro, GullwingPoint_Text_SkerryDefeat
	msgbox GullwingPoint_Text_SkerryPost, MSGBOX_AUTOCLOSE
	end

GullwingPoint_EventScript_Bilge::
	trainerbattle_single TRAINER_GULL_BILGE, GullwingPoint_Text_BilgeIntro, GullwingPoint_Text_BilgeDefeat
	msgbox GullwingPoint_Text_BilgePost, MSGBOX_AUTOCLOSE
	end

GullwingPoint_EventScript_Maree::
	trainerbattle_single TRAINER_GULL_MAREE, GullwingPoint_Text_MareeIntro, GullwingPoint_Text_MareeDefeat
	msgbox GullwingPoint_Text_MareePost, MSGBOX_AUTOCLOSE
	end

@ ---- text
GullwingPoint_Text_BoatmanBack:
	.string "Shall I row you back to DEWFORD?$"

GullwingPoint_Text_BoatmanGo:
	.string "Hold on to your hat!$"

GullwingPoint_Text_BoatmanStay:
	.string "Mind the pirates. They're not as\n"
	.string "friendly as the WINGULL.$"

GullwingPoint_Text_Sign:
	.string "GULLWING POINT\n"
	.string "Where the sea wind never rests.$"

GullwingPoint_Text_Plaque:
	.string "“The keeper's beam runs south from\n"
	.string "the lamp. Each mirror turns it to\l"
	.string "the left or to the right.\p"
	.string "Set them well and the light will\n"
	.string "reach any ship in the bay.”$"

GullwingPoint_Text_KeeperAsk:
	.string "Ah, a traveller. You've seen that\n"
	.string "ship in the bay? Pirates!\p"
	.string "They hide in the fog and slip out at\n"
	.string "night to rob the fishing boats.\p"
	.string "My beam could catch them red-handed,\n"
	.string "but the mirrors on the cliff are all\l"
	.string "turned the wrong way.\p"
	.string "Turn them so the beam lands on that\n"
	.string "ship, and the whole coast will see\l"
	.string "them!$"

GullwingPoint_Text_KeeperGo:
	.string "You did it! They've lowered their\n"
	.string "gangplank. They want a fight!\p"
	.string "Go and show them GULLWING POINT\n"
	.string "won't be pushed around.$"

GullwingPoint_Text_KeeperThanks:
	.string "The bay is quiet again, thanks to\n"
	.string "you. My lamp will shine for you\l"
	.string "every night.$"

GullwingPoint_Text_MirrorTurn:
	.string "A big mirror on a stone base. It\n"
	.string "can be turned. Turn it?$"

GullwingPoint_Text_MirrorDone:
	.string "The mirror is sending the beam on\n"
	.string "its way.$"

GullwingPoint_Text_BeamHits:
	.string "The lighthouse beam bounced from\n"
	.string "mirror to mirror… and lit up the\l"
	.string "pirate ship!$"

GullwingPoint_Text_Gangplank:
	.string "Someone on the ship shouted, and a\n"
	.string "gangplank thudded onto the pier!$"

GullwingPoint_Text_Fisher:
	.string "Those pirates took my best rod. If\n"
	.string "you go aboard, give them a thump\l"
	.string "for me.$"

GullwingPoint_Text_Woman:
	.string "We hang our washing out on the cape.\n"
	.string "The wind dries it in minutes!$"

GullwingPoint_Text_Boy:
	.string "The lighthouse keeper's mirrors are\n"
	.string "like a puzzle. I can never solve it.$"

GullwingPoint_Text_Pelipper:
	.string "PELIPPER is carrying a letter in its\n"
	.string "beak. It won't let go of it.$"

GullwingPoint_Text_Wingull:
	.string "WINGULL rides the wind without\n"
	.string "flapping its wings.$"

GullwingPoint_Text_SkerryIntro:
	.string "Arr! Ye shouldn't be snooping round\n"
	.string "the mirrors, landlubber!$"

GullwingPoint_Text_SkerryDefeat:
	.string "Shiver me timbers!$"

GullwingPoint_Text_SkerryPost:
	.string "The CAPTAIN won't be happy…$"

GullwingPoint_Text_BilgeIntro:
	.string "Nobody walks this pier without\n"
	.string "paying the toll! The toll is a\l"
	.string "battle!$"

GullwingPoint_Text_BilgeDefeat:
	.string "Toll paid… by me.$"

GullwingPoint_Text_BilgePost:
	.string "Our ship only shows itself when the\n"
	.string "fog lifts. Or when someone shines a\l"
	.string "light on it.$"

GullwingPoint_Text_MareeIntro:
	.string "I'm going to be a pirate when I grow\n"
	.string "up! Practice battle!$"

GullwingPoint_Text_MareeDefeat:
	.string "Walked the plank!$"

GullwingPoint_Text_MareePost:
	.string "Maybe I'll be a lighthouse keeper\n"
	.string "instead.$"

GullwingPoint_EventScript_Item_NetBall::
	finditem ITEM_NET_BALL
	end

GullwingPoint_EventScript_Item_TmRainDance::
	finditem ITEM_TM_RAIN_DANCE
	end

GullwingPoint_EventScript_Item_ShellBell::
	finditem ITEM_SHELL_BELL
	end

