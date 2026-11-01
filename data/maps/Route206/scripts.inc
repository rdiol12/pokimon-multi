Route206_MapScripts::
	.byte 0

Route206_EventScript_Bjorn::
	trainerbattle_single TRAINER_SOL_R206_BJORN, Route206_EventScript_Bjorn_Intro, Route206_EventScript_Bjorn_Defeat
	msgbox Route206_EventScript_Bjorn_Post, MSGBOX_AUTOCLOSE
	end

Route206_EventScript_Bjorn_Intro:
	.string "Brr! Battle me to warm up!$"

Route206_EventScript_Bjorn_Defeat:
	.string "Frozen solid...$"

Route206_EventScript_Bjorn_Post:
	.string "FROSTHOLM is south. Their bread\n"
	.string "is the best in SOLMARA!$"

Route206_EventScript_Elsa::
	trainerbattle_single TRAINER_SOL_R206_ELSA, Route206_EventScript_Elsa_Intro, Route206_EventScript_Elsa_Defeat
	msgbox Route206_EventScript_Elsa_Post, MSGBOX_AUTOCLOSE
	end

Route206_EventScript_Elsa_Intro:
	.string "The cold never bothered me.$"

Route206_EventScript_Elsa_Defeat:
	.string "Melted!$"

Route206_EventScript_Elsa_Post:
	.string "A FRIGIBAX plays in the snow\n"
	.string "past the frozen pond.$"

Route206_EventScript_Rune::
	trainerbattle_single TRAINER_SOL_R206_RUNE, Route206_EventScript_Rune_Intro, Route206_EventScript_Rune_Defeat
	msgbox Route206_EventScript_Rune_Post, MSGBOX_AUTOCLOSE
	end

Route206_EventScript_Rune_Intro:
	.string "Snow day! No school! Battle!$"

Route206_EventScript_Rune_Defeat:
	.string "Snow fair...$"

Route206_EventScript_Rune_Post:
	.string "DUSKMERE is north, in the fog.$"

Route206_EventScript_Freya::
	trainerbattle_single TRAINER_SOL_R206_FREYA, Route206_EventScript_Freya_Intro, Route206_EventScript_Freya_Defeat
	msgbox Route206_EventScript_Freya_Post, MSGBOX_AUTOCLOSE
	end

Route206_EventScript_Freya_Intro:
	.string "Only the toughest train in the\n"
	.string "snow. Are you tough?$"

Route206_EventScript_Freya_Defeat:
	.string "You are tough.$"

Route206_EventScript_Freya_Post:
	.string "SIGRID's gym floor is ice. Plan\n"
	.string "every step!$"

Route206_EventScript_Secret::
	lock
	faceplayer
	playmoncry SPECIES_FRIGIBAX, CRY_MODE_ENCOUNTER
	msgbox Route206_EventScript_Text_Secret, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_FRIGIBAX, 52
	dowildbattle
	setflag FLAG_SOL_R206_SECRET_MON
	removeobject VAR_LAST_TALKED
	release
	end

Route206_EventScript_Text_Secret:
	.string "It's a wild FRIGIBAX! It looks like\n"
	.string "it has been waiting for a\l"
	.string "challenger.$"

Route206_EventScript_Sign::
	msgbox Route206_EventScript_Sign_Text, MSGBOX_SIGN
	end

Route206_EventScript_Sign_Text:
	.string "ROUTE 206\n"
	.string "DUSKMERE - FROSTHOLM$"

Route206_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

Route206_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

Route206_EventScript_Item_PpUp::
	finditem ITEM_PP_UP
	end

Route206_EventScript_Item_NeverMeltIce::
	finditem ITEM_NEVER_MELT_ICE
	end

