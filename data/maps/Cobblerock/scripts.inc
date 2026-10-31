Cobblerock_MapScripts::
	.byte 0

Cobblerock_EventScript_Guide::
	lock
	faceplayer
	msgbox Cobblerock_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Cobblerock_EventScript_GuideStay
	msgbox Cobblerock_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_RUSTBORO_CITY, 13, 41
	waitstate
	release
	end

Cobblerock_EventScript_GuideStay::
	msgbox Cobblerock_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

Cobblerock_EventScript_Sign::
	msgbox Cobblerock_Text_Sign, MSGBOX_SIGN
	end

Cobblerock_EventScript_MineSign::
	msgbox Cobblerock_Text_MineSign, MSGBOX_SIGN
	end

Cobblerock_EventScript_Boulder::
	msgbox Cobblerock_Text_Boulder, MSGBOX_SIGN
	end

Cobblerock_EventScript_Foreman::
	lock
	faceplayer
	goto_if_set FLAG_COBBLE_ASSAY_DONE, Cobblerock_EventScript_ForemanDone
	msgbox Cobblerock_Text_Foreman, MSGBOX_DEFAULT
	release
	end

Cobblerock_EventScript_ForemanDone::
	msgbox Cobblerock_Text_ForemanDone, MSGBOX_DEFAULT
	release
	end

Cobblerock_EventScript_Merchant::
	lock
	faceplayer
	message Cobblerock_Text_Merchant
	waitmessage
	pokemart Cobblerock_Mart
	msgbox Cobblerock_Text_MerchantBye, MSGBOX_DEFAULT
	release
	end

	.align 2
Cobblerock_Mart:
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_REPEL
	.2byte ITEM_POTION
	.2byte ITEM_POKE_BALL
	.2byte ITEM_HARD_STONE
	pokemartlistend

Cobblerock_EventScript_Boy::
	msgbox Cobblerock_Text_Boy, MSGBOX_NPC
	end

Cobblerock_EventScript_OldMiner::
	msgbox Cobblerock_Text_OldMiner, MSGBOX_NPC
	end

Cobblerock_EventScript_Rolycoly::
	lock
	faceplayer
	playmoncry SPECIES_ROLYCOLY, CRY_MODE_NORMAL
	msgbox Cobblerock_Text_Rolycoly, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Cobblerock_EventScript_Rhoda::
	trainerbattle_single TRAINER_COBBLE_RHODA, Cobblerock_Text_RhodaIntro, Cobblerock_Text_RhodaDefeat
	msgbox Cobblerock_Text_RhodaPost, MSGBOX_AUTOCLOSE
	end

Cobblerock_Text_RhodaIntro:
	.string "Cobblerock's hills are perfect for\n"
	.string "a picnic. And a battle!$"

Cobblerock_Text_RhodaDefeat:
	.string "Oof! I picked the wrong hill.$"

Cobblerock_Text_RhodaPost:
	.string "My DRILBUR dug a hole right\n"
	.string "through our picnic blanket.$"


Cobblerock_Text_GuideBack:
	.string "Heading back down to RUSTBORO?$"

Cobblerock_Text_GuideLead:
	.string "Watch your step on the way down!$"

Cobblerock_Text_GuideStay:
	.string "Take your time. Mind the carts!$"

Cobblerock_Text_Sign:
	.string "COBBLEROCK\n"
	.string "The town that digs deep.\p"
	.string "COBBLEROCK MINE is to the north.\n"
	.string "Visitors, please keep off the rails.$"

Cobblerock_Text_OreAsk:
	.string "A vein of ore sparkles red and\n"
	.string "blue in the rock.\p"
	.string "Chip off a sample for the\n"
	.string "ASSAYER?$"

Cobblerock_Text_OreGot:
	.string "{PLAYER} chipped off a sample of\n"
	.string "the ore!$"

Cobblerock_Text_OreDone:
	.string "The vein has been chipped. Only\n"
	.string "plain rock is left.$"

Cobblerock_Text_MineSign:
	.string "COBBLEROCK MINE\n"
	.string "Miners and trainers only. Mind\l"
	.string "the carts!$"

Cobblerock_Text_Boulder:
	.string "A huge boulder blocks the old shaft.\n"
	.string "It won't budge an inch.$"

Cobblerock_Text_Foreman:
	.string "I'm the FOREMAN here. The old shaft\n"
	.string "over there caved in years ago.\p"
	.string "Nobody goes in now… but the\n"
	.string "ASSAYER says the best ore in\l"
	.string "Cobblerock is down there.$"

Cobblerock_Text_ForemanDone:
	.string "We moved the boulder off the old\n"
	.string "shaft for you. Something sparkles\l"
	.string "in there… be careful!$"

Cobblerock_Text_Merchant:
	.string "Supplies for the mine! Can't go\n"
	.string "down without a few ESCAPE ROPES.$"

Cobblerock_Text_MerchantBye:
	.string "Come back up safe!$"

Cobblerock_Text_Boy:
	.string "My dad drives the ore carts! The\n"
	.string "rails go all the way into the mine.$"

Cobblerock_Text_OldMiner:
	.string "I dug in that mine for fifty years.\p"
	.string "The veins with the red and blue\n"
	.string "sparkles are the good ones. Chip\l"
	.string "a little off, and the ASSAYER can\l"
	.string "tell you what's in it.$"

Cobblerock_Text_Rolycoly:
	.string "ROLYCOLY is rolling around the ore\n"
	.string "yard, looking for coal to munch.$"

Cobblerock_EventScript_Item_Repel::
	finditem ITEM_REPEL
	end

Cobblerock_EventScript_Item_GreatBall::
	finditem ITEM_GREAT_BALL
	end

