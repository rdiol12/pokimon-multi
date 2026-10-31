@ Second batch of online features, opened from the Online Plaza.
@ See src/netplay_ext.c, src/netplay_ext_features.c and docs/netplay_ext.md.

.macro npx_request req:req
	setvar VAR_0x8004, \req
	special NetplayExt_Request
.endm

@ Shows the server's reason and ends the script.
.macro npx_request_or_fail req:req
	npx_request \req
	goto_if_ne VAR_RESULT, NETPLAY_EXT_OK, NetplayExt_EventScript_Failed
.endm

.macro npx_board board:req
	setvar VAR_0x8004, \board
	special NetplayExt_ShowBoard
.endm

@ icon: the PLAZA_* feature whose icon goes beside the list.
.macro npx_choose list:req, icon:req
	setvar VAR_0x8004, \list
	setvar VAR_0x8008, \icon - 1
	special NetplayExt_ChooseRow
.endm

NetplayExt_EventScript_Plaza::
	lockall
	special NetplayExt_OpenPlaza
	switch VAR_RESULT
	case PLAZA_GUILDS, NetplayExt_EventScript_Guilds
	case PLAZA_WONDER, NetplayExt_EventScript_Wonder
	case PLAZA_BASES, NetplayExt_EventScript_Bases
	case PLAZA_TOWER, NetplayExt_EventScript_Tower
	case PLAZA_BOUNTIES, NetplayExt_EventScript_Bounties
	case PLAZA_TITLES, NetplayExt_EventScript_Titles
	case PLAZA_REPLAYS, NetplayExt_EventScript_Replays
	case PLAZA_DRAFT, NetplayExt_EventScript_Draft
	case PLAZA_MAIL, NetplayExt_EventScript_Mail
	case PLAZA_DEX, NetplayExt_EventScript_Dex
	releaseall
	end

@ For NPCs on the map (Starfall Isle): open one feature directly.
Netplay_EventScript_OpenTower::
	lockall
	goto NetplayExt_EventScript_Tower
	end

Netplay_EventScript_OpenBountyBoard::
	lockall
	goto NetplayExt_EventScript_Bounties
	end

NetplayExt_EventScript_Failed:
	msgbox gStringVar4, MSGBOX_DEFAULT
NetplayExt_EventScript_End:
	closemessage
	releaseall
	end

NetplayExt_EventScript_Connecting:
	message NetplayExt_Text_Connecting
	waitmessage
	return

@ Guilds ------------------------------------------------------------------

NetplayExt_EventScript_Guilds:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_GUILD_INFO
	special NetplayExt_CountRows
	goto_if_eq VAR_RESULT, 0, NetplayExt_EventScript_NoGuild
	closemessage
	npx_board NPX_BOARD_GUILD
	message NetplayExt_Text_GuildWhat
	waitmessage
	dynmultipush NetplayExt_Text_OptGuildladder, 0
	dynmultipush NetplayExt_Text_OptLeaveguild, 1
	dynmultipush NetplayExt_Text_OptCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_GuildLadder
	case 1, NetplayExt_EventScript_GuildLeave
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NoGuild:
	message NetplayExt_Text_NoGuild
	waitmessage
	dynmultipush NetplayExt_Text_OptFoundaguild, 0
	dynmultipush NetplayExt_Text_OptJoinaguild, 1
	dynmultipush NetplayExt_Text_OptGuildladder, 2
	dynmultipush NetplayExt_Text_OptCancel, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_GuildCreate
	case 1, NetplayExt_EventScript_GuildJoin
	case 2, NetplayExt_EventScript_GuildLadder
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_GuildLadder:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_GUILD_LIST
	closemessage
	npx_board NPX_BOARD_GUILD_LADDER
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_GuildCreate:
	msgbox NetplayExt_Text_GuildNameIt, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 0
	special NetplayExt_NameGuild
	waitstate
	setvar VAR_0x8004, 1
	special NetplayExt_NameGuild
	waitstate
	special NetplayExt_CheckGuildName
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_End
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_GUILD_CREATE
	playfanfare MUS_OBTAIN_BADGE
	message NetplayExt_Text_GuildFounded
	waitfanfare
	waitmessage
	waitbuttonpress
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_GuildJoin:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_GUILD_LIST
	special NetplayExt_CountRows
	goto_if_eq VAR_RESULT, 0, NetplayExt_EventScript_NoGuildsYet
	message NetplayExt_Text_GuildPick
	waitmessage
	npx_choose NPX_LIST_GUILDS, PLAZA_GUILDS
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	msgbox NetplayExt_Text_GuildJoinConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NetplayExt_EventScript_End
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_GUILD_JOIN
	playfanfare MUS_OBTAIN_ITEM
	message NetplayExt_Text_GuildJoined
	waitfanfare
	waitmessage
	waitbuttonpress
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NoGuildsYet:
	msgbox NetplayExt_Text_NoGuildsYet, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_GuildLeave:
	msgbox NetplayExt_Text_GuildLeaveConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NetplayExt_EventScript_End
	npx_request_or_fail NPX_REQ_GUILD_LEAVE
	msgbox NetplayExt_Text_GuildLeft, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

