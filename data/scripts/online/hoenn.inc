@ Original Hoenn for many players (docs/hoenn_multi.md, src/hoenn_multi.c,
@ server/online_hoenn.py): gym record boards and live battles at the gym
@ statues, HUB > HOENN RECORDS and TEAM UP, the quick-match desk upstairs in
@ every Pokémon Center, and the second nurse.

@ Each gym's statues, after their own text. VAR_0x8005: the gym, 0-7.
HoennMulti_EventScript_GymStatue::
	callnative HoennMulti_IsOnline
	goto_if_eq VAR_RESULT, FALSE, HoennMulti_EventScript_Return
	message HoennMulti_Text_StatueOnline
	waitmessage
	dynmultipush HoennMulti_Text_Records, 0
	dynmultipush HoennMulti_Text_WatchLive, 1
	dynmultipush Online_Text_HubClose, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, HoennMulti_EventScript_GymRecords
	goto_if_eq VAR_RESULT, 1, HoennMulti_EventScript_WatchLive
	closemessage
HoennMulti_EventScript_Return:
	return

HoennMulti_EventScript_GymRecords:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HREC
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	return

HoennMulti_EventScript_WatchLive:
	closemessage
	callnative HoennMulti_WatchLive
	waitstate
	return

HoennMulti_EventScript_HubRecords::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HREC_ALL
	closemessage
	onl_board
	goto Online_EventScript_End

@ HUB > TEAM UP: lend your team to another trainer, or bring theirs along.
HoennMulti_EventScript_TeamUp::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HTEAM_VIEW
	switch VAR_0x800B
	case HTEAM_INVITED, HoennMulti_EventScript_TeamInvited
	case HTEAM_ASKED, Online_EventScript_ShowMessage
	case HTEAM_READY, HoennMulti_EventScript_TeamReady
	case HTEAM_LENT, HoennMulti_EventScript_TeamLent
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox HoennMulti_Text_AskForTeam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HTEAM_WHO
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HTEAM_ASK
	goto Online_EventScript_ShowMessage

HoennMulti_EventScript_TeamInvited:
	msgbox gStringVar4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennMulti_EventScript_TeamDecline
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HTEAM_YES
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

HoennMulti_EventScript_TeamDecline:
	onl_request ONL_REQ_HTEAM_NO
	goto Online_EventScript_End

HoennMulti_EventScript_TeamReady:
	callnative HoennMulti_HasTeam
	goto_if_eq VAR_RESULT, TRUE, HoennMulti_EventScript_TeamBringing
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HTEAM_GET
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

HoennMulti_EventScript_TeamBringing:
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox HoennMulti_Text_GiveBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	goto HoennMulti_EventScript_TeamEnd

HoennMulti_EventScript_TeamLent:
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox HoennMulti_Text_TakeBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
HoennMulti_EventScript_TeamEnd:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_HTEAM_END
	goto Online_EventScript_ShowMessage

@ The left desk upstairs in every Pokémon Center (it was the link cable's
@ Wireless Club): finds another trainer online for a battle or a trade.
HoennMulti_EventScript_QuickMatchDesk::
	lock
	faceplayer
	callnative HoennMulti_IsOnline
	goto_if_eq VAR_RESULT, FALSE, HoennMulti_EventScript_DeskOffline
	message HoennMulti_Text_DeskWelcome
	waitmessage
	dynmultipush HoennMulti_Text_DeskBattle, 0
	dynmultipush HoennMulti_Text_DeskTrade, 1
	dynmultipush HoennMulti_Text_DeskLeave, 2
	dynmultipush Online_Text_HubClose, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 2, HoennMulti_EventScript_DeskLeave
	goto_if_gt VAR_RESULT, 1, HoennMulti_EventScript_DeskBye
	copyvar VAR_0x8005, VAR_RESULT
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_QM_JOIN
	goto_if_eq VAR_0x800B, QM_WAITING, Online_EventScript_ShowMessage
	msgbox gStringVar4, MSGBOX_DEFAULT
	setvar VAR_RESULT, NETPLAY_KIND_BATTLE
	goto_if_eq VAR_0x8005, 0, Netplay_EventScript_SendInvite
	setvar VAR_RESULT, NETPLAY_KIND_TRADE
	goto Netplay_EventScript_SendInvite

HoennMulti_EventScript_DeskLeave:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_QM_LEAVE
	goto Online_EventScript_ShowMessage

HoennMulti_EventScript_DeskOffline:
	msgbox HoennMulti_Text_DeskOffline, MSGBOX_DEFAULT
HoennMulti_EventScript_DeskBye:
	closemessage
	release
	end

@ A second nurse in the Pokémon Center's new east wing. No healing machine
@ there, so no animation of the Poké Balls.
HoennMulti_EventScript_SecondNurse::
	lock
	faceplayer
	msgbox gText_WouldYouLikeToRestYourPkmn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennMulti_EventScript_NurseBye
	incrementgamestat GAME_STAT_USED_POKECENTER
	message gText_IllTakeYourPkmn
	waitmessage
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	callnative UpdateFollowingPokemon
	message gText_RestoredPkmnToFullHealth
	waitmessage
	waitbuttonpress
HoennMulti_EventScript_NurseBye:
	msgbox gText_WeHopeToSeeYouAgain, MSGBOX_DEFAULT
	release
	end

HoennMulti_Text_StatueOnline:
	.string "The statue's screen is linked to\n"
	.string "every GYM in HOENN.$"

HoennMulti_Text_Records:
	.string "RECORDS$"

HoennMulti_Text_WatchLive:
	.string "WATCH LIVE$"

HoennMulti_Text_AskForTeam:
	.string "Ask another trainer for their team?$"

HoennMulti_Text_GiveBack:
	.string "Give their team back?$"

HoennMulti_Text_TakeBack:
	.string "Take your team back?$"

HoennMulti_Text_DeskWelcome:
	.string "Welcome to the QUICK MATCH desk!\n"
	.string "Who should I find you?$"

HoennMulti_Text_DeskBattle:
	.string "A BATTLE$"

HoennMulti_Text_DeskTrade:
	.string "A TRADE$"

HoennMulti_Text_DeskLeave:
	.string "LEAVE THE LIST$"

HoennMulti_Text_DeskOffline:
	.string "Welcome to the QUICK MATCH desk!\p"
	.string "I find trainers from all over HOENN,\n"
	.string "but only when you JOIN ONLINE.$"
