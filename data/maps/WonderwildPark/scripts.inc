WonderwildPark_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, WonderwildPark_OnTransition
	.byte 0

WonderwildPark_OnTransition:
	setflag FLAG_HIDE_PARK_PARADE
	goto_if_unset FLAG_PARK_PARADE, WonderwildPark_EventScript_NoParade
	clearflag FLAG_HIDE_PARK_PARADE
WonderwildPark_EventScript_NoParade::
	end

WonderwildPark_EventScript_Director::
	lock
	faceplayer
	goto_if_set FLAG_PARK_PARADE, WonderwildPark_EventScript_DirectorDone
	goto_if_unset FLAG_PARK_MASCOT_1, WonderwildPark_EventScript_DirectorAsk
	goto_if_unset FLAG_PARK_MASCOT_2, WonderwildPark_EventScript_DirectorAsk
	goto_if_unset FLAG_PARK_MASCOT_3, WonderwildPark_EventScript_DirectorAsk
	msgbox WonderwildPark_Text_DirectorThanks, MSGBOX_DEFAULT
	givemon SPECIES_TANDEMAUS, 18
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, WonderwildPark_EventScript_DirectorFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox WonderwildPark_Text_GotTandemaus, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_PARK_PARADE
	msgbox WonderwildPark_Text_DirectorParade, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	clearflag FLAG_HIDE_PARK_PARADE
	addobject 9
	addobject 10
	addobject 11
	playfanfare MUS_LEVEL_UP
	fadescreen FADE_FROM_BLACK
	waitfanfare
	release
	end

WonderwildPark_EventScript_DirectorFull::
	msgbox WonderwildPark_Text_DirectorFull, MSGBOX_DEFAULT
	release
	end

WonderwildPark_EventScript_DirectorAsk::
	msgbox WonderwildPark_Text_DirectorAsk, MSGBOX_DEFAULT
	release
	end

WonderwildPark_EventScript_DirectorDone::
	msgbox WonderwildPark_Text_DirectorDone, MSGBOX_DEFAULT
	release
	end

WonderwildPark_EventScript_Parade::
	msgbox WonderwildPark_Text_Parade, MSGBOX_NPC
	end

WonderwildPark_EventScript_Gate::
	lock
	faceplayer
	msgbox WonderwildPark_Text_GateBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WonderwildPark_EventScript_GateStay
	closemessage
	warp MAP_SLATEPORT_CITY, 17, 4
	waitstate
	release
	end

WonderwildPark_EventScript_GateStay::
	msgbox WonderwildPark_Text_GateStay, MSGBOX_DEFAULT
	release
	end

WonderwildPark_EventScript_CoasterHost::
	lock
	faceplayer
	msgbox WonderwildPark_Text_CoasterAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WonderwildPark_EventScript_CoasterNo
	msgbox WonderwildPark_Text_CoasterGo, MSGBOX_DEFAULT
	closemessage
	playse SE_TRUCK_DOOR
	waitse
	warp MAP_WONDERWILD_PARK, 40, 30
	waitstate
	release
	end

WonderwildPark_EventScript_CoasterNo::
	msgbox WonderwildPark_Text_CoasterNo, MSGBOX_DEFAULT
	release
	end

WonderwildPark_EventScript_TeacupHost::
	msgbox WonderwildPark_Text_TeacupHost, MSGBOX_NPC
	end

WonderwildPark_EventScript_MazeHost::
	msgbox WonderwildPark_Text_MazeHost, MSGBOX_NPC
	end

WonderwildPark_EventScript_Girl::
	msgbox WonderwildPark_Text_Girl, MSGBOX_NPC
	end

WonderwildPark_EventScript_Popcorn::
	msgbox WonderwildPark_Text_Popcorn, MSGBOX_NPC
	end

WonderwildPark_EventScript_Statue::
	msgbox WonderwildPark_Text_Statue, MSGBOX_SIGN
	end

WonderwildPark_EventScript_Booth::
	msgbox WonderwildPark_Text_Booth, MSGBOX_SIGN
	end

