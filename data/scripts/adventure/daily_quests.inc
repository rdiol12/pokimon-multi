@ Daily quests: one person each in Petalburg, Slateport and Fortree, next to
@ the Pokémon Center (src/daily_quest.c).

Adventure_EventScript_QuestPetalburg::
	setvar VAR_0x8004, DAILY_QUEST_PETALBURG
	goto Adventure_EventScript_QuestGiver

Adventure_EventScript_QuestSlateport::
	setvar VAR_0x8004, DAILY_QUEST_SLATEPORT
	goto Adventure_EventScript_QuestGiver

Adventure_EventScript_QuestFortree::
	setvar VAR_0x8004, DAILY_QUEST_FORTREE
	goto Adventure_EventScript_QuestGiver

Adventure_EventScript_QuestGiver:
	lock
	faceplayer
	callnative DailyQuest_Talk
	switch VAR_RESULT
	case DAILY_QUEST_TALK_OFFER, Adventure_EventScript_QuestOffer
	case DAILY_QUEST_TALK_PROGRESS, Adventure_EventScript_QuestProgress
	case DAILY_QUEST_TALK_COMPLETE, Adventure_EventScript_QuestComplete
	msgbox Adventure_Text_QuestComeBack, MSGBOX_DEFAULT
	release
	end

Adventure_EventScript_QuestOffer:
	msgbox Adventure_Text_QuestOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Adventure_EventScript_QuestDeclined
	callnative DailyQuest_Accept
	msgbox Adventure_Text_QuestAccepted, MSGBOX_DEFAULT
	release
	end

Adventure_EventScript_QuestDeclined:
	msgbox Adventure_Text_QuestDeclined, MSGBOX_DEFAULT
	release
	end

Adventure_EventScript_QuestProgress:
	msgbox Adventure_Text_QuestProgress, MSGBOX_DEFAULT
	release
	end

Adventure_EventScript_QuestComplete:
	callnative DailyQuest_Claim
	goto_if_eq VAR_RESULT, FALSE, Adventure_EventScript_QuestBagFull
	msgbox Adventure_Text_QuestYouDidIt, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Adventure_Text_QuestReward
	waitmessage
	waitfanfare
	msgbox Adventure_Text_QuestThanks, MSGBOX_DEFAULT
	release
	end

Adventure_EventScript_QuestBagFull:
	msgbox Adventure_Text_QuestBagFull, MSGBOX_DEFAULT
	release
	end

Adventure_Text_QuestOffer:
	.string "I've got a little favour to ask today.\p"
	.string "{STR_VAR_1}!\n"
	.string "I'll give you a {STR_VAR_2} for it.\p"
	.string "Will you do it?$"

Adventure_Text_QuestAccepted:
	.string "Thank you! Come back here when it's\n"
	.string "done. R > ADVENTURE shows how far\l"
	.string "along you are.$"

Adventure_Text_QuestDeclined:
	.string "Oh, okay. I'll be here all day if you\n"
	.string "change your mind.$"

Adventure_Text_QuestProgress:
	.string "How's it going?\n"
	.string "{STR_VAR_1}…$"

Adventure_Text_QuestYouDidIt:
	.string "You did it! Here's your reward.$"

Adventure_Text_QuestReward:
	.string "{PLAYER} received a {STR_VAR_2}\n"
	.string "and ¥{STR_VAR_3}!$"

Adventure_Text_QuestThanks:
	.string "See you tomorrow! I'll have something\n"
	.string "new for you.$"

Adventure_Text_QuestComeBack:
	.string "Thanks again for today! Come back\n"
	.string "tomorrow for a new request.$"

Adventure_Text_QuestBagFull:
	.string "You did it! But your BAG is full…\n"
	.string "Make some room and come back.$"
