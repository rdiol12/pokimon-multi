Route202_MapScripts::
	.byte 0

Route202_EventScript_Dusty::
	trainerbattle_single TRAINER_SOL_R202_DUSTY, Route202_EventScript_Dusty_Intro, Route202_EventScript_Dusty_Defeat
	msgbox Route202_EventScript_Dusty_Post, MSGBOX_AUTOCLOSE
	end

Route202_EventScript_Dusty_Intro:
	.string "Sand in my boots, sand in my\n"
	.string "teeth. Sand in my battles!$"

Route202_EventScript_Dusty_Defeat:
	.string "Buried!$"

Route202_EventScript_Dusty_Post:
	.string "The sandstorm never stops here.\n"
	.string "Stick close to your friends!$"

Route202_EventScript_Mira::
	trainerbattle_single TRAINER_SOL_R202_MIRA, Route202_EventScript_Mira_Intro, Route202_EventScript_Mira_Defeat
	msgbox Route202_EventScript_Mira_Post, MSGBOX_AUTOCLOSE
	end

Route202_EventScript_Mira_Intro:
	.string "Desert flowers are tough, just\n"
	.string "like me!$"

Route202_EventScript_Mira_Defeat:
	.string "Wilted...$"

Route202_EventScript_Mira_Post:
	.string "ASHCOMBE is north. Hot springs\n"
	.string "after a hot desert. Lovely.$"

Route202_EventScript_Oskar::
	trainerbattle_single TRAINER_SOL_R202_OSKAR, Route202_EventScript_Oskar_Intro, Route202_EventScript_Oskar_Defeat
	msgbox Route202_EventScript_Oskar_Post, MSGBOX_AUTOCLOSE
	end

Route202_EventScript_Oskar_Intro:
	.string "There are ruins under these\n"
	.string "dunes! Probably!$"

Route202_EventScript_Oskar_Defeat:
	.string "Ruined.$"

Route202_EventScript_Oskar_Post:
	.string "A GABITE was seen digging near\n"
	.string "the rocks in the east.$"

Route202_EventScript_Sandra::
	trainerbattle_single TRAINER_SOL_R202_SANDRA, Route202_EventScript_Sandra_Intro, Route202_EventScript_Sandra_Defeat
	msgbox Route202_EventScript_Sandra_Post, MSGBOX_AUTOCLOSE
	end

Route202_EventScript_Sandra_Intro:
	.string "Only the strong cross the dunes.\n"
	.string "Prove it!$"

Route202_EventScript_Sandra_Defeat:
	.string "You are strong.$"

Route202_EventScript_Sandra_Post:
	.string "Go on. The springs are waiting.$"

Route202_EventScript_Secret::
	lock
	faceplayer
	playmoncry SPECIES_GABITE, CRY_MODE_ENCOUNTER
	msgbox Route202_EventScript_Text_Secret, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_GABITE, 52
	dowildbattle
	setflag FLAG_SOL_R202_SECRET_MON
	removeobject VAR_LAST_TALKED
	release
	end

Route202_EventScript_Text_Secret:
	.string "It's a wild GABITE! It looks like\n"
	.string "it has been waiting for a\l"
	.string "challenger.$"

Route202_EventScript_Sign::
	msgbox Route202_EventScript_Sign_Text, MSGBOX_SIGN
	end

Route202_EventScript_Sign_Text:
	.string "ROUTE 202\n"
	.string "GEARHOLT - ASHCOMBE$"

Route202_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

Route202_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

Route202_EventScript_Item_PpUp::
	finditem ITEM_PP_UP
	end

Route202_EventScript_Item_SoftSand::
	finditem ITEM_SOFT_SAND
	end

