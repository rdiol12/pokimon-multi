@ The postgame rival (src/community_rival.c). After the Hall of Fame,
@ BRENDAN or MAY waits at one of four places (EVER GRANDE CITY, FORTREE
@ CITY, MOSSDEEP CITY, PACIFIDLOG TOWN). Win, and they move on to the next
@ with one more POKéMON. Each spot's map calls Community_EventScript_RivalSetup
@ on transition; the rival object there uses OBJ_EVENT_GFX_VAR_E and its
@ spot's FLAG_COMMUNITY_RIVAL_<n>.

Community_EventScript_RivalSetup::
	callnative CommunityRival_Setup
	return

Community_EventScript_Rival::
	lock
	faceplayer
	goto_if_set FLAG_COMMUNITY_RIVAL_MET, Community_EventScript_RivalAgain
	setflag FLAG_COMMUNITY_RIVAL_MET
	checkplayergender
	call_if_eq VAR_RESULT, MALE, Community_EventScript_RivalHelloMay
	call_if_eq VAR_RESULT, FEMALE, Community_EventScript_RivalHelloBrendan
	goto Community_EventScript_RivalBattle

Community_EventScript_RivalAgain:
	msgbox Community_Text_RivalAgain, MSGBOX_DEFAULT
Community_EventScript_RivalBattle:
	callnative CommunityRival_Prepare
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Community_Text_RivalTeam, MSGBOX_DEFAULT
	closemessage
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, Community_EventScript_RivalBattleBrendan
	trainerbattle_no_intro TRAINER_MAY_PLACEHOLDER, Community_Text_RivalDefeat
	goto Community_EventScript_RivalWon

Community_EventScript_RivalBattleBrendan:
	trainerbattle_no_intro TRAINER_BRENDAN_PLACEHOLDER, Community_Text_RivalDefeat
Community_EventScript_RivalWon:
	callnative CommunityRival_Won
	msgbox Community_Text_RivalNext, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	release
	end

Community_EventScript_RivalHelloMay:
	msgbox Community_Text_RivalHelloMay, MSGBOX_DEFAULT
	return

Community_EventScript_RivalHelloBrendan:
	msgbox Community_Text_RivalHelloBrendan, MSGBOX_DEFAULT
	return

Community_Text_RivalHelloMay::
	.string "MAY: {PLAYER}{KUN}! There you are!\p"
	.string "You're the CHAMPION now, but I've\n"
	.string "been training too.\p"
	.string "I'll keep finding you, all over\n"
	.string "HOENN, until I win!$"

Community_Text_RivalHelloBrendan::
	.string "BRENDAN: Hey, {PLAYER}!\p"
	.string "Being CHAMPION doesn't mean you\n"
	.string "get to stop battling me.\p"
	.string "I'll keep finding you, all over\n"
	.string "HOENN, until I win!$"

Community_Text_RivalAgain::
	.string "Found you again, {PLAYER}!\n"
	.string "My team keeps growing. Look!$"

Community_Text_RivalTeam::
	.string "I've got {STR_VAR_1} POKéMON with me.\n"
	.string "Let's battle!$"

Community_Text_RivalDefeat::
	.string "Argh! Not yet…$"

Community_Text_RivalNext::
	.string "Next time I'll have {STR_VAR_2} POKéMON.\p"
	.string "Meet me in {STR_VAR_1}!$"
