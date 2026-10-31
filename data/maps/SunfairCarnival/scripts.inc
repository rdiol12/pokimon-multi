SunfairCarnival_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SunfairCarnival_OnTransition
	.byte 0

SunfairCarnival_OnTransition:
	goto_if_set FLAG_CARNIVAL_SPINDA_FOUND, SunfairCarnival_OnTransitionHome
	setflag FLAG_HIDE_CARNIVAL_SPINDA_HOME
	end

SunfairCarnival_OnTransitionHome:
	setflag FLAG_HIDE_CARNIVAL_SPINDA_LOST
	clearflag FLAG_HIDE_CARNIVAL_SPINDA_HOME
	end

SunfairCarnival_EventScript_Guide::
	lock
	faceplayer
	msgbox SunfairCarnival_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunfairCarnival_EventScript_GuideStay
	msgbox SunfairCarnival_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_SLATEPORT_CITY, 9, 23
	waitstate
	release
	end

SunfairCarnival_EventScript_GuideStay::
	msgbox SunfairCarnival_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_Sign::
	msgbox SunfairCarnival_Text_Sign, MSGBOX_SIGN
	end

@ ---- the Ringmaster's runaway Spinda
SunfairCarnival_EventScript_Ringmaster::
	lock
	faceplayer
	goto_if_set FLAG_CARNIVAL_EGG_DONE, SunfairCarnival_EventScript_RingmasterDone
	goto_if_set FLAG_CARNIVAL_SPINDA_FOUND, SunfairCarnival_EventScript_RingmasterThanks
	msgbox SunfairCarnival_Text_RingmasterAsk, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_RingmasterThanks::
	msgbox SunfairCarnival_Text_RingmasterThanks, MSGBOX_DEFAULT
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, SunfairCarnival_EventScript_RingmasterFull
	giveegg SPECIES_MIME_JR
	setflag FLAG_CARNIVAL_EGG_DONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox SunfairCarnival_Text_GotEgg, MSGBOX_DEFAULT
	waitfanfare
	msgbox SunfairCarnival_Text_RingmasterEgg, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_RingmasterFull::
	msgbox SunfairCarnival_Text_RingmasterFull, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_RingmasterDone::
	msgbox SunfairCarnival_Text_RingmasterDone, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_SpindaLost::
	lock
	faceplayer
	playmoncry SPECIES_SPINDA, CRY_MODE_NORMAL
	msgbox SunfairCarnival_Text_SpindaLost, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	setflag FLAG_CARNIVAL_SPINDA_FOUND
	setflag FLAG_HIDE_CARNIVAL_SPINDA_LOST
	removeobject VAR_LAST_TALKED
	clearflag FLAG_HIDE_CARNIVAL_SPINDA_HOME
	addobject 11
	fadescreen FADE_FROM_BLACK
	msgbox SunfairCarnival_Text_SpindaBack, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_SpindaHome::
	lock
	faceplayer
	playmoncry SPECIES_SPINDA, CRY_MODE_NORMAL
	msgbox SunfairCarnival_Text_SpindaHome, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ ---- stalls
SunfairCarnival_EventScript_FoodStall::
	lock
	faceplayer
	message SunfairCarnival_Text_FoodStall
	waitmessage
	pokemart SunfairCarnival_Mart_Food
	msgbox SunfairCarnival_Text_StallBye, MSGBOX_DEFAULT
	release
	end

	.align 2
SunfairCarnival_Mart_Food:
	.2byte ITEM_FRESH_WATER
	.2byte ITEM_SODA_POP
	.2byte ITEM_LEMONADE
	.2byte ITEM_MOOMOO_MILK
	.2byte ITEM_BERRY_JUICE
	pokemartlistend

SunfairCarnival_EventScript_SweetStall::
	lock
	faceplayer
	message SunfairCarnival_Text_SweetStall
	waitmessage
	pokemart SunfairCarnival_Mart_Sweets
	msgbox SunfairCarnival_Text_StallBye, MSGBOX_DEFAULT
	release
	end

	.align 2