WonderwildPark_EventScript_Carousel::
	msgbox WonderwildPark_Text_Carousel, MSGBOX_SIGN
	end

WonderwildPark_EventScript_Mascot1::
	lock
	faceplayer
	playmoncry SPECIES_CLEFAIRY, CRY_MODE_NORMAL
	msgbox WonderwildPark_Text_Mascot1, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_PARK_MASCOT_1
	fadescreen FADE_FROM_BLACK
	msgbox WonderwildPark_Text_MascotGone, MSGBOX_DEFAULT
	release
	end

WonderwildPark_Text_Mascot1:
	.string "Clefairy! It's been going round in\n"
	.string "circles in here for hours.\p"
	.string "CLEFAIRY hopped off back to the\n"
	.string "castle!$"
WonderwildPark_EventScript_Mascot2::
	lock
	faceplayer
	playmoncry SPECIES_JIGGLYPUFF, CRY_MODE_NORMAL
	msgbox WonderwildPark_Text_Mascot2, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_PARK_MASCOT_2
	fadescreen FADE_FROM_BLACK
	msgbox WonderwildPark_Text_MascotGone, MSGBOX_DEFAULT
	release
	end

WonderwildPark_Text_Mascot2:
	.string "Puuuff… JIGGLYPUFF is dizzy from\n"
	.string "all the spinning.\p"
	.string "JIGGLYPUFF floated back to the\n"
	.string "castle!$"
WonderwildPark_EventScript_Mascot3::
	lock
	faceplayer
	playmoncry SPECIES_TOGEPI, CRY_MODE_NORMAL
	msgbox WonderwildPark_Text_Mascot3, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_PARK_MASCOT_3
	fadescreen FADE_FROM_BLACK
	msgbox WonderwildPark_Text_MascotGone, MSGBOX_DEFAULT
	release
	end

WonderwildPark_Text_Mascot3:
	.string "Togepriii! TOGEPI was posing for\n"
	.string "the ride photos.\p"
	.string "TOGEPI toddled back to the castle!$"
WonderwildPark_Text_MascotGone:
	.string "That's one mascot found. Better tell\n"
	.string "the DIRECTOR.$"

WonderwildPark_EventScript_Timmy::
	trainerbattle_single TRAINER_PARK_TIMMY, WonderwildPark_Text_TimmyIntro, WonderwildPark_Text_TimmyDefeat
	msgbox WonderwildPark_Text_TimmyPost, MSGBOX_AUTOCLOSE
	end

WonderwildPark_Text_TimmyIntro:
	.string "I've been on every ride twice! Now\nlet's battle!$"

WonderwildPark_Text_TimmyDefeat:
	.string "I'm dizzy…$"

WonderwildPark_Text_TimmyPost:
	.string "The maze is the best. Nobody finds\nthe middle!$"

WonderwildPark_EventScript_Poppy::
	trainerbattle_single TRAINER_PARK_POPPY, WonderwildPark_Text_PoppyIntro, WonderwildPark_Text_PoppyDefeat
	msgbox WonderwildPark_Text_PoppyPost, MSGBOX_AUTOCLOSE
	end

WonderwildPark_Text_PoppyIntro:
	.string "Teacups make me dizzy, but battles\nmake me happy!$"

WonderwildPark_Text_PoppyDefeat:
	.string "Round and round I go…$"

WonderwildPark_Text_PoppyPost:
	.string "The arrows on the cups send you\nflying! Look before you step.$"

WonderwildPark_EventScript_Remy::
	trainerbattle_single TRAINER_PARK_REMY, WonderwildPark_Text_RemyIntro, WonderwildPark_Text_RemyDefeat
	msgbox WonderwildPark_Text_RemyPost, MSGBOX_AUTOCLOSE
	end

WonderwildPark_Text_RemyIntro:
	.string "School trip! Battle me while the\nteacher's not looking!$"

WonderwildPark_Text_RemyDefeat:
	.string "Detention…$"