@ Titles ------------------------------------------------------------------

NetplayExt_EventScript_Titles:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_TITLE_LIST
	message NetplayExt_Text_TitlePick
	waitmessage
	npx_choose NPX_LIST_TITLES, PLAZA_TITLES
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	npx_request_or_fail NPX_REQ_TITLE_SET
	playse SE_SUCCESS
	msgbox NetplayExt_Text_TitleSet, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

@ Pokédex race ------------------------------------------------------------

NetplayExt_EventScript_Dex:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_DEX_SUBMIT
	npx_request_or_fail NPX_REQ_DEX_BOARD
	closemessage
	npx_board NPX_BOARD_DEX
	goto NetplayExt_EventScript_End
	end

@ Wonder Trade ------------------------------------------------------------

NetplayExt_EventScript_Wonder:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_WONDER_CHECK
	goto_if_eq VAR_0x8005, TRUE, NetplayExt_EventScript_WonderGotOne
	goto_if_eq VAR_0x8006, TRUE, NetplayExt_EventScript_WonderStillWaiting
	msgbox NetplayExt_Text_WonderIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NetplayExt_EventScript_End
	special ChoosePartyMon
	copyvar VAR_0x8005, VAR_0x8004
	special NetplayExt_CheckOfferedMon
	goto_if_eq VAR_RESULT, 1, NetplayExt_EventScript_End
	goto_if_eq VAR_RESULT, 2, NetplayExt_EventScript_NoEggs
	goto_if_eq VAR_RESULT, 3, NetplayExt_EventScript_LastMon
	msgbox NetplayExt_Text_WonderConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NetplayExt_EventScript_End
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_WONDER_SEND
	playse SE_M_SKY_UPPERCUT
	goto_if_eq VAR_0x8005, FALSE, NetplayExt_EventScript_WonderSent
	msgbox NetplayExt_Text_WonderSentFor, MSGBOX_DEFAULT
NetplayExt_EventScript_WonderGotOne:
	playfanfare MUS_OBTAIN_ITEM
	message NetplayExt_Text_WonderReceived
	waitfanfare
	waitmessage
	waitbuttonpress
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_WonderSent:
	msgbox NetplayExt_Text_WonderSent, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_WonderStillWaiting:
	msgbox NetplayExt_Text_WonderStillWaiting, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NoEggs:
	msgbox NetplayExt_Text_NoEggs, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_LastMon:
	msgbox NetplayExt_Text_LastMon, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

@ Bounties ----------------------------------------------------------------

NetplayExt_EventScript_Bounties:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_BOUNTY_COLLECT
	goto_if_eq VAR_0x8005, TRUE, NetplayExt_EventScript_BountyDelivered
	message NetplayExt_Text_BountyWhat
	waitmessage
	dynmultipush NetplayExt_Text_OptSeetheboard, 0
	dynmultipush NetplayExt_Text_OptPostabounty, 1
	dynmultipush NetplayExt_Text_OptCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_BountyBoard
	case 1, NetplayExt_EventScript_BountyPost
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BountyDelivered:
	playfanfare MUS_OBTAIN_ITEM
	message NetplayExt_Text_BountyDelivered
	waitfanfare
	waitmessage
	waitbuttonpress
	goto NetplayExt_EventScript_Bounties
	end

