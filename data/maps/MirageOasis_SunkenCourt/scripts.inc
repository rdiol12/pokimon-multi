MirageOasis_SunkenCourt_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, MirageOasis_SunkenCourt_OnLoad
	.byte 0

MirageOasis_SunkenCourt_OnLoad:
	call_if_set FLAG_MIRAGE_SPRING_STONE, MirageOasis_SunkenCourt_EventScript_AltarEmpty
	end

MirageOasis_SunkenCourt_EventScript_AltarEmpty::
	setmetatile 19, 8, 595, TRUE
	setmetatile 20, 8, 596, TRUE
	setmetatile 19, 9, 597, TRUE
	setmetatile 20, 9, 598, TRUE
	return

MirageOasis_SunkenCourt_EventScript_Altar::
	lockall
	goto_if_set FLAG_MIRAGE_SPRING_STONE, MirageOasis_SunkenCourt_EventScript_AltarBare
	msgbox MirageOasis_SunkenCourt_Text_Altar, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MirageOasis_SunkenCourt_EventScript_AltarLeave
	call MirageOasis_SunkenCourt_EventScript_AltarEmpty
	special DrawWholeMapView
	setflag FLAG_MIRAGE_SPRING_STONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox MirageOasis_SunkenCourt_Text_GotStone, MSGBOX_DEFAULT
	waitfanfare
	msgbox MirageOasis_SunkenCourt_Text_StoneCool, MSGBOX_DEFAULT
	releaseall
	end

MirageOasis_SunkenCourt_EventScript_AltarBare::
	msgbox MirageOasis_SunkenCourt_Text_AltarBare, MSGBOX_DEFAULT
MirageOasis_SunkenCourt_EventScript_AltarLeave::
	releaseall
	end

MirageOasis_SunkenCourt_EventScript_Nefera::
	trainerbattle_single TRAINER_MIRAGE_NEFERA, MirageOasis_SunkenCourt_Text_NeferaIntro, MirageOasis_SunkenCourt_Text_NeferaDefeat
	msgbox MirageOasis_SunkenCourt_Text_NeferaPost, MSGBOX_AUTOCLOSE
	end

MirageOasis_SunkenCourt_Text_NeferaIntro:
	.string "You fell in too? Ha! This court is\n"
	.string "cursed. Let me show you!$"

MirageOasis_SunkenCourt_Text_NeferaDefeat:
	.string "The curse is broken…$"

MirageOasis_SunkenCourt_Text_NeferaPost:
	.string "The stairs lead back up to the\n"
	.string "dunes. Every court has its own.$"


MirageOasis_SunkenCourt_Text_Altar:
	.string "A blue stone rests on the altar,\n"
	.string "cool and damp to the touch.\p"
	.string "Take the stone?$"

MirageOasis_SunkenCourt_Text_GotStone:
	.string "{PLAYER} found the SPRING STONE!$"

MirageOasis_SunkenCourt_Text_StoneCool:
	.string "Water beads on its surface, though\n"
	.string "the court is dry as bone.$"

MirageOasis_SunkenCourt_Text_AltarBare:
	.string "The altar is bare. Sand trickles\n"
	.string "where the stone used to sit.$"

MirageOasis_SunkenCourt_EventScript_Item_TmEarthquake::
	finditem ITEM_TM_EARTHQUAKE
	end

MirageOasis_SunkenCourt_EventScript_Item_BigNugget::
	finditem ITEM_BIG_NUGGET
	end

MirageOasis_SunkenCourt_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

MirageOasis_SunkenCourt_EventScript_Item_FullRestore::
	finditem ITEM_FULL_RESTORE
	end

