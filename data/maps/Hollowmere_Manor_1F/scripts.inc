Hollowmere_Manor_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Hollowmere_Manor_1F_OnTransition
	.byte 0

@ With the key, the butler stands aside from the stairs.
Hollowmere_Manor_1F_OnTransition:
	goto_if_unset FLAG_HOLLOW_MANOR_KEY, Hollowmere_Manor_1F_OnTransitionEnd
	setobjectxyperm LOCALID_HOLLOW_BUTLER, 17, 3
	setobjectmovementtype LOCALID_HOLLOW_BUTLER, MOVEMENT_TYPE_FACE_LEFT
Hollowmere_Manor_1F_OnTransitionEnd:
	end

Hollowmere_Manor_1F_EventScript_Butler::
	lock
	faceplayer
	goto_if_set FLAG_HOLLOW_MANOR_KEY, Hollowmere_Manor_1F_EventScript_ButlerOk
	msgbox Hollowmere_Manor_1F_Text_ButlerNo, MSGBOX_DEFAULT
	release
	end

Hollowmere_Manor_1F_EventScript_ButlerOk::
	msgbox Hollowmere_Manor_1F_Text_ButlerOk, MSGBOX_DEFAULT
	release
	end

Hollowmere_Manor_1F_EventScript_Portrait::
	msgbox Hollowmere_Manor_1F_Text_Portrait, MSGBOX_SIGN
	end

Hollowmere_Manor_1F_EventScript_Mortimer::
	trainerbattle_single TRAINER_HOLLOW_MORTIMER, Hollowmere_Manor_1F_Text_MortimerIntro, Hollowmere_Manor_1F_Text_MortimerDefeat
	msgbox Hollowmere_Manor_1F_Text_MortimerPost, MSGBOX_AUTOCLOSE
	end

Hollowmere_Manor_1F_EventScript_RareCandy::
	finditem ITEM_RARE_CANDY
	end

Hollowmere_Text_Misdreavus:
	.string "MISDREAVUS is humming a song to\n"
	.string "the portraits.$"

Hollowmere_Manor_1F_Text_ButlerNo:
	.string "The upper floor is closed to\n"
	.string "guests. Only the caretaker holds\l"
	.string "the key.$"

Hollowmere_Manor_1F_Text_ButlerOk:
	.string "Ah, you have the key. The master\n"
	.string "is expecting you. Upstairs, please.$"

Hollowmere_Manor_1F_Text_Portrait:
	.string "A dusty portrait. Its eyes seem to\n"
	.string "follow you around the room…$"

Hollowmere_Manor_1F_Text_MortimerIntro:
	.string "Welcome to the manor. I insist you\n"
	.string "stay for a battle.$"

Hollowmere_Manor_1F_Text_MortimerDefeat:
	.string "How rude of me to lose.$"

Hollowmere_Manor_1F_Text_MortimerPost:
	.string "The master upstairs loves dolls.\n"
	.string "One of them moves on its own.$"
