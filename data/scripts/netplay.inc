@ Online multiplayer: talking to another player, and answering their invites.
@ See src/netplay.c.

Netplay_EventScript_TalkToPlayer::
	lockall
	special Netplay_GetTalkedPlayer
	goto_if_eq VAR_RESULT, FALSE, Netplay_EventScript_ReleaseAll
	message Netplay_Text_WhatToDo
	waitmessage
	dynmultipush Netplay_Text_Battle, NETPLAY_KIND_BATTLE
	dynmultipush Netplay_Text_Trade, NETPLAY_KIND_TRADE
	dynmultipush Netplay_Text_BuySell, NETPLAY_KIND_DEAL
	dynmultipush Netplay_Text_Cancel, NETPLAY_KIND_NONE
	dynmultistack 0, 0, FALSE, 4, FALSE, NETPLAY_KIND_BATTLE, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case NETPLAY_KIND_BATTLE, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_TRADE, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_DEAL, Netplay_EventScript_SendInvite
	goto Netplay_EventScript_CloseAndRelease
	end

Netplay_EventScript_SendInvite:
	copyvar VAR_0x8004, VAR_RESULT
	special Netplay_SendInvite
	message Netplay_Text_WaitingForAnswer
	waitmessage
	special Netplay_WaitForResponse
	goto_if_eq VAR_RESULT, NETPLAY_RESPONSE_ACCEPTED, Netplay_EventScript_StartActivity
	goto_if_eq VAR_RESULT, NETPLAY_RESPONSE_CANCELED, Netplay_EventScript_CloseAndRelease
	msgbox Netplay_Text_Declined, MSGBOX_DEFAULT
	releaseall
	end

Netplay_EventScript_Invite::
	lockall
	special Netplay_BufferInvite
	goto_if_eq VAR_RESULT, NETPLAY_KIND_NONE, Netplay_EventScript_ReleaseAll
	goto_if_eq VAR_RESULT, NETPLAY_KIND_TRADE, Netplay_EventScript_TradeInvite
	goto_if_eq VAR_RESULT, NETPLAY_KIND_DEAL, Netplay_EventScript_DealInvite
	playse SE_PIN
	msgbox Netplay_Text_BattleInvite, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

Netplay_EventScript_TradeInvite:
	playse SE_PIN
	msgbox Netplay_Text_TradeInvite, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

Netplay_EventScript_DealInvite:
	playse SE_PIN
	msgbox Netplay_Text_DealInvite, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

Netplay_EventScript_AnswerInvite:
	special Netplay_AnswerInvite
	goto_if_eq VAR_RESULT, NO, Netplay_EventScript_ReleaseAll
	message Netplay_Text_Connecting
	waitmessage
	special Netplay_WaitForStart
	goto_if_eq VAR_RESULT, TRUE, Netplay_EventScript_StartActivity
	msgbox Netplay_Text_InviteWithdrawn, MSGBOX_DEFAULT
	releaseall
	end

@ Only comes back here if linking failed; otherwise the battle or trade starts.
Netplay_EventScript_StartActivity:
	goto_if_eq VAR_0x8004, NETPLAY_KIND_DEAL, Netplay_EventScript_Deal
	message Netplay_Text_Linking
	waitmessage
	special Netplay_StartActivity
	msgbox Netplay_Text_LinkFailed, MSGBOX_DEFAULT
	releaseall
	end

@ Buy/sell: each player offers a Pokémon and/or money, both see both offers,
@ and the deal happens if both say yes.
Netplay_EventScript_Deal:
	special Netplay_DealBegin
	msgbox Netplay_Text_DealChooseMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x8008, VAR_0x8004
Netplay_EventScript_DealChooseMoney:
	message Netplay_Text_DealHowMuch
	waitmessage
	dynmultipush Netplay_Text_Money0, 0
	dynmultipush Netplay_Text_Money100, 1
	dynmultipush Netplay_Text_Money500, 2
	dynmultipush Netplay_Text_Money1000, 3
	dynmultipush Netplay_Text_Money2000, 4
	dynmultipush Netplay_Text_Money5000, 5
	dynmultipush Netplay_Text_Money10000, 6
	dynmultipush Netplay_Text_Money20000, 7
	dynmultipush Netplay_Text_Money50000, 8
	dynmultipush Netplay_Text_Money100000, 9
	dynmultistack 0, 0, FALSE, 5, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Netplay_EventScript_DealCanceled
	copyvar VAR_0x8005, VAR_RESULT
	copyvar VAR_0x8004, VAR_0x8008
	special Netplay_DealSendOffer
	goto_if_eq VAR_RESULT, TRUE, Netplay_EventScript_DealWaitForOffer
	msgbox Netplay_Text_DealNotEnoughMoney, MSGBOX_DEFAULT
	goto Netplay_EventScript_DealChooseMoney
	end