WonderwildPark_Text_RemyPost:
	.string "The coaster's last stop is a photo\ndeck. You can only get there by\lriding!$"

WonderwildPark_EventScript_Gloria::
	trainerbattle_single TRAINER_PARK_GLORIA, WonderwildPark_Text_GloriaIntro, WonderwildPark_Text_GloriaDefeat
	msgbox WonderwildPark_Text_GloriaPost, MSGBOX_AUTOCLOSE
	end

WonderwildPark_Text_GloriaIntro:
	.string "My babies want to see the parade!\nAnd beat you, of course.$"

WonderwildPark_Text_GloriaDefeat:
	.string "My little darlings!$"

WonderwildPark_Text_GloriaPost:
	.string "No mascots, no parade. Have you\nseen them?$"

WonderwildPark_Text_DirectorAsk:
	.string "Welcome to WONDERWILD PARK! I'm the\n"
	.string "DIRECTOR.\p"
	.string "Our mascots CLEFAIRY, JIGGLYPUFF and\n"
	.string "TOGEPI ran off before the parade!\p"
	.string "One went into the hedge maze, one\n"
	.string "onto the teacups, and one rode the\l"
	.string "coaster. Please, find them!$"

WonderwildPark_Text_DirectorThanks:
	.string "All three mascots are home! The\n"
	.string "parade can go on!\p"
	.string "Here, this TANDEMAUS family has been\n"
	.string "helping backstage. They'd love to\l"
	.string "travel with you.$"

WonderwildPark_Text_GotTandemaus:
	.string "{PLAYER} received a TANDEMAUS!$"

WonderwildPark_Text_DirectorFull:
	.string "Your party's full! Come back for\n"
	.string "TANDEMAUS when you have room.$"

WonderwildPark_Text_DirectorParade:
	.string "Let the parade begin!$"

WonderwildPark_Text_DirectorDone:
	.string "The parade's the best it's ever\n"
	.string "been. Thank you!$"

WonderwildPark_Text_Parade:
	.string "The mascots are dancing in the\n"
	.string "parade!$"

WonderwildPark_Text_GateBack:
	.string "Leaving the park? The tram goes\n"
	.string "back to SLATEPORT CITY.$"

WonderwildPark_Text_GateStay:
	.string "Enjoy the rides!$"

WonderwildPark_Text_CoasterAsk:
	.string "This is the RAPIDASH RACER, the\n"
	.string "fastest coaster in HOENN!\p"
	.string "Want a ride?$"

WonderwildPark_Text_CoasterGo:
	.string "Keep your arms inside the car!$"

WonderwildPark_Text_CoasterNo:
	.string "Chicken!$"

WonderwildPark_Text_TeacupHost:
	.string "Step on a teacup and it'll spin you\n"
	.string "off the way its arrow points, until\l"
	.string "you land on a gold plate.$"

WonderwildPark_Text_MazeHost:
	.string "The hedge maze! Some say there's a\n"
	.string "present tucked in the far corner.$"

WonderwildPark_Text_Girl:
	.string "I want to ride the carousel\n"
	.string "again!$"

WonderwildPark_Text_Popcorn:
	.string "Popcorn! Get your popcorn!\n"
	.string "…Sorry, I've just sold out.$"

WonderwildPark_Text_Statue:
	.string "WONDERWILD PARK\n"
	.string "Where every POKéMON is a star!$"

WonderwildPark_Text_Booth:
	.string "TICKETS\n"
	.string "Free entry for trainers today!$"

WonderwildPark_Text_Carousel:
	.string "The carousel is turning slowly.\n"
	.string "The music box plays a merry tune.$"

WonderwildPark_EventScript_Item_Lemonade::
	finditem ITEM_LEMONADE
	end

WonderwildPark_EventScript_Item_SuperPotion::
	finditem ITEM_SUPER_POTION
	end

WonderwildPark_EventScript_Item_GreatBall::
	finditem ITEM_GREAT_BALL
	end

WonderwildPark_EventScript_Item_MoonStone::
	finditem ITEM_MOON_STONE
	end

