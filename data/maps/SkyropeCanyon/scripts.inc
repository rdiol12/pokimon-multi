SkyropeCanyon_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SkyropeCanyon_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, SkyropeCanyon_OnTransition
	.byte 0

@ The mended bridge stays mended.
SkyropeCanyon_OnLoad:
	call_if_set FLAG_SKYROPE_BRIDGE_FIXED, SkyropeCanyon_EventScript_FixBridge
	end

SkyropeCanyon_OnTransition:
	call_if_set FLAG_SKYROPE_HAWLUCHA_DONE, SkyropeCanyon_EventScript_HideHawlucha
	end

SkyropeCanyon_EventScript_HideHawlucha::
	setflag FLAG_HIDE_SKYROPE_HAWLUCHA
	return

SkyropeCanyon_EventScript_FixBridge::
	setmetatile 51, 26, 712, FALSE
	setmetatile 52, 26, 659, FALSE
	setmetatile 53, 26, 659, FALSE
	setmetatile 54, 26, 713, FALSE
	setmetatile 51, 27, 712, FALSE
	setmetatile 52, 27, 659, FALSE
	setmetatile 53, 27, 659, FALSE
	setmetatile 54, 27, 713, FALSE
	setmetatile 51, 28, 712, FALSE
	setmetatile 52, 28, 659, FALSE
	setmetatile 53, 28, 659, FALSE
	setmetatile 54, 28, 713, FALSE
	setmetatile 51, 29, 712, FALSE
	setmetatile 52, 29, 659, FALSE
	setmetatile 53, 29, 659, FALSE
	setmetatile 54, 29, 713, FALSE
	setmetatile 51, 30, 712, FALSE
	setmetatile 52, 30, 659, FALSE
	setmetatile 53, 30, 659, FALSE
	setmetatile 54, 30, 713, FALSE
	setmetatile 51, 31, 712, FALSE
	setmetatile 52, 31, 659, FALSE
	setmetatile 53, 31, 659, FALSE
	setmetatile 54, 31, 713, FALSE
	setmetatile 51, 32, 712, FALSE
	setmetatile 52, 32, 659, FALSE
	setmetatile 53, 32, 659, FALSE
	setmetatile 54, 32, 713, FALSE
	return

SkyropeCanyon_EventScript_Guide::
	lock
	faceplayer
	msgbox SkyropeCanyon_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SkyropeCanyon_EventScript_GuideStay
	msgbox SkyropeCanyon_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_FALLARBOR_TOWN, 17, 11
	waitstate
	release
	end

SkyropeCanyon_EventScript_GuideStay::
	msgbox SkyropeCanyon_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

SkyropeCanyon_EventScript_Sign::
	msgbox SkyropeCanyon_Text_Sign, MSGBOX_SIGN
	end

@ ---- the Bridgewright and his ropes
SkyropeCanyon_EventScript_Bridgewright::
	lock
	faceplayer
	goto_if_set FLAG_SKYROPE_BRIDGE_FIXED, SkyropeCanyon_EventScript_BridgewrightDone
	goto_if_unset FLAG_SKYROPE_ROPE_1, SkyropeCanyon_EventScript_BridgewrightAsk
	goto_if_unset FLAG_SKYROPE_ROPE_2, SkyropeCanyon_EventScript_BridgewrightAsk
	goto_if_unset FLAG_SKYROPE_ROPE_3, SkyropeCanyon_EventScript_BridgewrightAsk
	msgbox SkyropeCanyon_Text_BridgewrightThanks, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_STRENGTH
	call SkyropeCanyon_EventScript_FixBridge
	special DrawWholeMapView
	setflag FLAG_SKYROPE_BRIDGE_FIXED
	delay 30
	fadescreen FADE_FROM_BLACK
	msgbox SkyropeCanyon_Text_BridgewrightFixed, MSGBOX_DEFAULT
	release
	end

SkyropeCanyon_EventScript_BridgewrightAsk::
	setflag FLAG_SKYROPE_QUEST_STARTED
	msgbox SkyropeCanyon_Text_BridgewrightAsk, MSGBOX_DEFAULT
	release
	end

SkyropeCanyon_EventScript_BridgewrightDone::
	msgbox SkyropeCanyon_Text_BridgewrightDone, MSGBOX_DEFAULT
	release
	end

SkyropeCanyon_EventScript_Rope1::
	lock
	setflag FLAG_SKYROPE_ROPE_1
	goto SkyropeCanyon_EventScript_RopeFound

SkyropeCanyon_EventScript_Rope2::
	lock
	setflag FLAG_SKYROPE_ROPE_2
	goto SkyropeCanyon_EventScript_RopeFound

SkyropeCanyon_EventScript_Rope3::
	lock
	setflag FLAG_SKYROPE_ROPE_3
	goto SkyropeCanyon_EventScript_RopeFound