SunfairCarnival_Mart_Sweets:
	.2byte ITEM_LAVA_COOKIE
	.2byte ITEM_RAGE_CANDY_BAR
	.2byte ITEM_SWEET_HEART
	.2byte ITEM_CASTELIACONE
	.2byte ITEM_LUMIOSE_GALETTE
	pokemartlistend

SunfairCarnival_EventScript_RingToss::
	lock
	faceplayer
	msgbox SunfairCarnival_Text_RingTossAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunfairCarnival_EventScript_RingTossNo
	checkmoney 500
	goto_if_eq VAR_RESULT, FALSE, SunfairCarnival_EventScript_RingTossBroke
	removemoney 500
	updatemoneybox
	playse SE_BALL_THROW
	msgbox SunfairCarnival_Text_RingTossThrow, MSGBOX_DEFAULT
	random 10
	switch VAR_RESULT
	case 0, SunfairCarnival_EventScript_RingTossCandy
	case 1, SunfairCarnival_EventScript_RingTossScale
	case 2, SunfairCarnival_EventScript_RingTossLemonade
	msgbox SunfairCarnival_Text_RingTossMiss, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_RingTossScale::
	playfanfare MUS_LEVEL_UP
	msgbox SunfairCarnival_Text_RingTossWin, MSGBOX_DEFAULT
	waitfanfare
	giveitem ITEM_HEART_SCALE
	release
	end

SunfairCarnival_EventScript_RingTossLemonade::
	playfanfare MUS_LEVEL_UP
	msgbox SunfairCarnival_Text_RingTossWin, MSGBOX_DEFAULT
	waitfanfare
	giveitem ITEM_LEMONADE
	release
	end

SunfairCarnival_EventScript_RingTossCandy::
	playfanfare MUS_LEVEL_UP
	msgbox SunfairCarnival_Text_RingTossWin, MSGBOX_DEFAULT
	waitfanfare
	giveitem ITEM_RARE_CANDY
	release
	end

SunfairCarnival_EventScript_RingTossNo::
	msgbox SunfairCarnival_Text_RingTossNo, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_RingTossBroke::
	msgbox SunfairCarnival_Text_RingTossBroke, MSGBOX_DEFAULT
	release
	end

@ ---- visitors
SunfairCarnival_EventScript_BalloonGirl::
	msgbox SunfairCarnival_Text_BalloonGirl, MSGBOX_NPC
	end

SunfairCarnival_EventScript_Pikachu::
	lock
	faceplayer
	playmoncry SPECIES_PIKACHU, CRY_MODE_NORMAL
	msgbox SunfairCarnival_Text_Pikachu, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunfairCarnival_EventScript_MrMime::
	lock
	faceplayer
	playmoncry SPECIES_MR_MIME, CRY_MODE_NORMAL
	msgbox SunfairCarnival_Text_MrMime, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunfairCarnival_EventScript_Fortune::
	lock
	faceplayer
	msgbox SunfairCarnival_Text_FortuneIntro, MSGBOX_DEFAULT
	random 4
	switch VAR_RESULT
	case 0, SunfairCarnival_EventScript_Fortune0
	case 1, SunfairCarnival_EventScript_Fortune1
	case 2, SunfairCarnival_EventScript_Fortune2
	msgbox SunfairCarnival_Text_Fortune3, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_Fortune0::
	msgbox SunfairCarnival_Text_Fortune0, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_Fortune1::
	msgbox SunfairCarnival_Text_Fortune1, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_Fortune2::
	msgbox SunfairCarnival_Text_Fortune2, MSGBOX_DEFAULT
	release
	end

SunfairCarnival_EventScript_OldMan::
	msgbox SunfairCarnival_Text_OldMan, MSGBOX_NPC
	end

SunfairCarnival_EventScript_Woman::
	msgbox SunfairCarnival_Text_Woman, MSGBOX_NPC
	end

