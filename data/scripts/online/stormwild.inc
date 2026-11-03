@ The Stormwild, the extraction run (src/stormwild.c, server/online_xrun.py,
@ docs/stormwild.md). The Storm Warden stands by Mauville's Pokémon Center.

Stormwild_EventScript_Warden::
	lock
	faceplayer
	callnative Stormwild_IsOnScript
	goto_if_eq VAR_RESULT, TRUE, Stormwild_EventScript_WardenBusy
	msgbox Stormwild_Text_WardenHello, MSGBOX_DEFAULT
Stormwild_EventScript_WardenMenu:
	message Stormwild_Text_WardenWhat
	waitmessage
	dynmultipush Stormwild_Text_Enter, 0
	dynmultipush Stormwild_Text_Hauls, 1
	dynmultipush Stormwild_Text_HowItWorks, 2
	dynmultipush gText_Cancel, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Stormwild_EventScript_Enter
	case 1, Stormwild_EventScript_Hauls
	case 2, Stormwild_EventScript_Rules
	goto Stormwild_EventScript_Bye

Stormwild_EventScript_WardenBusy:
	msgbox Stormwild_Text_WardenBusy, MSGBOX_DEFAULT
	release
	end

Stormwild_EventScript_Bye:
	msgbox Stormwild_Text_Bye, MSGBOX_DEFAULT
	release
	end

Stormwild_EventScript_Rules:
	msgbox Stormwild_Text_Rules, MSGBOX_DEFAULT
	goto Stormwild_EventScript_WardenMenu

Stormwild_EventScript_Hauls:
	callnative Stormwild_GetRecord
	goto_if_eq VAR_RESULT, 0, Stormwild_EventScript_HaulsBoard
	msgbox Stormwild_Text_MyRecord, MSGBOX_DEFAULT
Stormwild_EventScript_HaulsBoard:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_XRUN_BOARD
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto Online_EventScript_End

Stormwild_EventScript_Enter:
	callnative Stormwild_CheckTeam
	goto_if_eq VAR_RESULT, 1, Stormwild_EventScript_TooMany
	goto_if_eq VAR_RESULT, 2, Stormwild_EventScript_NoTeam
	msgbox Stormwild_Text_Pouch, MSGBOX_DEFAULT
Stormwild_EventScript_Pack:
	callnative Stormwild_PouchInfo
	goto_if_eq VAR_RESULT, TRUE, Stormwild_EventScript_Packed
	message Stormwild_Text_PackWhich
	waitmessage
	callnative Online_ChooseBagItem, waitstate=1
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Stormwild_EventScript_Packed
	callnative Stormwild_PackChosen
	goto_if_eq VAR_RESULT, FALSE, Stormwild_EventScript_Pack
	playse SE_SELECT
	goto Stormwild_EventScript_Pack

Stormwild_EventScript_Packed:
	msgbox Stormwild_Text_Ready, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Stormwild_EventScript_NotReady
	callnative Stormwild_CheckTeam
	call Online_EventScript_Connecting
	onl_request ONL_REQ_XRUN_JOIN
	goto_if_ne VAR_RESULT, ONLINE_OK, Stormwild_EventScript_JoinFailed
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	callnative Stormwild_Joined
	callnative Stormwild_LobbyCountdown, waitstate=1
	call Online_EventScript_Connecting
	onl_request ONL_REQ_XRUN_GO
	goto_if_ne VAR_RESULT, ONLINE_OK, Stormwild_EventScript_JoinFailed
	playse SE_THUNDER
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_WHITE
	callnative Stormwild_Enter
	waitstate
	release
	end

Stormwild_EventScript_JoinFailed:
	callnative Stormwild_Unpack
	goto Online_EventScript_Failed

Stormwild_EventScript_NotReady:
	callnative Stormwild_Unpack
	msgbox Stormwild_Text_Unpacked, MSGBOX_DEFAULT
	release
	end

Stormwild_EventScript_TooMany:
	msgbox Stormwild_Text_TooMany, MSGBOX_DEFAULT
	release
	end

Stormwild_EventScript_NoTeam:
	msgbox Stormwild_Text_NoTeam, MSGBOX_DEFAULT
	release
	end

@ In the Stormwild ----------------------------------------------------------

Stormwild_EventScript_Cache::
	lock
	callnative Stormwild_PickCache
	call Online_EventScript_Connecting
	onl_request ONL_REQ_XRUN_LOOT
	goto_if_eq VAR_RESULT, ONLINE_FAILED, Online_EventScript_Failed
	goto_if_eq VAR_RESULT, ONLINE_REFUSED, Stormwild_EventScript_CacheEmpty
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	callnative Stormwild_CacheGone
	goto Online_EventScript_End

Stormwild_EventScript_CacheEmpty:
	msgbox gStringVar4, MSGBOX_DEFAULT
	callnative Stormwild_CacheGone
	goto Online_EventScript_End

Stormwild_EventScript_Beacon::
	lock
	callnative Stormwild_BeaconText
	msgbox gStringVar4, MSGBOX_DEFAULT
	release
	end

@ Ten seconds by a lit beacon: VAR_0x8005 is which.
Stormwild_EventScript_Extract::
	lockall
	playse SE_M_REFLECT
	msgbox Stormwild_Text_Pickup, MSGBOX_DEFAULT
	call Online_EventScript_Connecting
	onl_request ONL_REQ_XRUN_OUT
	goto_if_ne VAR_RESULT, ONLINE_OK, Stormwild_EventScript_ExtractRefused
	closemessage
	playse SE_WARP_OUT
	fadescreen FADE_TO_WHITE
	callnative Stormwild_Extracted
	waitstate
	releaseall
	end

