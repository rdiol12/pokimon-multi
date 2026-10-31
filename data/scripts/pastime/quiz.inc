@ Battle quiz hall (src/pastime_quiz.c), in Mauville.

Quiz_EventScript_Quizmaster::
	lock
	faceplayer
	msgbox Quiz_Text_Hello, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Quiz_EventScript_Declined
	msgbox Quiz_Text_Rules, MSGBOX_DEFAULT
	callnative Quiz_Start
Quiz_EventScript_Question:
	callnative Quiz_NextQuestion
	goto_if_eq VAR_RESULT, FALSE, Quiz_EventScript_Finish
	setvar VAR_0x8006, 3
	setvar VAR_0x8007, 2
	callnative Pastime_ShowMonPic
	message gPastimeText
	waitmessage
	dynmultipush Quiz_Text_Super, QUIZ_SUPER
	dynmultipush Quiz_Text_Normal, QUIZ_NORMAL
	dynmultipush Quiz_Text_NotVery, QUIZ_NOT_VERY
	dynmultipush Quiz_Text_NoEffect, QUIZ_NO_EFFECT
	dynmultistack 19, 1, FALSE, 4, TRUE, QUIZ_SUPER, DYN_MULTICHOICE_CB_NONE
	callnative Pastime_HideMonPic
	callnative Quiz_Answer
	goto_if_eq VAR_RESULT, FALSE, Quiz_EventScript_Wrong
	playse SE_SUCCESS
	goto Quiz_EventScript_Reply

Quiz_EventScript_Wrong:
	playse SE_FAILURE
Quiz_EventScript_Reply:
	msgbox gPastimeText, MSGBOX_DEFAULT
	goto Quiz_EventScript_Question

Quiz_EventScript_Finish:
	callnative Quiz_Finish
	playfanfare MUS_LEVEL_UP
	msgbox gPastimeText, MSGBOX_DEFAULT
	waitfanfare
	goto_if_eq VAR_RESULT, FALSE, Quiz_EventScript_NoPrize
	msgbox Quiz_Text_Prize, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
Quiz_EventScript_NoPrize:
	msgbox Quiz_Text_Bye, MSGBOX_DEFAULT
	release
	end

Quiz_EventScript_Declined:
	msgbox Quiz_Text_Declined, MSGBOX_DEFAULT
	release
	end

Quiz_Text_Hello:
	.string "Welcome to the BATTLE QUIZ HALL!\n"
	.string "How well do you know your types?\p"
	.string "Ten questions a round. Ready?$"

Quiz_Text_Rules:
	.string "I name a move's type and the Pokémon\n"
	.string "it hits. You say how well it works!\p"
	.string "Streaks score extra, and today's\n"
	.string "first good round wins a prize!$"

Quiz_Text_Super:
	.string "SUPER EFFECTIVE$"
Quiz_Text_Normal:
	.string "NORMAL$"
Quiz_Text_NotVery:
	.string "NOT VERY$"
Quiz_Text_NoEffect:
	.string "NO EFFECT$"

Quiz_Text_Prize:
	.string "Today's prize for that score!$"

Quiz_Text_Bye:
	.string "Your best score goes up on the\n"
	.string "QUIZ CHAMPS board online. Come back\l"
	.string "and play again!$"

Quiz_Text_Declined:
	.string "Brush up on your types and come back!$"
