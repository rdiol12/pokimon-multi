PrismCaverns_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PrismCaverns_OnLoad
	.byte 0

PrismCaverns_OnLoad:
	call_if_set FLAG_PRISM_RUBY, PrismCaverns_EventScript_LitRed
	call_if_set FLAG_PRISM_SAPPHIRE, PrismCaverns_EventScript_LitBlue
	call_if_set FLAG_PRISM_RUBY, PrismCaverns_EventScript_WakeRed
	call_if_set FLAG_PRISM_SAPPHIRE, PrismCaverns_EventScript_WakeBlue
	call_if_set FLAG_PRISM_SEAL_OPEN, PrismCaverns_EventScript_WakeWhite
	end

PrismCaverns_EventScript_WakeRed::
	setmetatile 22, 39, 969, FALSE
	setmetatile 9, 38, 969, FALSE
	return

PrismCaverns_EventScript_WakeBlue::
	setmetatile 11, 7, 971, FALSE
	setmetatile 24, 10, 971, FALSE
	setmetatile 34, 32, 971, FALSE
	setmetatile 33, 10, 971, FALSE
	return

PrismCaverns_EventScript_WakeWhite::
	setmetatile 28, 30, 977, FALSE
	setmetatile 50, 9, 977, FALSE
	return


PrismCaverns_EventScript_LitRed::
	setmetatile 46, 15, 955, TRUE
	setmetatile 46, 16, 956, TRUE
	return

PrismCaverns_EventScript_ResRed::
	lockall
	goto_if_set FLAG_PRISM_RUBY, PrismCaverns_EventScript_ResAlreadyLit
	msgbox PrismCaverns_Text_ResTouch, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PrismCaverns_EventScript_ResLeave
	playse SE_M_HEAL_BELL
	call PrismCaverns_EventScript_LitRed
	special DrawWholeMapView
	waitse
	setflag FLAG_PRISM_RUBY
	msgbox PrismCaverns_Text_ResRed, MSGBOX_DEFAULT
	call PrismCaverns_EventScript_WakeRed
	playse SE_GLASS_FLUTE
	special DrawWholeMapView
	waitse
	msgbox PrismCaverns_Text_PadsWakeRed, MSGBOX_DEFAULT
	releaseall
	end


PrismCaverns_EventScript_LitBlue::
	setmetatile 8, 3, 959, TRUE
	setmetatile 8, 4, 960, TRUE
	return

PrismCaverns_EventScript_ResBlue::
	lockall
	goto_if_set FLAG_PRISM_SAPPHIRE, PrismCaverns_EventScript_ResAlreadyLit
	msgbox PrismCaverns_Text_ResTouch, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PrismCaverns_EventScript_ResLeave
	playse SE_M_HEAL_BELL
	call PrismCaverns_EventScript_LitBlue
	special DrawWholeMapView
	waitse
	setflag FLAG_PRISM_SAPPHIRE
	msgbox PrismCaverns_Text_ResBlue, MSGBOX_DEFAULT
	call PrismCaverns_EventScript_WakeBlue
	playse SE_GLASS_FLUTE
	special DrawWholeMapView
	waitse
	msgbox PrismCaverns_Text_PadsWakeBlue, MSGBOX_DEFAULT
	releaseall
	end


PrismCaverns_EventScript_ResAlreadyLit::
	msgbox PrismCaverns_Text_ResAlreadyLit, MSGBOX_DEFAULT
	releaseall
	end

PrismCaverns_EventScript_ResLeave::
	releaseall
	end

PrismCaverns_Text_ResTouch:
	.string "A tall crystal on a stone pedestal.\n"
	.string "It's dull and grey, but it hums\l"
	.string "faintly when you come near.\p"
	.string "Touch the crystal?$"

PrismCaverns_Text_ResAlreadyLit:
	.string "The crystal shines brightly,\n"
	.string "humming a clear, steady note.$"

PrismCaverns_Text_ResRed:
	.string "The crystal rang like a bell and\n"
	.string "began to glow a deep ruby red!$"

PrismCaverns_Text_ResBlue:
	.string "The crystal rang like a bell and\n"
	.string "began to glow a cool sapphire blue!$"

PrismCaverns_Text_ResGreen:
	.string "The crystal rang like a bell and\n"
	.string "began to glow a bright emerald green!$"

PrismCaverns_Text_ResGold:
	.string "The crystal rang like a bell and\n"
	.string "began to glow a warm topaz gold!$"

PrismCaverns_Text_PadsWakeRed:
	.string "Somewhere in the caverns, the red\n"
	.string "plates in the floor lit up!$"