NetplayExt_EventScript_BountyBoard:
	npx_request_or_fail NPX_REQ_BOUNTY_LIST
	special NetplayExt_CountRows
	goto_if_eq VAR_RESULT, 0, NetplayExt_EventScript_NoBounties
	message NetplayExt_Text_BountyPick
	waitmessage
	npx_choose NPX_LIST_BOUNTIES, PLAZA_BOUNTIES
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	goto_if_eq VAR_0x8007, TRUE, NetplayExt_EventScript_BountyMine
	msgbox NetplayExt_Text_BountyHandIn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NetplayExt_EventScript_End
	special ChoosePartyMon
	copyvar VAR_0x8005, VAR_0x8004
	special NetplayExt_CheckOfferedMon
	goto_if_eq VAR_RESULT, 1, NetplayExt_EventScript_End
	goto_if_eq VAR_RESULT, 2, NetplayExt_EventScript_NoEggs
	goto_if_eq VAR_RESULT, 3, NetplayExt_EventScript_LastMon
	special NetplayExt_CheckBountyMon
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_BountyWrongMon
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_BOUNTY_FILL
	playse SE_SHOP
	msgbox NetplayExt_Text_BountyPaid, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BountyWrongMon:
	msgbox NetplayExt_Text_BountyWrongMon, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BountyMine:
	msgbox NetplayExt_Text_BountyCancelConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NetplayExt_EventScript_End
	npx_request_or_fail NPX_REQ_BOUNTY_CANCEL
	msgbox NetplayExt_Text_BountyRefunded, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NoBounties:
	msgbox NetplayExt_Text_NoBounties, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BountyPost:
	message NetplayExt_Text_BountyWhich
	waitmessage
	npx_choose NPX_LIST_SPECIES, PLAZA_BOUNTIES
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	message NetplayExt_Text_BountyReward
	waitmessage
	dynmultipush NetplayExt_Text_OptYen500, 0
	dynmultipush NetplayExt_Text_OptYen1000, 1
	dynmultipush NetplayExt_Text_OptYen3000, 2
	dynmultipush NetplayExt_Text_OptYen10000, 3
	dynmultipush NetplayExt_Text_OptCancel, 4
	dynmultistack 0, 0, FALSE, 5, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_Reward5
	case 1, NetplayExt_EventScript_Reward10
	case 2, NetplayExt_EventScript_Reward30
	case 3, NetplayExt_EventScript_Reward100
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_Reward5:
	setvar VAR_0x8006, 5
	goto NetplayExt_EventScript_BountyPostIt
NetplayExt_EventScript_Reward10:
	setvar VAR_0x8006, 10
	goto NetplayExt_EventScript_BountyPostIt
NetplayExt_EventScript_Reward30:
	setvar VAR_0x8006, 30
	goto NetplayExt_EventScript_BountyPostIt
NetplayExt_EventScript_Reward100:
	setvar VAR_0x8006, 100
NetplayExt_EventScript_BountyPostIt:
	special NetplayExt_CheckMoney
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_NotEnoughMoney
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_BOUNTY_POST
	playse SE_SHOP
	msgbox NetplayExt_Text_BountyPosted, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NotEnoughMoney:
	msgbox NetplayExt_Text_NotEnoughMoney, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

@ Mail and gifts ----------------------------------------------------------

NetplayExt_EventScript_Mail:
	special NetplayExt_ClearNewMail
	message NetplayExt_Text_MailWhat
	waitmessage
	dynmultipush NetplayExt_Text_OptReadmail, 0
	dynmultipush NetplayExt_Text_OptSendmail, 1
	dynmultipush NetplayExt_Text_OptCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_Inbox
	case 1, NetplayExt_EventScript_MailSend
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_Inbox:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_MAIL_INBOX
	special NetplayExt_CountRows
	goto_if_eq VAR_RESULT, 0, NetplayExt_EventScript_NoMail
	message NetplayExt_Text_MailPick
	waitmessage
	npx_choose NPX_LIST_INBOX, PLAZA_MAIL
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	special NetplayExt_CheckGiftRoom
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_BagFull
	npx_request_or_fail NPX_REQ_MAIL_OPEN
	msgbox NetplayExt_Text_MailRead, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8005, FALSE, NetplayExt_EventScript_Inbox
	playfanfare MUS_OBTAIN_ITEM
	message NetplayExt_Text_MailGift
	waitfanfare
	waitmessage
	waitbuttonpress
	goto NetplayExt_EventScript_Inbox
	end

