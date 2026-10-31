@ Trainer cards: START > ONLINE > COMMUNITY > CARDS.
@ The server is server/community_cards.py; the card is drawn by
@ src/community_cards.c.

Community_EventScript_Cards::
	lockall
	callnative CommunityCards_Publish, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
Community_EventScript_CardsList:
	setvar VAR_0x8004, COMMUNITY_REQ_CARD_LIST
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	callnative Community_ChooseRow, waitstate=1
	goto_if_eq VAR_RESULT, 0xFF, Community_EventScript_Done
	callnative CommunityCards_View, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	playse SE_CARD
	callnative CommunityCards_Show, waitstate=1
	goto Community_EventScript_CardsList
