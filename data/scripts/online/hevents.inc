@ Player-hosted events (server/online_hevents.py): a catch hunt or a
@ tournament with a prize the host puts up, held by the server until the
@ event ends. Prizes and refunds are paid when the menu opens.

Online_EventScript_HostedEvents::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_VIEW
	goto_if_ne VAR_0x800B, 1, Online_EventScript_HostedEventsMenu
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
Online_EventScript_HostedEventsMenu:
	message Online_Text_HEventWhat
	waitmessage
	dynmultipush Online_Text_HEventBoard, 0
	dynmultipush Online_Text_HEventJoin, 1
	dynmultipush Online_Text_HEventHost, 2
	dynmultipush Online_Text_HEventMine, 3
	dynmultipush Online_Text_HubClose, 4
	dynmultistack 0, 0, FALSE, 5, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Online_EventScript_HEventBoard
	case 1, Online_EventScript_HEventJoin
	case 2, Online_EventScript_HEventHost
	case 3, Online_EventScript_HEventMine
	goto Online_EventScript_End

Online_EventScript_HEventBoard:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_BOARD
	closemessage
	onl_board
	goto Online_EventScript_End

Online_EventScript_HEventJoin:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_LIST
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_SHOW
	msgbox gStringVar4, MSGBOX_DEFAULT
	switch VAR_0x800B
	case HEVENT_BRACKET, Online_EventScript_HEventBracket
	case HEVENT_OWN, Online_EventScript_End
	case HEVENT_JOINED, Online_EventScript_HEventLeave
	msgbox Online_Text_HEventJoinIt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_JOIN
	playse SE_SUCCESS
	goto Online_EventScript_ShowMessage

Online_EventScript_HEventBracket:
	closemessage
	onl_board
	goto Online_EventScript_End

Online_EventScript_HEventLeave:
	msgbox Online_Text_HEventLeaveIt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_LEAVE
	goto Online_EventScript_ShowMessage

@ Hosting: the kind, what to hunt or how many players, how long, then the
@ prize (money, an item from the bag, or both), then a last look.
Online_EventScript_HEventHost:
	message Online_Text_HEventKind
	waitmessage
	dynmultipush Online_Text_HEventHunt, HEVENT_KIND_HUNT
	dynmultipush Online_Text_HEventCup, HEVENT_KIND_CUP
	dynmultipush gText_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_gt VAR_RESULT, HEVENT_KIND_CUP, Online_EventScript_End
	copyvar VAR_0x8005, VAR_RESULT
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_NEW
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	goto_if_eq VAR_0x8005, HEVENT_KIND_CUP, Online_EventScript_HEventSize
	onl_request_or_fail ONL_REQ_HEVENT_TARGET
	goto Online_EventScript_HEventTime

Online_EventScript_HEventSize:
	onl_request_or_fail ONL_REQ_HEVENT_SIZE
Online_EventScript_HEventTime:
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_TIME
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_MONEY
	msgbox Online_Text_HEventAddItem, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_HEventCheck
	message Online_Text_HEventWhichItem
	waitmessage
	callnative Online_ChooseBagItem, waitstate=1
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_HEventCheck
	message Online_Text_PQuestHowMany
	waitmessage
	call Online_EventScript_ShopCount
	goto_if_eq VAR_RESULT, 3, Online_EventScript_HEventCheck
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_ITEM
Online_EventScript_HEventCheck:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_CHECK
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Online_Text_HEventPostIt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_POST
	playse SE_SHOP
	goto Online_EventScript_ShowMessage

Online_EventScript_HEventMine:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_MINE
	goto_if_ne VAR_0x800B, 1, Online_EventScript_ShowMessage
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Online_Text_HEventCallOff, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HEVENT_CANCEL
	playse SE_SHOP
	goto Online_EventScript_ShowMessage

Online_Text_HEventWhat:
	.string "Players host hunts and tournaments\n"
	.string "here, with prizes they put up.$"

Online_Text_HEventBoard:
	.string "EVENTS BOARD$"

Online_Text_HEventJoin:
	.string "JOIN AN EVENT$"

Online_Text_HEventHost:
	.string "HOST AN EVENT$"

Online_Text_HEventMine:
	.string "MY EVENTS$"

Online_Text_HEventKind:
	.string "What kind of event will you host?$"

Online_Text_HEventHunt:
	.string "CATCH HUNT$"

Online_Text_HEventCup:
	.string "TOURNAMENT$"

Online_Text_HEventJoinIt:
	.string "Join this event?$"

Online_Text_HEventLeaveIt:
	.string "You're in this one. Leave it?$"

Online_Text_HEventAddItem:
	.string "Add an item from your BAG\n"
	.string "to the prize?$"

Online_Text_HEventWhichItem:
	.string "Which item?$"

Online_Text_HEventPostIt:
	.string "Post it on the events board?$"

Online_Text_HEventCallOff:
	.string "Nobody has joined yet. Call it off\n"
	.string "and take the prize back?$"
