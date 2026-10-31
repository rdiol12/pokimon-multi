EmberSprings_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, EmberSprings_OnTransition
	.byte 0

EmberSprings_OnTransition:
	call_if_set FLAG_EMBER_QUEST_DONE, EmberSprings_EventScript_HideGuard
	end

EmberSprings_EventScript_HideGuard::
	setflag FLAG_HIDE_EMBER_SPRING_GUARD
	return

EmberSprings_EventScript_Guide::
	lock
	faceplayer
	msgbox EmberSprings_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EmberSprings_EventScript_GuideStay
	closemessage
	warp MAP_LAVARIDGE_TOWN, 17, 10
	waitstate
	release
	end

EmberSprings_EventScript_GuideStay::
	msgbox EmberSprings_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_TownSign::
	msgbox EmberSprings_Text_TownSign, MSGBOX_SIGN
	end

EmberSprings_EventScript_SpringSign::
	msgbox EmberSprings_Text_SpringSign, MSGBOX_SIGN
	end

@ A soak heals the party and raises friendship (src/area_specials.c).
EmberSprings_EventScript_Attendant::
	lock
	faceplayer
	msgbox EmberSprings_Text_Soak, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EmberSprings_EventScript_SoakNo
	closemessage
	fadescreen FADE_TO_WHITE
	playfanfare MUS_HEAL
	callnative Area_SoakInHotSpring
	waitfanfare
	fadescreen FADE_FROM_WHITE
	msgbox EmberSprings_Text_Soaked, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_SoakNo::
	msgbox EmberSprings_Text_SoakNo, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_SpringKeeper::
	lock
	faceplayer
	goto_if_set FLAG_EMBER_QUEST_DONE, EmberSprings_EventScript_KeeperAfter
	goto_if_unset FLAG_EMBER_SLUGMA_WEST, EmberSprings_EventScript_KeeperAsk
	goto_if_unset FLAG_EMBER_SLUGMA_NORTH, EmberSprings_EventScript_KeeperAsk
	goto_if_unset FLAG_EMBER_SLUGMA_EAST, EmberSprings_EventScript_KeeperAsk
	msgbox EmberSprings_Text_KeeperThanks, MSGBOX_DEFAULT
	giveitem ITEM_SOOTHE_BELL
	giveitem ITEM_TM_OVERHEAT
	setflag FLAG_EMBER_QUEST_DONE
	msgbox EmberSprings_Text_KeeperOldSpring, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_KeeperAsk::
	msgbox EmberSprings_Text_KeeperAsk, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_KeeperAfter::
	msgbox EmberSprings_Text_KeeperAfter, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_VentCleared::
	goto_if_unset FLAG_EMBER_SLUGMA_WEST, EmberSprings_EventScript_VentsLeft
	goto_if_unset FLAG_EMBER_SLUGMA_NORTH, EmberSprings_EventScript_VentsLeft
	goto_if_unset FLAG_EMBER_SLUGMA_EAST, EmberSprings_EventScript_VentsLeft
	msgbox EmberSprings_Text_AllVents, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_VentsLeft::
	msgbox EmberSprings_Text_VentCleared, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_SlugmaWest::
	lock
	faceplayer
	playmoncry SPECIES_SLUGMA, CRY_MODE_ENCOUNTER
	msgbox EmberSprings_Text_SlugmaVent, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_SLUGMA, 31
	dowildbattle
	setflag FLAG_EMBER_SLUGMA_WEST
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_EMBER_SLUGMA_WEST
	fadescreen FADE_FROM_BLACK
	goto EmberSprings_EventScript_VentCleared

EmberSprings_EventScript_SlugmaNorth::
	lock
	faceplayer
	playmoncry SPECIES_SLUGMA, CRY_MODE_ENCOUNTER
	msgbox EmberSprings_Text_SlugmaVent, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_SLUGMA, 32
	dowildbattle
	setflag FLAG_EMBER_SLUGMA_NORTH
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_EMBER_SLUGMA_NORTH
	fadescreen FADE_FROM_BLACK
	goto EmberSprings_EventScript_VentCleared

EmberSprings_EventScript_SlugmaEast::
	lock
	faceplayer
	playmoncry SPECIES_SLUGMA, CRY_MODE_ENCOUNTER
	msgbox EmberSprings_Text_SlugmaVent, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_SLUGMA, 33
	dowildbattle
	setflag FLAG_EMBER_SLUGMA_EAST
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_EMBER_SLUGMA_EAST
	fadescreen FADE_FROM_BLACK
	goto EmberSprings_EventScript_VentCleared

