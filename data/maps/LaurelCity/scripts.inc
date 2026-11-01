LaurelCity_MapScripts::
	.byte 0

LaurelCity_EventScript_Ferry::
	lock
	faceplayer
	msgbox LaurelCity_EventScript_Text_FerryAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LaurelCity_EventScript_FerryNo
	msgbox LaurelCity_EventScript_Text_FerryGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SLATEPORT_CITY, 33, 14
	waitstate
	release
	end

LaurelCity_EventScript_FerryNo::
	msgbox LaurelCity_EventScript_Text_FerryNo, MSGBOX_DEFAULT
	release
	end

LaurelCity_EventScript_StadiumSign::
	msgbox LaurelCity_EventScript_Text_StadiumSign, MSGBOX_SIGN
	end

LaurelCity_EventScript_Text_FerryAsk:
	.string "The league ferry goes back to\n"
	.string "SLATEPORT. All aboard?$"

LaurelCity_EventScript_Text_FerryGo:
	.string "Anchors aweigh!$"

LaurelCity_EventScript_Text_FerryNo:
	.string "I'll be here when the match is\n"
	.string "over.$"

LaurelCity_EventScript_Text_StadiumSign:
	.string "ROVER LEAGUE STADIUM\n"
	.string "Every week, eight rovers. One\l"
	.string "champion.$"

LaurelCity_EventScript_Fan::
	msgbox LaurelCity_EventScript_Fan_Text, MSGBOX_NPC
	end

LaurelCity_EventScript_Fan_Text:
	.string "The ROVER LEAGUE is the best show\n"
	.string "in HOENN! Eight rovers, one\l"
	.string "champion, every single week.$"

LaurelCity_EventScript_Lass::
	msgbox LaurelCity_EventScript_Lass_Text, MSGBOX_NPC
	end

LaurelCity_EventScript_Lass_Text:
	.string "The league rovers come here from\n"
	.string "all over HOENN at the start of\l"
	.string "the week.\p"
	.string "Talk to them! They love to chat\n"
	.string "before a match.$"

LaurelCity_EventScript_Pokefan::
	msgbox LaurelCity_EventScript_Pokefan_Text, MSGBOX_NPC
	end

LaurelCity_EventScript_Pokefan_Text:
	.string "A rover that gets knocked out\n"
	.string "packs up and heads back out on\l"
	.string "the road. The champion stays to\l"
	.string "celebrate until Sunday!$"

LaurelCity_EventScript_OldMan::
	msgbox LaurelCity_EventScript_OldMan_Text, MSGBOX_NPC
	end

LaurelCity_EventScript_OldMan_Text:
	.string "They call it LAUREL CITY after the\n"
	.string "laurel wreath champions wear.\p"
	.string "The HALL OF CHAMPIONS keeps every\n"
	.string "winner's name.$"

LaurelCity_EventScript_Cameraman::
	msgbox LaurelCity_EventScript_Cameraman_Text, MSGBOX_NPC
	end

LaurelCity_EventScript_Cameraman_Text:
	.string "I film every final for the\n"
	.string "evening news. Lights, camera,\l"
	.string "battle!$"

LaurelCity_EventScript_BlackBelt::
	msgbox LaurelCity_EventScript_BlackBelt_Text, MSGBOX_NPC
	end

LaurelCity_EventScript_BlackBelt_Text:
	.string "Players can take a low seed's\n"
	.string "place until the quarterfinals\l"
	.string "end on Wednesday.\p"
	.string "Bring your strongest team!$"

