MireboughMarsh_MapScripts::
	.byte 0

MireboughMarsh_EventScript_Guide::
	lock
	faceplayer
	msgbox MireboughMarsh_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MireboughMarsh_EventScript_GuideStay
	msgbox MireboughMarsh_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_FORTREE_CITY, 5, 9
	waitstate
	release
	end

MireboughMarsh_EventScript_GuideStay::
	msgbox MireboughMarsh_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

MireboughMarsh_EventScript_Sign::
	msgbox MireboughMarsh_Text_Sign, MSGBOX_SIGN
	end

@ ---- the Bog Witch's stew
MireboughMarsh_EventScript_Witch::
	lock
	faceplayer
	goto_if_set FLAG_MIRE_STEW_DONE, MireboughMarsh_EventScript_WitchDone
	setflag FLAG_MIRE_WITCH_TALKED
	checkitem ITEM_TINY_MUSHROOM
	goto_if_eq VAR_RESULT, FALSE, MireboughMarsh_EventScript_WitchAsk
	checkitem ITEM_BIG_ROOT
	goto_if_eq VAR_RESULT, FALSE, MireboughMarsh_EventScript_WitchAsk
	checkitem ITEM_BLACK_SLUDGE
	goto_if_eq VAR_RESULT, FALSE, MireboughMarsh_EventScript_WitchAsk
	msgbox MireboughMarsh_Text_WitchHave, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MireboughMarsh_EventScript_WitchLater
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, MireboughMarsh_EventScript_WitchFull
	removeitem ITEM_TINY_MUSHROOM
	removeitem ITEM_BIG_ROOT
	removeitem ITEM_BLACK_SLUDGE
	msgbox MireboughMarsh_Text_WitchStew, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_BUBBLE_BEAM
	waitse
	fadescreen FADE_FROM_BLACK
	giveegg SPECIES_GOOMY
	setflag FLAG_MIRE_STEW_DONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox MireboughMarsh_Text_GotEgg, MSGBOX_DEFAULT
	waitfanfare
	msgbox MireboughMarsh_Text_WitchAfter, MSGBOX_DEFAULT
	release
	end

MireboughMarsh_EventScript_WitchAsk::
	msgbox MireboughMarsh_Text_WitchAsk, MSGBOX_DEFAULT
	release
	end

MireboughMarsh_EventScript_WitchLater::
	msgbox MireboughMarsh_Text_WitchLater, MSGBOX_DEFAULT
	release
	end

MireboughMarsh_EventScript_WitchFull::
	msgbox MireboughMarsh_Text_WitchFull, MSGBOX_DEFAULT
	release
	end

MireboughMarsh_EventScript_WitchDone::
	msgbox MireboughMarsh_Text_WitchDone, MSGBOX_DEFAULT
	release
	end

@ ---- a Stunfisk hiding in the mud
MireboughMarsh_EventScript_Stunfisk::
	goto_if_set FLAG_MIRE_STUNFISK_DONE, MireboughMarsh_EventScript_StunfiskEnd
	lockall
	setflag FLAG_MIRE_STUNFISK_DONE
	playse SE_M_THUNDERBOLT
	msgbox MireboughMarsh_Text_Stunfisk, MSGBOX_DEFAULT
	closemessage
	setwildbattle SPECIES_STUNFISK, 38
	dowildbattle
	releaseall
MireboughMarsh_EventScript_StunfiskEnd::
	end

@ ---- Pokémon and people
MireboughMarsh_EventScript_Wooper::
	lock
	faceplayer
	playmoncry SPECIES_WOOPER, CRY_MODE_NORMAL
	msgbox MireboughMarsh_Text_Wooper, MSGBOX_DEFAULT
	waitmoncry
	release
	end

MireboughMarsh_EventScript_Goomy::
	lock
	faceplayer
	playmoncry SPECIES_GOOMY, CRY_MODE_NORMAL
	msgbox MireboughMarsh_Text_Goomy, MSGBOX_DEFAULT
	waitmoncry
	release
	end

MireboughMarsh_EventScript_Croagunk::
	lock
	faceplayer
	playmoncry SPECIES_CROAGUNK, CRY_MODE_NORMAL
	msgbox MireboughMarsh_Text_Croagunk, MSGBOX_DEFAULT
	waitmoncry
	release
	end

MireboughMarsh_EventScript_Finn::
	trainerbattle_single TRAINER_MIRE_FINN, MireboughMarsh_Text_FinnIntro, MireboughMarsh_Text_FinnDefeat
	msgbox MireboughMarsh_Text_FinnPost, MSGBOX_AUTOCLOSE
	end

MireboughMarsh_EventScript_Marsha::
	trainerbattle_single TRAINER_MIRE_MARSHA, MireboughMarsh_Text_MarshaIntro, MireboughMarsh_Text_MarshaDefeat
	msgbox MireboughMarsh_Text_MarshaPost, MSGBOX_AUTOCLOSE
	end