PrismCaverns_Text_PadsWakeBlue:
	.string "Somewhere in the caverns, the blue\n"
	.string "plates in the floor lit up!$"

PrismCaverns_Text_PadsWakeGreen:
	.string "Somewhere in the caverns, the green\n"
	.string "plates in the floor lit up!$"

PrismCaverns_Text_PadsWakeGold:
	.string "Somewhere in the caverns, the gold\n"
	.string "plates in the floor lit up!$"


PrismCaverns_EventScript_CampSign::
	msgbox PrismCaverns_Text_CampSign, MSGBOX_SIGN
	end

PrismCaverns_EventScript_Carving::
	msgbox PrismCaverns_Text_Carving, MSGBOX_SIGN
	end

PrismCaverns_EventScript_Medic::
	lock
	faceplayer
	msgbox PrismCaverns_Text_Medic, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PrismCaverns_EventScript_MedicNo
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox PrismCaverns_Text_MedicDone, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_MedicNo::
	msgbox PrismCaverns_Text_MedicNo, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_Assistant::
	lock
	faceplayer
	goto_if_set FLAG_PRISM_SEAL_OPEN, PrismCaverns_EventScript_AssistantWhite
	msgbox PrismCaverns_Text_Assistant, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_AssistantWhite::
	msgbox PrismCaverns_Text_AssistantWhite, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_Geologist::
	lock
	faceplayer
	goto_if_set FLAG_PRISM_GLIMMET, PrismCaverns_EventScript_GeologistDone
	goto_if_set FLAG_PRISM_SEAL_OPEN, PrismCaverns_EventScript_GeologistReward
	goto_if_set FLAG_PRISM_SAPPHIRE, PrismCaverns_EventScript_GeologistTwo
	goto_if_set FLAG_PRISM_RUBY, PrismCaverns_EventScript_GeologistOne
	goto_if_set FLAG_PRISM_GEOLOGIST_TALKED, PrismCaverns_EventScript_GeologistAgain
	setflag FLAG_PRISM_GEOLOGIST_TALKED
	msgbox PrismCaverns_Text_Geologist, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_GeologistAgain::
	msgbox PrismCaverns_Text_GeologistAgain, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_GeologistOne::
	msgbox PrismCaverns_Text_GeologistOne, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_GeologistTwo::
	msgbox PrismCaverns_Text_GeologistTwo, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_GeologistReward::
	msgbox PrismCaverns_Text_GeologistThanks, MSGBOX_DEFAULT
	givemon SPECIES_GLIMMET, 30
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, PrismCaverns_EventScript_GeologistFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox PrismCaverns_Text_GotGlimmet, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_PRISM_GLIMMET
	msgbox PrismCaverns_Text_GeologistDone, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_GeologistFull::
	msgbox PrismCaverns_Text_GeologistFull, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_GeologistDone::
	msgbox PrismCaverns_Text_GeologistDone, MSGBOX_DEFAULT
	release
	end

PrismCaverns_EventScript_Garnet::
	trainerbattle_single TRAINER_PRISM_GARNET, PrismCaverns_Text_GarnetIntro, PrismCaverns_Text_GarnetDefeat
	msgbox PrismCaverns_Text_GarnetPost, MSGBOX_AUTOCLOSE
	end

PrismCaverns_Text_GarnetIntro:
	.string "These crystals ring when you knock\n"
	.string "'em! Let's make some noise!$"

PrismCaverns_Text_GarnetDefeat:
	.string "Rang my bell, you did.$"

PrismCaverns_Text_GarnetPost:
	.string "The ruby's past the big formations.\n"
	.string "Follow the shards on the floor.$"


PrismCaverns_EventScript_Quinn::
	trainerbattle_single TRAINER_PRISM_QUINN, PrismCaverns_Text_QuinnIntro, PrismCaverns_Text_QuinnDefeat
	msgbox PrismCaverns_Text_QuinnPost, MSGBOX_AUTOCLOSE
	end

PrismCaverns_Text_QuinnIntro:
	.string "Shiny things, shiny POKéMON.\n"
	.string "Show me what sparkles on your team!$"

PrismCaverns_Text_QuinnDefeat:
	.string "Outshone!$"

PrismCaverns_Text_QuinnPost:
	.string "Light a crystal and its plates wake\n"
	.string "up all over the caverns. Pretty!$"


PrismCaverns_EventScript_Lucas::
	trainerbattle_single TRAINER_PRISM_LUCAS, PrismCaverns_Text_LucasIntro, PrismCaverns_Text_LucasDefeat
	msgbox PrismCaverns_Text_LucasPost, MSGBOX_AUTOCLOSE
	end

PrismCaverns_Text_LucasIntro:
	.string "The gallery sings to me…\n"
	.string "Can you hear it? No? Then battle!$"

