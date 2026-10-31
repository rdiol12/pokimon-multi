@ Server-wide catch goals: everyone's catches count towards the week's
@ goals, and each goal reached gives everyone who helped a reward.

Online_EventScript_Goals::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GOAL_VIEW
	closemessage
	onl_board
	goto_if_eq VAR_0x800B, 0, Online_EventScript_End
	msgbox Online_Text_GoalClaimAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GOAL_CLAIM
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_Text_GoalClaimAsk:
	.string "Goals were reached this week!\n"
	.string "Collect your rewards?$"
