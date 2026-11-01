Route203_MapScripts::
	.byte 0

Route203_EventScript_Blaze::
	trainerbattle_single TRAINER_SOL_R203_BLAZE, Route203_EventScript_Blaze_Intro, Route203_EventScript_Blaze_Defeat
	msgbox Route203_EventScript_Blaze_Post, MSGBOX_AUTOCLOSE
	end

Route203_EventScript_Blaze_Intro:
	.string "My fire burns hotter than the\n"
	.string "volcano!$"

Route203_EventScript_Blaze_Defeat:
	.string "Doused!$"

Route203_EventScript_Blaze_Post:
	.string "The ash makes the grass grow\n"
	.string "thick around here.$"

Route203_EventScript_Rosa::
	trainerbattle_single TRAINER_SOL_R203_ROSA, Route203_EventScript_Rosa_Intro, Route203_EventScript_Rosa_Defeat
	msgbox Route203_EventScript_Rosa_Post, MSGBOX_AUTOCLOSE
	end

Route203_EventScript_Rosa_Intro:
	.string "A good trainer keeps cool in the\n"
	.string "heat!$"

Route203_EventScript_Rosa_Defeat:
	.string "Too hot to handle!$"

Route203_EventScript_Rosa_Post:
	.string "SILKWOOD is north, in the woods.$"

Route203_EventScript_Gordo::
	trainerbattle_single TRAINER_SOL_R203_GORDO, Route203_EventScript_Gordo_Intro, Route203_EventScript_Gordo_Defeat
	msgbox Route203_EventScript_Gordo_Post, MSGBOX_AUTOCLOSE
	end

Route203_EventScript_Gordo_Intro:
	.string "Smoke? That's just my TORKOAL\n"
	.string "saying hi!$"

Route203_EventScript_Gordo_Defeat:
	.string "Puff...$"

Route203_EventScript_Gordo_Post:
	.string "A LARVESTA lives in a hollow by\n"
	.string "the lava rocks. Shy little thing.$"

Route203_EventScript_Juno::
	trainerbattle_single TRAINER_SOL_R203_JUNO, Route203_EventScript_Juno_Intro, Route203_EventScript_Juno_Defeat
	msgbox Route203_EventScript_Juno_Post, MSGBOX_AUTOCLOSE
	end

Route203_EventScript_Juno_Intro:
	.string "Hot iron, hard fists!$"

Route203_EventScript_Juno_Defeat:
	.string "My fists are smoking!$"

Route203_EventScript_Juno_Post:
	.string "Train hard. The gyms here grow\n"
	.string "with you.$"

Route203_EventScript_Secret::
	lock
	faceplayer
	playmoncry SPECIES_LARVESTA, CRY_MODE_ENCOUNTER
	msgbox Route203_EventScript_Text_Secret, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_LARVESTA, 52
	dowildbattle
	setflag FLAG_SOL_R203_SECRET_MON
	removeobject VAR_LAST_TALKED
	release
	end

Route203_EventScript_Text_Secret:
	.string "It's a wild LARVESTA! It looks like\n"
	.string "it has been waiting for a\l"
	.string "challenger.$"

Route203_EventScript_Sign::
	msgbox Route203_EventScript_Sign_Text, MSGBOX_SIGN
	end

Route203_EventScript_Sign_Text:
	.string "ROUTE 203\n"
	.string "ASHCOMBE - SILKWOOD$"

Route203_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

Route203_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

Route203_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

Route203_EventScript_Item_Charcoal::
	finditem ITEM_CHARCOAL
	end

