Route205_MapScripts::
	.byte 0

Route205_EventScript_Cliff::
	trainerbattle_single TRAINER_SOL_R205_CLIFF, Route205_EventScript_Cliff_Intro, Route205_EventScript_Cliff_Defeat
	msgbox Route205_EventScript_Cliff_Post, MSGBOX_AUTOCLOSE
	end

Route205_EventScript_Cliff_Intro:
	.string "These canyon walls are older\n"
	.string "than POKéMON battles!$"

Route205_EventScript_Cliff_Defeat:
	.string "Rockslide!$"

Route205_EventScript_Cliff_Post:
	.string "DUSKMERE is west, past the\n"
	.string "canyon. Bring a light!$"

Route205_EventScript_Kestrel::
	trainerbattle_single TRAINER_SOL_R205_KESTREL, Route205_EventScript_Kestrel_Intro, Route205_EventScript_Kestrel_Defeat
	msgbox Route205_EventScript_Kestrel_Post, MSGBOX_AUTOCLOSE
	end

Route205_EventScript_Kestrel_Intro:
	.string "My birds rule the canyon skies!$"

Route205_EventScript_Kestrel_Defeat:
	.string "Grounded!$"

Route205_EventScript_Kestrel_Post:
	.string "A SKARMORY nests up on the\n"
	.string "ledge to the north.$"

Route205_EventScript_Tina::
	trainerbattle_single TRAINER_SOL_R205_TINA, Route205_EventScript_Tina_Intro, Route205_EventScript_Tina_Defeat
	msgbox Route205_EventScript_Tina_Post, MSGBOX_AUTOCLOSE
	end

Route205_EventScript_Tina_Intro:
	.string "Hammer time!$"

Route205_EventScript_Tina_Defeat:
	.string "Flattened!$"

Route205_EventScript_Tina_Post:
	.string "ROSEMEAD's gym is east.$"

Route205_EventScript_Graham::
	trainerbattle_single TRAINER_SOL_R205_GRAHAM, Route205_EventScript_Graham_Intro, Route205_EventScript_Graham_Defeat
	msgbox Route205_EventScript_Graham_Post, MSGBOX_AUTOCLOSE
	end

Route205_EventScript_Graham_Intro:
	.string "I dig and dig, and find only\n"
	.string "trainers to battle!$"

Route205_EventScript_Graham_Defeat:
	.string "Dug too deep.$"

Route205_EventScript_Graham_Post:
	.string "Trainers meet up at the overlook\n"
	.string "to trade. Wide enough for all.$"

Route205_EventScript_Secret::
	lock
	faceplayer
	playmoncry SPECIES_SKARMORY, CRY_MODE_ENCOUNTER
	msgbox Route205_EventScript_Text_Secret, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_SKARMORY, 52
	dowildbattle
	setflag FLAG_SOL_R205_SECRET_MON
	removeobject VAR_LAST_TALKED
	release
	end

Route205_EventScript_Text_Secret:
	.string "It's a wild SKARMORY! It looks like\n"
	.string "it has been waiting for a\l"
	.string "challenger.$"

Route205_EventScript_Sign::
	msgbox Route205_EventScript_Sign_Text, MSGBOX_SIGN
	end

Route205_EventScript_Sign_Text:
	.string "ROUTE 205\n"
	.string "ROSEMEAD - DUSKMERE$"

Route205_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

Route205_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

Route205_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

Route205_EventScript_Item_HardStone::
	finditem ITEM_HARD_STONE
	end

