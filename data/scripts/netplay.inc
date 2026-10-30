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
	dynmultipush Netplay_Text_Cancel, NETPLAY_KIND_NONE
	dynmultistack 0, 0, FALSE, 3, FALSE, NETPLAY_KIND_BATTLE, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case NETPLAY_KIND_BATTLE, Netplay_EventScript_SendInvite
	case NETPLAY_KIND_TRADE, Netplay_EventScript_SendInvite
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
	playse SE_PIN
	msgbox Netplay_Text_BattleInvite, MSGBOX_YESNO
	goto Netplay_EventScript_AnswerInvite
	end

Netplay_EventScript_TradeInvite:
	playse SE_PIN
	msgbox Netplay_Text_TradeInvite, MSGBOX_YESNO
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
	message Netplay_Text_Linking
	waitmessage
	special Netplay_StartActivity
	msgbox Netplay_Text_LinkFailed, MSGBOX_DEFAULT
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
