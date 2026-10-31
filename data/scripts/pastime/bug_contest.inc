@ Bug-catching contest (src/pastime_bug_contest.c), by Route 119's tall grass.

BugContest_EventScript_Officer::
	lock
	faceplayer
	callnative BugContest_GetState
	goto_if_eq VAR_RESULT, BUG_CONTEST_RUNNING, BugContest_EventScript_Running
	goto_if_eq VAR_RESULT, BUG_CONTEST_DONE_TODAY, BugContest_EventScript_Tomorrow
	msgbox BugContest_Text_Welcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, BugContest_EventScript_Bye
	callnative BugContest_Start
	playfanfare MUS_OBTAIN_ITEM
	message BugContest_Text_GotBalls
	waitmessage
	waitfanfare
	msgbox BugContest_Text_Go, MSGBOX_DEFAULT
	release
	end

BugContest_EventScript_Running:
	goto_if_eq VAR_0x8005, TRUE, BugContest_EventScript_Judge
	msgbox BugContest_Text_EndNow, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, BugContest_EventScript_KeepGoing
BugContest_EventScript_Judge:
	msgbox BugContest_Text_Judging, MSGBOX_DEFAULT
	callnative BugContest_Judge
	msgbox gPastimeText, MSGBOX_DEFAULT
	switch VAR_RESULT
	case 1, BugContest_EventScript_First
	case 2, BugContest_EventScript_Second
	case 3, BugContest_EventScript_Third
	msgbox BugContest_Text_Fourth, MSGBOX_DEFAULT
	goto BugContest_EventScript_Prize

BugContest_EventScript_First:
	playfanfare MUS_LEVEL_UP
	message BugContest_Text_First
	waitmessage
	waitfanfare
	goto BugContest_EventScript_Prize

BugContest_EventScript_Second:
	msgbox BugContest_Text_Second, MSGBOX_DEFAULT
	goto BugContest_EventScript_Prize

BugContest_EventScript_Third:
	msgbox BugContest_Text_Third, MSGBOX_DEFAULT
BugContest_EventScript_Prize:
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox BugContest_Text_Thanks, MSGBOX_DEFAULT
	release
	end

BugContest_EventScript_KeepGoing:
	msgbox BugContest_Text_KeepGoing, MSGBOX_DEFAULT
	release
	end

BugContest_EventScript_Tomorrow:
	msgbox BugContest_Text_Tomorrow, MSGBOX_DEFAULT
	release
	end

BugContest_EventScript_Bye:
	msgbox BugContest_Text_Bye, MSGBOX_DEFAULT
	release
	end

BugContest_Text_Welcome:
	.string "This is the ROUTE 119\n"
	.string "BUG-CATCHING CONTEST!\p"
	.string "You get 10 minutes and 20 SPORT\n"
	.string "BALLS. Catch the best Bug you can!\p"
	.string "Rarer, stronger Bugs score more.\n"
	.string "Want to enter?$"

BugContest_Text_GotBalls:
	.string "{PLAYER} received 20 SPORT BALLS!$"

BugContest_Text_Go:
	.string "Only Bug Pokémon live in the grass\n"
	.string "while the contest is on. Good luck!$"

BugContest_Text_EndNow:
	.string "Still time on the clock! Do you\n"
	.string "want to end the contest now?$"

BugContest_Text_KeepGoing:
	.string "Go get 'em!$"

BugContest_Text_Judging:
	.string "Time to judge! Let's see\n"
	.string "what everyone caught…$"

BugContest_Text_First:
	.string "{PLAYER} wins the BUG-CATCHING\n"
	.string "CONTEST! Here's the 1st prize!$"

BugContest_Text_Second:
	.string "{PLAYER} came 2nd! Not bad at all!$"

BugContest_Text_Third:
	.string "{PLAYER} came 3rd. Here's a prize!$"

BugContest_Text_Fourth:
	.string "Better luck next time!\n"
	.string "Here's a little something.$"

BugContest_Text_Thanks:
	.string "Your score is on the BUG CONTEST\n"
	.string "board online. See you tomorrow!$"

BugContest_Text_Tomorrow:
	.string "Today's contest is over.\n"
	.string "Come back tomorrow!$"

BugContest_Text_Bye:
	.string "Come back if you change your mind!$"