PrismCaverns_Text_LucasDefeat:
	.string "The song went quiet…$"

PrismCaverns_Text_LucasPost:
	.string "The blue crystal sleeps at the end of\n"
	.string "this gallery. Wake it gently.$"


PrismCaverns_EventScript_Oswin::
	trainerbattle_single TRAINER_PRISM_OSWIN, PrismCaverns_Text_OswinIntro, PrismCaverns_Text_OswinDefeat
	msgbox PrismCaverns_Text_OswinPost, MSGBOX_AUTOCLOSE
	end

PrismCaverns_Text_OswinIntro:
	.string "Old carvings, crystal halls… This is\n"
	.string "a ruin maniac's dream! Battle me!$"

PrismCaverns_Text_OswinDefeat:
	.string "I got carried away.$"

PrismCaverns_Text_OswinPost:
	.string "Read the carving on the big\n"
	.string "formation. It speaks of a heart.$"


PrismCaverns_Text_CampSign:
	.string "GEOLOGY TEAM CAMP\n"
	.string "Survey of the PRISM CAVERNS.\l"
	.string "Do not lick the crystals.$"

PrismCaverns_Text_Carving:
	.string "Words are cut into the crystal:\p"
	.string "RUBY, SAPPHIRE, EMERALD, TOPAZ.\n"
	.string "When four sing as one, the seal\l"
	.string "breaks and the heart awakens.\p"
	.string "Then the white way opens for those\n"
	.string "who listen.$"

PrismCaverns_Text_Medic:
	.string "I'm the team's medic. Long day in\n"
	.string "the caverns? Shall I patch up\l"
	.string "your POKéMON?$"

PrismCaverns_Text_MedicDone:
	.string "All better! Mind the sharp edges.$"

PrismCaverns_Text_MedicNo:
	.string "I'll be here if you need me.$"

PrismCaverns_Text_Assistant:
	.string "The plate behind the camp is white.\n"
	.string "We've never seen it light up.\p"
	.string "What could wake it?$"

PrismCaverns_Text_AssistantWhite:
	.string "The white plate is glowing now!\n"
	.string "Where does it go? I'm too scared\l"
	.string "to find out. You go first!$"

PrismCaverns_Text_Geologist:
	.string "I'm DR. MICA, the team leader.\n"
	.string "Welcome to the PRISM CAVERNS!\p"
	.string "Four resonance crystals sleep in\n"
	.string "here. Wake one by touching it, and\l"
	.string "the plates of its colour light up.\p"
	.string "Step on a lit plate and it carries\n"
	.string "you somewhere else. The red crystal\l"
	.string "is east, through the garden.$"

PrismCaverns_Text_GeologistAgain:
	.string "The ruby crystal is at the north end\n"
	.string "of the garden, east of camp.\p"
	.string "Once it's lit, try the red plate\n"
	.string "by our crates.$"

PrismCaverns_Text_GeologistOne:
	.string "You woke the ruby! The red plate\n"
	.string "by our crates is glowing.\p"
	.string "My guess: it leads to the gallery\n"
	.string "where the sapphire sleeps.$"

PrismCaverns_Text_GeologistTwo:
	.string "Ruby and sapphire, both lit!\n"
	.string "The blue plates reach the north\l"
	.string "hall and the ladder to the DEPTHS.\p"
	.string "The emerald and topaz must be down\n"
	.string "there, in the dark.$"

PrismCaverns_Text_GeologistThanks:
	.string "We felt it up here: the seal broke!\n"
	.string "You woke all four crystals!\p"
	.string "Please, take this GLIMMET. It hatched\n"
	.string "from a crystal at our camp, and it\l"
	.string "has been following you around.$"

PrismCaverns_Text_GotGlimmet:
	.string "{PLAYER} received a GLIMMET!$"

PrismCaverns_Text_GeologistFull:
	.string "Your party's full! Make room and\n"
	.string "come back for GLIMMET.$"

PrismCaverns_Text_GeologistDone:
	.string "The white plate behind our camp\n"
	.string "lit up when the seal broke.\p"
	.string "The carving says the white way\n"
	.string "opens for those who listen…$"

PrismCaverns_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

PrismCaverns_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

PrismCaverns_EventScript_Item_MaxRepel::
	finditem ITEM_MAX_REPEL
	end

PrismCaverns_EventScript_Item_TmLightScreen::
	finditem ITEM_TM_LIGHT_SCREEN
	end

PrismCaverns_EventScript_Item_CometShard::
	finditem ITEM_COMET_SHARD
	end

PrismCaverns_EventScript_Item_PpUp::
	finditem ITEM_PP_UP
	end

