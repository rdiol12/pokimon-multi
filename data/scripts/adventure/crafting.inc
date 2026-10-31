@ Crafting (src/crafting.c): the crafter outside the Devon Corporation in
@ Rustboro teaches it and sells Apricorns; R > ADVENTURE > CRAFTING.

Adventure_EventScript_Crafting::
	goto_if_unset FLAG_ADVENTURE_CRAFTING, Adventure_EventScript_CantCraftYet
	callnative Crafting_ShowList, waitstate=1
	releaseall
	end

Adventure_EventScript_CantCraftYet:
	msgbox Adventure_Text_CantCraftYet, MSGBOX_DEFAULT
	releaseall
	end

Adventure_EventScript_Crafter::
	lock
	faceplayer
	goto_if_set FLAG_ADVENTURE_CRAFTING, Adventure_EventScript_CrafterShop
	msgbox Adventure_Text_CrafterIntro, MSGBOX_DEFAULT
	setflag FLAG_ADVENTURE_CRAFTING
	giveitem ITEM_RED_APRICORN, 4
	giveitem ITEM_WHITE_APRICORN, 2
	msgbox Adventure_Text_CrafterHowTo, MSGBOX_DEFAULT
	release
	end

Adventure_EventScript_CrafterShop:
	message Adventure_Text_CrafterShop
	waitmessage
	pokemart Adventure_Pokemart_Apricorns
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Adventure_Pokemart_Apricorns:
	.2byte ITEM_RED_APRICORN
	.2byte ITEM_BLUE_APRICORN
	.2byte ITEM_YELLOW_APRICORN
	.2byte ITEM_GREEN_APRICORN
	.2byte ITEM_PINK_APRICORN
	.2byte ITEM_WHITE_APRICORN
	.2byte ITEM_BLACK_APRICORN
	pokemartlistend

Adventure_Text_CantCraftYet:
	.string "You don't know how to craft yet.\p"
	.string "There's a crafter outside the DEVON\n"
	.string "CORPORATION in RUSTBORO CITY.$"

Adventure_Text_CrafterIntro:
	.string "DEVON makes the newest POKé BALLS,\n"
	.string "but I make mine the old way!\p"
	.string "An APRICORN, a little care… and\n"
	.string "you've got a BALL. BERRIES make\l"
	.string "medicine, MUSHROOMS make REPELS,\l"
	.string "and SHARDS turn into stones.\p"
	.string "Here, try a few APRICORNS.$"

Adventure_Text_CrafterHowTo:
	.string "Open ADVENTURE with the R Button\n"
	.string "and pick CRAFTING. Anywhere you go!\p"
	.string "Come back if you need APRICORNS.\n"
	.string "I grow all seven colours.$"

Adventure_Text_CrafterShop:
	.string "APRICORNS? Sure, take your pick!$"