EmberSprings_EventScript_Guard::
	msgbox EmberSprings_Text_Guard, MSGBOX_NPC
	end

EmberSprings_EventScript_Torkoal::
	lock
	faceplayer
	playmoncry SPECIES_TORKOAL, CRY_MODE_NORMAL
	msgbox EmberSprings_Text_Torkoal, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ The Old Spring: a free soak, and once, an Egg from the lady who keeps it.
EmberSprings_EventScript_OldSpringLady::
	lock
	faceplayer
	goto_if_set FLAG_EMBER_EGG_GIVEN, EmberSprings_EventScript_OldSpringSoak
	msgbox EmberSprings_Text_EggOffer, MSGBOX_DEFAULT
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, EmberSprings_EventScript_EggNoRoom
	setflag FLAG_EMBER_EGG_GIVEN
	playfanfare MUS_OBTAIN_ITEM
	message EmberSprings_Text_GotEgg
	waitfanfare
	giveegg SPECIES_CHARCADET
	msgbox EmberSprings_Text_EggCare, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_EggNoRoom::
	msgbox EmberSprings_Text_EggNoRoom, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_OldSpringSoak::
	msgbox EmberSprings_Text_OldSoak, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EmberSprings_EventScript_OldSoakNo
	closemessage
	fadescreen FADE_TO_WHITE
	playfanfare MUS_HEAL
	callnative Area_SoakInHotSpring
	callnative Area_SoakInHotSpring
	waitfanfare
	fadescreen FADE_FROM_WHITE
	msgbox EmberSprings_Text_OldSoaked, MSGBOX_DEFAULT
EmberSprings_EventScript_OldSoakNo::
	release
	end

EmberSprings_EventScript_Blaze::
	trainerbattle_single TRAINER_EMBER_BLAZE, EmberSprings_Text_BlazeIntro, EmberSprings_Text_BlazeDefeat
	msgbox EmberSprings_Text_BlazePost, MSGBOX_AUTOCLOSE
	end

EmberSprings_EventScript_Cindy::
	trainerbattle_single TRAINER_EMBER_CINDY, EmberSprings_Text_CindyIntro, EmberSprings_Text_CindyDefeat
	msgbox EmberSprings_Text_CindyPost, MSGBOX_AUTOCLOSE
	end

EmberSprings_EventScript_Rocco::
	trainerbattle_single TRAINER_EMBER_ROCCO, EmberSprings_Text_RoccoIntro, EmberSprings_Text_RoccoDefeat
	msgbox EmberSprings_Text_RoccoPost, MSGBOX_AUTOCLOSE
	end

EmberSprings_EventScript_Saki::
	trainerbattle_single TRAINER_EMBER_SAKI, EmberSprings_Text_SakiIntro, EmberSprings_Text_SakiDefeat
	msgbox EmberSprings_Text_SakiPost, MSGBOX_AUTOCLOSE
	end

EmberSprings_EventScript_Boy::
	msgbox EmberSprings_Text_Boy, MSGBOX_NPC
	end

EmberSprings_EventScript_Numel::
	lock
	faceplayer
	playmoncry SPECIES_NUMEL, CRY_MODE_NORMAL
	msgbox EmberSprings_Text_Numel, MSGBOX_DEFAULT
	waitmoncry
	release
	end

EmberSprings_EventScript_Tourist::
	msgbox EmberSprings_Text_Tourist, MSGBOX_NPC
	end

EmberSprings_Text_GuideBack:
	.string "Heading back down to LAVARIDGE?\n"
	.string "I can show you the short trail.$"

EmberSprings_Text_GuideStay:
	.string "Stay as long as you like. The water\n"
	.string "is always warm here.$"

EmberSprings_Text_TownSign:
	.string "EMBER SPRINGS\n"
	.string "“Where the mountain keeps you warm.”$"

EmberSprings_Text_SpringSign:
	.string "PUBLIC SPRING\n"
	.string "Ask the attendant for a soak.$"

EmberSprings_Text_Soak:
	.string "Welcome to the springs! A soak heals\n"
	.string "your POKéMON and brings you closer.\p"
	.string "Would you like to take a soak?$"

EmberSprings_Text_Soaked:
	.string "Your POKéMON look relaxed and happy.\n"
	.string "They seem to like you even more!$"

EmberSprings_Text_SoakNo:
	.string "Come back whenever you're weary.$"

