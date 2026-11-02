Route201_MapScripts::
	.byte 0

Route201_EventScript_Ella::
	trainerbattle_single TRAINER_SOL_R201_ELLA, Route201_EventScript_Ella_Intro, Route201_EventScript_Ella_Defeat
	msgbox Route201_EventScript_Ella_Post, MSGBOX_AUTOCLOSE
	end

Route201_EventScript_Ella_Intro:
	.string "You came on the ship too? Then\n"
	.string "let's see what HOENN taught you!$"

Route201_EventScript_Ella_Defeat:
	.string "HOENN taught you well!$"

Route201_EventScript_Ella_Post:
	.string "There are trainers from all over\n"
	.string "on these roads now. It's great!$"

Route201_EventScript_Toby::
	trainerbattle_single TRAINER_SOL_R201_TOBY, Route201_EventScript_Toby_Intro, Route201_EventScript_Toby_Defeat
	msgbox Route201_EventScript_Toby_Post, MSGBOX_AUTOCLOSE
	end

Route201_EventScript_Toby_Intro:
	.string "I'm training for the PORT SOLACE\n"
	.string "gym! Battle me!$"

Route201_EventScript_Toby_Defeat:
	.string "Aww, splash!$"

Route201_EventScript_Toby_Post:
	.string "MARINA's WATER types are strong.\n"
	.string "GRASS and ELECTRIC moves help.$"

Route201_EventScript_Nadia::
	trainerbattle_single TRAINER_SOL_R201_NADIA, Route201_EventScript_Nadia_Intro, Route201_EventScript_Nadia_Defeat
	msgbox Route201_EventScript_Nadia_Post, MSGBOX_AUTOCLOSE
	end

Route201_EventScript_Nadia_Intro:
	.string "A picnic is better after a\n"
	.string "battle!$"

Route201_EventScript_Nadia_Defeat:
	.string "Time for a snack.$"

Route201_EventScript_Nadia_Post:
	.string "GEARHOLT is east. You can hear\n"
	.string "the factories from here!$"

Route201_EventScript_Rico::
	trainerbattle_single TRAINER_SOL_R201_RICO, Route201_EventScript_Rico_Intro, Route201_EventScript_Rico_Defeat
	msgbox Route201_EventScript_Rico_Post, MSGBOX_AUTOCLOSE
	end

Route201_EventScript_Rico_Intro:
	.string "Birds of a feather battle\n"
	.string "together!$"

Route201_EventScript_Rico_Defeat:
	.string "Flown away...$"

Route201_EventScript_Rico_Post:
	.string "The clearing north of the lake\n"
	.string "has strange tracks in it.$"

Route201_EventScript_Secret::
	lock
	faceplayer
	playmoncry SPECIES_MAREEP, CRY_MODE_ENCOUNTER
	msgbox Route201_EventScript_Text_Secret, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_MAREEP, 52
	dowildbattle
	setflag FLAG_SOL_R201_SECRET_MON
	removeobject VAR_LAST_TALKED
	release
	end

Route201_EventScript_Text_Secret:
	.string "It's a wild MAREEP! It looks like\n"
	.string "it has been waiting for a\l"
	.string "challenger.$"

Route201_EventScript_Sign::
	msgbox Route201_EventScript_Sign_Text, MSGBOX_SIGN
	end

Route201_EventScript_Sign_Text:
	.string "ROUTE 201\n"
	.string "PORT SOLACE - GEARHOLT$"

Route201_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

Route201_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

Route201_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

Route201_EventScript_Item_RareCandy::
	finditem ITEM_RARE_CANDY
	end