SunfairCarnival_EventScript_Blaze::
	trainerbattle_single TRAINER_CARNIVAL_BLAZE, SunfairCarnival_Text_BlazeIntro, SunfairCarnival_Text_BlazeDefeat
	msgbox SunfairCarnival_Text_BlazePost, MSGBOX_AUTOCLOSE
	end

SunfairCarnival_EventScript_KitAndKat::
	trainerbattle_double TRAINER_CARNIVAL_KIT_AND_KAT, SunfairCarnival_Text_KitIntro, SunfairCarnival_Text_KitDefeat, SunfairCarnival_Text_KitNotEnough
	msgbox SunfairCarnival_Text_KitPost, MSGBOX_AUTOCLOSE
	end

SunfairCarnival_EventScript_Shade::
	trainerbattle_single TRAINER_CARNIVAL_SHADE, SunfairCarnival_Text_ShadeIntro, SunfairCarnival_Text_ShadeDefeat
	msgbox SunfairCarnival_Text_ShadePost, MSGBOX_AUTOCLOSE
	end

SunfairCarnival_EventScript_Riff::
	trainerbattle_single TRAINER_CARNIVAL_RIFF, SunfairCarnival_Text_RiffIntro, SunfairCarnival_Text_RiffDefeat
	msgbox SunfairCarnival_Text_RiffPost, MSGBOX_AUTOCLOSE
	end

@ ---- text
SunfairCarnival_Text_GuideBack:
	.string "Had your fill of fun? I can take you\n"
	.string "back to SLATEPORT.$"

SunfairCarnival_Text_GuideLead:
	.string "All aboard!$"

SunfairCarnival_Text_GuideStay:
	.string "The carnival's open all day and all\n"
	.string "night. Enjoy!$"

SunfairCarnival_Text_Sign:
	.string "SUNFAIR CARNIVAL\n"
	.string "Rides, games and treats for all!$"

SunfairCarnival_Text_RingmasterAsk:
	.string "Ladies and gentlemen… Oh, it's\n"
	.string "just you. Sorry.\p"
	.string "My star performer, SPINDA, spun so\n"
	.string "hard in rehearsal that it wobbled\l"
	.string "right out of the tent!\p"
	.string "It can't walk straight, so it can't\n"
	.string "have gone far. Could you find it?$"

SunfairCarnival_Text_RingmasterThanks:
	.string "SPINDA! You found it! The show can\n"
	.string "go on!\p"
	.string "Here, take this as thanks. A little\n"
	.string "performer of your own.$"

SunfairCarnival_Text_GotEgg:
	.string "{PLAYER} received an EGG from the\n"
	.string "RINGMASTER!$"

SunfairCarnival_Text_RingmasterEgg:
	.string "Whatever hatches from it is a born\n"
	.string "mime. Practise your invisible walls!$"

SunfairCarnival_Text_RingmasterFull:
	.string "Oh, your party's full. Come back\n"
	.string "when you have room for my gift.$"

SunfairCarnival_Text_RingmasterDone:
	.string "SPINDA's dizzy dance is the best act\n"
	.string "in the whole show!$"

SunfairCarnival_Text_SpindaLost:
	.string "SPINDA is wobbling about, very dizzy.\n"
	.string "It seems happy to see you.$"

SunfairCarnival_Text_SpindaBack:
	.string "{PLAYER} led SPINDA back to the big\n"
	.string "top!$"

SunfairCarnival_Text_SpindaHome:
	.string "SPINDA is practising its dizzy dance.$"

SunfairCarnival_Text_FoodStall:
	.string "Thirsty? We've got the coldest\n"
	.string "drinks in the carnival!$"

SunfairCarnival_Text_SweetStall:
	.string "Sweets! Get your sweets here!$"

SunfairCarnival_Text_StallBye:
	.string "Enjoy the carnival!$"

SunfairCarnival_Text_RingTossAsk:
	.string "Ring toss! Land a ring on a bottle\n"
	.string "and win a prize! Just ¥500 a throw.\l"
	.string "Want to play?$"

