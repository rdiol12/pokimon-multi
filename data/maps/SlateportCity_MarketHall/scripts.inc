SlateportCity_MarketHall_MapScripts::
	.byte 0

@ The MARKET MASTER runs the hall: a word from him, then the market board
@ (data/scripts/netplay.inc), the same one as on every POKé MART wall.
SlateportCity_MarketHall_EventScript_MarketMaster::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_1, SlateportCity_MarketHall_EventScript_MarketMasterAgain
	setflag FLAG_TEMP_1
	msgbox SlateportCity_MarketHall_Text_MarketMasterHello, MSGBOX_DEFAULT
	goto Netplay_EventScript_MarketBoard
	end

SlateportCity_MarketHall_EventScript_MarketMasterAgain::
	msgbox SlateportCity_MarketHall_Text_MarketMasterAgain, MSGBOX_DEFAULT
	goto Netplay_EventScript_MarketBoard
	end

SlateportCity_MarketHall_EventScript_Shopper::
	msgbox SlateportCity_MarketHall_Text_Shopper, MSGBOX_NPC
	end

SlateportCity_MarketHall_EventScript_Sailor::
	msgbox SlateportCity_MarketHall_Text_Sailor, MSGBOX_NPC
	end

SlateportCity_MarketHall_EventScript_Hiker::
	msgbox SlateportCity_MarketHall_Text_Hiker, MSGBOX_NPC
	end

SlateportCity_MarketHall_EventScript_Boy::
	msgbox SlateportCity_MarketHall_Text_Boy, MSGBOX_NPC
	end

SlateportCity_MarketHall_EventScript_Scale::
	msgbox SlateportCity_MarketHall_Text_Scale, MSGBOX_SIGN
	end

SlateportCity_MarketHall_Text_MarketMasterHello:
	.string "Welcome to SLATEPORT's MARKET HALL!\n"
	.string "I'm BARNABY, the MARKET MASTER.\p"
	.string "Trainers from every town post\n"
	.string "POKéMON here for sale.\p"
	.string "Pick one off the board and it's\n"
	.string "yours, or post one of your own.\p"
	.string "Let's see what's on the board\n"
	.string "today!$"

SlateportCity_MarketHall_Text_MarketMasterAgain:
	.string "BARNABY: Fresh listings came in!\n"
	.string "Let's have a look.$"

SlateportCity_MarketHall_Text_Shopper:
	.string "I came for the fruit stalls, but\n"
	.string "the real bargains are on the board.\p"
	.string "Somebody sold a shiny ZIGZAGOON for\n"
	.string "next to nothing this morning!$"

SlateportCity_MarketHall_Text_Sailor:
	.string "Every port in HOENN has a MART,\n"
	.string "but only SLATEPORT has a MARKET HALL.\p"
	.string "I list the POKéMON I fish up at sea\n"
	.string "and pick up my money on the next trip.$"

SlateportCity_MarketHall_Text_Hiker:
	.string "Those trainers who roam HOENN sell\n"
	.string "here too, you know.\p"
	.string "Their POKéMON are well trained.\n"
	.string "Worth a look before you buy!$"

SlateportCity_MarketHall_Text_Boy:
	.string "Mom said I can buy one POKéMON\n"
	.string "if I save my allowance.\p"
	.string "The board in a POKé MART shows the\n"
	.string "same listings. This hall is nicer!$"

SlateportCity_MarketHall_Text_Scale:
	.string "A fruit seller's scale.\n"
	.string "It's perfectly balanced.$"
