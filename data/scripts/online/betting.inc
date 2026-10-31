@ Spectator betting: bet Game Corner COINS on the PvP battles going on.

Online_EventScript_Betting::
	dynmultipush Online_Text_BetOn, 0
	dynmultipush Online_Text_BetCollect, 1
	dynmultipush Online_Text_BetBoard, 2
	dynmultipush Online_Text_HubClose, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Online_EventScript_BetOn
	case 1, Online_EventScript_BetCollect
	case 2, Online_EventScript_BetBoard
	goto Online_EventScript_End

Online_EventScript_BetOn:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_BET_LIST
	goto_if_eq VAR_0x800B, 0, Online_EventScript_ShowMessage
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_BET_SHOW
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	showcoinsbox 20, 0
	message Online_Text_BetHowMuch
	waitmessage
	dynmultipush Online_Text_Bet10, 10
	dynmultipush Online_Text_Bet50, 50
	dynmultipush Online_Text_Bet100, 100
	dynmultipush Online_Text_Bet500, 500
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Online_EventScript_BetEnd
	copyvar VAR_0x8005, VAR_RESULT
	call Online_EventScript_Connecting
	onl_request ONL_REQ_BET_PLACE
	goto_if_ne VAR_RESULT, ONLINE_OK, Online_EventScript_BetFailed
	playse SE_SHOP
	message gStringVar4
	waitmessage
	updatecoinsbox @ after the message: it prints the coins into gStringVar4
	waitbuttonpress
Online_EventScript_BetEnd:
	hidecoinsbox
	goto Online_EventScript_End

Online_EventScript_BetFailed:
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Online_EventScript_BetEnd

Online_EventScript_BetCollect:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_BET_COLLECT
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	showcoinsbox 20, 0 @ as above
	waitbuttonpress
	goto Online_EventScript_BetEnd

Online_EventScript_BetBoard:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_BET_BOARD
	closemessage
	onl_board
	goto Online_EventScript_End

Online_Text_BetOn:
	.string "BET ON A BATTLE$"

Online_Text_BetCollect:
	.string "COLLECT WINNINGS$"

Online_Text_BetBoard:
	.string "BEST BETTORS$"

Online_Text_BetHowMuch:
	.string "How many COINS will you bet?$"

Online_Text_Bet10:
	.string "10 COINS$"

Online_Text_Bet50:
	.string "50 COINS$"

Online_Text_Bet100:
	.string "100 COINS$"

Online_Text_Bet500:
	.string "500 COINS$"
