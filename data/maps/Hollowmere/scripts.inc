Hollowmere_MapScripts::
	.byte 0

Hollowmere_EventScript_Ferryman::
	lock
	faceplayer
	msgbox Hollowmere_Text_FerrymanBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Hollowmere_EventScript_FerrymanStay
	msgbox Hollowmere_Text_FerrymanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_LILYCOVE_CITY, 27, 17
	waitstate
	release
	end

Hollowmere_EventScript_FerrymanStay::
	msgbox Hollowmere_Text_FerrymanStay, MSGBOX_DEFAULT
	release
	end

Hollowmere_EventScript_Sign::
	msgbox Hollowmere_Text_Sign, MSGBOX_SIGN
	end

Hollowmere_EventScript_Boarded::
	msgbox Hollowmere_Text_Boarded, MSGBOX_SIGN
	end

@ ---- the caretaker and her four lost ghosts
Hollowmere_EventScript_Caretaker::
	lock
	faceplayer
	goto_if_set FLAG_HOLLOW_MANOR_KEY, Hollowmere_EventScript_CaretakerDone
	goto_if_unset FLAG_HOLLOW_GHOST_GASTLY, Hollowmere_EventScript_CaretakerAsk
	goto_if_unset FLAG_HOLLOW_GHOST_DUSKULL, Hollowmere_EventScript_CaretakerAsk
	goto_if_unset FLAG_HOLLOW_GHOST_SHUPPET, Hollowmere_EventScript_CaretakerAsk
	goto_if_unset FLAG_HOLLOW_GHOST_MISDREAVUS, Hollowmere_EventScript_CaretakerAsk
	msgbox Hollowmere_Text_CaretakerThanks, MSGBOX_DEFAULT
	giveitem ITEM_SPELL_TAG
	setflag FLAG_HOLLOW_MANOR_KEY
	playfanfare MUS_OBTAIN_ITEM
	msgbox Hollowmere_Text_GotKey, MSGBOX_DEFAULT
	waitfanfare
	msgbox Hollowmere_Text_CaretakerKey, MSGBOX_DEFAULT
	release
	end

Hollowmere_EventScript_CaretakerAsk::
	setflag FLAG_HOLLOW_QUEST_STARTED
	msgbox Hollowmere_Text_CaretakerAsk, MSGBOX_DEFAULT
	release
	end

Hollowmere_EventScript_CaretakerDone::
	msgbox Hollowmere_Text_CaretakerDone, MSGBOX_DEFAULT
	release
	end

Hollowmere_EventScript_GhostIgnore::
	release
	end

Hollowmere_EventScript_Gastly::
	lock
	faceplayer
	playmoncry SPECIES_GASTLY, CRY_MODE_NORMAL
	msgbox Hollowmere_Text_Gastly, MSGBOX_DEFAULT
	waitmoncry
	goto_if_unset FLAG_HOLLOW_QUEST_STARTED, Hollowmere_EventScript_GhostIgnore
	setflag FLAG_HOLLOW_GHOST_GASTLY
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox Hollowmere_Text_GhostHome, MSGBOX_DEFAULT
	release
	end

Hollowmere_EventScript_Duskull::
	lock
	faceplayer
	playmoncry SPECIES_DUSKULL, CRY_MODE_NORMAL
	msgbox Hollowmere_Text_Duskull, MSGBOX_DEFAULT
	waitmoncry
	goto_if_unset FLAG_HOLLOW_QUEST_STARTED, Hollowmere_EventScript_GhostIgnore
	setflag FLAG_HOLLOW_GHOST_DUSKULL
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox Hollowmere_Text_GhostHome, MSGBOX_DEFAULT
	release
	end

Hollowmere_EventScript_Shuppet::
	lock
	faceplayer
	playmoncry SPECIES_SHUPPET, CRY_MODE_NORMAL
	msgbox Hollowmere_Text_Shuppet, MSGBOX_DEFAULT
	waitmoncry
	goto_if_unset FLAG_HOLLOW_QUEST_STARTED, Hollowmere_EventScript_GhostIgnore
	setflag FLAG_HOLLOW_GHOST_SHUPPET
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox Hollowmere_Text_GhostHome, MSGBOX_DEFAULT
	release
	end

