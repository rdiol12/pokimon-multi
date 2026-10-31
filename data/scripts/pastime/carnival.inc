@ Carnival games (src/pastime_carnival.c), at the Sunfair Carnival. The
@ hosts' map scripts (data/scripts/areas/carnival_games.inc) come here.

Carnival_EventScript_Prize:
	goto_if_eq VAR_0x8005, ITEM_NONE, Carnival_EventScript_NoPrize
	msgbox Carnival_Text_Prize, MSGBOX_DEFAULT
	giveitem VAR_0x8005
Carnival_EventScript_NoPrize:
	return

Carnival_EventScript_Bye:
	msgbox Carnival_Text_Bye, MSGBOX_DEFAULT
	release
	end

@ Bowling ----------------------------------------------------------------

Pastime_EventScript_Bowling::
	lock
	faceplayer
	msgbox Carnival_Text_BowlingIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Carnival_EventScript_Bye
	callnative Bowl_Start
Carnival_EventScript_BowlingRoll:
	callnative Bowl_Prepare
	goto_if_eq VAR_RESULT, FALSE, Carnival_EventScript_BowlingDone
	message gPastimeText
	waitmessage
	callnative Bowl_Roll, waitstate=1
	msgbox gPastimeText, MSGBOX_DEFAULT
	goto Carnival_EventScript_BowlingRoll

Carnival_EventScript_BowlingDone:
	callnative Bowl_Finish
	msgbox Carnival_Text_BowlingScore, MSGBOX_DEFAULT
	callnative Carnival_CloseCourt
	goto_if_eq VAR_RESULT, FALSE, Carnival_EventScript_BowlingPrize
	playfanfare MUS_LEVEL_UP
	message Carnival_Text_NewBest
	waitmessage
	waitfanfare
	waitbuttonpress
Carnival_EventScript_BowlingPrize:
	call Carnival_EventScript_Prize
	release
	end

@ Mini golf --------------------------------------------------------------

Pastime_EventScript_Golf::
	lock
	faceplayer
	msgbox Carnival_Text_GolfIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Carnival_EventScript_Bye
	callnative Golf_Start
Carnival_EventScript_GolfStroke:
	callnative Golf_Prepare
	goto_if_eq VAR_RESULT, FALSE, Carnival_EventScript_GolfDone
	message gPastimeText
	waitmessage
	callnative Golf_Swing, waitstate=1
	msgbox gPastimeText, MSGBOX_DEFAULT
	goto Carnival_EventScript_GolfStroke

Carnival_EventScript_GolfDone:
	callnative Golf_Finish
	msgbox Carnival_Text_GolfScore, MSGBOX_DEFAULT
	callnative Carnival_CloseCourt
	goto_if_eq VAR_RESULT, FALSE, Carnival_EventScript_GolfPrize
	playfanfare MUS_LEVEL_UP
	message Carnival_Text_NewBest
	waitmessage
	waitfanfare
	waitbuttonpress
Carnival_EventScript_GolfPrize:
	call Carnival_EventScript_Prize
	release
	end

@ Soccer penalty shootout ------------------------------------------------

Pastime_EventScript_Soccer::
	lock
	faceplayer
	msgbox Carnival_Text_SoccerIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Carnival_EventScript_Bye
	callnative Soccer_Start
Carnival_EventScript_SoccerKick:
	callnative Soccer_Prepare
	goto_if_eq VAR_RESULT, SOCCER_OVER, Carnival_EventScript_SoccerDone
	copyvar VAR_0x8006, VAR_RESULT
	message gPastimeText
	waitmessage
	dynmultipush Carnival_Text_Left, 0
	dynmultipush Carnival_Text_Middle, 1
	dynmultipush Carnival_Text_Right, 2
	dynmultistack 0, 0, TRUE, 3, FALSE, 1, DYN_MULTICHOICE_CB_NONE
	copyvar VAR_0x8005, VAR_RESULT
	goto_if_eq VAR_0x8006, SOCCER_KEEP, Carnival_EventScript_SoccerKeep
	callnative Soccer_Shoot, waitstate=1
	msgbox gPastimeText, MSGBOX_DEFAULT
	goto Carnival_EventScript_SoccerKick

Carnival_EventScript_SoccerKeep:
	callnative Soccer_Keep
	msgbox gPastimeText, MSGBOX_DEFAULT
	goto Carnival_EventScript_SoccerKick

Carnival_EventScript_SoccerDone:
	callnative Soccer_Finish
	callnative Carnival_CloseCourt
	goto_if_eq VAR_RESULT, SOCCER_WON, Carnival_EventScript_SoccerWon
	goto_if_eq VAR_RESULT, SOCCER_DRAW, Carnival_EventScript_SoccerDraw
	msgbox Carnival_Text_SoccerLost, MSGBOX_DEFAULT
	release
	end

Carnival_EventScript_SoccerDraw:
	msgbox Carnival_Text_SoccerDraw, MSGBOX_DEFAULT
	call Carnival_EventScript_Prize
	release
	end