MireboughMarsh_EventScript_Otto::
	trainerbattle_single TRAINER_MIRE_OTTO, MireboughMarsh_Text_OttoIntro, MireboughMarsh_Text_OttoDefeat
	msgbox MireboughMarsh_Text_OttoPost, MSGBOX_AUTOCLOSE
	end

MireboughMarsh_EventScript_Ivy::
	trainerbattle_single TRAINER_MIRE_IVY, MireboughMarsh_Text_IvyIntro, MireboughMarsh_Text_IvyDefeat
	msgbox MireboughMarsh_Text_IvyPost, MSGBOX_AUTOCLOSE
	end

@ ---- text
MireboughMarsh_Text_GuideBack:
	.string "Soaked through? I can lead you back\n"
	.string "to FORTREE.$"

MireboughMarsh_Text_GuideLead:
	.string "Stick to the boardwalks!$"

MireboughMarsh_Text_GuideStay:
	.string "Mind the mud. Some of it bites.$"

MireboughMarsh_Text_Sign:
	.string "MIREBOUGH MARSH\n"
	.string "Boardwalks may be slippery when wet.$"

MireboughMarsh_Text_WitchAsk:
	.string "Heh heh. A visitor in the rain!\p"
	.string "I'm brewing my famous marsh stew,\n"
	.string "but I'm missing three things:\p"
	.string "a TINY MUSHROOM, a BIG ROOT and a\n"
	.string "dab of BLACK SLUDGE.\p"
	.string "They're all somewhere in the marsh.\n"
	.string "Bring them, and I'll make it worth\l"
	.string "your while.$"

MireboughMarsh_Text_WitchHave:
	.string "Oho! You've got all three! Will you\n"
	.string "give them to me for the stew?$"

MireboughMarsh_Text_WitchStew:
	.string "In they go… stir, stir, stir…$"

MireboughMarsh_Text_GotEgg:
	.string "{PLAYER} received an EGG from the\n"
	.string "BOG WITCH!$"

MireboughMarsh_Text_WitchAfter:
	.string "Found that egg in the reeds while\n"
	.string "gathering. Something slimy will\l"
	.string "hatch. Slimy things are the best!$"

MireboughMarsh_Text_WitchLater:
	.string "Hmph. The stew can wait. It's been\n"
	.string "waiting forty years.$"

MireboughMarsh_Text_WitchFull:
	.string "Your party's full, dearie. Make\n"
	.string "room and come back.$"

MireboughMarsh_Text_WitchDone:
	.string "The stew is bubbling nicely. Want a\n"
	.string "taste? No? Your loss.$"

MireboughMarsh_Text_Stunfisk:
	.string "Something squelched underfoot!\n"
	.string "It's a STUNFISK!$"

MireboughMarsh_Text_Wooper:
	.string "WOOPER is splashing in the puddles,\n"
	.string "very happy about the rain.$"

MireboughMarsh_Text_Goomy:
	.string "GOOMY is soaking up the rain. It's\n"
	.string "very slimy.$"

MireboughMarsh_Text_Croagunk:
	.string "CROAGUNK puffs out its cheeks and\n"
	.string "croaks at you.$"

MireboughMarsh_Text_FinnIntro:
	.string "The marsh is full of whiskered\n"
	.string "fish! Let me show you!$"

MireboughMarsh_Text_FinnDefeat:
	.string "The big one got away!$"

MireboughMarsh_Text_FinnPost:
	.string "BARBOACH love the muddy water here.$"

MireboughMarsh_Text_MarshaIntro:
	.string "The marsh smells lovely in the rain,\n"
	.string "doesn't it?$"

MireboughMarsh_Text_MarshaDefeat:
	.string "Wilted…$"

MireboughMarsh_Text_MarshaPost:
	.string "I saw a mushroom by the east reeds.\n"
	.string "The witch might want it.$"

MireboughMarsh_Text_OttoIntro:
	.string "Bugs love swamps! I love bugs!$"

MireboughMarsh_Text_OttoDefeat:
	.string "Squashed!$"

MireboughMarsh_Text_OttoPost:
	.string "YANMEGA hunt above the water. Look\n"
	.string "up sometimes!$"

MireboughMarsh_Text_IvyIntro:
	.string "I'm a RANGER. I keep the marsh safe.\n"
	.string "Let's see if you're safe too!$"

MireboughMarsh_Text_IvyDefeat:
	.string "You're clearly no danger.$"

MireboughMarsh_Text_IvyPost:
	.string "Careful near the south reeds. A\n"
	.string "STUNFISK hides in the mud there.$"

MireboughMarsh_EventScript_Item_TinyMushroom::
	finditem ITEM_TINY_MUSHROOM
	end

MireboughMarsh_EventScript_Item_BigRoot::
	finditem ITEM_BIG_ROOT
	end

MireboughMarsh_EventScript_Item_BlackSludge::
	finditem ITEM_BLACK_SLUDGE
	end

MireboughMarsh_EventScript_Item_TmSludgeBomb::
	finditem ITEM_TM_SLUDGE_BOMB
	end

MireboughMarsh_EventScript_Item_DampRock::
	finditem ITEM_DAMP_ROCK
	end

MireboughMarsh_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