NetplayExt_EventScript_NoMail:
	msgbox NetplayExt_Text_NoMail, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BagFull:
	msgbox NetplayExt_Text_BagFull, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_MailSend:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_MAIL_WHO
	special NetplayExt_CountRows
	goto_if_eq VAR_RESULT, 0, NetplayExt_EventScript_NobodyToMail
	message NetplayExt_Text_MailWho
	waitmessage
	npx_choose NPX_LIST_PLAYERS, PLAZA_MAIL
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	message NetplayExt_Text_MailPhrase
	waitmessage
	npx_choose NPX_LIST_PHRASES, PLAZA_MAIL
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	setvar VAR_0x8006, 0
	setvar VAR_0x8007, ITEM_NONE
	message NetplayExt_Text_MailGiftWhat
	waitmessage
	dynmultipush NetplayExt_Text_OptNogift, 0
	dynmultipush NetplayExt_Text_OptMoney, 1
	dynmultipush NetplayExt_Text_OptAnitem, 2
	dynmultipush NetplayExt_Text_OptCancel, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 1, NetplayExt_EventScript_GiftMoney
	case 2, NetplayExt_EventScript_GiftItem
	case 3, NetplayExt_EventScript_End
	case MULTI_B_PRESSED, NetplayExt_EventScript_End
	goto NetplayExt_EventScript_MailSendIt
	end

NetplayExt_EventScript_GiftMoney:
	message NetplayExt_Text_MailHowMuch
	waitmessage
	dynmultipush NetplayExt_Text_OptYen500, 0
	dynmultipush NetplayExt_Text_OptYen1000, 1
	dynmultipush NetplayExt_Text_OptYen3000, 2
	dynmultipush NetplayExt_Text_OptYen10000, 3
	dynmultipush NetplayExt_Text_OptCancel, 4
	dynmultistack 0, 0, FALSE, 5, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_Gift5
	case 1, NetplayExt_EventScript_Gift10
	case 2, NetplayExt_EventScript_Gift30
	case 3, NetplayExt_EventScript_Gift100
	goto NetplayExt_EventScript_End
	end
NetplayExt_EventScript_Gift5:
	setvar VAR_0x8006, 5
	goto NetplayExt_EventScript_MailSendIt
NetplayExt_EventScript_Gift10:
	setvar VAR_0x8006, 10
	goto NetplayExt_EventScript_MailSendIt
NetplayExt_EventScript_Gift30:
	setvar VAR_0x8006, 30
	goto NetplayExt_EventScript_MailSendIt
NetplayExt_EventScript_Gift100:
	setvar VAR_0x8006, 100
	goto NetplayExt_EventScript_MailSendIt

NetplayExt_EventScript_GiftItem:
	message NetplayExt_Text_MailWhichItem
	waitmessage
	npx_choose NPX_LIST_GIFT_ITEM, PLAZA_MAIL
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
NetplayExt_EventScript_MailSendIt:
	special NetplayExt_CheckGift
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_NotEnoughMoney
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_MAIL_SEND
	playse SE_POKENAV_CALL
	msgbox NetplayExt_Text_MailSent, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NobodyToMail:
	msgbox NetplayExt_Text_NobodyToMail, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

@ Secret bases ------------------------------------------------------------

NetplayExt_EventScript_Bases:
	message NetplayExt_Text_BaseWhat
	waitmessage
	dynmultipush NetplayExt_Text_OptSharemybase, 0
	dynmultipush NetplayExt_Text_OptVisitabase, 1
	dynmultipush NetplayExt_Text_OptCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_BaseShare
	case 1, NetplayExt_EventScript_BaseVisit
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BaseShare:
	special NetplayExt_HasSecretBase
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_NoBase
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_BASE_PUT
	playse SE_SUCCESS
	msgbox NetplayExt_Text_BaseShared, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NoBase:
	msgbox NetplayExt_Text_NoBase, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BaseVisit:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_BASE_LIST
	special NetplayExt_CountRows
	goto_if_eq VAR_RESULT, 0, NetplayExt_EventScript_NoBases
	message NetplayExt_Text_BasePick
	waitmessage
	npx_choose NPX_LIST_BASES, PLAZA_BASES
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	npx_request_or_fail NPX_REQ_BASE_GET
	goto_if_eq VAR_0x8005, FALSE, NetplayExt_EventScript_BadData
	message NetplayExt_Text_BaseVisitHow
	waitmessage
	dynmultipush NetplayExt_Text_OptGoinside, 0
	dynmultipush NetplayExt_Text_OptBattlenow, 1
	dynmultipush NetplayExt_Text_OptCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_BaseGoIn
	case 1, NetplayExt_EventScript_BaseBattle
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BaseGoIn:
	special NetplayExt_VisitBase
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_BaseSameSpot
	closemessage
	setvar VAR_INIT_SECRET_BASE, 1
	playse SE_EXIT
	clearflag FLAG_HIDE_SECRET_BASE_TRAINER
	special EnterSecretBase
	setvar VAR_SECRET_BASE_INITIALIZED, 0
	waitstate
	end

