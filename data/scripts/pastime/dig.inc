@ Archaeology digs (src/pastime_dig.c): the archaeologist in Fallarbor Town
@ and the dig sites.

Dig_EventScript_Archaeologist::
	lock
	faceplayer
	callnative Dig_TalkToArchaeologist
	goto_if_ne VAR_0x8005, ITEM_NONE, Dig_EventScript_Reward
	msgbox Dig_Text_Welcome, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Dig_EventScript_Bye
	msgbox Dig_Text_Next, MSGBOX_DEFAULT
Dig_EventScript_Bye:
	msgbox Dig_Text_Museum, MSGBOX_DEFAULT
	release
	end

Dig_EventScript_Reward:
	msgbox Dig_Text_Reward, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	callnative Dig_GetNextReward
	goto_if_eq VAR_RESULT, FALSE, Dig_EventScript_Bye
	msgbox Dig_Text_Next, MSGBOX_DEFAULT
	goto Dig_EventScript_Bye

Dig_EventScript_Site::
	lock
	callnative Dig_CheckSite
	goto_if_eq VAR_RESULT, DIG_RESULT_DUG_TODAY, Dig_EventScript_DugToday
	msgbox Dig_Text_Site, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Dig_EventScript_SiteEnd
	msgbox Dig_Text_HowTo, MSGBOX_DEFAULT
	closemessage
	callnative Dig_Start, waitstate=1
	callnative Dig_Finish
	goto_if_eq VAR_RESULT, 0, Dig_EventScript_Nothing
	playfanfare MUS_OBTAIN_ITEM
	message gPastimeText
	waitmessage
	waitfanfare
	waitbuttonpress
	closemessage
	release
	end

Dig_EventScript_Nothing:
	msgbox gPastimeText, MSGBOX_DEFAULT
Dig_EventScript_SiteEnd:
	release
	end

Dig_EventScript_DugToday:
	msgbox Dig_Text_DugToday, MSGBOX_DEFAULT
	release
	end

Dig_Text_Welcome:
	.string "I'm digging into the history buried\n"
	.string "around MT. CHIMNEY.\p"
	.string "Look for bones sticking out of the\n"
	.string "ground on the routes nearby. Those\l"
	.string "are my dig sites. Dig as you like!\p"
	.string "Each site can be worked once a day.\p"
	.string "You've unearthed {STR_VAR_1} finds so far,\n"
	.string "{STR_VAR_2} of them fossils.$"

Dig_Text_Reward:
	.string "{STR_VAR_1} finds already! You've got a\n"
	.string "real eye for the ground.\p"
	.string "Here, a little something from my\n"
	.string "own collection.$"

Dig_Text_Next:
	.string "Bring my count to {STR_VAR_3} finds, and\n"
	.string "I'll have something else for you.$"

Dig_Text_Museum:
	.string "Fossils belong in a museum! The\n"
	.string "LILYCOVE MUSEUM will take them.$"

Dig_Text_Site:
	.string "It's one of the archaeologist's dig\n"
	.string "sites. A bone is poking out…\p"
	.string "Dig here?$"

Dig_Text_HowTo:
	.string "Move with the + Control Pad and dig\n"
	.string "with A. B stops digging.\p"
	.string "An empty square glows red next to a\n"
	.string "find, then orange, then yellow.$"

Dig_Text_DugToday:
	.string "Somebody has already dug here\n"
	.string "today. Better come back tomorrow.$"
