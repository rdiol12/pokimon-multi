@ Sunfair Carnival guide in Slateport City (tools/areas/carnival.py).
SlateportCity_EventScript_CarnivalGuide::
	lock
	faceplayer
	msgbox SlateportCity_Text_CarnivalGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SlateportCity_EventScript_CarnivalGuideNo
	msgbox SlateportCity_Text_CarnivalGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SUNFAIR_CARNIVAL, 29, 46
	waitstate
	release
	end

SlateportCity_EventScript_CarnivalGuideNo::
	msgbox SlateportCity_Text_CarnivalGuideNo, MSGBOX_DEFAULT
	release
	end

SlateportCity_Text_CarnivalGuideAsk:
	.string "The SUNFAIR CARNIVAL is in town!\n"
	.string "Ferris wheel, games, sweets…\p"
	.string "My boat goes there all day. Want a\n"
	.string "ride?$"

SlateportCity_Text_CarnivalGuideGo:
	.string "Off we go!$"

SlateportCity_Text_CarnivalGuideNo:
	.string "Come along whenever you like!$"
