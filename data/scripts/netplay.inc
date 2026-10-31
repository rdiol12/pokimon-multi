@ Online multiplayer: talking to another player, and answering their invites.
@ See src/netplay.c.

Netplay_EventScript_TalkToPlayer::
	lockall
	special Netplay_GetTalkedPlayer
	goto_if_eq VAR_RESULT, FALSE, Netplay_EventScript_ReleaseAll
	callnative Online_HideIsSeeking
	goto_if_eq VAR_RESULT, TRUE, Online_EventScript_HideTag
	message Netplay_Text_WhatToDo
	waitmessage
	dynmultipush Netplay_Text_Battle, NETPLAY_KIND_BATTLE
	dynmultipush Netplay_Text_Trade, NETPLAY_KIND_TRADE
	dynmultipush Netplay_Text_BuySell, NETPLAY_KIND_DEAL
	dynmultipush Netplay_Text_AddFriend, NETPLAY_KIND_FRIEND
	dynmultipush Netplay_Text_Coop, NETPLAY_KIND_COOP
	dynmultipush Netplay_Text_Double, NETPLAY_KIND_DOUBLE
	dynmultipush Netplay_Text_Multi, NETPLAY_KIND_MULTI
	dynmultipush Netplay_Text_Cancel, NETPLAY_KIND_NONE
	dynmultistack 0, 0, FALSE, 6, FALSE, NETPLAY_KIND_BATTLE, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case NETPLAY_KIND_BATTLE, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_DOUBLE, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_MULTI, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_COOP, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_TRADE, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_DEAL, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_FRIEND, Netplay_EventScript_AddFriend
	goto Netplay_EventScript_CloseAndRelease
	end

Netplay_EventScript_AddFriend:
	special Netplay_AddFriend
	goto_if_eq VAR_RESULT, NETPLAY_FRIEND_ADD_OK, Netplay_EventScript_FriendAdded
	goto_if_eq VAR_RESULT, NETPLAY_FRIEND_ADD_FULL, Netplay_EventScript_FriendsFull
	msgbox Netplay_Text_NoAnswer, MSGBOX_DEFAULT
	releaseall
	end

Netplay_EventScript_FriendAdded:
	playse SE_SUCCESS
	msgbox Netplay_Text_FriendAdded, MSGBOX_DEFAULT
	releaseall
	end

Netplay_EventScript_FriendsFull:
	msgbox Netplay_Text_FriendsFull, MSGBOX_DEFAULT
	releaseall
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
	goto_if_eq VAR_RESULT, NETPLAY_KIND_COOP, Netplay_EventScript_CoopInvite
	goto_if_eq VAR_RESULT, NETPLAY_KIND_DOUBLE, Netplay_EventScript_DoubleInvite
	goto_if_eq VAR_RESULT, NETPLAY_KIND_MULTI, Netplay_EventScript_MultiInvite
	playse SE_PIN
	msgbox Netplay_Text_BattleInvite, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

Netplay_EventScript_TradeInvite:
	playse SE_PIN
	msgbox Netplay_Text_TradeInvite, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

Netplay_EventScript_DoubleInvite:
	playse SE_PIN
	msgbox Netplay_Text_DoubleInvite, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

@ A team battle's partners are often elsewhere, out of sight of who started it.
Netplay_EventScript_MultiInvite:
	playse SE_PIN
	goto_if_eq VAR_0x8005, FALSE, Netplay_EventScript_MultiInviteUnseen
	msgbox Netplay_Text_MultiInvite, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

Netplay_EventScript_MultiInviteUnseen:
	msgbox Netplay_Text_MultiInviteUnseen, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

Netplay_EventScript_CoopInvite:
	playse SE_PIN
	msgbox Netplay_Text_CoopInvite, MSGBOX_YESNO
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

@ Buy/sell: both players build an offer of Pokémon and money on the deal
@ screen, and the deal happens once both accept. ADD leaves the screen for the
@ party menu and comes back.
Netplay_EventScript_Deal:
	closemessage
	special Netplay_DealBegin
