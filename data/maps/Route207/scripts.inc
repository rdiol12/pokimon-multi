Route207_MapScripts::
	.byte 0

Route207_EventScript_Draco::
	trainerbattle_single TRAINER_SOL_R207_DRACO, Route207_EventScript_Draco_Intro, Route207_EventScript_Draco_Defeat
	msgbox Route207_EventScript_Draco_Post, MSGBOX_AUTOCLOSE
	end

Route207_EventScript_Draco_Intro:
	.string "DRAGONS answer only to the\n"
	.string "worthy!$"

Route207_EventScript_Draco_Defeat:
	.string "You are worthy.$"

Route207_EventScript_Draco_Post:
	.string "DRAKESPIRE is just south.$"

Route207_EventScript_Luna::
	trainerbattle_single TRAINER_SOL_R207_LUNA, Route207_EventScript_Luna_Intro, Route207_EventScript_Luna_Defeat
	msgbox Route207_EventScript_Luna_Post, MSGBOX_AUTOCLOSE
	end

Route207_EventScript_Luna_Intro:
	.string "Can your team handle dragons?$"

Route207_EventScript_Luna_Defeat:
	.string "Clipped wings!$"

Route207_EventScript_Luna_Post:
	.string "A JANGMO-O trains alone in the\n"
	.string "clearing past the pines.$"

Route207_EventScript_Brick::
	trainerbattle_single TRAINER_SOL_R207_BRICK, Route207_EventScript_Brick_Intro, Route207_EventScript_Brick_Defeat
	msgbox Route207_EventScript_Brick_Post, MSGBOX_AUTOCLOSE
	end

Route207_EventScript_Brick_Intro:
	.string "Scales and fists!$"

Route207_EventScript_Brick_Defeat:
	.string "My scales are cracked!$"

Route207_EventScript_Brick_Post:
	.string "VALEN trains up on the cliffs\n"
	.string "every morning.$"

Route207_EventScript_Gwen::
	trainerbattle_single TRAINER_SOL_R207_GWEN, Route207_EventScript_Gwen_Intro, Route207_EventScript_Gwen_Defeat
	msgbox Route207_EventScript_Gwen_Post, MSGBOX_AUTOCLOSE
	end

Route207_EventScript_Gwen_Intro:
	.string "I raised these from eggs!$"

Route207_EventScript_Gwen_Defeat:
	.string "They'll grow from this.$"

Route207_EventScript_Gwen_Post:
	.string "FROSTHOLM is north, up in the\n"
	.string "snow.$"

Route207_EventScript_Secret::
	lock
	faceplayer
	playmoncry SPECIES_JANGMO_O, CRY_MODE_ENCOUNTER
	msgbox Route207_EventScript_Text_Secret, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_JANGMO_O, 52
	dowildbattle
	setflag FLAG_SOL_R207_SECRET_MON
	removeobject VAR_LAST_TALKED
	release
	end

Route207_EventScript_Text_Secret:
	.string "It's a wild JANGMO-O! It looks like\n"
	.string "it has been waiting for a\l"
	.string "challenger.$"

Route207_EventScript_Sign::
	msgbox Route207_EventScript_Sign_Text, MSGBOX_SIGN
	end

Route207_EventScript_Sign_Text:
	.string "ROUTE 207\n"
	.string "FROSTHOLM - DRAKESPIRE$"

Route207_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

Route207_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

Route207_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

Route207_EventScript_Item_DragonFang::
	finditem ITEM_DRAGON_FANG
	end

