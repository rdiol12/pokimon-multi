CinderCaverns_1F_MapScripts::
	.byte 0

CinderCaverns_1F_EventScript_Ignatius::
	trainerbattle_single TRAINER_CINDER_IGNATIUS, CinderCaverns_1F_Text_IgnatiusIntro, CinderCaverns_1F_Text_IgnatiusDefeat
	msgbox CinderCaverns_1F_Text_IgnatiusPost, MSGBOX_AUTOCLOSE
	end

CinderCaverns_1F_EventScript_Rosa::
	trainerbattle_single TRAINER_CINDER_ROSA, CinderCaverns_1F_Text_RosaIntro, CinderCaverns_1F_Text_RosaDefeat
	msgbox CinderCaverns_1F_Text_RosaPost, MSGBOX_AUTOCLOSE
	end

CinderCaverns_1F_EventScript_Scientist::
	lock
	faceplayer
	msgbox CinderCaverns_1F_Text_Scientist, MSGBOX_DEFAULT
	setflag FLAG_CINDER_SCIENTIST_TALKED
	release
	end

CinderCaverns_1F_Text_IgnatiusIntro:
	.string "It's so hot down here my POKéMON\n"
	.string "are glowing! Battle!$"

CinderCaverns_1F_Text_IgnatiusDefeat:
	.string "Burned out…$"

CinderCaverns_1F_Text_IgnatiusPost:
	.string "Don't touch the lava. Seriously.$"

CinderCaverns_1F_Text_RosaIntro:
	.string "The candles flicker… They see your\n"
	.string "soul. Let's battle…$"

CinderCaverns_1F_Text_RosaDefeat:
	.string "The flame went out…$"

CinderCaverns_1F_Text_RosaPost:
	.string "LITWICK love the warmth down here.\n"
	.string "Some say they love souls more…$"

CinderCaverns_1F_Text_Scientist:
	.string "I'm a volcanologist. Long ago, TEAM\n"
	.string "MAGMA built valves on the floor below\l"
	.string "to steer the lava.\p"
	.string "If someone turned all three, the\n"
	.string "lava lake would cool into a path.\p"
	.string "Where would it lead? Deeper than\n"
	.string "anyone has been…$"

CinderCaverns_1F_EventScript_Item_Charcoal::
	finditem ITEM_CHARCOAL
	end

CinderCaverns_1F_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

CinderCaverns_1F_EventScript_Item_FireGem::
	finditem ITEM_FIRE_GEM
	end