NetplayExt_EventScript_BaseSameSpot:
	msgbox NetplayExt_Text_BaseSameSpot, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BaseBattle:
	msgbox NetplayExt_Text_BaseBattleIntro, MSGBOX_DEFAULT
	closemessage
	call NetplayExt_EventScript_StoredBattle
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, NetplayExt_EventScript_BaseWon
	msgbox NetplayExt_Text_BaseLost, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BaseWon:
	msgbox NetplayExt_Text_BaseWon, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NoBases:
	msgbox NetplayExt_Text_NoBases, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_BadData:
	msgbox NetplayExt_Text_BadData, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

@ A battle against the downloaded team. VAR_RESULT: the battle outcome.
NetplayExt_EventScript_StoredBattle:
	special NetplayExt_PrepStoredBattle
	setvar VAR_0x8004, SPECIAL_BATTLE_SECRET_BASE
	setvar VAR_0x8005, 0
	special DoSpecialTrainerBattle
	special NetplayExt_EndStoredBattle
	return

@ Battle Tower ------------------------------------------------------------

NetplayExt_EventScript_Tower:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_TOWER_STATE
	msgbox NetplayExt_Text_TowerWelcome, MSGBOX_DEFAULT
	message NetplayExt_Text_TowerWhat
	waitmessage
	dynmultipush NetplayExt_Text_OptClimb, 0
	dynmultipush NetplayExt_Text_OptRecords, 1
	dynmultipush NetplayExt_Text_OptCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, NetplayExt_EventScript_TowerClimb
	case 1, NetplayExt_EventScript_TowerRecords
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_TowerRecords:
	npx_request_or_fail NPX_REQ_TOWER_BOARD
	closemessage
	npx_board NPX_BOARD_TOWER
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_TowerClimb:
	goto_if_ne VAR_0x8005, 0, NetplayExt_EventScript_TowerFloor
	msgbox NetplayExt_Text_TowerTeam, MSGBOX_DEFAULT
	npx_request_or_fail NPX_REQ_TOWER_START
NetplayExt_EventScript_TowerFloor:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_TOWER_NEXT
	goto_if_eq VAR_0x8005, FALSE, NetplayExt_EventScript_BadData
	msgbox NetplayExt_Text_TowerOpponent, MSGBOX_DEFAULT
	closemessage
	call NetplayExt_EventScript_StoredBattle
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, NetplayExt_EventScript_TowerWon
	npx_request_or_fail NPX_REQ_TOWER_LOST
	msgbox NetplayExt_Text_TowerLost, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_TowerWon:
	npx_request_or_fail NPX_REQ_TOWER_WON
	playfanfare MUS_OBTAIN_SYMBOL
	message NetplayExt_Text_TowerCleared
	waitfanfare
	waitmessage
	msgbox NetplayExt_Text_TowerContinue, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, NetplayExt_EventScript_TowerFloor
	msgbox NetplayExt_Text_TowerSaved, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

@ Draft battles -----------------------------------------------------------

NetplayExt_EventScript_Draft:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_DRAFT_POOL
	closemessage
	npx_board NPX_BOARD_DRAFT_POOL
	msgbox NetplayExt_Text_DraftStart, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NetplayExt_EventScript_End
NetplayExt_EventScript_DraftPick:
	special NetplayExt_DraftPickCount
	goto_if_ge VAR_RESULT, 3, NetplayExt_EventScript_DraftReady
	message NetplayExt_Text_DraftPick
	waitmessage
	npx_choose NPX_LIST_DRAFT, PLAZA_DRAFT
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	playse SE_SELECT
	goto NetplayExt_EventScript_DraftPick
	end

