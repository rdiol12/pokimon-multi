SnowdriftVillage_Lodge_MapScripts::
	.byte 0

SnowdriftVillage_Lodge_EventScript_Keeper::
	lock
	faceplayer
	msgbox SnowdriftVillage_Lodge_Text_Keeper, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SnowdriftVillage_Lodge_EventScript_KeeperNo
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox SnowdriftVillage_Lodge_Text_KeeperDone, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_Lodge_EventScript_KeeperNo::
	msgbox SnowdriftVillage_Lodge_Text_KeeperNo, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_Lodge_EventScript_Chief::
	lock
	faceplayer
	goto_if_set FLAG_FROST_RESCUE_DONE, SnowdriftVillage_Lodge_EventScript_ChiefDone
	goto_if_unset FLAG_FROST_CLIMBER_1, SnowdriftVillage_Lodge_EventScript_ChiefAsk
	goto_if_unset FLAG_FROST_CLIMBER_2, SnowdriftVillage_Lodge_EventScript_ChiefAsk
	goto_if_unset FLAG_FROST_CLIMBER_3, SnowdriftVillage_Lodge_EventScript_ChiefAsk
	msgbox SnowdriftVillage_Lodge_Text_ChiefThanks, MSGBOX_DEFAULT
	givemon SPECIES_FRIGIBAX, 35
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, SnowdriftVillage_Lodge_EventScript_ChiefFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox SnowdriftVillage_Lodge_Text_GotFrigibax, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_FROST_RESCUE_DONE
	msgbox SnowdriftVillage_Lodge_Text_ChiefDig, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_Lodge_EventScript_ChiefFull::
	msgbox SnowdriftVillage_Lodge_Text_ChiefFull, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_Lodge_EventScript_ChiefAsk::
	msgbox SnowdriftVillage_Lodge_Text_ChiefAsk, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_Lodge_EventScript_ChiefDone::
	msgbox SnowdriftVillage_Lodge_Text_ChiefDig, MSGBOX_DEFAULT
	release
	end

SnowdriftVillage_Lodge_Text_Keeper:
	.string "Welcome to the lodge! Would you\n"
	.string "like to rest by the stove?$"

SnowdriftVillage_Lodge_Text_KeeperDone:
	.string "All warmed up! Your POKéMON look\n"
	.string "ready for anything.$"

SnowdriftVillage_Lodge_Text_KeeperNo:
	.string "The stove's always lit.$"

SnowdriftVillage_Lodge_Text_ChiefAsk:
	.string "I'm the chief of MOUNTAIN RESCUE.\p"
	.string "Three climbers went missing on the\n"
	.string "trail in the blizzard: one near the\l"
	.string "tall grass, one by a fallen rock,\l"
	.string "one somewhere in the pines.\p"
	.string "If you find them, send them down to\n"
	.string "the ranger's camp. Please hurry!$"

SnowdriftVillage_Lodge_Text_ChiefThanks:
	.string "The ranger radioed up: all three\n"
	.string "climbers are safe at the camp!\p"
	.string "You've done MOUNTAIN RESCUE proud.\n"
	.string "Take this FRIGIBAX. We raised it\l"
	.string "here at the lodge.$"

SnowdriftVillage_Lodge_Text_GotFrigibax:
	.string "{PLAYER} received a FRIGIBAX!$"

SnowdriftVillage_Lodge_Text_ChiefFull:
	.string "Your party's full! Make some room\n"
	.string "and come back for FRIGIBAX.$"

SnowdriftVillage_Lodge_Text_ChiefDig:
	.string "Now my crew can dig out the summit\n"
	.string "path. Go and see the view!$"
