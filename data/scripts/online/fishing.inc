@ Fishing tournaments: the live standings or the last results, and prizes.

Online_EventScript_Fishing::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_FISHT_BOARD
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto_if_eq VAR_0x800B, 0, Online_EventScript_End
	msgbox Online_Text_FishingPrize, MSGBOX_DEFAULT
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_FISHT_CLAIM
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_Text_FishingPrize:
	.string "You won a prize in a tournament!\n"
	.string "Here you go!$"
