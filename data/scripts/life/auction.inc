@ Auction house: START > ONLINE > AUCTION, or the auctioneer in the
@ Lilycove Department Store. See src/life_auction.c.

.macro life_auction req:req
	setvar VAR_0x8004, \req
	callnative LifeAuction_Request, waitstate=1
.endm

Life_EventScript_Auctioneer::
	lock
	faceplayer
	msgbox Life_Text_AuctionHello, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect
	end

Life_EventScript_Auction::
	lockall
Life_EventScript_AuctionCollect:
	message Life_Text_AuctionConnecting
	waitmessage
	life_auction LIFE_AUC_REQ_COLLECT
	goto_if_ne VAR_RESULT, NETPLAY_EXT_OK, Life_EventScript_AuctionFailed
	switch VAR_0x8005
	case LIFE_AUC_GOT_WON, Life_EventScript_AuctionWon
	case LIFE_AUC_GOT_SOLD, Life_EventScript_AuctionSold
	case LIFE_AUC_GOT_OUTBID, Life_EventScript_AuctionOutbid
	case LIFE_AUC_GOT_UNSOLD, Life_EventScript_AuctionUnsold
	case LIFE_AUC_GOT_CANCELED, Life_EventScript_AuctionReturned
Life_EventScript_AuctionMenu:
	message Life_Text_AuctionWhat
	waitmessage
	dynmultipush Life_Text_AuctionBrowse, 0
	dynmultipush Life_Text_AuctionSell, 1
	dynmultipush Life_Text_AuctionLeave, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Life_EventScript_AuctionBrowse
	case 1, Life_EventScript_AuctionSell
	goto Life_EventScript_AuctionEnd

Life_EventScript_AuctionWon:
	playfanfare MUS_OBTAIN_ITEM
	message Life_Text_AuctionWon
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Life_EventScript_AuctionCollect

Life_EventScript_AuctionSold:
	playse SE_SHOP
	msgbox Life_Text_AuctionSold, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect

Life_EventScript_AuctionOutbid:
	msgbox Life_Text_AuctionOutbid, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect

Life_EventScript_AuctionUnsold:
	msgbox Life_Text_AuctionUnsold, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect

Life_EventScript_AuctionReturned:
	msgbox Life_Text_AuctionReturned, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect

Life_EventScript_AuctionBrowse:
	message Life_Text_AuctionConnecting
	waitmessage
	life_auction LIFE_AUC_REQ_LIST
	goto_if_ne VAR_RESULT, NETPLAY_EXT_OK, Life_EventScript_AuctionFailed
	closemessage
	callnative LifeAuction_Browse, waitstate=1
	switch VAR_RESULT
	case LIFE_AUC_PICK_BID, Life_EventScript_AuctionBid
	case LIFE_AUC_PICK_CANCEL, Life_EventScript_AuctionTakeBack
	goto Life_EventScript_AuctionMenu

Life_EventScript_AuctionBid:
	callnative LifeAuction_BufferPicked
	msgbox Life_Text_AuctionBidConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Life_EventScript_AuctionBrowse
	life_auction LIFE_AUC_REQ_BID
	goto_if_ne VAR_RESULT, NETPLAY_EXT_OK, Life_EventScript_AuctionBidFailed
	playse SE_SHOP
	msgbox Life_Text_AuctionBidDone, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionBrowse

Life_EventScript_AuctionBidFailed:
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionBrowse

Life_EventScript_AuctionTakeBack:
	callnative LifeAuction_BufferPicked
	msgbox Life_Text_AuctionTakeBackConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Life_EventScript_AuctionBrowse
	life_auction LIFE_AUC_REQ_CANCEL
	goto_if_ne VAR_RESULT, NETPLAY_EXT_OK, Life_EventScript_AuctionBidFailed
	@ The Pokémon comes back through COLLECT.
	goto Life_EventScript_AuctionCollect