EmberSprings_Text_KeeperAsk:
	.string "I keep the springs flowing. But the\n"
	.string "steam vents on the hills are plugged!\p"
	.string "SLUGMA have curled up on all three\n"
	.string "of them to keep warm.\p"
	.string "West, north and east… Could you\n"
	.string "chase them off for me?$"

EmberSprings_Text_KeeperThanks:
	.string "The vents are steaming again! The\n"
	.string "springs will be hot for years.\p"
	.string "Please, take these.$"

EmberSprings_Text_KeeperOldSpring:
	.string "And I've told my son to stop\n"
	.string "guarding the OLD SPRING.\p"
	.string "Go and soak in it. It's the oldest\n"
	.string "water in HOENN.$"

EmberSprings_Text_KeeperAfter:
	.string "Steam, steam, glorious steam!$"

EmberSprings_Text_SlugmaVent:
	.string "A SLUGMA is curled up on a steam\n"
	.string "vent. It won't budge!$"

EmberSprings_Text_VentCleared:
	.string "The vent hisses back to life!$"

EmberSprings_Text_AllVents:
	.string "That was the last plugged vent!\n"
	.string "The whole valley fills with steam.\p"
	.string "The SPRING KEEPER will want to\n"
	.string "hear about this.$"

EmberSprings_Text_Guard:
	.string "The OLD SPRING is only for those\n"
	.string "my father trusts.\p"
	.string "He's worried about the steam vents\n"
	.string "on the hills, you know.$"

EmberSprings_Text_Torkoal:
	.string "TORKOAL: Koooh…\p"
	.string "It's puffing happy smoke rings.$"

EmberSprings_Text_EggOffer:
	.string "Oh, you cleared the vents! The OLD\n"
	.string "SPRING is warm enough to hatch eggs\l"
	.string "again.\p"
	.string "This one was laid in the warm sand.\n"
	.string "Please raise it.$"

EmberSprings_Text_GotEgg:
	.string "{PLAYER} received an EGG!$"

EmberSprings_Text_EggCare:
	.string "Keep it warm and it'll hatch into\n"
	.string "something fiery.$"

EmberSprings_Text_EggNoRoom:
	.string "Your party is full. Make room and\n"
	.string "come back.$"

EmberSprings_Text_OldSoak:
	.string "The OLD SPRING is extra warm today.\n"
	.string "Take a soak?$"

EmberSprings_Text_OldSoaked:
	.string "Your POKéMON look blissful.\n"
	.string "They trust you more than ever!$"

EmberSprings_Text_BlazeIntro:
	.string "My POKéMON and I train in the\n"
	.string "steam. Feel the heat!$"

EmberSprings_Text_BlazeDefeat:
	.string "I'm steamed!$"

EmberSprings_Text_BlazePost:
	.string "After a battle, a soak is the best.$"

EmberSprings_Text_CindyIntro:
	.string "The springs keep my skin glowing.\n"
	.string "And my POKéMON's fur!$"

EmberSprings_Text_CindyDefeat:
	.string "Oh, I'm all flustered.$"

EmberSprings_Text_CindyPost:
	.string "PONYTA's mane gets so shiny after\n"
	.string "a soak.$"

EmberSprings_Text_RoccoIntro:
	.string "I climbed all the way up here just\n"
	.string "for a battle!$"

EmberSprings_Text_RoccoDefeat:
	.string "That rocked!$"

EmberSprings_Text_RoccoPost:
	.string "I found a HEAT ROCK on the north\n"
	.string "hill once.$"

EmberSprings_Text_SakiIntro:
	.string "I guard the path to the OLD\n"
	.string "SPRING's hollow. Prove yourself!$"

EmberSprings_Text_SakiDefeat:
	.string "Your spirit burns bright.$"

EmberSprings_Text_SakiPost:
	.string "Some say a fiery legend sleeps\n"
	.string "deep inside the volcano.$"

EmberSprings_Text_Boy:
	.string "My NUMEL won't get out of the\n"
	.string "springs. It's too comfy!$"

EmberSprings_Text_Numel:
	.string "NUMEL: Nuuu…$"

EmberSprings_Text_Tourist:
	.string "I came all the way from SLATEPORT\n"
	.string "for the springs. Worth the climb!$"

EmberSprings_EventScript_Item_FireStone::
	finditem ITEM_FIRE_STONE
	end

EmberSprings_EventScript_Item_HeatRock::
	finditem ITEM_HEAT_ROCK
	end

EmberSprings_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

EmberSprings_EventScript_Item_LavaCookie::
	finditem ITEM_LAVA_COOKIE
	end