SunfairCarnival_Text_RingTossThrow:
	.string "{PLAYER} threw the ring…$"

SunfairCarnival_Text_RingTossWin:
	.string "It landed! We have a winner!$"

SunfairCarnival_Text_RingTossMiss:
	.string "Ooh, so close! It bounced off.$"

SunfairCarnival_Text_RingTossNo:
	.string "Maybe later, then!$"

SunfairCarnival_Text_RingTossBroke:
	.string "Sorry, it's ¥500 a throw.$"

SunfairCarnival_Text_BalloonGirl:
	.string "I got a PIKACHU balloon! And a real\n"
	.string "PIKACHU! Best day ever!$"

SunfairCarnival_Text_Pikachu:
	.string "PIKACHU is staring at the balloons.$"

SunfairCarnival_Text_MrMime:
	.string "MR. MIME is miming a wall. Nobody\n"
	.string "can get past it. Wait, there's no\l"
	.string "wall at all…$"

SunfairCarnival_Text_FortuneIntro:
	.string "Welcome to my tent of mysteries.\n"
	.string "Let me gaze into the crystal ball…$"

SunfairCarnival_Text_Fortune0:
	.string "I see… a shiny POKéMON in your\n"
	.string "future. Or a shiny coin. It's\l"
	.string "a bit blurry.$"

SunfairCarnival_Text_Fortune1:
	.string "I see… a long journey across the\n"
	.string "sea. Bring snacks.$"

SunfairCarnival_Text_Fortune2:
	.string "I see… a great battle, and a friend\n"
	.string "who will challenge you online.$"

SunfairCarnival_Text_Fortune3:
	.string "I see… a SPINDA. Dizzy. Somewhere\n"
	.string "north and east of here.$"

SunfairCarnival_Text_OldMan:
	.string "I've ridden the Ferris wheel every\n"
	.string "summer for fifty years. On a clear\l"
	.string "day you can see all of SLATEPORT.$"

SunfairCarnival_Text_Woman:
	.string "The RINGMASTER's SPINDA ran off\n"
	.string "again. Last I saw, it was wobbling\l"
	.string "past the Ferris wheel.$"

SunfairCarnival_Text_BlazeIntro:
	.string "Watch me breathe fire! Well, my\n"
	.string "POKéMON will!$"

SunfairCarnival_Text_BlazeDefeat:
	.string "Fizzled out!$"

SunfairCarnival_Text_BlazePost:
	.string "Don't try fire-breathing at home.$"

SunfairCarnival_Text_KitIntro:
	.string "KIT: We're the carnival's double act!\n"
	.string "KAT: Two against one!$"

SunfairCarnival_Text_KitDefeat:
	.string "KIT and KAT: We lost together!$"

SunfairCarnival_Text_KitNotEnough:
	.string "KIT: You need two POKéMON for our\n"
	.string "double act!$"

SunfairCarnival_Text_KitPost:
	.string "KAT: Next time we'll juggle too!$"

SunfairCarnival_Text_ShadeIntro:
	.string "You found me behind the big top!\n"
	.string "My hiding act is ruined!$"

SunfairCarnival_Text_ShadeDefeat:
	.string "Vanished… in defeat.$"

SunfairCarnival_Text_ShadePost:
	.string "Ninjas love carnivals. So many\n"
	.string "places to hide.$"

SunfairCarnival_Text_RiffIntro:
	.string "Every carnival needs a band! Let me\n"
	.string "play you a battle song!$"

SunfairCarnival_Text_RiffDefeat:
	.string "Broke a string!$"

SunfairCarnival_Text_RiffPost:
	.string "The crowd loved it anyway.$"

SunfairCarnival_EventScript_Item_StarPiece::
	finditem ITEM_STAR_PIECE
	end

SunfairCarnival_EventScript_Item_PpUp::
	finditem ITEM_PP_UP
	end

SunfairCarnival_EventScript_Item_TmAttract::
	finditem ITEM_TM_ATTRACT
	end