Life_EventScript_AuctionSell:
	msgbox Life_Text_AuctionSellWhich, MSGBOX_DEFAULT
	special ChoosePartyMon
	copyvar VAR_0x8005, VAR_0x8004
	special NetplayExt_CheckOfferedMon
	goto_if_eq VAR_RESULT, 1, Life_EventScript_AuctionMenu
	goto_if_eq VAR_RESULT, 2, Life_EventScript_AuctionNoEggs
	goto_if_eq VAR_RESULT, 3, Life_EventScript_AuctionLastMon
	message Life_Text_AuctionSellPrice
	waitmessage
	callnative LifeAuction_PickPrice, waitstate=1
	goto_if_eq VAR_RESULT, FALSE, Life_EventScript_AuctionMenu
	message Life_Text_AuctionSellHours
	waitmessage
	dynmultipush Life_Text_Auction1Hour, 1
	dynmultipush Life_Text_Auction8Hours, 8
	dynmultipush Life_Text_Auction24Hours, 24
	dynmultistack 0, 0, FALSE, 3, FALSE, 1, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Life_EventScript_AuctionMenu
	copyvar VAR_0x8006, VAR_RESULT
	buffernumberstring STR_VAR_3, VAR_0x8006
	msgbox Life_Text_AuctionSellConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Life_EventScript_AuctionMenu
	life_auction LIFE_AUC_REQ_POST
	goto_if_ne VAR_RESULT, NETPLAY_EXT_OK, Life_EventScript_AuctionSellFailed
	playse SE_M_SKY_UPPERCUT
	msgbox Life_Text_AuctionListed, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionMenu

Life_EventScript_AuctionSellFailed:
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionMenu

Life_EventScript_AuctionNoEggs:
	msgbox Life_Text_AuctionNoEggs, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionMenu

Life_EventScript_AuctionLastMon:
	msgbox Life_Text_AuctionLastMon, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionMenu

Life_EventScript_AuctionFailed:
	msgbox gStringVar4, MSGBOX_DEFAULT
Life_EventScript_AuctionEnd:
	closemessage
	releaseall
	end

Life_Text_AuctionHello:
	.string "Welcome to the AUCTION HOUSE! Trainers\n"
	.string "all over the world bid on POKéMON here.$"

Life_Text_AuctionConnecting:
	.string "Checking the AUCTION HOUSE…$"

Life_Text_AuctionWhat:
	.string "AUCTION HOUSE: what would you like?$"

Life_Text_AuctionBrowse:
	.string "BROWSE$"

Life_Text_AuctionSell:
	.string "SELL$"

Life_Text_AuctionLeave:
	.string "LEAVE$"

Life_Text_AuctionWon:
	.string "You won the auction! {STR_VAR_1}'s\n"
	.string "{STR_VAR_2} is yours for ¥{STR_VAR_3}!$"

Life_Text_AuctionSold:
	.string "{STR_VAR_1} won your {STR_VAR_2}!\n"
	.string "After the house's 5% you get ¥{STR_VAR_3}.$"

Life_Text_AuctionOutbid:
	.string "{STR_VAR_1} outbid you on {STR_VAR_2}.\n"
	.string "Your ¥{STR_VAR_3} bid is returned.$"

Life_Text_AuctionUnsold:
	.string "Nobody bid on your {STR_VAR_2}, so\n"
	.string "it's back with you.$"

Life_Text_AuctionReturned:
	.string "Your {STR_VAR_2} is back with you.$"

Life_Text_AuctionBidConfirm:
	.string "Bid ¥{STR_VAR_2} on {STR_VAR_1}?\n"
	.string "The money is held until you're outbid.$"

Life_Text_AuctionBidDone:
	.string "You're the top bidder on {STR_VAR_1}!$"

Life_Text_AuctionTakeBackConfirm:
	.string "Take your {STR_VAR_1} off the\n"
	.string "auction?$"

Life_Text_AuctionSellWhich:
	.string "Which POKéMON will you put up?$"

Life_Text_AuctionSellPrice:
	.string "What's the opening bid for\n"
	.string "{STR_VAR_1}?$"

Life_Text_AuctionSellHours:
	.string "How long should the auction run?$"

Life_Text_Auction1Hour:
	.string "1 HOUR$"

Life_Text_Auction8Hours:
	.string "8 HOURS$"

Life_Text_Auction24Hours:
	.string "24 HOURS$"

Life_Text_AuctionSellConfirm:
	.string "Put {STR_VAR_1} up from ¥{STR_VAR_2}\n"
	.string "for {STR_VAR_3}h?$"

Life_Text_AuctionListed:
	.string "{STR_VAR_1} is up for auction! Come\n"
	.string "back to collect when it ends.$"

Life_Text_AuctionNoEggs:
	.string "EGGS can't be auctioned.$"

Life_Text_AuctionLastMon:
	.string "That's your last POKéMON!$"
