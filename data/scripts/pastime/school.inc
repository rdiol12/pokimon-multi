@ Trainer school (src/pastime_school.c), at the Rustboro Trainer's School.

School_EventScript_Teacher::
	lock
	faceplayer
	msgbox School_Text_Welcome, MSGBOX_DEFAULT
School_EventScript_Menu:
	message School_Text_WhichLesson
	waitmessage
	callnative School_PushLessonMenu
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, SCHOOL_MENU_CANCEL, School_EventScript_Bye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, School_EventScript_Bye
	copyvar VAR_0x8005, VAR_RESULT
	callnative School_StartLesson
	msgbox School_Text_Start, MSGBOX_DEFAULT
School_EventScript_Question:
	callnative School_NextQuestion
	goto_if_eq VAR_RESULT, FALSE, School_EventScript_Finish
	message gPastimeText
	waitmessage
	callnative School_PushAnswers
	dynmultistack 16, 0, TRUE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	callnative School_Answer
	goto_if_eq VAR_RESULT, FALSE, School_EventScript_Wrong
	playse SE_SUCCESS
	msgbox School_Text_Right, MSGBOX_DEFAULT
	goto School_EventScript_Question

School_EventScript_Wrong:
	playse SE_FAILURE
	msgbox School_Text_Wrong, MSGBOX_DEFAULT
	goto School_EventScript_Question

School_EventScript_Finish:
	callnative School_Finish
	goto_if_eq VAR_RESULT, SCHOOL_FAILED, School_EventScript_Failed
	goto_if_eq VAR_RESULT, SCHOOL_PASSED, School_EventScript_PassedAgain
	playfanfare MUS_LEVEL_UP
	message School_Text_Passed
	waitmessage
	waitfanfare
	waitbuttonpress
	giveitem VAR_0x8005
	goto_if_eq VAR_0x8006, FALSE, School_EventScript_Menu
	playfanfare MUS_OBTAIN_BADGE
	message School_Text_Graduated
	waitmessage
	waitfanfare
	waitbuttonpress
	giveitem ITEM_AMULET_COIN
	release
	end

School_EventScript_PassedAgain:
	msgbox School_Text_PassedAgain, MSGBOX_DEFAULT
	goto School_EventScript_Menu

School_EventScript_Failed:
	msgbox School_Text_Failed, MSGBOX_DEFAULT
	goto School_EventScript_Menu

School_EventScript_Bye:
	msgbox School_Text_Bye, MSGBOX_DEFAULT
	release
	end

School_Text_Welcome:
	.string "Welcome to my class! I teach what\n"
	.string "every Trainer should know.\p"
	.string "Pass a test with 4 right answers\n"
	.string "out of 5. Pass all five to graduate!$"

School_Text_WhichLesson:
	.string "Which lesson today?$"

School_Text_Start:
	.string "The {STR_VAR_1} test. Pencils ready!$"

School_Text_Right:
	.string "Correct! Well done.$"

School_Text_Wrong:
	.string "Not quite. The answer is\n"
	.string "{STR_VAR_2}.$"

School_Text_Passed:
	.string "{STR_VAR_1} out of 5! You passed!\n"
	.string "Here's a little prize.$"

School_Text_PassedAgain:
	.string "{STR_VAR_1} out of 5! You know this\n"
	.string "one by heart already.$"

School_Text_Failed:
	.string "{STR_VAR_1} out of 5. You need 4 to\n"
	.string "pass. Study up and try again!$"

School_Text_Graduated:
	.string "That's every lesson! {PLAYER}\n"
	.string "graduates from the TRAINER'S SCHOOL!$"

School_Text_Bye:
	.string "Class dismissed!$"
