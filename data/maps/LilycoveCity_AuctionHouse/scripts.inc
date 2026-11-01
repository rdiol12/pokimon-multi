LilycoveCity_AuctionHouse_MapScripts::
	.byte 0

@ GAVIN runs the sales from the podium: a word from him, then the auction
@ (data/scripts/life/auction.inc): won lots and sales are paid out first.
LilycoveCity_AuctionHouse_EventScript_Auctioneer::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_1, LilycoveCity_AuctionHouse_EventScript_AuctioneerAgain
	setflag FLAG_TEMP_1
	msgbox LilycoveCity_AuctionHouse_Text_AuctioneerHello, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect
	end

LilycoveCity_AuctionHouse_EventScript_AuctioneerAgain::
	msgbox LilycoveCity_AuctionHouse_Text_AuctioneerAgain, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect
	end

LilycoveCity_AuctionHouse_EventScript_LotBoard::
	msgbox LilycoveCity_AuctionHouse_Text_LotBoard, MSGBOX_SIGN
	goto Life_EventScript_Auction
	end

LilycoveCity_AuctionHouse_EventScript_Clerk::
	msgbox LilycoveCity_AuctionHouse_Text_Clerk, MSGBOX_NPC
	end

LilycoveCity_AuctionHouse_EventScript_Lady::
	msgbox LilycoveCity_AuctionHouse_Text_Lady, MSGBOX_NPC
	end

LilycoveCity_AuctionHouse_EventScript_RichBoy::
	msgbox LilycoveCity_AuctionHouse_Text_RichBoy, MSGBOX_NPC
	end

LilycoveCity_AuctionHouse_EventScript_Collector::
	msgbox LilycoveCity_AuctionHouse_Text_Collector, MSGBOX_NPC
	end

LilycoveCity_AuctionHouse_EventScript_Usher::
	msgbox LilycoveCity_AuctionHouse_Text_Usher, MSGBOX_NPC
	end

LilycoveCity_AuctionHouse_Text_AuctioneerHello:
	.string "Order, order! Welcome to the\n"
	.string "LILYCOVE AUCTION HOUSE!\p"
	.string "I'm GAVIN, the AUCTIONEER.\n"
	.string "Trainers all over the world bid here.\p"
	.string "Put a POKéMON up for sale, or bid\n"
	.string "on another trainer's lot.\p"
	.string "The highest bid when the hammer\n"
	.string "falls takes it home!$"

LilycoveCity_AuctionHouse_Text_AuctioneerAgain:
	.string "GAVIN: Back for more? Let me check\n"
	.string "your lots first!$"

LilycoveCity_AuctionHouse_Text_LotBoard:
	.string "The LOT BOARD lists every POKéMON\n"
	.string "going under the hammer today.$"

LilycoveCity_AuctionHouse_Text_Clerk:
	.string "Welcome! I keep the books for the\n"
	.string "AUCTION HOUSE.\p"
	.string "When your POKéMON sells, the house\n"
	.string "keeps 5% and you get the rest.\p"
	.string "Bids are held until somebody\n"
	.string "outbids you. Then they come back.$"

LilycoveCity_AuctionHouse_Text_Lady:
	.string "I've been bidding on a lovely\n"
	.string "MILOTIC all week.\p"
	.string "Somebody keeps outbidding me by\n"
	.string "exactly ¥100… The nerve!$"

LilycoveCity_AuctionHouse_Text_RichBoy:
	.string "Daddy says a real collector never\n"
	.string "bids first. You wait, then pounce!$"

LilycoveCity_AuctionHouse_Text_Collector:
	.string "Even the trainers who roam HOENN\n"
	.string "bid here. They're shrewd, too!$"

LilycoveCity_AuctionHouse_Text_Usher:
	.string "The sale room is up the stairs.\n"
	.string "Talk to GAVIN at the podium.$"