SkyropeCanyon_EventScript_RopeFound::
	removeobject VAR_LAST_TALKED
	playfanfare MUS_OBTAIN_ITEM
	msgbox SkyropeCanyon_Text_RopeFound, MSGBOX_DEFAULT
	waitfanfare
	goto_if_unset FLAG_SKYROPE_ROPE_1, SkyropeCanyon_EventScript_RopeMore
	goto_if_unset FLAG_SKYROPE_ROPE_2, SkyropeCanyon_EventScript_RopeMore
	goto_if_unset FLAG_SKYROPE_ROPE_3, SkyropeCanyon_EventScript_RopeMore
	msgbox SkyropeCanyon_Text_RopeAll, MSGBOX_DEFAULT
	release
	end

SkyropeCanyon_EventScript_RopeMore::
	release
	end

@ ---- Hawlucha on the ledge
SkyropeCanyon_EventScript_Hawlucha::
	lock
	faceplayer
	setwildbattle SPECIES_HAWLUCHA, 36
	playmoncry SPECIES_HAWLUCHA, CRY_MODE_ENCOUNTER
	msgbox SkyropeCanyon_Text_Hawlucha, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, SkyropeCanyon_EventScript_HawluchaStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, SkyropeCanyon_EventScript_HawluchaStays
	setflag FLAG_SKYROPE_HAWLUCHA_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, SkyropeCanyon_EventScript_HawluchaEnd
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_SKYROPE_HAWLUCHA
	setflag FLAG_HIDE_SKYROPE_HAWLUCHA
	fadescreen FADE_FROM_BLACK
	msgbox SkyropeCanyon_Text_HawluchaFlew, MSGBOX_DEFAULT
SkyropeCanyon_EventScript_HawluchaEnd::
	release
	end

SkyropeCanyon_EventScript_HawluchaStays::
	msgbox SkyropeCanyon_Text_HawluchaStays, MSGBOX_DEFAULT
	release
	end

@ ---- people
SkyropeCanyon_EventScript_Artist::
	lock
	faceplayer
	goto_if_set FLAG_SKYROPE_ARTIST_GIFT, SkyropeCanyon_EventScript_ArtistDone
	msgbox SkyropeCanyon_Text_Artist, MSGBOX_DEFAULT
	goto_if_unset FLAG_SKYROPE_BRIDGE_FIXED, SkyropeCanyon_EventScript_ArtistEnd
	msgbox SkyropeCanyon_Text_ArtistBridge, MSGBOX_DEFAULT
	giveitem ITEM_STAR_PIECE
	goto_if_eq VAR_RESULT, FALSE, SkyropeCanyon_EventScript_ArtistEnd
	setflag FLAG_SKYROPE_ARTIST_GIFT
SkyropeCanyon_EventScript_ArtistEnd::
	release
	end

SkyropeCanyon_EventScript_ArtistDone::
	msgbox SkyropeCanyon_Text_ArtistDone, MSGBOX_DEFAULT
	release
	end

SkyropeCanyon_EventScript_Mudbray::
	lock
	faceplayer
	playmoncry SPECIES_MUDBRAY, CRY_MODE_NORMAL
	msgbox SkyropeCanyon_Text_Mudbray, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SkyropeCanyon_EventScript_Rufflet::
	lock
	faceplayer
	playmoncry SPECIES_RUFFLET, CRY_MODE_NORMAL
	msgbox SkyropeCanyon_Text_Rufflet, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SkyropeCanyon_EventScript_Kid::
	msgbox SkyropeCanyon_Text_Kid, MSGBOX_NPC
	end

SkyropeCanyon_EventScript_Hamish::
	trainerbattle_single TRAINER_SKYROPE_HAMISH, SkyropeCanyon_Text_HamishIntro, SkyropeCanyon_Text_HamishDefeat
	msgbox SkyropeCanyon_Text_HamishPost, MSGBOX_AUTOCLOSE
	end

SkyropeCanyon_EventScript_Wren::
	trainerbattle_single TRAINER_SKYROPE_WREN, SkyropeCanyon_Text_WrenIntro, SkyropeCanyon_Text_WrenDefeat
	msgbox SkyropeCanyon_Text_WrenPost, MSGBOX_AUTOCLOSE
	end

SkyropeCanyon_EventScript_Daisy::
	trainerbattle_single TRAINER_SKYROPE_DAISY, SkyropeCanyon_Text_DaisyIntro, SkyropeCanyon_Text_DaisyDefeat
	msgbox SkyropeCanyon_Text_DaisyPost, MSGBOX_AUTOCLOSE
	end

SkyropeCanyon_EventScript_Koji::
	trainerbattle_single TRAINER_SKYROPE_KOJI, SkyropeCanyon_Text_KojiIntro, SkyropeCanyon_Text_KojiDefeat
	msgbox SkyropeCanyon_Text_KojiPost, MSGBOX_AUTOCLOSE
	end

@ ---- text
SkyropeCanyon_Text_GuideBack:
	.string "Had enough wind for one day?\n"
	.string "I can lead you back to FALLARBOR.$"

SkyropeCanyon_Text_GuideLead:
	.string "Mind the gusts on the way down!$"

SkyropeCanyon_Text_GuideStay:
	.string "Take your time. The canyon is at\n"
	.string "its best when the sun is low.$"

