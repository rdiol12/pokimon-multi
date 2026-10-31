EmberSprings_HermitHouse_MapScripts::
	.byte 0

EmberSprings_HermitHouse_EventScript_Hermit::
	lock
	faceplayer
	goto_if_set FLAG_EMBER_HERMIT_GIFT, EmberSprings_HermitHouse_EventScript_After
	msgbox EmberSprings_HermitHouse_Text_Hermit, MSGBOX_DEFAULT
	giveitem ITEM_FLAME_ORB
	goto_if_eq VAR_RESULT, FALSE, EmberSprings_HermitHouse_EventScript_End
	setflag FLAG_EMBER_HERMIT_GIFT
EmberSprings_HermitHouse_EventScript_After::
	msgbox EmberSprings_HermitHouse_Text_After, MSGBOX_DEFAULT
EmberSprings_HermitHouse_EventScript_End::
	release
	end

EmberSprings_HermitHouse_Text_Hermit:
	.string "I've lived by the volcano for sixty\n"
	.string "years. It speaks, if you listen.\p"
	.string "Here. This orb was warmed in its\n"
	.string "heart.$"

EmberSprings_HermitHouse_Text_After:
	.string "Deep in the mountain, the lava runs\n"
	.string "in rivers. One day someone brave\l"
	.string "will walk them.$"
