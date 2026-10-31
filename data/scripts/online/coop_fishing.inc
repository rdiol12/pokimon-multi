@ Co-op fishing: two players reel in a giant Pokémon together.

Online_EventScript_CoFish::
	dynmultipush Online_Text_CoFishTogether, 0
	dynmultipush Online_Text_CoFishBoard, 1
	dynmultipush Online_Text_HubClose, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Online_EventScript_CoFishBoard
	goto_if_ne VAR_RESULT, 0, Online_EventScript_End
	callnative Online_CheckFacingWater
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_CoFishNoWater
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_COFISH_WHO
	message Online_Text_CoFishWho
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	onl_request_or_fail ONL_REQ_COFISH_ASK
	message Online_Text_CoFishWaiting
	waitmessage
Online_EventScript_CoFishWaitLoop:
	delay 30
	onl_request_or_fail ONL_REQ_COFISH_WAIT
	goto_if_eq VAR_0x800B, COFISH_DECLINED, Online_EventScript_ShowMessage
	goto_if_eq VAR_0x800B, COFISH_WAITING, Online_EventScript_CoFishWaitLoop
	goto Online_EventScript_CoFishReel

Online_EventScript_CoFishInvite::
	lockall
	playse SE_PIN
	msgbox Online_Text_CoFishInvite, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_CoFishDecline
	onl_request_or_fail ONL_REQ_COFISH_JOIN
Online_EventScript_CoFishReel:
	playse SE_PIN
	msgbox Online_Text_CoFishBite, MSGBOX_DEFAULT
	closemessage
	callnative Online_PlayReel, waitstate=1
	onl_request_or_fail ONL_REQ_COFISH_REEL
	message Online_Text_CoFishPartner
	waitmessage
Online_EventScript_CoFishResultLoop:
	delay 30
	onl_request_or_fail ONL_REQ_COFISH_RESULT
	goto_if_eq VAR_0x800B, COFISH_WAITING, Online_EventScript_CoFishResultLoop
	goto_if_ne VAR_0x800B, COFISH_CAUGHT, Online_EventScript_ShowMessage
	playfanfare MUS_CAUGHT
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	msgbox Online_Text_CoFishSent, MSGBOX_DEFAULT
	goto Online_EventScript_End

Online_EventScript_CoFishDecline:
	onl_request ONL_REQ_COFISH_NO
	goto Online_EventScript_End

Online_EventScript_CoFishNoWater:
	msgbox Online_Text_CoFishNoWater, MSGBOX_DEFAULT
	goto Online_EventScript_End

Online_EventScript_CoFishBoard:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_COFISH_BOARD
	closemessage
	onl_board
	goto Online_EventScript_End

Online_Text_CoFishTogether:
	.string "FISH TOGETHER$"

Online_Text_CoFishBoard:
	.string "GIANT CATCHES$"

Online_Text_CoFishNoWater:
	.string "Stand facing the water with a friend\n"
	.string "to fish together!$"

Online_Text_CoFishWho:
	.string "Who will you fish with?$"

Online_Text_CoFishWaiting:
	.string "Waiting for {STR_VAR_1}…$"

Online_Text_CoFishInvite:
	.string "{STR_VAR_1} hooked something huge and\n"
	.string "needs a hand! Help reel it in?$"

Online_Text_CoFishBite:
	.string "Something enormous is on the line!\p"
	.string "Reel it in together! Press A when\n"
	.string "the needle is on the green!$"

Online_Text_CoFishPartner:
	.string "Waiting for your partner…$"

Online_Text_CoFishSent:
	.string "The giant POKéMON joined your team!\n"
	.string "(If your party is full, it's in a BOX.)$"
