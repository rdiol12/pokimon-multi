Route204_MapScripts::
	.byte 0

Route204_EventScript_Pip::
	trainerbattle_single TRAINER_SOL_R204_PIP, Route204_EventScript_Pip_Intro, Route204_EventScript_Pip_Defeat
	msgbox Route204_EventScript_Pip_Post, MSGBOX_AUTOCLOSE
	end

Route204_EventScript_Pip_Intro:
	.string "The marsh is full of BUGS. I\n"
	.string "love it here!$"

Route204_EventScript_Pip_Defeat:
	.string "Squashed!$"

Route204_EventScript_Pip_Post:
	.string "ROSEMEAD is west. It smells of\n"
	.string "flowers, not swamp!$"

Route204_EventScript_Mae::
	trainerbattle_single TRAINER_SOL_R204_MAE, Route204_EventScript_Mae_Intro, Route204_EventScript_Mae_Defeat
	msgbox Route204_EventScript_Mae_Post, MSGBOX_AUTOCLOSE
	end

Route204_EventScript_Mae_Intro:
	.string "My POKéMON love the rain. Do\n"
	.string "yours?$"

Route204_EventScript_Mae_Defeat:
	.string "Rained out!$"

Route204_EventScript_Mae_Post:
	.string "A GOOMY hides in the reeds by\n"
	.string "the far pond. Too cute!$"

Route204_EventScript_Wade::
	trainerbattle_single TRAINER_SOL_R204_WADE, Route204_EventScript_Wade_Intro, Route204_EventScript_Wade_Defeat
	msgbox Route204_EventScript_Wade_Post, MSGBOX_AUTOCLOSE
	end

Route204_EventScript_Wade_Intro:
	.string "Shh! You'll scare the fish!\n"
	.string "Battle me quietly!$"

Route204_EventScript_Wade_Defeat:
	.string "They got away...$"

Route204_EventScript_Wade_Post:
	.string "Fish here with a friend. It's\n"
	.string "more fun and you can trade\l"
	.string "what you catch.$"

Route204_EventScript_Ivy::
	trainerbattle_single TRAINER_SOL_R204_IVY, Route204_EventScript_Ivy_Intro, Route204_EventScript_Ivy_Defeat
	msgbox Route204_EventScript_Ivy_Post, MSGBOX_AUTOCLOSE
	end

Route204_EventScript_Ivy_Intro:
	.string "Even a swamp has a sweet smell\n"
	.string "if you look for it.$"

Route204_EventScript_Ivy_Defeat:
	.string "How refreshing.$"

Route204_EventScript_Ivy_Post:
	.string "SILKWOOD's gym is just east.$"

Route204_EventScript_Secret::
	lock
	faceplayer
	playmoncry SPECIES_GOOMY, CRY_MODE_ENCOUNTER
	msgbox Route204_EventScript_Text_Secret, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_GOOMY, 52
	dowildbattle
	setflag FLAG_SOL_R204_SECRET_MON
	removeobject VAR_LAST_TALKED
	release
	end

Route204_EventScript_Text_Secret:
	.string "It's a wild GOOMY! It looks like\n"
	.string "it has been waiting for a\l"
	.string "challenger.$"

Route204_EventScript_Sign::
	msgbox Route204_EventScript_Sign_Text, MSGBOX_SIGN
	end

Route204_EventScript_Sign_Text:
	.string "ROUTE 204\n"
	.string "SILKWOOD - ROSEMEAD$"

Route204_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

Route204_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

Route204_EventScript_Item_PpUp::
	finditem ITEM_PP_UP
	end

Route204_EventScript_Item_BigRoot::
	finditem ITEM_BIG_ROOT
	end