Hollowmere_EventScript_Misdreavus::
	lock
	faceplayer
	playmoncry SPECIES_MISDREAVUS, CRY_MODE_NORMAL
	msgbox Hollowmere_Text_Misdreavus, MSGBOX_DEFAULT
	waitmoncry
	goto_if_unset FLAG_HOLLOW_QUEST_STARTED, Hollowmere_EventScript_GhostIgnore
	setflag FLAG_HOLLOW_GHOST_MISDREAVUS
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox Hollowmere_Text_GhostHome, MSGBOX_DEFAULT
	release
	end

Hollowmere_EventScript_Drifloon::
	lock
	faceplayer
	playmoncry SPECIES_DRIFLOON, CRY_MODE_NORMAL
	msgbox Hollowmere_Text_Drifloon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Hollowmere_EventScript_Boy::
	msgbox Hollowmere_Text_Boy, MSGBOX_NPC
	end

Hollowmere_EventScript_Vesper::
	trainerbattle_single TRAINER_HOLLOW_VESPER, Hollowmere_Text_VesperIntro, Hollowmere_Text_VesperDefeat
	msgbox Hollowmere_Text_VesperPost, MSGBOX_AUTOCLOSE
	end

Hollowmere_EventScript_Luna::
	trainerbattle_single TRAINER_HOLLOW_LUNA, Hollowmere_Text_LunaIntro, Hollowmere_Text_LunaDefeat
	msgbox Hollowmere_Text_LunaPost, MSGBOX_AUTOCLOSE
	end

Hollowmere_Text_FerrymanBack:
	.string "The mist is lifting soon. Shall I\n"
	.string "row you back to LILYCOVE?$"

Hollowmere_Text_FerrymanGo:
	.string "Don't look back at the lights…$"

Hollowmere_Text_FerrymanStay:
	.string "I'll wait. I always wait.$"

Hollowmere_Text_Sign:
	.string "HOLLOWMERE\n"
	.string "Where the lamps are never lit.$"

Hollowmere_Text_Boarded:
	.string "The door is boarded up. Something\n"
	.string "scratches on the other side…$"

Hollowmere_Text_CaretakerAsk:
	.string "Oh, a living visitor! How rare.\p"
	.string "I look after this town's little\n"
	.string "ghosts. Four of them slipped out\l"
	.string "tonight and won't come home.\p"
	.string "A GASTLY, a DUSKULL, a SHUPPET and\n"
	.string "a MISDREAVUS. Two are out here, one\l"
	.string "is in a house and one in the manor.\p"
	.string "Please, find them and send them\n"
	.string "home to me.$"

Hollowmere_Text_CaretakerThanks:
	.string "All four are home! Thank you, dear.\n"
	.string "Take this, and this old key too.$"

Hollowmere_Text_GotKey:
	.string "{PLAYER} received the MANOR KEY!$"

Hollowmere_Text_CaretakerKey:
	.string "It opens the upper floor of the\n"
	.string "manor. Nobody has gone up there\l"
	.string "for a hundred years…$"

Hollowmere_Text_CaretakerDone:
	.string "The little ones are sleeping. Hush,\n"
	.string "now.$"

Hollowmere_Text_GhostHome:
	.string "It giggled and drifted off home!$"

Hollowmere_Text_Gastly:
	.string "GASTLY is hiding behind a tree,\n"
	.string "giggling.$"

Hollowmere_Text_Duskull:
	.string "DUSKULL is staring at the pond.$"

Hollowmere_Text_Drifloon:
	.string "DRIFLOON bobs in the air. It seems\n"
	.string "to want to hold your hand…$"

Hollowmere_Text_Boy:
	.string "My grandma says this town is only\n"
	.string "here at night. In the morning it's\l"
	.string "just fog over the water!$"

Hollowmere_Text_VesperIntro:
	.string "The spirits whispered your name.\n"
	.string "Shall we battle, hehehe?$"

Hollowmere_Text_VesperDefeat:
	.string "The spirits lied to me!$"

Hollowmere_Text_VesperPost:
	.string "The manor's master never left. They\n"
	.string "say he still walks upstairs.$"

Hollowmere_Text_LunaIntro:
	.string "I can see the future. You lose.$"

Hollowmere_Text_LunaDefeat:
	.string "I need new glasses.$"

Hollowmere_Text_LunaPost:
	.string "A GASTLY was giggling near the west\n"
	.string "trees. Or will be. Time is funny.$"

Hollowmere_EventScript_Item_DuskStone::
	finditem ITEM_DUSK_STONE
	end

Hollowmere_EventScript_Item_TmShadowBall::
	finditem ITEM_TM_SHADOW_BALL
	end