SkyropeCanyon_Text_Sign:
	.string "SKYROPE CANYON\n"
	.string "Cross the bridges one at a time!$"

SkyropeCanyon_Text_BridgewrightAsk:
	.string "I'm the BRIDGEWRIGHT. I keep the\n"
	.string "canyon's rope bridges in shape.\p"
	.string "A gust took the planks off the east\n"
	.string "bridge, and my three coils of rope\l"
	.string "blew all over the canyon!\p"
	.string "One went west, one went over the\n"
	.string "gorge to the north, and one is out\l"
	.string "in the grass here somewhere.\p"
	.string "Bring them back and I'll fix the\n"
	.string "bridge in no time.$"

SkyropeCanyon_Text_BridgewrightThanks:
	.string "All three coils! You've got good\n"
	.string "eyes. Stand back, now!$"

SkyropeCanyon_Text_BridgewrightFixed:
	.string "Good as new! There's a ledge on the\n"
	.string "other side nobody has visited in\l"
	.string "years. A HAWLUCHA trains up there.$"

SkyropeCanyon_Text_BridgewrightDone:
	.string "Every bridge in the canyon is\n"
	.string "sound. Cross away!$"

SkyropeCanyon_Text_RopeFound:
	.string "{PLAYER} found a coil of the\n"
	.string "BRIDGEWRIGHT's rope!$"

SkyropeCanyon_Text_RopeAll:
	.string "That's all three coils. Better take\n"
	.string "them to the BRIDGEWRIGHT.$"

SkyropeCanyon_Text_Hawlucha:
	.string "Hawaa!$"

SkyropeCanyon_Text_HawluchaFlew:
	.string "HAWLUCHA leapt off the ledge and\n"
	.string "glided away on the wind.$"

SkyropeCanyon_Text_HawluchaStays:
	.string "HAWLUCHA is still striking poses\n"
	.string "on the ledge.$"

SkyropeCanyon_Text_Artist:
	.string "I paint the canyon every day. The\n"
	.string "rock changes color with the light.\p"
	.string "If only the east bridge were fixed,\n"
	.string "I could paint the far ledge too.$"

SkyropeCanyon_Text_ArtistBridge:
	.string "Oh! The east bridge is fixed? Then\n"
	.string "this painting is done. Thank you!\p"
	.string "Here, I found this in the river\n"
	.string "while washing my brushes.$"

SkyropeCanyon_Text_ArtistDone:
	.string "Look at the light on the mesas.\n"
	.string "Perfect.$"

SkyropeCanyon_Text_Mudbray:
	.string "MUDBRAY is happily kicking up dust.$"

SkyropeCanyon_Text_Rufflet:
	.string "The RUFFLET puffs up its chest and\n"
	.string "glares. It's guarding its nest!$"

SkyropeCanyon_Text_Kid:
	.string "If you look down from the bridge,\n"
	.string "you can see BASCULIN fighting in\l"
	.string "the river!$"

SkyropeCanyon_Text_HamishIntro:
	.string "These mesas are made of layers and\n"
	.string "layers of old rock. Like my team!$"

SkyropeCanyon_Text_HamishDefeat:
	.string "Crumbled…$"

SkyropeCanyon_Text_HamishPost:
	.string "Some ledges hide items. Look\n"
	.string "closely at the dead ends.$"

SkyropeCanyon_Text_WrenIntro:
	.string "My birds ride the canyon winds all\n"
	.string "day long!$"

SkyropeCanyon_Text_WrenDefeat:
	.string "Blown away!$"

SkyropeCanyon_Text_WrenPost:
	.string "RUFFLET nest up in the cliffs.\n"
	.string "They're brave little birds.$"

SkyropeCanyon_Text_DaisyIntro:
	.string "A picnic in the canyon! Want to\n"
	.string "battle before lunch?$"

SkyropeCanyon_Text_DaisyDefeat:
	.string "My sandwiches went flying!$"

SkyropeCanyon_Text_DaisyPost:
	.string "The BRIDGEWRIGHT's camp is just up\n"
	.string "the trail. He looks worried.$"

SkyropeCanyon_Text_KojiIntro:
	.string "I came over the old bridge to train\n"
	.string "with the HAWLUCHA. Show me your\l"
	.string "strength!$"

SkyropeCanyon_Text_KojiDefeat:
	.string "A fine flying press!$"

SkyropeCanyon_Text_KojiPost:
	.string "HAWLUCHA only respect those who\n"
	.string "beat them fair and square.$"

SkyropeCanyon_EventScript_Item_RareCandy::
	finditem ITEM_RARE_CANDY
	end

SkyropeCanyon_EventScript_Item_SharpBeak::
	finditem ITEM_SHARP_BEAK
	end

SkyropeCanyon_EventScript_Item_HardStone::
	finditem ITEM_HARD_STONE
	end

SkyropeCanyon_EventScript_Item_MaxRepel::
	finditem ITEM_MAX_REPEL
	end

SkyropeCanyon_EventScript_Item_TmAerialAce::
	finditem ITEM_TM_AERIAL_ACE
	end