Netplay_EventScript_DealScreen:
	special Netplay_DealScreen
	switch VAR_RESULT
	case NETPLAY_DEAL_ADD_MON, Netplay_EventScript_DealAddMon
	case NETPLAY_DEAL_DONE, Netplay_EventScript_DealDone
	case NETPLAY_DEAL_PEER_LEFT, Netplay_EventScript_DealPeerLeft
	goto Netplay_EventScript_DealEnd
	end

Netplay_EventScript_DealAddMon:
	special ChoosePartyMon
	waitstate
	special Netplay_DealAddMon
	goto Netplay_EventScript_DealScreen
	end

Netplay_EventScript_DealDone:
	special Netplay_DealApply
	playfanfare MUS_OBTAIN_ITEM
	message Netplay_Text_DealDone
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Netplay_EventScript_DealEnd
	end

Netplay_EventScript_DealPeerLeft:
	msgbox Netplay_Text_DealPeerLeft, MSGBOX_DEFAULT
Netplay_EventScript_DealEnd:
	special Netplay_DealEnd
	closemessage
	releaseall
	end

@ The market board on the wall of every Poké Mart (src/netplay.c).
Netplay_EventScript_MarketBoard::
	lockall
	specialvar VAR_RESULT, Netplay_MarketAvailable
	goto_if_eq VAR_RESULT, FALSE, Netplay_EventScript_MarketOffline
	special Netplay_MarketBegin
Netplay_EventScript_MarketScreen:
	special Netplay_MarketScreen
	goto_if_eq VAR_RESULT, NETPLAY_MARKET_SCREEN_SELL, Netplay_EventScript_MarketSell
	releaseall
	end

Netplay_EventScript_MarketSell:
	special ChoosePartyMon
	waitstate
	special Netplay_MarketPickedMon
	goto Netplay_EventScript_MarketScreen
	end

Netplay_EventScript_MarketOffline:
	msgbox Netplay_Text_MarketOffline, MSGBOX_DEFAULT
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

Netplay_Text_Double:
	.string "Ranked double battle$"

Netplay_Text_DoubleInvite:
	.string "{STR_VAR_1} challenges you to a\n"
	.string "ranked double battle! Accept?$"

Netplay_Text_Multi:
	.string "2v2 team battle$"

Netplay_Text_MultiInvite:
	.string "{STR_VAR_1} started a 2v2 team battle!\n"
	.string "Will you join in?$"

Netplay_Text_MultiInviteUnseen:
	.string "Your team has a 2v2 team battle!\n"
	.string "Will you join in?$"

Netplay_Text_Coop:
	.string "Co-op battle$"

Netplay_Text_Trade:
	.string "Trade$"

Netplay_Text_BuySell:
	.string "Buy/Sell$"

Netplay_Text_AddFriend:
	.string "Add friend$"

Netplay_Text_FriendAdded:
	.string "{STR_VAR_1} is now on your friend list!\p"
	.string "See where your friends are with\n"
	.string "START {RIGHT_ARROW} ONLINE {RIGHT_ARROW} FRIENDS.$"

Netplay_Text_FriendsFull:
	.string "Your friend list is full.\n"
	.string "Remove someone first.$"

Netplay_Text_NoAnswer:
	.string "The server didn't answer.$"

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

Netplay_Text_CoopInvite:
	.string "{STR_VAR_1} wants to team up against\n"
	.string "two tough TRAINERS. Join in?$"

Netplay_Text_DealInvite:
	.string "{STR_VAR_1} wants to make a deal\n"
	.string "with POKéMON or money. Accept?$"

Netplay_Text_MarketOffline:
	.string "MARKET BOARD\n"
	.string "Trainers buy and sell POKéMON here.\p"
	.string "Pick JOIN ONLINE on the title menu\n"
	.string "to use it.$"

Netplay_Text_DealDone:
	.string "The deal is done!$"

Netplay_Text_DealPeerLeft:
	.string "{STR_VAR_1} left the deal.$"
