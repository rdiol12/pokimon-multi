@ Art studio (src/pastime_art.c), at the Lilycove Museum.

Art_EventScript_Painter::
	lock
	faceplayer
	msgbox Art_Text_Welcome, MSGBOX_DEFAULT
	message Art_Text_Which
	waitmessage
	callnative Art_PushPaintingMenu
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, ART_MENU_CANCEL, Art_EventScript_Bye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Art_EventScript_Bye
	copyvar VAR_0x8005, VAR_RESULT
	msgbox Art_Text_HowTo, MSGBOX_DEFAULT
	message Art_Text_Controls
	waitmessage
	callnative Art_Paint, waitstate=1
	callnative Art_Judge
	msgbox Art_Text_Judged, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Art_EventScript_NotHung
	playfanfare MUS_OBTAIN_ITEM
	message Art_Text_Hung
	waitmessage
	waitfanfare
	waitbuttonpress
	playse SE_SHOP
	msgbox Art_Text_Paid, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8005, ITEM_NONE, Art_EventScript_Bye
	msgbox Art_Text_Gallery, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	goto Art_EventScript_Bye

Art_EventScript_NotHung:
	msgbox Art_Text_NotHung, MSGBOX_DEFAULT
Art_EventScript_Bye:
	msgbox Art_Text_Bye, MSGBOX_DEFAULT
	release
	end

Art_Text_Welcome:
	.string "Welcome to my ART STUDIO! Anyone\n"
	.string "can paint here, it's by numbers!$"

Art_Text_Which:
	.string "What would you like to paint?$"

Art_Text_HowTo:
	.string "Each blank square has a dot of the\n"
	.string "color it should be. Fill them in!\p"
	.string "Get 90% right and the museum will\n"
	.string "hang it, and pay you for it!$"

Art_Text_Controls:
	.string "A: paint  L/R: color\n"
	.string "START: finished!$"

Art_Text_Judged:
	.string "Let's see… The {STR_VAR_2}…\n"
	.string "{STR_VAR_1}% just right!$"

Art_Text_Hung:
	.string "A masterpiece! It goes up in\n"
	.string "the gallery!$"

Art_Text_Paid:
	.string "The museum paid ¥{STR_VAR_3}\n"
	.string "for your painting!$"

Art_Text_Gallery:
	.string "Every painting of yours hangs in the\n"
	.string "gallery now! Take this, artist!$"

Art_Text_NotHung:
	.string "Not bad, but it needs 90% for the\n"
	.string "gallery. Try again any time!$"

Art_Text_Bye:
	.string "Keep painting!$"