NetplayExt_EventScript_DraftReady:
	special NetplayExt_DraftBegin
	call NetplayExt_EventScript_Connecting
	npx_request NPX_REQ_DRAFT_QUEUE
	goto_if_ne VAR_RESULT, NETPLAY_EXT_OK, NetplayExt_EventScript_DraftFailed
	message NetplayExt_Text_DraftWaiting
	waitmessage
	special NetplayExt_DraftWait
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_DraftNoMatch
	setvar VAR_0x8004, NETPLAY_KIND_BATTLE
	goto Netplay_EventScript_StartActivity
	end

NetplayExt_EventScript_DraftNoMatch:
	npx_request NPX_REQ_DRAFT_LEAVE
	special NetplayExt_DraftCancel
	msgbox NetplayExt_Text_DraftNoMatch, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_DraftFailed:
	special NetplayExt_DraftCancel
	goto NetplayExt_EventScript_Failed
	end

@ Replays -----------------------------------------------------------------

NetplayExt_EventScript_Replays:
	call NetplayExt_EventScript_Connecting
	npx_request_or_fail NPX_REQ_REPLAY_LIST
	special NetplayExt_CountRows
	goto_if_eq VAR_RESULT, 0, NetplayExt_EventScript_NoReplays
	message NetplayExt_Text_ReplayPick
	waitmessage
	npx_choose NPX_LIST_REPLAYS, PLAZA_REPLAYS
	goto_if_eq VAR_RESULT, NPX_CHOSE_NOTHING, NetplayExt_EventScript_End
	message NetplayExt_Text_ReplayLoading
	waitmessage
	special NetplayExt_DownloadReplay
	goto_if_eq VAR_RESULT, FALSE, NetplayExt_EventScript_BadData
	closemessage
	special NetplayExt_PlayReplay
	waitstate
	msgbox NetplayExt_Text_ReplayOver, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

NetplayExt_EventScript_NoReplays:
	msgbox NetplayExt_Text_NoReplays, MSGBOX_DEFAULT
	goto NetplayExt_EventScript_End
	end

@ Text --------------------------------------------------------------------

NetplayExt_Text_Connecting:
	.string "Talking to the server…$"

NetplayExt_Text_GuildWhat:
	.string "What would you like to do?$"

NetplayExt_Text_NoGuild:
	.string "You aren't in a guild yet.\n"
	.string "Found one, or join a friend's?$"

NetplayExt_Text_GuildNameIt:
	.string "First pick your guild's tag: 2 to 4\n"
	.string "letters shown next to members' names.\p"
	.string "Then give your guild a name.$"

NetplayExt_Text_GuildFounded:
	.string "You founded ({STR_VAR_1}) {STR_VAR_2}!\n"
	.string "Invite your friends to join.$"

NetplayExt_Text_GuildPick:
	.string "Which guild would you like to join?$"

NetplayExt_Text_GuildJoinConfirm:
	.string "Join ({STR_VAR_1}) {STR_VAR_2}?$"

NetplayExt_Text_GuildJoined:
	.string "You joined ({STR_VAR_1}) {STR_VAR_2}!$"

NetplayExt_Text_NoGuildsYet:
	.string "Nobody has founded a guild yet.\n"
	.string "Why not be the first?$"

NetplayExt_Text_GuildLeaveConfirm:
	.string "Leave your guild?\n"
	.string "You can join another one after.$"

NetplayExt_Text_GuildLeft:
	.string "You left the guild.$"

NetplayExt_Text_TitlePick:
	.string "Which title would you like to show\n"
	.string "above your name?$"

NetplayExt_Text_TitleSet:
	.string "Other players now see you as\n"
	.string "{STR_VAR_1}!$"

NetplayExt_Text_WonderIntro:
	.string "Wonder Trade sends a POKéMON to a\n"
	.string "random player for a random one.\p"
	.string "You won't know what you get until it\n"
	.string "arrives. Send one?$"

NetplayExt_Text_WonderConfirm:
	.string "Send {STR_VAR_1} off on a Wonder Trade?$"

NetplayExt_Text_WonderSentFor:
	.string "{STR_VAR_3} was sent off!\n"
	.string "Another trainer sent one back…$"