Carnival_EventScript_SoccerWon:
	playfanfare MUS_LEVEL_UP
	message Carnival_Text_SoccerWon
	waitmessage
	waitfanfare
	waitbuttonpress
	call Carnival_EventScript_Prize
	release
	end

@ Ferris wheel -----------------------------------------------------------

Pastime_EventScript_FerrisWheel::
	lock
	faceplayer
	msgbox Carnival_Text_FerrisIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Carnival_EventScript_Bye
	checkmoney FERRIS_WHEEL_FARE
	goto_if_eq VAR_RESULT, FALSE, Carnival_EventScript_FerrisNoMoney
	removemoney FERRIS_WHEEL_FARE
	playse SE_SHOP
	callnative Ferris_Ride
	msgbox Carnival_Text_FerrisBoard, MSGBOX_DEFAULT
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, Carnival_Movement_Board
	waitmovement 0
	special SpawnCameraObject
	applymovement OBJ_EVENT_ID_CAMERA, Carnival_Movement_RideUp
	waitmovement 0
	msgbox Carnival_Text_FerrisTop, MSGBOX_DEFAULT
	msgbox gPastimeText, MSGBOX_DEFAULT
	closemessage
	applymovement OBJ_EVENT_ID_CAMERA, Carnival_Movement_RideDown
	waitmovement 0
	special RemoveCameraObject
	applymovement OBJ_EVENT_ID_PLAYER, Carnival_Movement_GetOff
	waitmovement 0
	msgbox Carnival_Text_FerrisBye, MSGBOX_DEFAULT
	release
	end

Carnival_EventScript_FerrisNoMoney:
	msgbox Carnival_Text_FerrisNoMoney, MSGBOX_DEFAULT
	release
	end

Carnival_Movement_Board:
	face_up
	set_invisible
	step_end

Carnival_Movement_GetOff:
	set_visible
	face_down
	step_end

Carnival_Movement_RideUp:
	walk_slow_up
	walk_slow_up
	walk_slow_up
	walk_slow_up
	walk_slow_up
	walk_slow_up
	walk_slow_up
	walk_slow_up
	step_end

Carnival_Movement_RideDown:
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	step_end

@ Text -------------------------------------------------------------------

Carnival_Text_Bye:
	.string "Come back any time!$"

Carnival_Text_Prize:
	.string "Nice playing! Here's a prize!$"

Carnival_Text_NewBest:
	.string "That's your new best!$"

Carnival_Text_BowlingIntro:
	.string "Welcome to SUNFAIR LANES! Five\n"
	.string "frames, two balls a frame.\p"
	.string "Stop the needle right in the middle\n"
	.string "of the green for a STRIKE!\p"
	.string "Want to bowl a game?$"

Carnival_Text_BowlingScore:
	.string "That's the game! You scored\n"
	.string "{STR_VAR_1}. Your best is {STR_VAR_2}.$"

Carnival_Text_GolfIntro:
	.string "Three holes of mini golf, par 3\n"
	.string "each. Mind the pond on the last!\p"
	.string "The closer to the middle of the\n"
	.string "green you stop, the better the shot.\p"
	.string "Want to play a round?$"

Carnival_Text_GolfScore:
	.string "All done! {STR_VAR_1} strokes (par 9).\n"
	.string "Your best round is {STR_VAR_2}.$"

Carnival_Text_SoccerIntro:
	.string "A POKéMON soccer shootout! Five\n"
	.string "kicks each.\p"
	.string "Pick a side to shoot, then stop the\n"
	.string "needle. A perfect shot beats any\l"
	.string "keeper! Then you're in goal.\p"
	.string "Ready to play?$"

Carnival_Text_Left:
	.string "LEFT$"

Carnival_Text_Middle:
	.string "MIDDLE$"

Carnival_Text_Right:
	.string "RIGHT$"

Carnival_Text_SoccerWon:
	.string "{STR_VAR_1} - {STR_VAR_2}! You win the\n"
	.string "shootout!$"

Carnival_Text_SoccerDraw:
	.string "{STR_VAR_1} - {STR_VAR_2}! A draw! Good\n"
	.string "game!$"

Carnival_Text_SoccerLost:
	.string "{STR_VAR_1} - {STR_VAR_2}… They win this\n"
	.string "time. Try again!$"

Carnival_Text_FerrisIntro:
	.string "Ride the SUNFAIR WHEEL! You can\n"
	.string "see the whole carnival from the top.\p"
	.string "It's ¥100 a ride. Hop on?$"

Carnival_Text_FerrisBoard:
	.string "Step right in! Hold on tight!$"

Carnival_Text_FerrisTop:
	.string "Up, up… to the very top!$"

Carnival_Text_FerrisBye:
	.string "Hope you enjoyed the view!$"

Carnival_Text_FerrisNoMoney:
	.string "Sorry, it's ¥100 a ride.$"
