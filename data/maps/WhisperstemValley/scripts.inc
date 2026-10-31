WhisperstemValley_MapScripts::
	.byte 0

WhisperstemValley_EventScript_Guide::
	lock
	faceplayer
	msgbox WhisperstemValley_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WhisperstemValley_EventScript_GuideStay
	msgbox WhisperstemValley_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_PETALBURG_CITY, 7, 12
	waitstate
	release
	end

WhisperstemValley_EventScript_GuideStay::
	msgbox WhisperstemValley_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

WhisperstemValley_EventScript_Sign::
	msgbox WhisperstemValley_Text_Sign, MSGBOX_SIGN
	end

WhisperstemValley_EventScript_OldMan::
	msgbox WhisperstemValley_Text_OldMan, MSGBOX_NPC
	end

WhisperstemValley_EventScript_Fisher::
	msgbox WhisperstemValley_Text_Fisher, MSGBOX_NPC
	end

WhisperstemValley_EventScript_Girl::
	msgbox WhisperstemValley_Text_Girl, MSGBOX_NPC
	end

@ ---- the tea master's runaway Pancham
WhisperstemValley_EventScript_Pancham1::
	setflag FLAG_BAMBOO_PANCHAM_1
	goto WhisperstemValley_EventScript_PanchamFound

WhisperstemValley_EventScript_Pancham2::
	setflag FLAG_BAMBOO_PANCHAM_2
	goto WhisperstemValley_EventScript_PanchamFound

WhisperstemValley_EventScript_Pancham3::
	setflag FLAG_BAMBOO_PANCHAM_3
	goto WhisperstemValley_EventScript_PanchamFound

WhisperstemValley_EventScript_PanchamFound::
	lock
	faceplayer
	playmoncry SPECIES_PANCHAM, CRY_MODE_NORMAL
	msgbox WhisperstemValley_Text_Pancham, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox WhisperstemValley_Text_PanchamHome, MSGBOX_DEFAULT
	release
	end

@ ---- the secret grove
WhisperstemValley_EventScript_Mienfoo::
	lock
	faceplayer
	msgbox WhisperstemValley_Text_MienfooTrains, MSGBOX_DEFAULT
	setwildbattle SPECIES_MIENFOO, 14
	playmoncry SPECIES_MIENFOO, CRY_MODE_ENCOUNTER
	msgbox WhisperstemValley_Text_Mienfoo, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, WhisperstemValley_EventScript_MienfooStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, WhisperstemValley_EventScript_MienfooStays
	setflag FLAG_BAMBOO_MIENFOO_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, WhisperstemValley_EventScript_MienfooEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox WhisperstemValley_Text_MienfooGone, MSGBOX_DEFAULT
WhisperstemValley_EventScript_MienfooEnd::
	release
	end

WhisperstemValley_EventScript_MienfooStays::
	msgbox WhisperstemValley_Text_MienfooStays, MSGBOX_DEFAULT
	release
	end

@ ---- trainers
WhisperstemValley_EventScript_Jun::
	trainerbattle_single TRAINER_BAMBOO_JUN, WhisperstemValley_Text_JunIntro, WhisperstemValley_Text_JunDefeat
	msgbox WhisperstemValley_Text_JunPost, MSGBOX_AUTOCLOSE
	end

WhisperstemValley_EventScript_Lila::
	trainerbattle_single TRAINER_BAMBOO_LILA, WhisperstemValley_Text_LilaIntro, WhisperstemValley_Text_LilaDefeat
	msgbox WhisperstemValley_Text_LilaPost, MSGBOX_AUTOCLOSE
	end

WhisperstemValley_EventScript_Mei::
	trainerbattle_single TRAINER_BAMBOO_MEI, WhisperstemValley_Text_MeiIntro, WhisperstemValley_Text_MeiDefeat
	msgbox WhisperstemValley_Text_MeiPost, MSGBOX_AUTOCLOSE
	end

WhisperstemValley_EventScript_Kaito::
	trainerbattle_single TRAINER_BAMBOO_KAITO, WhisperstemValley_Text_KaitoIntro, WhisperstemValley_Text_KaitoDefeat
	msgbox WhisperstemValley_Text_KaitoPost, MSGBOX_AUTOCLOSE
	end

