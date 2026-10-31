@ The guide (docs/guide.md, src/guide*.c): R > GUIDE, its notices, and the
@ social screens. Uses the online request macros from online.inc.

Guide_EventScript_Menu::
	lockall
Guide_EventScript_MenuFromR::
	dynmultipush Guide_Text_MenuList, GUIDE_MENU_LIST
	dynmultipush Guide_Text_MenuMap, GUIDE_MENU_MAP
	dynmultipush Guide_Text_MenuNearby, GUIDE_MENU_NEARBY
	dynmultipush Guide_Text_MenuInvite, GUIDE_MENU_INVITE
	dynmultipush Guide_Text_MenuClose, GUIDE_MENU_CLOSE
	dynmultistack 0, 0, FALSE, 5, FALSE, GUIDE_MENU_LIST, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case GUIDE_MENU_LIST, Guide_EventScript_List
	case GUIDE_MENU_MAP, Guide_EventScript_Map
	case GUIDE_MENU_NEARBY, Guide_EventScript_Nearby
	case GUIDE_MENU_INVITE, Guide_EventScript_Invite
	releaseall
	end

@ The list; A on an entry shows it on the map, and closing the map comes
@ back to the list.
Guide_EventScript_List::
	callnative Guide_ShowList, waitstate=1
	goto_if_eq VAR_RESULT, GUIDE_NONE, Guide_EventScript_End
	copyvar VAR_0x8004, VAR_RESULT
	call Guide_EventScript_OpenMap
	goto Guide_EventScript_List

Guide_EventScript_Map::
	setvar VAR_0x8004, GUIDE_NONE
	call Guide_EventScript_OpenMap
	goto Guide_EventScript_End

Guide_EventScript_OpenMap:
	fadescreen FADE_TO_BLACK
	callnative Guide_ShowMap
	waitstate
	return

Guide_EventScript_End::
	releaseall
	end

@ From the overworld, when a badge opens something.
Guide_EventScript_NewInGuide::
	lockall
	playfanfare MUS_OBTAIN_ITEM
	goto_if_ne VAR_0x8005, 0, Guide_EventScript_NewInGuideMore
	message Guide_Text_NewInGuide
	goto Guide_EventScript_NewInGuideShown
Guide_EventScript_NewInGuideMore:
	message Guide_Text_NewInGuideMore
Guide_EventScript_NewInGuideShown:
	waitfanfare
	waitmessage
	waitbuttonpress
	closemessage
	releaseall
	end

@ Players online near you; pick one to add them as a friend.
Guide_EventScript_Nearby::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GUIDE_NEARBY
	goto_if_eq VAR_0x800B, 0, Online_EventScript_ShowMessage
	message gStringVar4
	waitmessage
	onl_choose
	goto_if_eq VAR_RESULT, ONLINE_CHOSE_NOTHING, Online_EventScript_End
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GUIDE_FRIEND
	playse SE_SUCCESS
	goto Online_EventScript_ShowMessage

Guide_EventScript_Invite::
	msgbox Guide_Text_InviteSure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Guide_EventScript_End
Guide_EventScript_DoInvite:
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_GUIDE_INVITE
	playse SE_POKENAV_CALL
	goto Online_EventScript_ShowMessage

@ INVITE FRIENDS on the title menu: once online, the invites go out.
Guide_EventScript_SendInvites::
	lockall
	goto Guide_EventScript_DoInvite

Guide_Text_MenuGuide:
	.string "GUIDE$"
Guide_Text_MenuList:
	.string "THINGS TO DO$"
Guide_Text_MenuMap:
	.string "GUIDE MAP$"
Guide_Text_MenuNearby:
	.string "PLAYERS NEARBY$"
Guide_Text_MenuInvite:
	.string "INVITE FRIENDS$"
Guide_Text_MenuClose:
	.string "CLOSE$"

Guide_Text_NewInGuide:
	.string "New in your GUIDE: {STR_VAR_1}!\n"
	.string "R > GUIDE shows where it starts.$"

Guide_Text_NewInGuideMore:
	.string "New in your GUIDE: {STR_VAR_1}\n"
	.string "and {STR_VAR_2} more! See R > GUIDE.$"

Guide_Text_InviteSure:
	.string "Tell your friends you're online and\n"
	.string "invite them to play?$"