Netplay_EventScript_DealWaitForOffer:
	message Netplay_Text_DealWaitingForOffer
	waitmessage
	special Netplay_DealWaitForOffer
	goto_if_eq VAR_RESULT, FALSE, Netplay_EventScript_DealOff
	special Netplay_DealBufferOffers
	goto_if_eq VAR_RESULT, FALSE, Netplay_EventScript_DealImpossible
	msgbox Netplay_Text_DealOffers, MSGBOX_YESNO
	special Netplay_DealAnswer
	message Netplay_Text_DealWaitingForAnswer
	waitmessage
	special Netplay_DealWaitForAnswer
	goto_if_eq VAR_RESULT, FALSE, Netplay_EventScript_DealOff
	special Netplay_DealApply
	playfanfare MUS_OBTAIN_ITEM
	message Netplay_Text_DealDone
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Netplay_EventScript_DealEnd
	end

Netplay_EventScript_DealImpossible:
	setvar VAR_RESULT, FALSE
	special Netplay_DealAnswer
	msgbox Netplay_Text_DealImpossible, MSGBOX_DEFAULT
	goto Netplay_EventScript_DealEnd
	end

Netplay_EventScript_DealCanceled:
	setvar VAR_RESULT, FALSE
	special Netplay_DealAnswer
Netplay_EventScript_DealOff:
	msgbox Netplay_Text_DealOff, MSGBOX_DEFAULT
Netplay_EventScript_DealEnd:
	special Netplay_DealEnd
	closemessage
	releaseall
	end

Netplay_EventScript_CloseAndRelease:
	closemessage
Netplay_EventScript_ReleaseAll:
	releaseall
	end

Netplay_Text_WhatToDo:
	.string "What would you like to do\n"
	.string "with {STR_VAR_1}?$"

Netplay_Text_Battle:
	.string "Battle$"

Netplay_Text_Trade:
	.string "Trade$"

Netplay_Text_BuySell:
	.string "Buy/Sell$"

Netplay_Text_Cancel:
	.string "Cancel$"

Netplay_Text_WaitingForAnswer:
	.string "Waiting for {STR_VAR_1} to answer…\n"
	.string "Press B to cancel.$"

Netplay_Text_Declined:
	.string "{STR_VAR_1} can't right now.$"

Netplay_Text_BattleInvite:
	.string "{STR_VAR_1} wants to battle you!\n"
	.string "Accept the challenge?$"

Netplay_Text_TradeInvite:
	.string "{STR_VAR_1} wants to trade with you.\n"
	.string "Accept?$"

Netplay_Text_Connecting:
	.string "Connecting…$"

Netplay_Text_InviteWithdrawn:
	.string "{STR_VAR_1} is no longer waiting.$"

Netplay_Text_Linking:
	.string "Linking with the other player…$"

Netplay_Text_LinkFailed:
	.string "The link with the other player\n"
	.string "couldn't be made.$"

Netplay_Text_DealInvite:
	.string "{STR_VAR_1} wants to make a deal\n"
	.string "with POKéMON or money. Accept?$"

Netplay_Text_DealChooseMon:
	.string "Choose a POKéMON to offer.\n"
	.string "Cancel to offer no POKéMON.$"

Netplay_Text_DealHowMuch:
	.string "How much money will you add?$"

Netplay_Text_DealNotEnoughMoney:
	.string "You don't have that much money.$"

Netplay_Text_DealWaitingForOffer:
	.string "Waiting for {STR_VAR_1}'s offer…\n"
	.string "Press B to cancel.$"

Netplay_Text_DealOffers:
	.string "{STR_VAR_1} offers {STR_VAR_2}.\p"
	.string "You offer {STR_VAR_3}.\n"
	.string "Make this deal?$"

Netplay_Text_DealWaitingForAnswer:
	.string "Waiting for {STR_VAR_1} to decide…$"

Netplay_Text_DealDone:
	.string "The deal is done!$"

Netplay_Text_DealImpossible:
	.string "This deal can't work: a party\n"
	.string "would end up empty or too full.$"

Netplay_Text_DealOff:
	.string "The deal is off.$"

Netplay_Text_Money0:
	.string "No money$"

Netplay_Text_Money100:
	.string "¥100$"

Netplay_Text_Money500:
	.string "¥500$"

Netplay_Text_Money1000:
	.string "¥1,000$"

Netplay_Text_Money2000:
	.string "¥2,000$"

Netplay_Text_Money5000:
	.string "¥5,000$"

Netplay_Text_Money10000:
	.string "¥10,000$"

Netplay_Text_Money20000:
	.string "¥20,000$"

Netplay_Text_Money50000:
	.string "¥50,000$"

Netplay_Text_Money100000:
	.string "¥100,000$"
