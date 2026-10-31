@ Pokémon pageants: enter a Pokémon, vote for others' (with their
@ pictures), see the standings and collect prizes.

Online_EventScript_Pageant::
	dynmultipush Online_Text_PageantVote, 0
	dynmultipush Online_Text_PageantEnter, 1
	dynmultipush Online_Text_PageantStandings, 2
	dynmultipush Online_Text_HubClose, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Online_EventScript_PageantVote
	case 1, Online_EventScript_PageantEnter
	case 2, Online_EventScript_PageantBoard
	goto Online_EventScript_End

Online_EventScript_PageantEnter:
	msgbox Online_Text_PageantWhich, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Online_EventScript_End
	copyvar VAR_0x8005, VAR_0x8004
	callnative Online_ShowPartyMonPic
	call Online_EventScript_Connecting
	onl_request ONL_REQ_PAGEANT_ENTER
	goto_if_ne VAR_RESULT, ONLINE_OK, Online_EventScript_PageantFailed
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_PageantEnd

Online_EventScript_PageantVote:
	setvar VAR_0x8005, 0
Online_EventScript_PageantVoteLoop:
	call Online_EventScript_Connecting
	onl_request ONL_REQ_PAGEANT_SHOW
	goto_if_ne VAR_RESULT, ONLINE_OK, Online_EventScript_PageantFailed
	callnative Online_ShowPageantEntry
	message gStringVar4
	waitmessage
	dynmultipush Online_Text_PageantVoteIt, 0
	dynmultipush Online_Text_PageantNext, 1
	dynmultipush Online_Text_HubClose, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Online_EventScript_PageantNext
	goto_if_ne VAR_RESULT, 0, Online_EventScript_PageantEnd
	call Online_EventScript_Connecting
	onl_request ONL_REQ_PAGEANT_VOTE
	goto_if_ne VAR_RESULT, ONLINE_OK, Online_EventScript_PageantSay
	playse SE_APPLAUSE
	msgbox gStringVar4, MSGBOX_DEFAULT
Online_EventScript_PageantNext:
	copyvar VAR_0x8005, VAR_0x8007
	goto Online_EventScript_PageantVoteLoop

@ A refusal while voting (already voted...): say why, then the next one.
Online_EventScript_PageantSay:
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, ONLINE_FAILED, Online_EventScript_PageantEnd
	goto Online_EventScript_PageantNext

Online_EventScript_PageantBoard:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PAGEANT_BOARD
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto_if_eq VAR_0x800B, 0, Online_EventScript_End
	msgbox Online_Text_PageantPrize, MSGBOX_DEFAULT
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_PAGEANT_CLAIM
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_EventScript_PageantFailed:
	msgbox gStringVar4, MSGBOX_DEFAULT
Online_EventScript_PageantEnd:
	callnative Online_HidePageantPic
	goto Online_EventScript_End

Online_Text_PageantVote:
	.string "VOTE$"

Online_Text_PageantEnter:
	.string "ENTER A POKéMON$"

Online_Text_PageantStandings:
	.string "STANDINGS$"

Online_Text_PageantVoteIt:
	.string "VOTE FOR IT$"

Online_Text_PageantNext:
	.string "NEXT$"

Online_Text_PageantWhich:
	.string "Which POKéMON will you enter?$"

Online_Text_PageantPrize:
	.string "Your POKéMON placed in a pageant!\n"
	.string "Here's your prize!$"
