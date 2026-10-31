VerdantRiver_Hut_MapScripts::
	.byte 0

VerdantRiver_Hut_EventScript_Healer::
	lock
	faceplayer
	msgbox VerdantRiver_Hut_Text_Healer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VerdantRiver_Hut_EventScript_HealerNo
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox VerdantRiver_Hut_Text_HealerDone, MSGBOX_DEFAULT
	release
	end

VerdantRiver_Hut_EventScript_HealerNo::
	msgbox VerdantRiver_Hut_Text_HealerNo, MSGBOX_DEFAULT
	release
	end

VerdantRiver_Hut_Text_Healer:
	.string "Tired from the river? My herbal tea\n"
	.string "will set your POKéMON right. Rest\l"
	.string "a while?$"

VerdantRiver_Hut_Text_HealerDone:
	.string "There! Good as new.$"

VerdantRiver_Hut_Text_HealerNo:
	.string "The kettle's always on.$"
