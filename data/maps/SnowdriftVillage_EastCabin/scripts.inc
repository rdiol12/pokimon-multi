SnowdriftVillage_EastCabin_MapScripts::
	.byte 0

SnowdriftVillage_EastCabin_EventScript_Sculptor::
	lock
	faceplayer
	goto_if_set FLAG_FROST_SCULPTOR_GIFT, SnowdriftVillage_EastCabin_EventScript_SculptorDone
	goto_if_unset FLAG_FROST_ARTICUNO_DONE, SnowdriftVillage_EastCabin_EventScript_SculptorAsk
	msgbox SnowdriftVillage_EastCabin_Text_SculptorSaw, MSGBOX_DEFAULT
	giveitem ITEM_ICY_ROCK
	setflag FLAG_FROST_SCULPTOR_GIFT
	release
	end

SnowdriftVillage_EastCabin_EventScript_SculptorAsk::
	msgbox SnowdriftVillage_EastCabin_Text_SculptorAsk, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_EastCabin_EventScript_SculptorDone::
	msgbox SnowdriftVillage_EastCabin_Text_SculptorDone, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_EastCabin_Text_SculptorAsk:
	.string "I carve ice sculptures. My dream is\n"
	.string "to carve the bird that roosts by\l"
	.string "the summit cairn.\p"
	.string "If you ever meet it, tell me all\n"
	.string "about it!$"

SnowdriftVillage_EastCabin_Text_SculptorSaw:
	.string "You met ARTICUNO? Tell me… wow!\n"
	.string "Wings like that!\p"
	.string "Here, take this ICY ROCK. It keeps\n"
	.string "the cold in. I carve with it.$"

SnowdriftVillage_EastCabin_Text_SculptorDone:
	.string "My ARTICUNO sculpture is coming\n"
	.string "along nicely. Just the beak left.$"
