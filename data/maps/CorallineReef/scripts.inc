CorallineReef_MapScripts::
	.byte 0

CorallineReef_EventScript_Diver::
	lock
	faceplayer
	msgbox CorallineReef_Text_DiverBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CorallineReef_EventScript_DiverStay
	msgbox CorallineReef_Text_DiverUp, MSGBOX_DEFAULT
	closemessage
	warp MAP_PACIFIDLOG_TOWN, 8, 19
	waitstate
	release
	end

CorallineReef_EventScript_DiverStay::
	msgbox CorallineReef_Text_DiverStay, MSGBOX_DEFAULT
	release
	end

CorallineReef_EventScript_Clam::
	lockall
	goto_if_set FLAG_REEF_CLAM_PEARL, CorallineReef_EventScript_ClamEmpty
	msgbox CorallineReef_Text_ClamOpen, MSGBOX_DEFAULT
	setflag FLAG_REEF_CLAM_PEARL
	giveitem ITEM_BIG_PEARL
	releaseall
	end

CorallineReef_EventScript_ClamEmpty::
	msgbox CorallineReef_Text_ClamEmpty, MSGBOX_DEFAULT
	releaseall
	end

CorallineReef_EventScript_Corsola::
	lock
	faceplayer
	playmoncry SPECIES_CORSOLA, CRY_MODE_NORMAL
	msgbox CorallineReef_Text_Corsola, MSGBOX_DEFAULT
	waitmoncry
	release
	end

CorallineReef_EventScript_Luvdisc::
	lock
	faceplayer
	playmoncry SPECIES_LUVDISC, CRY_MODE_NORMAL
	msgbox CorallineReef_Text_Luvdisc, MSGBOX_DEFAULT
	waitmoncry
	release
	end

CorallineReef_EventScript_Clamperl::
	lock
	faceplayer
	playmoncry SPECIES_CLAMPERL, CRY_MODE_NORMAL
	msgbox CorallineReef_Text_Clamperl, MSGBOX_DEFAULT
	waitmoncry
	release
	end

CorallineReef_EventScript_Nerissa::
	trainerbattle_single TRAINER_REEF_NERISSA, CorallineReef_Text_NerissaIntro, CorallineReef_Text_NerissaDefeat
	msgbox CorallineReef_Text_NerissaPost, MSGBOX_AUTOCLOSE
	end

CorallineReef_EventScript_Dorian::
	trainerbattle_single TRAINER_REEF_DORIAN, CorallineReef_Text_DorianIntro, CorallineReef_Text_DorianDefeat
	msgbox CorallineReef_Text_DorianPost, MSGBOX_AUTOCLOSE
	end

CorallineReef_EventScript_Pelagia::
	trainerbattle_single TRAINER_REEF_PELAGIA, CorallineReef_Text_PelagiaIntro, CorallineReef_Text_PelagiaDefeat
	msgbox CorallineReef_Text_PelagiaPost, MSGBOX_AUTOCLOSE
	end

CorallineReef_Text_DiverBack:
	.string "Running low on air? I can take you\n"
	.string "back up to PACIFIDLOG.$"

CorallineReef_Text_DiverUp:
	.string "Up we go. Slowly, now!$"

CorallineReef_Text_DiverStay:
	.string "The sunken city is north of the\n"
	.string "coral garden. Swim safe!$"

CorallineReef_Text_ClamOpen:
	.string "The giant clam slowly opened…\n"
	.string "There's a pearl inside!$"

CorallineReef_Text_ClamEmpty:
	.string "The giant clam is fast asleep.$"

CorallineReef_Text_Corsola:
	.string "CORSOLA is tending the coral like a\n"
	.string "little gardener.$"

CorallineReef_Text_Luvdisc:
	.string "LUVDISC drifts by and gives you a\n"
	.string "friendly nudge.$"

CorallineReef_Text_Clamperl:
	.string "CLAMPERL is guarding the giant clam.\n"
	.string "It looks like a proud little sibling.$"

CorallineReef_Text_NerissaIntro:
	.string "The reef is the prettiest place in\n"
	.string "the whole sea! Let's battle in it!$"

CorallineReef_Text_NerissaDefeat:
	.string "Swept away!$"

CorallineReef_Text_NerissaPost:
	.string "Don't break the coral, okay?$"

CorallineReef_Text_DorianIntro:
	.string "Nobody gets to the sunken city\n"
	.string "without getting past me!$"

CorallineReef_Text_DorianDefeat:
	.string "You swim faster than a SHARPEDO!$"

CorallineReef_Text_DorianPost:
	.string "The city's lamps went dark ages\n"
	.string "ago. Maybe you can light them.$"

CorallineReef_Text_PelagiaIntro:
	.string "That old wreck is haunted! Or maybe\n"
	.string "I'm just scared. Battle me!$"

CorallineReef_Text_PelagiaDefeat:
	.string "I'm still scared…$"

CorallineReef_Text_PelagiaPost:
	.string "Something moved inside the wreck.\n"
	.string "Something like an anchor…$"

CorallineReef_EventScript_Item_DeepSeaScale::
	finditem ITEM_DEEP_SEA_SCALE
	end

CorallineReef_EventScript_Item_DeepSeaTooth::
	finditem ITEM_DEEP_SEA_TOOTH
	end

CorallineReef_EventScript_Item_TmWaterPulse::
	finditem ITEM_TM_WATER_PULSE
	end

CorallineReef_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

