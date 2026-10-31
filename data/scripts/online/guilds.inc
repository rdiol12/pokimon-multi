@ Guild halls, clan wars and guild base raids. HUB > GUILD HALL takes a
@ guild member to their hall; its clerk, war officer and raid captain are in
@ data/maps/OnlineGuildHall/scripts.inc.

Online_EventScript_GuildHall::
	callnative Online_InGuildHall
	goto_if_eq VAR_RESULT, TRUE, Online_EventScript_GuildInHall
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GHALL_VIEW
	goto_if_eq VAR_0x800B, GUILD_NONE, Online_EventScript_ShowMessage
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	callnative Online_SetGuildHallReturn
	playse SE_WARP_IN
	warp MAP_ONLINE_GUILD_HALL, 6, 12
	waitstate
	releaseall
	end

Online_EventScript_GuildInHall:
	msgbox Online_Text_GuildInHall, MSGBOX_DEFAULT
	goto Online_EventScript_End

@ Clerk: the members, and the guild chest.
Online_EventScript_GuildClerk::
	message Online_Text_GuildClerkHello
	waitmessage
	dynmultipush Online_Text_GuildMembers, 0
	dynmultipush Online_Text_GuildPut, 1
	dynmultipush Online_Text_GuildTake, 2
	dynmultipush gText_Cancel, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Online_EventScript_GuildMembers
	case 1, Online_EventScript_GuildPut
	case 2, Online_EventScript_GuildTake
	goto Online_EventScript_End

Online_EventScript_GuildMembers:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GHALL_MEMBERS
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto Online_EventScript_End

Online_EventScript_GuildPut:
	message Online_Text_GuildPutWhich
	waitmessage
	callnative Online_ChooseBagItem, waitstate=1
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	message Online_Text_GuildPutHowMany
	waitmessage
	call Online_EventScript_ShopCount
	goto_if_eq VAR_RESULT, 3, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GHALL_PUT
	playse SE_SHOP
	goto Online_EventScript_ShowMessage

Online_EventScript_GuildTake:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GHALL_CHEST
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	callnative Online_TakeChosenShelf
	checkitemspace VAR_0x8005, VAR_0x8006
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_ShopNoRoom
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GHALL_TAKE
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

@ War officer: this week's clan war, and last weeks' prizes.
Online_EventScript_GuildWar::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_WAR_BOARD
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	onl_board
	goto_if_eq VAR_0x800B, 0, Online_EventScript_GuildWarRules
	msgbox Online_Text_GuildWarPrize, MSGBOX_DEFAULT
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_WAR_CLAIM
	playfanfare MUS_OBTAIN_ITEM
	message gStringVar4
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Online_EventScript_End

Online_EventScript_GuildWarRules:
	msgbox Online_Text_GuildWarRules, MSGBOX_DEFAULT
	goto Online_EventScript_End

@ Raid captain: guard the guild's base, or raid another guild's.
Online_EventScript_GuildRaids::
	message Online_Text_GuildRaidHello
	waitmessage
	dynmultipush Online_Text_GuildRaid, 0
	dynmultipush Online_Text_GuildDefend, 1
	dynmultipush gText_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Online_EventScript_GuildRaid
	case 1, Online_EventScript_GuildDefend
	goto Online_EventScript_End

Online_EventScript_GuildDefend:
	msgbox Online_Text_GuildDefendSure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_RAID_DEFEND
	goto Online_EventScript_ShowMessage

Online_EventScript_GuildRaid:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_RAID_LIST
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_RAID_GET
	goto_if_eq VAR_0x8005, FALSE, NetplayExt_EventScript_BadData
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	call NetplayExt_EventScript_StoredBattle
	setvar VAR_0x8005, FALSE
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, Online_EventScript_GuildRaidOver
	setvar VAR_0x8005, TRUE
Online_EventScript_GuildRaidOver:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_RAID_RESULT
	goto Online_EventScript_ShowMessage

Online_Text_GuildInHall:
	.string "You're in your guild hall already!$"

Online_Text_GuildClerkHello:
	.string "Welcome home! What can I do for you?$"

Online_Text_GuildMembers:
	.string "MEMBERS$"

Online_Text_GuildPut:
	.string "PUT IN CHEST$"

Online_Text_GuildTake:
	.string "TAKE FROM CHEST$"

Online_Text_GuildPutWhich:
	.string "What will you put in the chest?$"

Online_Text_GuildPutHowMany:
	.string "How many {STR_VAR_1}?$"

Online_Text_GuildWarPrize:
	.string "And your guild won a prize in last\n"
	.string "week's clan war! Here you go!$"

Online_Text_GuildWarRules:
	.string "Beat other guilds' members in battle\n"
	.string "and raid their bases to score.\p"
	.string "The three best guilds of the week win\n"
	.string "prizes for every member!$"

Online_Text_GuildRaidHello:
	.string "Ready to raid, or to guard our base?$"

Online_Text_GuildRaid:
	.string "RAID A BASE$"

Online_Text_GuildDefend:
	.string "GUARD OUR BASE$"

Online_Text_GuildDefendSure:
	.string "Your best POKéMON will guard our\n"
	.string "base against raiders. All right?$"