WhisperstemValley_Text_GuideBack:
	.string "Shall I walk you back to PETALBURG\n"
	.string "CITY?$"

WhisperstemValley_Text_GuideLead:
	.string "Mind the bamboo shoots on the path!$"

WhisperstemValley_Text_GuideStay:
	.string "Take your time. The valley is\n"
	.string "loveliest when the wind blows.$"

WhisperstemValley_Text_Sign:
	.string "WHISPERSTEM VALLEY\p"
	.string "Listen to the bamboo. It knows\n"
	.string "where the paths go.$"

WhisperstemValley_Text_OldMan:
	.string "When the wind comes off the pond,\n"
	.string "I watch the bamboo sway.\p"
	.string "Past the north bank, one stand\n"
	.string "of stalks always leans apart…\l"
	.string "as if something walks through it.$"

WhisperstemValley_Text_Fisher:
	.string "The koi in this pond are GOLDEEN.\n"
	.string "The plank bridge is the only way\l"
	.string "over to the north bank.$"

WhisperstemValley_Text_Girl:
	.string "The TEA MASTER's PANCHAM ran off\n"
	.string "into the bamboo again!\p"
	.string "They hide where the paths stop.\n"
	.string "One even crossed the pond!$"

WhisperstemValley_Text_Pancham:
	.string "A PANCHAM is chewing on a bamboo\n"
	.string "leaf. It looks at you sheepishly…\p"
	.string "It must be one of the TEA MASTER's!$"

WhisperstemValley_Text_PanchamHome:
	.string "PANCHAM scampered off toward the\n"
	.string "teahouse.$"

WhisperstemValley_Text_MienfooTrains:
	.string "A MIENFOO is training alone in\n"
	.string "the hidden grove…\p"
	.string "It turns to face you and takes\n"
	.string "a stance!$"

WhisperstemValley_Text_Mienfoo:
	.string "Hyaah!$"

WhisperstemValley_Text_MienfooGone:
	.string "MIENFOO bowed, and vanished into\n"
	.string "the bamboo.$"

WhisperstemValley_Text_MienfooStays:
	.string "MIENFOO is still waiting, perfectly\n"
	.string "balanced on one foot.$"

WhisperstemValley_Text_JunIntro:
	.string "Bug POKéMON love bamboo! Let's\n"
	.string "see whose are tougher!$"

WhisperstemValley_Text_JunDefeat:
	.string "Bzzt! You win!$"

WhisperstemValley_Text_JunPost:
	.string "My SEWADDLE wants to make leaf\n"
	.string "clothes out of bamboo leaves.$"

WhisperstemValley_Text_LilaIntro:
	.string "I came for a picnic, but I'll\n"
	.string "take a battle too!$"

WhisperstemValley_Text_LilaDefeat:
	.string "That was a sweet battle.$"

WhisperstemValley_Text_LilaPost:
	.string "Have you tried the tea at the\n"
	.string "teahouse? It's so good.$"

WhisperstemValley_Text_MeiIntro:
	.string "I train among the bamboo every\n"
	.string "morning. Show me your stance!$"

WhisperstemValley_Text_MeiDefeat:
	.string "Your form is better than mine!$"

WhisperstemValley_Text_MeiPost:
	.string "Bamboo bends but never breaks.\n"
	.string "A fighter should be the same.$"

WhisperstemValley_Text_KaitoIntro:
	.string "You saw me? Then my hiding\n"
	.string "practice has failed! Battle!$"

WhisperstemValley_Text_KaitoDefeat:
	.string "I vanish… in defeat.$"

WhisperstemValley_Text_KaitoPost:
	.string "A real ninja could slip right\n"
	.string "through a bamboo wall.$"

WhisperstemValley_EventScript_Item_SuperPotion::
	finditem ITEM_SUPER_POTION
	end

WhisperstemValley_EventScript_Item_GreatBall::
	finditem ITEM_GREAT_BALL
	end

WhisperstemValley_EventScript_Item_BigPearl::
	finditem ITEM_BIG_PEARL
	end

WhisperstemValley_EventScript_Item_TmBulkUp::
	finditem ITEM_TM_BULK_UP
	end

WhisperstemValley_EventScript_Item_Ether::
	finditem ITEM_ETHER
	end

