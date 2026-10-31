@ Team check: START > ONLINE > COMMUNITY > CHECK TEAM.
@ The server (server/community_legal.py) reads the party and lists any
@ Pokémon that couldn't have come from the game.

Community_EventScript_LegalCheck::
	lockall
	setvar VAR_0x8004, COMMUNITY_REQ_LEGAL_CHECK
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	copyvar VAR_0x8008, VAR_RESULT
	callnative Community_ShowRows, waitstate=1
	goto_if_eq VAR_0x8008, COMMUNITY_OK, Community_EventScript_LegalOk
	playse SE_FAILURE
	msgbox Community_Text_LegalBad, MSGBOX_DEFAULT
	releaseall
	end

Community_EventScript_LegalOk:
	playse SE_SUCCESS
	msgbox Community_Text_LegalOk, MSGBOX_DEFAULT
	releaseall
	end

Community_Text_LegalOk::
	.string "Your team passed the check!$"

Community_Text_LegalBad::
	.string "Battles with this team won't count\n"
	.string "on the ladder, and trades with it\l"
	.string "will be stopped.$"
