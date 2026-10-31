@ Player shops: visit other players' shops, or stock and run your own.

Online_EventScript_Shops::
	dynmultipush Online_Text_ShopVisit, 0
	dynmultipush Online_Text_ShopMine, 1
	dynmultipush Online_Text_HubClose, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Online_EventScript_MyShop
	goto_if_ne VAR_RESULT, 0, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_LIST
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_OPEN
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
Online_EventScript_ShopBuy:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_ITEMS
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	callnative Online_TakeChosenShelf
	buffernumberstring STR_VAR_2, VAR_0x8007
	message Online_Text_ShopHowMany
	waitmessage
	call Online_EventScript_ShopCount
	goto_if_eq VAR_RESULT, 3, Online_EventScript_ShopBuy
	checkitemspace VAR_0x8005, VAR_0x8006
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_ShopNoRoom
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_BUY
	playse SE_SHOP
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Online_EventScript_ShopBuy

@ VAR_RESULT: 0 one, 1 five, 2 all, 3 cancel; VAR_0x8006 becomes how many.
Online_EventScript_ShopCount:
	dynmultipush Online_Text_ShopOne, 0
	dynmultipush Online_Text_ShopFive, 1
	dynmultipush Online_Text_ShopAll, 2
	dynmultipush gText_Cancel, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_gt VAR_RESULT, 2, Online_EventScript_ShopCountCancel
	callnative Online_SetShopCount
	return

Online_EventScript_ShopCountCancel:
	setvar VAR_RESULT, 3
	return

Online_EventScript_ShopNoRoom:
	msgbox Online_Text_ShopNoRoom, MSGBOX_DEFAULT
	goto Online_EventScript_End

Online_EventScript_MyShop:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_MINE
	message gStringVar4
	waitmessage
	dynmultipush Online_Text_ShopStock, 0
	dynmultipush Online_Text_ShopTake, 1
	dynmultipush Online_Text_ShopCollect, 2
	dynmultipush Online_Text_HubClose, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Online_EventScript_ShopStock
	case 1, Online_EventScript_ShopTake
	case 2, Online_EventScript_ShopCollect
	goto Online_EventScript_End

Online_EventScript_ShopStock:
	message Online_Text_ShopWhich
	waitmessage
	callnative Online_ChooseBagItem, waitstate=1
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	message Online_Text_ShopHowManySell
	waitmessage
	call Online_EventScript_ShopCount
	goto_if_eq VAR_RESULT, 3, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_PRICES
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_STOCK
	playse SE_SHOP
	goto Online_EventScript_ShowMessage

Online_EventScript_ShopTake:
	goto_if_eq VAR_0x800A, 0, Online_EventScript_ShopEmpty
	message Online_Text_ShopTakeWhich
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	callnative Online_TakeChosenShelf
	checkitemspace VAR_0x8005, VAR_0x8006
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_ShopNoRoom
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_TAKE
	goto Online_EventScript_ShowMessage

Online_EventScript_ShopEmpty:
	msgbox Online_Text_ShopEmpty, MSGBOX_DEFAULT
	goto Online_EventScript_End

Online_EventScript_ShopCollect:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SHOP_COLLECT
	playse SE_SHOP
	goto Online_EventScript_ShowMessage

Online_Text_ShopVisit:
	.string "VISIT A SHOP$"

Online_Text_ShopMine:
	.string "MY SHOP$"

Online_Text_ShopStock:
	.string "STOCK AN ITEM$"

Online_Text_ShopTake:
	.string "TAKE BACK$"

Online_Text_ShopCollect:
	.string "COLLECT MONEY$"

Online_Text_ShopOne:
	.string "1$"

Online_Text_ShopFive:
	.string "5$"

Online_Text_ShopAll:
	.string "ALL$"

Online_Text_ShopHowMany:
	.string "{STR_VAR_1}, ¥{STR_VAR_2} each.\n"
	.string "How many would you like?$"

Online_Text_ShopHowManySell:
	.string "How many {STR_VAR_1} will you put\n"
	.string "on the shelf?$"

Online_Text_ShopWhich:
	.string "Which item will you sell?$"

Online_Text_ShopTakeWhich:
	.string "Which shelf will you empty?$"

Online_Text_ShopEmpty:
	.string "Your shelves are empty.$"

Online_Text_ShopNoRoom:
	.string "There's no room in your BAG.$"
