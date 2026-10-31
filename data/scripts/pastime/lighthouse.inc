@ Lighthouse keeper (src/pastime_lighthouse.c), at Dewford's harbor.

Lighthouse_EventScript_Keeper::
	lock
	faceplayer
	callnative Lighthouse_CheckShift
	goto_if_eq VAR_RESULT, LIGHTHOUSE_RESULT_DAY, Lighthouse_EventScript_Day
	goto_if_eq VAR_RESULT, LIGHTHOUSE_RESULT_DONE, Lighthouse_EventScript_Done
	msgbox Lighthouse_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Lighthouse_EventScript_Bye
	msgbox Lighthouse_Text_HowTo, MSGBOX_DEFAULT
	closemessage
	callnative Lighthouse_Play, waitstate=1
	callnative Lighthouse_Finish
	call_if_eq VAR_RESULT, TRUE, Lighthouse_EventScript_Best
	msgbox Lighthouse_Text_Paid, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8005, ITEM_NONE, Lighthouse_EventScript_Bye
	msgbox Lighthouse_Text_Perfect, MSGBOX_DEFAULT
	giveitem VAR_0x8005
Lighthouse_EventScript_Bye:
	msgbox Lighthouse_Text_Bye, MSGBOX_DEFAULT
	release
	end

Lighthouse_EventScript_Best:
	playfanfare MUS_LEVEL_UP
	message Lighthouse_Text_Best
	waitmessage
	waitfanfare
	waitbuttonpress
	return

Lighthouse_EventScript_Day:
	msgbox Lighthouse_Text_Day, MSGBOX_DEFAULT
	release
	end

Lighthouse_EventScript_Done:
	msgbox Lighthouse_Text_Done, MSGBOX_DEFAULT
	release
	end

Lighthouse_Text_Day:
	.string "I keep the harbor light. Ships come\n"
	.string "in at night, and the rocks off this\l"
	.string "island have sunk many a careless one.\p"
	.string "My eyes aren't what they were.\n"
	.string "Come back after dark, would you?\l"
	.string "From seven in the evening.$"

Lighthouse_Text_Offer:
	.string "Ah, there you are. The ships will be\n"
	.string "coming in soon.\p"
	.string "Will you keep the light for me\n"
	.string "tonight?$"

Lighthouse_Text_HowTo:
	.string "Turn the beam LEFT and RIGHT. Hold it\n"
	.string "on a ship until her crew sees the\l"
	.string "rocks and steers clear.\p"
	.string "A ship left in the dark runs aground.\n"
	.string "Here they come!$"

Lighthouse_Text_Best:
	.string "That's your best shift yet!$"

Lighthouse_Text_Paid:
	.string "You guided {STR_VAR_1} ships past the rocks.\n"
	.string "Here's your pay: ¥{STR_VAR_2}.$"

Lighthouse_Text_Perfect:
	.string "Not one ship lost! A sailor left this\n"
	.string "with me for the keeper. It's yours.$"

Lighthouse_Text_Bye:
	.string "Thank you. Get some sleep now.$"

Lighthouse_Text_Done:
	.string "You've done your shift for tonight.\n"
	.string "Come back tomorrow night.$"
