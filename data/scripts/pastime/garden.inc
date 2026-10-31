@ Community garden (src/pastime_garden.c), at the north end of Route 104.

Garden_EventScript_Gardener::
	lock
	faceplayer
	callnative Garden_GetShow
	goto_if_eq VAR_RESULT, TRUE, Garden_EventScript_Show
	msgbox Garden_Text_Welcome, MSGBOX_DEFAULT
	callnative Garden_GiveStarterBerries
	giveitem ITEM_ORAN_BERRY, 5
	giveitem ITEM_PECHA_BERRY, 5
	msgbox Garden_Text_Mutations, MSGBOX_DEFAULT
Garden_EventScript_Show:
	msgbox Garden_Text_TodaysShow, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Garden_EventScript_Bye
	callnative Garden_CheckShow
	goto_if_eq VAR_RESULT, GARDEN_SHOW_DONE, Garden_EventScript_ShowDone
	goto_if_eq VAR_RESULT, GARDEN_SHOW_MISSING, Garden_EventScript_ShowMissing
	playse SE_SUCCESS
	msgbox Garden_Text_ShowPoints, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8005, ITEM_NONE, Garden_EventScript_Bye
	playfanfare MUS_LEVEL_UP
	message Garden_Text_RankUp
	waitmessage
	waitfanfare
	waitbuttonpress
	giveitem VAR_0x8005
	release
	end

Garden_EventScript_ShowDone:
	msgbox Garden_Text_ShowDone, MSGBOX_DEFAULT
	release
	end

Garden_EventScript_ShowMissing:
	msgbox Garden_Text_ShowMissing, MSGBOX_DEFAULT
	release
	end

Garden_EventScript_Bye:
	msgbox Garden_Text_Bye, MSGBOX_DEFAULT
	release
	end

Garden_Text_Welcome:
	.string "Welcome to the COMMUNITY GARDEN!\n"
	.string "These eight plots are for everyone.\p"
	.string "Here, some Berries to get you\n"
	.string "started!$"

Garden_Text_Mutations:
	.string "Here's a secret: plant two kinds\n"
	.string "of Berries side by side…\p"
	.string "…and sometimes a brand-new Berry\n"
	.string "grows between them! Try ORAN and\l"
	.string "PECHA next to each other!$"

Garden_Text_TodaysShow:
	.string "Today's GARDEN SHOW wants\n"
	.string "{STR_VAR_2}. Got them?$"

Garden_Text_ShowPoints:
	.string "Splendid Berries! That's {STR_VAR_3}\n"
	.string "Garden Points for you!$"

Garden_Text_RankUp:
	.string "You're a {STR_VAR_1} now!\n"
	.string "Take this for your garden.$"

Garden_Text_ShowDone:
	.string "You've already entered today's show.\n"
	.string "Come back tomorrow!$"

Garden_Text_ShowMissing:
	.string "Hmm, you don't have them yet.\n"
	.string "Grow some and come back!$"

Garden_Text_Bye:
	.string "Happy gardening!$"
