@ Sunscar Desert guide in Mauville City (tools/areas/desertruins.py).
MauvilleCity_EventScript_SunscarGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE06_GET, MauvilleCity_EventScript_SunscarGuideNoBadge
	msgbox MauvilleCity_Text_SunscarGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MauvilleCity_EventScript_SunscarGuideNo
	msgbox MauvilleCity_Text_SunscarGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SUNSCAR_DESERT, 24, 41
	waitstate
	release
	end

MauvilleCity_EventScript_SunscarGuideNoBadge::
	msgbox MauvilleCity_Text_SunscarGuideNoBadge, MSGBOX_DEFAULT
	release
	end

MauvilleCity_EventScript_SunscarGuideNo::
	msgbox MauvilleCity_Text_SunscarGuideNo, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Text_SunscarGuideNoBadge:
	.string "Deep in the desert there are ruins\n"
	.string "full of UNOWN riddles.\p"
	.string "It's a harsh trip. Come back with\n"
	.string "WINONA's badge and I'll guide you.$"

MauvilleCity_Text_SunscarGuideAsk:
	.string "WINONA's badge! You're ready for\n"
	.string "the desert.\p"
	.string "Shall I guide you to SUNSCAR\n"
	.string "DESERT?$"

MauvilleCity_Text_SunscarGuideGo:
	.string "Keep your head covered!$"

MauvilleCity_Text_SunscarGuideNo:
	.string "The ruins have kept their secrets\n"
	.string "this long. They can wait.$"
