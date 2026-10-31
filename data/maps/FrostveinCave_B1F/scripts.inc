FrostveinCave_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FrostveinCave_B1F_OnTransition
	.byte 0

FrostveinCave_B1F_OnTransition:
	call_if_set FLAG_FROSTVEIN_AVALUGG_DONE, FrostveinCave_B1F_EventScript_HideAvalugg
	end

FrostveinCave_B1F_EventScript_HideAvalugg::
	setflag FLAG_HIDE_FROSTVEIN_AVALUGG
	return

@ Avalugg sleeps in the doorway of the shrine.
FrostveinCave_B1F_EventScript_Avalugg::
	lock
	faceplayer
	msgbox FrostveinCave_B1F_Text_AvaluggSleeps, MSGBOX_DEFAULT
	setwildbattle SPECIES_AVALUGG, 44
	playmoncry SPECIES_AVALUGG, CRY_MODE_ENCOUNTER
	msgbox FrostveinCave_B1F_Text_AvaluggWakes, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, FrostveinCave_B1F_EventScript_AvaluggStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, FrostveinCave_B1F_EventScript_AvaluggStays
	setflag FLAG_FROSTVEIN_AVALUGG_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, FrostveinCave_B1F_EventScript_AvaluggEnd
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_FROSTVEIN_AVALUGG
	setflag FLAG_HIDE_FROSTVEIN_AVALUGG
	fadescreen FADE_FROM_BLACK
	msgbox FrostveinCave_B1F_Text_AvaluggLeft, MSGBOX_DEFAULT
FrostveinCave_B1F_EventScript_AvaluggEnd::
	release
	end

FrostveinCave_B1F_EventScript_AvaluggStays::
	msgbox FrostveinCave_B1F_Text_AvaluggStays, MSGBOX_DEFAULT
	release
	end

@ The Ice Priestess gives a Frigibax to whoever reaches the shrine.
FrostveinCave_B1F_EventScript_Priestess::
	lock
	faceplayer
	goto_if_set FLAG_FROSTVEIN_FRIGIBAX, FrostveinCave_B1F_EventScript_PriestessDone
	msgbox FrostveinCave_B1F_Text_Priestess, MSGBOX_DEFAULT
	givemon SPECIES_FRIGIBAX, 25
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, FrostveinCave_B1F_EventScript_PriestessFull
	setflag FLAG_FROSTVEIN_FRIGIBAX
	playfanfare MUS_OBTAIN_ITEM
	msgbox FrostveinCave_B1F_Text_GotFrigibax, MSGBOX_DEFAULT
	waitfanfare
	msgbox FrostveinCave_B1F_Text_PriestessAfter, MSGBOX_DEFAULT
	release
	end

FrostveinCave_B1F_EventScript_PriestessFull::
	msgbox FrostveinCave_B1F_Text_PriestessFull, MSGBOX_DEFAULT
	release
	end

FrostveinCave_B1F_EventScript_PriestessDone::
	msgbox FrostveinCave_B1F_Text_PriestessDone, MSGBOX_DEFAULT
	release
	end

FrostveinCave_B1F_EventScript_Niva::
	trainerbattle_single TRAINER_FROSTVEIN_NIVA, FrostveinCave_B1F_Text_NivaIntro, FrostveinCave_B1F_Text_NivaDefeat
	msgbox FrostveinCave_B1F_Text_NivaPost, MSGBOX_AUTOCLOSE
	end

FrostveinCave_B1F_EventScript_Sora::
	trainerbattle_single TRAINER_FROSTVEIN_SORA, FrostveinCave_B1F_Text_SoraIntro, FrostveinCave_B1F_Text_SoraDefeat
	msgbox FrostveinCave_B1F_Text_SoraPost, MSGBOX_AUTOCLOSE
	end

FrostveinCave_B1F_Text_AvaluggSleeps:
	.string "A huge AVALUGG is asleep in the\n"
	.string "doorway. Its breath fogs the air…$"

FrostveinCave_B1F_Text_AvaluggWakes:
	.string "Grrooooh!$"

FrostveinCave_B1F_Text_AvaluggLeft:
	.string "AVALUGG lumbered off into the dark.$"

FrostveinCave_B1F_Text_AvaluggStays:
	.string "AVALUGG settled back down in the\n"
	.string "doorway.$"

FrostveinCave_B1F_Text_Priestess:
	.string "You crossed the ice and woke the\n"
	.string "guardian. Few have done both.\p"
	.string "This shrine looks after a FRIGIBAX\n"
	.string "egg that hatched in the cold.\p"
	.string "It needs a trainer who can keep a\n"
	.string "cool head. Please take it.$"

FrostveinCave_B1F_Text_GotFrigibax:
	.string "{PLAYER} received FRIGIBAX!$"

FrostveinCave_B1F_Text_PriestessAfter:
	.string "Raise it well. One day it may grow\n"
	.string "into a mighty BAXCALIBUR.$"

FrostveinCave_B1F_Text_PriestessFull:
	.string "Your party and boxes are full.\n"
	.string "Make some room and come back.$"

FrostveinCave_B1F_Text_PriestessDone:
	.string "The cave is quieter now. The ice\n"
	.string "remembers you.$"

FrostveinCave_B1F_Text_NivaIntro:
	.string "You made it down here? Then let's\n"
	.string "see how cool you really are!$"

FrostveinCave_B1F_Text_NivaDefeat:
	.string "Frozen solid…$"

FrostveinCave_B1F_Text_NivaPost:
	.string "I always slide too far on that big\n"
	.string "field. There's a trick to it.$"

FrostveinCave_B1F_Text_SoraIntro:
	.string "I sensed you coming across the ice.\n"
	.string "Every slide!$"

FrostveinCave_B1F_Text_SoraDefeat:
	.string "I didn't sense that coming.$"

FrostveinCave_B1F_Text_SoraPost:
	.string "The guardian sleeps, but it dreams\n"
	.string "of the shrine.$"

FrostveinCave_B1F_EventScript_Item_NeverMeltIce::
	finditem ITEM_NEVER_MELT_ICE
	end

FrostveinCave_B1F_EventScript_Item_TmIceBeam::
	finditem ITEM_TM_ICE_BEAM
	end

FrostveinCave_B1F_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

FrostveinCave_B1F_EventScript_Item_IcyRock::
	finditem ITEM_ICY_ROCK
	end

