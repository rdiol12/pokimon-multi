@ The shared daycare: START > ONLINE > COMMUNITY > DAYCARE.
@ The server is server/community_daycare.py; the egg is made by
@ src/community_daycare.c.

	.set DAYCARE_NONE, 2
	.set DAYCARE_EGG, 5
	.set EGG_MADE, 0
	.set EGG_INCOMPATIBLE, 2

Community_EventScript_Daycare::
	lockall
	setvar VAR_0x8004, COMMUNITY_REQ_DAYCARE_STATUS
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	copyvar VAR_0x8008, VAR_RESULT
	callnative Community_BufferRows
	msgbox Community_Text_DaycareStatus, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8008, DAYCARE_NONE, Community_EventScript_DaycarePair
	goto_if_eq VAR_0x8008, DAYCARE_EGG, Community_EventScript_DaycareEgg
	msgbox Community_Text_DaycareTakeBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Community_EventScript_Done
	setvar VAR_0x8004, COMMUNITY_REQ_DAYCARE_TAKE
	callnative Community_ScriptRequest, waitstate=1
	msgbox Community_Text_DaycareTookBack, MSGBOX_DEFAULT
	releaseall
	end

Community_EventScript_DaycarePair:
	msgbox Community_Text_DaycarePairUp, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Community_EventScript_Done
	setvar VAR_0x8004, COMMUNITY_REQ_DAYCARE_LIST
	callnative Community_ScriptRequest, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	callnative Community_ChooseRow, waitstate=1
	goto_if_eq VAR_RESULT, 0xFF, Community_EventScript_Done
	copyvar VAR_0x8009, VAR_RESULT
	msgbox Community_Text_DaycareWhichMon, MSGBOX_DEFAULT
	closemessage
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, Community_EventScript_Done
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, Community_EventScript_DaycareNoEggs
	copyvar VAR_0x8005, VAR_0x8009
	callnative CommunityDaycare_Leave, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	callnative Community_BufferRows
	msgbox Community_Text_DaycareLeft, MSGBOX_DEFAULT
	releaseall
	end

Community_EventScript_DaycareNoEggs:
	msgbox Community_Text_DaycareNoEggs, MSGBOX_DEFAULT
	releaseall
	end

Community_EventScript_DaycareEgg:
	setvar VAR_0x8004, COMMUNITY_REQ_DAYCARE_EGG
	setvar VAR_0x8005, 0
	callnative Community_ScriptRequest, waitstate=1
	goto_if_ne VAR_RESULT, DAYCARE_EGG, Community_EventScript_Offline
	callnative CommunityDaycare_MakeEgg
	goto_if_eq VAR_RESULT, EGG_INCOMPATIBLE, Community_EventScript_DaycareIncompatible
	goto_if_ne VAR_RESULT, EGG_MADE, Community_EventScript_DaycarePartyFull
	setvar VAR_0x8004, COMMUNITY_REQ_DAYCARE_EGG
	setvar VAR_0x8005, 1
	callnative Community_ScriptRequest, waitstate=1
	playfanfare MUS_OBTAIN_ITEM
	msgbox Community_Text_DaycareGotEgg, MSGBOX_DEFAULT
	waitfanfare
	releaseall
	end

Community_EventScript_DaycareIncompatible:
	msgbox Community_Text_DaycareIncompatible, MSGBOX_DEFAULT
	releaseall
	end

Community_EventScript_DaycarePartyFull:
	msgbox Community_Text_DaycarePartyFull, MSGBOX_DEFAULT
	releaseall
	end

Community_Text_DaycareStatus:
	.string "{STR_VAR_1}\n"
	.string "{STR_VAR_2}\p"
	.string "{STR_VAR_3}$"

Community_Text_DaycarePairUp:
	.string "Pair up with another trainer?$"

Community_Text_DaycareWhichMon:
	.string "Which POKéMON will be the parent?\n"
	.string "It stays in your party too.$"

Community_Text_DaycareLeft:
	.string "{STR_VAR_1}\n"
	.string "{STR_VAR_2}$"

Community_Text_DaycareNoEggs:
	.string "An EGG can't be a parent.$"

Community_Text_DaycareTakeBack:
	.string "End the pairing and take your\n"
	.string "POKéMON back?$"

Community_Text_DaycareTookBack:
	.string "Your POKéMON came back.$"

Community_Text_DaycareGotEgg:
	.string "{PLAYER} received an EGG!$"

Community_Text_DaycareIncompatible:
	.string "The two POKéMON don't seem to like\n"
	.string "each other. There's no EGG…\p"
	.string "End the pairing and try another\n"
	.string "pair of POKéMON.$"

Community_Text_DaycarePartyFull:
	.string "Your party is full.\n"
	.string "Make room for the EGG first.$"
