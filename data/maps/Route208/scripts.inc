Route208_MapScripts::
	.byte 0

Route208_EventScript_Coral::
	trainerbattle_single TRAINER_SOL_R208_CORAL, Route208_EventScript_Coral_Intro, Route208_EventScript_Coral_Defeat
	msgbox Route208_EventScript_Coral_Post, MSGBOX_AUTOCLOSE
	end

Route208_EventScript_Coral_Intro:
	.string "The sea here is perfect for\n"
	.string "training!$"

Route208_EventScript_Coral_Defeat:
	.string "Washed up!$"

Route208_EventScript_Coral_Post:
	.string "PORT SOLACE is east, along the\n"
	.string "beach.$"

Route208_EventScript_Skip::
	trainerbattle_single TRAINER_SOL_R208_SKIP, Route208_EventScript_Skip_Intro, Route208_EventScript_Skip_Defeat
	msgbox Route208_EventScript_Skip_Post, MSGBOX_AUTOCLOSE
	end

Route208_EventScript_Skip_Intro:
	.string "Splash battle!$"

Route208_EventScript_Skip_Defeat:
	.string "Glub...$"

Route208_EventScript_Skip_Post:
	.string "A WIMPOD hides under the rocks\n"
	.string "at the end of the beach!$"

Route208_EventScript_Marlin::
	trainerbattle_single TRAINER_SOL_R208_MARLIN, Route208_EventScript_Marlin_Intro, Route208_EventScript_Marlin_Defeat
	msgbox Route208_EventScript_Marlin_Post, MSGBOX_AUTOCLOSE
	end

Route208_EventScript_Marlin_Intro:
	.string "Reeled you in!$"

Route208_EventScript_Marlin_Defeat:
	.string "The one that got away...$"

Route208_EventScript_Marlin_Post:
	.string "DRAKESPIRE is west. Watch the\n"
	.string "sky for dragons!$"

Route208_EventScript_Sunny::
	trainerbattle_single TRAINER_SOL_R208_SUNNY, Route208_EventScript_Sunny_Intro, Route208_EventScript_Sunny_Defeat
	msgbox Route208_EventScript_Sunny_Post, MSGBOX_AUTOCLOSE
	end

Route208_EventScript_Sunny_Intro:
	.string "Sun, sand and battles!$"

Route208_EventScript_Sunny_Defeat:
	.string "What a day!$"

Route208_EventScript_Sunny_Post:
	.string "Everyone meets at HARBOR\n"
	.string "SQUARE. Want to come?$"

Route208_EventScript_Secret::
	lock
	faceplayer
	playmoncry SPECIES_WIMPOD, CRY_MODE_ENCOUNTER
	msgbox Route208_EventScript_Text_Secret, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_WIMPOD, 52
	dowildbattle
	setflag FLAG_SOL_R208_SECRET_MON
	removeobject VAR_LAST_TALKED
	release
	end

Route208_EventScript_Text_Secret:
	.string "It's a wild WIMPOD! It looks like\n"
	.string "it has been waiting for a\l"
	.string "challenger.$"

Route208_EventScript_Sign::
	msgbox Route208_EventScript_Sign_Text, MSGBOX_SIGN
	end

Route208_EventScript_Sign_Text:
	.string "ROUTE 208\n"
	.string "DRAKESPIRE - PORT SOLACE$"

Route208_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

Route208_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

Route208_EventScript_Item_PpUp::
	finditem ITEM_PP_UP
	end

Route208_EventScript_Item_PearlString::
	finditem ITEM_PEARL_STRING
	end

