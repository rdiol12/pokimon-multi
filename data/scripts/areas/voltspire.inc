@ Voltspire Power Plant guide in Mauville City (tools/areas/voltspire.py).
MauvilleCity_EventScript_VoltspireGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE03_GET, MauvilleCity_EventScript_VoltspireNoBadge
	msgbox MauvilleCity_Text_VoltspireAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MauvilleCity_EventScript_VoltspireNo
	msgbox MauvilleCity_Text_VoltspireGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_VOLTSPIRE_PLANT, 21, 28
	waitstate
	release
	end

MauvilleCity_EventScript_VoltspireNoBadge::
	msgbox MauvilleCity_Text_VoltspireNoBadge, MSGBOX_DEFAULT
	release
	end

MauvilleCity_EventScript_VoltspireNo::
	msgbox MauvilleCity_Text_VoltspireNo, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Text_VoltspireNoBadge:
	.string "I work at VOLTSPIRE POWER PLANT.\n"
	.string "Something's gone haywire in there.\p"
	.string "WATTSON wants a trainer with his\n"
	.string "badge to help. Have you beaten him?$"

MauvilleCity_Text_VoltspireAsk:
	.string "That's WATTSON's badge! Please, the\n"
	.string "power plant needs your help.\p"
	.string "Will you come to VOLTSPIRE?$"

MauvilleCity_Text_VoltspireGo:
	.string "Thank you! This way!$"

MauvilleCity_Text_VoltspireNo:
	.string "Please hurry… the lights in town\n"
	.string "keep flickering.$"
