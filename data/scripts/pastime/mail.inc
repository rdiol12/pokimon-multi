@ Mail courier routes (src/pastime_mail.c). VAR_0x8004 = the postmaster's
@ town or the recipient.

Mail_EventScript_Postmaster::
	lock
	faceplayer
	callnative Mail_TalkToPostmaster
	switch VAR_RESULT
	case MAIL_OTHER_TOWN, Mail_EventScript_OtherTown
	case MAIL_IN_PROGRESS, Mail_EventScript_InProgress
	case MAIL_COMPLETE, Mail_EventScript_Complete
	case MAIL_DONE_TODAY, Mail_EventScript_DoneToday
	msgbox Mail_Text_Hello, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Mail_EventScript_Declined
	callnative Mail_StartRoute
	playfanfare MUS_OBTAIN_ITEM
	message Mail_Text_GotLetters
	waitmessage
	waitfanfare
	goto_if_eq VAR_RESULT, FALSE, Mail_EventScript_NoBird
	msgbox Mail_Text_BirdFlies, MSGBOX_DEFAULT
	release
	end

Mail_EventScript_NoBird:
	msgbox Mail_Text_NoBird, MSGBOX_DEFAULT
	release
	end

Mail_EventScript_Declined:
	msgbox Mail_Text_Declined, MSGBOX_DEFAULT
	release
	end

Mail_EventScript_OtherTown:
	msgbox Mail_Text_OtherTown, MSGBOX_DEFAULT
	release
	end

Mail_EventScript_InProgress:
	msgbox Mail_Text_InProgress, MSGBOX_DEFAULT
	release
	end

Mail_EventScript_DoneToday:
	msgbox Mail_Text_DoneToday, MSGBOX_DEFAULT
	release
	end

Mail_EventScript_Complete:
	msgbox Mail_Text_Complete, MSGBOX_DEFAULT
	callnative Mail_FinishRoute
	playse SE_SHOP
	msgbox Mail_Text_Paid, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Mail_EventScript_PaidDone
	msgbox Mail_Text_Stamp, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
Mail_EventScript_PaidDone:
	release
	end

Mail_EventScript_Recipient::
	lock
	faceplayer
	msgbox Mail_Text_ForMe, MSGBOX_DEFAULT
	callnative Mail_TalkToRecipient
	msgbox gPastimeText, MSGBOX_DEFAULT
	goto_if_ne VAR_RESULT, 0, Mail_EventScript_RecipientDone
	msgbox Mail_Text_AllDelivered, MSGBOX_DEFAULT
Mail_EventScript_RecipientDone:
	release
	end

Mail_Text_Hello:
	.string "PIDGEY & WINGULL POST! Today's\n"
	.string "route is right here in town.\p"
	.string "Three letters to hand out.\n"
	.string "Will you be our courier?$"

Mail_Text_GotLetters:
	.string "{PLAYER} got three letters!$"

Mail_Text_BirdFlies:
	.string "Oh, a bird Pokémon! {STR_VAR_1} can\n"
	.string "fly one letter over to {STR_VAR_2}.\p"
	.string "{STR_VAR_1} took off with it!\p"
	.string "Hand out the other two, then come\n"
	.string "back for your pay.$"

Mail_Text_NoBird:
	.string "Hand them out around town, then\n"
	.string "come back for your pay.\p"
	.string "Got a PIDGEY or a WINGULL? Bring\n"
	.string "one along and it'll help out!$"

Mail_Text_Declined:
	.string "Maybe tomorrow, then.$"

Mail_Text_OtherTown:
	.string "PIDGEY & WINGULL POST! Today's\n"
	.string "route is in {STR_VAR_1}.\p"
	.string "We run one route a day, each day\n"
	.string "in a different town.$"

Mail_Text_InProgress:
	.string "{STR_VAR_2} letter(s) left to hand out.\n"
	.string "The people are waiting!$"

Mail_Text_DoneToday:
	.string "That's all for today. Thanks again!\n"
	.string "Tomorrow's route is somewhere else.$"

Mail_Text_Complete:
	.string "All delivered? Splendid work!$"

Mail_Text_Paid:
	.string "{PLAYER} got ¥{STR_VAR_1}!$"

Mail_Text_Stamp:
	.string "And for a loyal courier, a stamp\n"
	.string "of thanks!$"

Mail_Text_ForMe:
	.string "Mail? For me?\n"
	.string "{PLAYER} handed over the letter.$"

Mail_Text_AllDelivered:
	.string "That was the last letter!\n"
	.string "Back to the postmaster for the pay.$"
