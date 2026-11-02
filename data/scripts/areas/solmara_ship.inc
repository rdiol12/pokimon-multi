@ The S.S. Meridian's sailor in Lilycove's harbor (tools/solmara/ship.py).
LilycoveCity_Harbor_EventScript_SolmaraSailor::
	lock
	faceplayer
	goto_if_unset FLAG_SYS_GAME_CLEAR, LilycoveCity_Harbor_EventScript_SolmaraNotYet
	msgbox LilycoveCity_Harbor_Text_SolmaraAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LilycoveCity_Harbor_EventScript_SolmaraNo
	msgbox LilycoveCity_Harbor_Text_SolmaraGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SOLMARA_SHIP_LOUNGE, 2, 11
	waitstate
	release
	end

LilycoveCity_Harbor_EventScript_SolmaraNo::
	msgbox LilycoveCity_Harbor_Text_SolmaraNo, MSGBOX_DEFAULT
	release
	end

LilycoveCity_Harbor_EventScript_SolmaraNotYet::
	msgbox LilycoveCity_Harbor_Text_SolmaraNotYet, MSGBOX_DEFAULT
	release
	end

LilycoveCity_Harbor_Text_SolmaraAsk:
	.string "The S.S. MERIDIAN sails to the\n"
	.string "SOLMARA region, far to the east.\p"
	.string "Eight new gyms and a LEAGUE of\n"
	.string "its own! All aboard?$"

LilycoveCity_Harbor_Text_SolmaraGo:
	.string "Welcome aboard, Champion!$"

LilycoveCity_Harbor_Text_SolmaraNo:
	.string "We sail all day. Come back any\n"
	.string "time!$"

LilycoveCity_Harbor_Text_SolmaraNotYet:
	.string "The S.S. MERIDIAN only takes\n"
	.string "trainers who have beaten the\l"
	.string "HOENN POKéMON LEAGUE.\p"
	.string "Become Champion, then come back!$"
