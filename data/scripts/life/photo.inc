@ Photo safari: the photo ranger at the Safari Zone entrance, the shutter,
@ and the album in the R menu.

Life_EventScript_PhotoRanger::
	lock
	faceplayer
	callnative LifePhoto_RangerState
	goto_if_eq VAR_RESULT, LIFE_PHOTO_RANGER_STILL_OUT, Life_EventScript_PhotoStillOut
	goto_if_eq VAR_RESULT, LIFE_PHOTO_RANGER_PAYOUT, Life_EventScript_PhotoPayout
	msgbox Life_Text_PhotoIntro, MSGBOX_DEFAULT
	showmoneybox 0, 0
	msgbox Life_Text_PhotoAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Life_EventScript_PhotoDecline
	checkmoney 500
	goto_if_eq VAR_RESULT, FALSE, Life_EventScript_PhotoNoMoney
	playse SE_SHOP
	removemoney 500
	updatemoneybox
	msgbox Life_Text_PhotoHowTo, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	callnative LifePhoto_Start
	setvar VAR_SAFARI_ZONE_STATE, 2
	setflag FLAG_GOOD_LUCK_SAFARI_ZONE
	warp MAP_SAFARI_ZONE_SOUTH, 32, 33
	waitstate
	end

Life_EventScript_PhotoDecline:
	hidemoneybox
	msgbox Life_Text_PhotoDecline, MSGBOX_DEFAULT
	release
	end

Life_EventScript_PhotoNoMoney:
	hidemoneybox
	msgbox Life_Text_PhotoNoMoney, MSGBOX_DEFAULT
	release
	end

Life_EventScript_PhotoStillOut:
	msgbox Life_Text_PhotoStillOut, MSGBOX_DEFAULT
	release
	end

Life_EventScript_PhotoPayout:
	msgbox Life_Text_PhotoPayout, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	msgbox Life_Text_PhotoAgain, MSGBOX_DEFAULT
	release
	end

@ R in the Safari Zone with film left (LifePhoto_TrySnap).
Life_EventScript_PhotoSnap::
	lockall
	playse SE_CLICK
	waitse
	fadescreen FADE_TO_WHITE
	callnative LifePhoto_ShowPhoto
	waitstate
	goto_if_eq VAR_RESULT, 0, Life_EventScript_PhotoOutOfFilm
	releaseall
	end

Life_EventScript_PhotoOutOfFilm:
	msgbox Life_Text_PhotoOutOfFilm, MSGBOX_DEFAULT
	closemessage
	setvar VAR_SAFARI_ZONE_STATE, 1
	warpdoor MAP_ROUTE121_SAFARI_ZONE_ENTRANCE, 2, 5
	waitstate
	end

@ Talking to one of the roaming Pokémon.
Life_EventScript_PhotoMon::
	lock
	msgbox Life_Text_PhotoMon, MSGBOX_DEFAULT
	release
	end

Life_EventScript_Album::
	callnative LifePhoto_ShowAlbum, waitstate=1
	releaseall
	end

Life_Text_MenuAlbum:
	.string "ALBUM$"

Life_Text_PhotoIntro:
	.string "I'm a POKéMON photographer. Wild\n"
	.string "POKéMON here make the best pictures!$"

Life_Text_PhotoAsk:
	.string "For ¥500 I'll lend you a camera with\n"
	.string "12 shots for the SAFARI ZONE. Okay?$"

Life_Text_PhotoHowTo:
	.string "No balls, no battles: the POKéMON\n"
	.string "won't fight. Press R to take a shot.\p"
	.string "Get close, keep it centered, and\n"
	.string "catch it looking at you!\p"
	.string "Rare POKéMON score far more. Come\n"
	.string "back when your film runs out!$"

Life_Text_PhotoDecline:
	.string "Come back if you want to try!$"

Life_Text_PhotoNoMoney:
	.string "Sorry, you need ¥500 for the film.$"

Life_Text_PhotoStillOut:
	.string "You've still got {STR_VAR_1} shots left!\n"
	.string "The SAFARI ZONE is through there.$"

Life_Text_PhotoPayout:
	.string "Let's see your photos…\p"
	.string "A total score of {STR_VAR_1}!\n"
	.string "That's worth a prize!$"

Life_Text_PhotoAgain:
	.string "Your best shots are in your ALBUM.\n"
	.string "Come back for more film anytime!$"

Life_Text_PhotoOutOfFilm:
	.string "That was the last shot! Time to\n"
	.string "take the film back.$"

Life_Text_PhotoMon:
	.string "It doesn't seem to mind the camera.\n"
	.string "Press R to take a photo!$"