NetplayExt_Text_WonderReceived:
	.string "You received {STR_VAR_2} from\n"
	.string "{STR_VAR_1}!$"

NetplayExt_Text_WonderSent:
	.string "{STR_VAR_3} was sent off!\p"
	.string "Nobody else is trading right now.\n"
	.string "Check back later for your POKéMON.$"

NetplayExt_Text_WonderStillWaiting:
	.string "Your POKéMON is still waiting for a\n"
	.string "trade partner. Check back later!$"

NetplayExt_Text_NoEggs:
	.string "EGGS can't be sent.$"

NetplayExt_Text_LastMon:
	.string "That's your last POKéMON!$"

NetplayExt_Text_BountyWhat:
	.string "The bounty board. Post a reward for a\n"
	.string "POKéMON, or claim someone's reward.$"

NetplayExt_Text_BountyDelivered:
	.string "{STR_VAR_1} filled your bounty!\n"
	.string "You received {STR_VAR_2}!$"

NetplayExt_Text_BountyPick:
	.string "Which bounty?$"

NetplayExt_Text_BountyHandIn:
	.string "{STR_VAR_3} will pay ¥{STR_VAR_2} for\n"
	.string "{STR_VAR_1}. Hand one in?$"

NetplayExt_Text_BountyWrongMon:
	.string "That's not the POKéMON they want.$"

NetplayExt_Text_BountyPaid:
	.string "Bounty claimed! You were paid\n"
	.string "¥{STR_VAR_2}.$"

NetplayExt_Text_BountyCancelConfirm:
	.string "That's your bounty for {STR_VAR_1}.\n"
	.string "Take it down and get the money back?$"

NetplayExt_Text_BountyRefunded:
	.string "Your bounty was taken down and\n"
	.string "¥{STR_VAR_2} was refunded.$"

NetplayExt_Text_NoBounties:
	.string "There are no bounties up right now.$"

NetplayExt_Text_BountyWhich:
	.string "Which POKéMON do you want?\n"
	.string "(ones you've seen but not caught)$"

NetplayExt_Text_BountyReward:
	.string "How big a reward for {STR_VAR_1}?$"

NetplayExt_Text_BountyPosted:
	.string "Your bounty is up! ¥{STR_VAR_2} for\n"
	.string "{STR_VAR_1}.$"

NetplayExt_Text_NotEnoughMoney:
	.string "You don't have enough money.$"

NetplayExt_Text_MailWhat:
	.string "Mail and gifts. Read your mail,\n"
	.string "or send some to a friend?$"

NetplayExt_Text_MailPick:
	.string "Which mail would you like to read?$"

NetplayExt_Text_MailRead:
	.string "{STR_VAR_1} wrote:\n"
	.string "“{STR_VAR_2}”$"

NetplayExt_Text_MailGift:
	.string "It came with {STR_VAR_3}!$"

NetplayExt_Text_NoMail:
	.string "There's no new mail.$"

NetplayExt_Text_BagFull:
	.string "Your BAG is too full for the gift.\n"
	.string "Make some room first.$"

NetplayExt_Text_MailWho:
	.string "Who would you like to write to?$"

NetplayExt_Text_MailPhrase:
	.string "What would you like to say?$"

NetplayExt_Text_MailGiftWhat:
	.string "Send a gift with it?$"

NetplayExt_Text_MailHowMuch:
	.string "How much money?$"

NetplayExt_Text_MailWhichItem:
	.string "Which item would you like to send?$"

NetplayExt_Text_MailSent:
	.string "Your mail to {STR_VAR_1} was sent!$"

NetplayExt_Text_NobodyToMail:
	.string "There's nobody to write to yet.\n"
	.string "Players online and guildmates show up.$"

NetplayExt_Text_BaseWhat:
	.string "Share your secret base with other\n"
	.string "players, or visit theirs?$"

NetplayExt_Text_BaseShared:
	.string "Your secret base and team are shared!\n"
	.string "Others can visit and battle them.$"

NetplayExt_Text_NoBase:
	.string "You don't have a secret base yet.\n"
	.string "Use SECRET POWER to make one.$"

NetplayExt_Text_BasePick:
	.string "Whose base would you like to visit?$"

NetplayExt_Text_BaseVisitHow:
	.string "{STR_VAR_1}'s base. Go in and look\n"
	.string "around, or battle their team now?$"

