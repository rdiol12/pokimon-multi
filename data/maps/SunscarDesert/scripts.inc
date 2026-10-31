SunscarDesert_MapScripts::
	.byte 0

SunscarDesert_EventScript_Guide::
	lock
	faceplayer
	msgbox SunscarDesert_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunscarDesert_EventScript_GuideStay
	msgbox SunscarDesert_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_MAUVILLE_CITY, 26, 8
	waitstate
	release
	end

SunscarDesert_EventScript_GuideStay::
	msgbox SunscarDesert_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

SunscarDesert_EventScript_Sign::
	msgbox SunscarDesert_Text_Sign, MSGBOX_SIGN
	end

SunscarDesert_EventScript_Archaeologist::
	msgbox SunscarDesert_Text_Archaeologist, MSGBOX_NPC
	end

@ The digger gives a Relic Gold once the vault is open.
SunscarDesert_EventScript_Digger::
	lock
	faceplayer
	goto_if_set FLAG_SUNSCAR_DIGGER_GIFT, SunscarDesert_EventScript_DiggerDone
	goto_if_set FLAG_SUNSCAR_RIDDLE_3, SunscarDesert_EventScript_DiggerGift
	msgbox SunscarDesert_Text_Digger, MSGBOX_DEFAULT
	release
	end

SunscarDesert_EventScript_DiggerGift::
	msgbox SunscarDesert_Text_DiggerGift, MSGBOX_DEFAULT
	giveitem ITEM_RELIC_GOLD
	goto_if_eq VAR_RESULT, FALSE, SunscarDesert_EventScript_DiggerEnd
	setflag FLAG_SUNSCAR_DIGGER_GIFT
SunscarDesert_EventScript_DiggerEnd::
	release
	end

SunscarDesert_EventScript_DiggerDone::
	msgbox SunscarDesert_Text_DiggerDone, MSGBOX_DEFAULT
	release
	end

SunscarDesert_EventScript_Sandile::
	lock
	faceplayer
	playmoncry SPECIES_SANDILE, CRY_MODE_NORMAL
	msgbox SunscarDesert_Text_Sandile, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunscarDesert_EventScript_Trapinch::
	lock
	faceplayer
	playmoncry SPECIES_TRAPINCH, CRY_MODE_NORMAL
	msgbox SunscarDesert_Text_Trapinch, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunscarDesert_EventScript_Dexter::
	trainerbattle_single TRAINER_SUNSCAR_DEXTER, SunscarDesert_Text_DexterIntro, SunscarDesert_Text_DexterDefeat
	msgbox SunscarDesert_Text_DexterPost, MSGBOX_AUTOCLOSE
	end

SunscarDesert_EventScript_Milo::
	trainerbattle_single TRAINER_SUNSCAR_MILO, SunscarDesert_Text_MiloIntro, SunscarDesert_Text_MiloDefeat
	msgbox SunscarDesert_Text_MiloPost, MSGBOX_AUTOCLOSE
	end

SunscarDesert_EventScript_Sable::
	trainerbattle_single TRAINER_SUNSCAR_SABLE, SunscarDesert_Text_SableIntro, SunscarDesert_Text_SableDefeat
	msgbox SunscarDesert_Text_SablePost, MSGBOX_AUTOCLOSE
	end

SunscarDesert_Text_GuideBack:
	.string "Thirsty? I can lead you back to\n"
	.string "MAUVILLE.$"

SunscarDesert_Text_GuideLead:
	.string "Follow my footprints!$"

SunscarDesert_Text_GuideStay:
	.string "Drink plenty of water out there!$"

SunscarDesert_Text_Sign:
	.string "SUNSCAR DESERT\n"
	.string "The ruins to the north are sealed.$"

SunscarDesert_Text_Archaeologist:
	.string "These ruins were built by people\n"
	.string "who worshipped the UNOWN.\p"
	.string "Inside, three UNOWN statues ask\n"
	.string "riddles. The answers are on the\l"
	.string "glyph tablets. Touch the right one!$"

SunscarDesert_Text_Digger:
	.string "I've been digging here for years.\n"
	.string "All I find is sand and old bones.\p"
	.string "If only someone could open that\n"
	.string "sealed vault…$"

SunscarDesert_Text_DiggerGift:
	.string "The vault is open?! You solved the\n"
	.string "riddles! Here, you've earned this.\l"
	.string "I dug it up years ago.$"

SunscarDesert_Text_DiggerDone:
	.string "Back to digging. Maybe there's a\n"
	.string "second vault!$"

SunscarDesert_Text_Sandile:
	.string "SANDILE is basking in the sun with\n"
	.string "its eyes shut.$"

SunscarDesert_Text_Trapinch:
	.string "TRAPINCH is waiting at the bottom\n"
	.string "of its sand pit. Better not step in!$"

SunscarDesert_Text_DexterIntro:
	.string "I've read every glyph in these\n"
	.string "ruins! Except the ones I haven't.$"

SunscarDesert_Text_DexterDefeat:
	.string "That wasn't written in the glyphs!$"

SunscarDesert_Text_DexterPost:
	.string "The UNOWN statues speak in rhymes.\n"
	.string "Think about the sky.$"

SunscarDesert_Text_MiloIntro:
	.string "Camping in the desert is the best.\n"
	.string "No bugs! Well, almost none.$"

SunscarDesert_Text_MiloDefeat:
	.string "Sand in my sleeping bag again.$"

SunscarDesert_Text_MiloPost:
	.string "TRAPINCH dig pits all over. Watch\n"
	.string "your step.$"

SunscarDesert_Text_SableIntro:
	.string "The desert tests everyone. Let me\n"
	.string "test you too.$"

SunscarDesert_Text_SableDefeat:
	.string "You passed.$"

SunscarDesert_Text_SablePost:
	.string "My GABITE hatched here, on the\n"
	.string "hottest day of the year.$"

SunscarDesert_EventScript_Item_SoftSand::
	finditem ITEM_SOFT_SAND
	end

SunscarDesert_EventScript_Item_TmSandstorm::
	finditem ITEM_TM_SANDSTORM
	end

SunscarDesert_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