Stormwild_EventScript_ExtractRefused:
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	callnative Stormwild_ExtractRefused
	releaseall
	end

@ Another runner beat the player.
Stormwild_EventScript_Raided::
	lockall
	call Online_EventScript_Connecting
	onl_request ONL_REQ_XRUN_ROB
	goto_if_ne VAR_RESULT, ONLINE_OK, Stormwild_EventScript_RaidDone
	playse SE_FAILURE
	msgbox gStringVar4, MSGBOX_DEFAULT
Stormwild_EventScript_RaidDone:
	closemessage
	callnative Stormwild_Resume
	releaseall
	end

Stormwild_EventScript_TimeUp::
	lockall
	playse SE_THUNDER
	msgbox Stormwild_Text_TimeUp, MSGBOX_DEFAULT
	goto Stormwild_EventScript_Fail

Stormwild_EventScript_LeftZone::
	lockall
	msgbox Stormwild_Text_LeftZone, MSGBOX_DEFAULT
	goto Stormwild_EventScript_Fail

@ The game was reset during a run.
Stormwild_EventScript_Lost::
	lockall
	msgbox Stormwild_Text_Lost, MSGBOX_DEFAULT
Stormwild_EventScript_Fail:
	onl_request ONL_REQ_XRUN_FAIL
	closemessage
	fadescreen FADE_TO_BLACK
	callnative Stormwild_Fail
	waitstate
	releaseall
	end

@ Back at the gate.
Stormwild_EventScript_After::
	lockall
	callnative Stormwild_AfterText
	goto_if_eq VAR_RESULT, TRUE, Stormwild_EventScript_AfterFailed
	playfanfare MUS_OBTAIN_B_POINTS
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	msgbox Stormwild_Text_Welcome, MSGBOX_DEFAULT
	goto Stormwild_EventScript_AfterSave

Stormwild_EventScript_AfterFailed:
	playse SE_FAILURE
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Stormwild_Text_Rescued, MSGBOX_DEFAULT
Stormwild_EventScript_AfterSave:
	closemessage
	callnative Stormwild_Save
	releaseall
	end

Stormwild_Text_WardenHello:
	.string "I'm the STORM WARDEN. North of here\n"
	.string "a storm never leaves the STORMWILD.\p"
	.string "Rare POKéMON and lost supplies wait\n"
	.string "inside. So do other trainers.$"

Stormwild_Text_WardenWhat:
	.string "Will you brave the storm?$"

Stormwild_Text_WardenBusy:
	.string "Hold on. The gate's about to open!$"

Stormwild_Text_Bye:
	.string "Come back when you feel lucky.$"

Stormwild_Text_Enter:
	.string "ENTER THE STORMWILD$"

Stormwild_Text_Hauls:
	.string "BEST HAULS$"

Stormwild_Text_HowItWorks:
	.string "HOW IT WORKS$"

Stormwild_Text_Rules:
	.string "Take up to three POKéMON and a pouch\n"
	.string "of up to ten kinds of items.\p"
	.string "The rest of your bag stays here with\n"
	.string "me, safe and sound.\p"
	.string "The storm closes in after 15 minutes.\n"
	.string "Open supply caches before others do.\p"
	.string "One beacon at a time lights up. Stand\n"
	.string "by a lit one for 10 seconds and\l"
	.string "we'll pull you out with everything.\p"
	.string "If the storm catches you, or your team\n"
	.string "all faints, or you leave another way,\l"
	.string "you lose your whole pouch, everything\l"
	.string "you found, and every POKéMON you\l"
	.string "caught in there.\p"
	.string "Lose a battle to another trainer and\n"
	.string "they take your best items and up to\l"
	.string "two of your catches.\p"
	.string "Your own POKéMON always come home.$"

Stormwild_Text_MyRecord:
	.string "Your runs: {STR_VAR_2}. Got out: {STR_VAR_3}.\n"
	.string "Best haul: ¥{STR_VAR_1}.$"

Stormwild_Text_TooMany:
	.string "Only {STR_VAR_1} POKéMON may go in. Leave\n"
	.string "the rest in a PC first.$"

Stormwild_Text_NoTeam:
	.string "Your POKéMON can't fight! Heal them\n"
	.string "first.$"

Stormwild_Text_Pouch:
	.string "Pick what goes in your pouch. Up to ten\n"
	.string "kinds of items, twenty of each.\p"
	.string "Whatever you take in, you can lose!$"

Stormwild_Text_PackWhich:
	.string "What will you bring? ({STR_VAR_1}/10 packed)\n"
	.string "Press B when you're done.$"

Stormwild_Text_Ready:
	.string "The rest of your bag stays with me.\n"
	.string "Ready to go in?$"

Stormwild_Text_Unpacked:
	.string "I've put it all back in your bag.$"

Stormwild_Text_Pickup:
	.string "The beacon flares! A rescue flight\n"
	.string "is coming down through the storm…$"

Stormwild_Text_TimeUp:
	.string "The storm closes in! Lightning\n"
	.string "everywhere!$"

Stormwild_Text_LeftZone:
	.string "You left the STORMWILD without\n"
	.string "reaching a beacon!$"

Stormwild_Text_Lost:
	.string "Your last run in the STORMWILD was\n"
	.string "cut short…$"

Stormwild_Text_Welcome:
	.string "STORM WARDEN: Welcome back! Your bag\n"
	.string "was waiting for you, and it's all\l"
	.string "yours to keep.$"

Stormwild_Text_Rescued:
	.string "STORM WARDEN: We found you out cold\n"
	.string "by the gate. Here's the bag you left\l"
	.string "with me.$"