NetplayExt_Text_BaseSameSpot:
	.string "Their base is where yours is, so it\n"
	.string "can't be visited. Battle them instead!$"

NetplayExt_Text_BaseBattleIntro:
	.string "{STR_VAR_1}'s team is waiting for you!$"

NetplayExt_Text_BaseWon:
	.string "You beat {STR_VAR_1}'s team!$"

NetplayExt_Text_BaseLost:
	.string "{STR_VAR_1}'s team was too strong…$"

NetplayExt_Text_NoBases:
	.string "Nobody has shared a base yet.$"

NetplayExt_Text_BadData:
	.string "That couldn't be loaded.\n"
	.string "It may be from another version.$"

NetplayExt_Text_TowerWelcome:
	.string "Welcome to the online BATTLE TOWER!\p"
	.string "Each floor is another player's team.\n"
	.string "Lose once and the climb is over.\p"
	.string "Your next floor: {STR_VAR_1}.\n"
	.string "Your best: {STR_VAR_2} floors cleared.$"

NetplayExt_Text_TowerWhat:
	.string "What would you like to do?$"

NetplayExt_Text_TowerTeam:
	.string "Your current party will be your team,\n"
	.string "and the team others meet in the TOWER.$"

NetplayExt_Text_TowerOpponent:
	.string "Floor {STR_VAR_1}!\n"
	.string "{STR_VAR_2}'s team awaits!$"

NetplayExt_Text_TowerCleared:
	.string "Floor cleared!$"

NetplayExt_Text_TowerContinue:
	.string "Go on to the next floor?$"

NetplayExt_Text_TowerSaved:
	.string "Your climb is saved. Come back to\n"
	.string "take on the next floor!$"

NetplayExt_Text_TowerLost:
	.string "Your climb is over.\n"
	.string "Best: {STR_VAR_2} floors cleared.$"

NetplayExt_Text_DraftStart:
	.string "Draft three of today's POKéMON and\n"
	.string "battle another drafter. Start?$"

NetplayExt_Text_DraftPick:
	.string "Pick a POKéMON for your team.$"

NetplayExt_Text_DraftWaiting:
	.string "Looking for another drafter…\n"
	.string "Press B to stop.$"

NetplayExt_Text_DraftNoMatch:
	.string "Nobody else is drafting right now.\n"
	.string "Your party is back.$"

NetplayExt_Text_ReplayPick:
	.string "Which battle would you like to watch?$"

NetplayExt_Text_ReplayLoading:
	.string "Loading the replay…$"

NetplayExt_Text_ReplayOver:
	.string "That was the replay.$"

NetplayExt_Text_NoReplays:
	.string "No replays yet. Every online battle\n"
	.string "you play is saved here.$"

NetplayExt_Text_OptGuildladder:
	.string "Guild ladder$"

NetplayExt_Text_OptLeaveguild:
	.string "Leave guild$"

NetplayExt_Text_OptCancel:
	.string "Cancel$"

NetplayExt_Text_OptFoundaguild:
	.string "Found a guild$"

NetplayExt_Text_OptJoinaguild:
	.string "Join a guild$"

NetplayExt_Text_OptSeetheboard:
	.string "See the board$"

NetplayExt_Text_OptPostabounty:
	.string "Post a bounty$"

NetplayExt_Text_OptYen500:
	.string "¥500$"

NetplayExt_Text_OptYen1000:
	.string "¥1,000$"

NetplayExt_Text_OptYen3000:
	.string "¥3,000$"

NetplayExt_Text_OptYen10000:
	.string "¥10,000$"

NetplayExt_Text_OptReadmail:
	.string "Read mail$"

NetplayExt_Text_OptSendmail:
	.string "Send mail$"

NetplayExt_Text_OptNogift:
	.string "No gift$"

NetplayExt_Text_OptMoney:
	.string "Money$"

NetplayExt_Text_OptAnitem:
	.string "An item$"

NetplayExt_Text_OptSharemybase:
	.string "Share my base$"

NetplayExt_Text_OptVisitabase:
	.string "Visit a base$"

NetplayExt_Text_OptGoinside:
	.string "Go inside$"

NetplayExt_Text_OptBattlenow:
	.string "Battle now$"

NetplayExt_Text_OptClimb:
	.string "Climb$"

NetplayExt_Text_OptRecords:
	.string "Records$"
