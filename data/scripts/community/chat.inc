@ Chat and lobby rooms: START > ONLINE > COMMUNITY > CHAT.
@ The server is server/community_chat.py; the room window is in
@ src/community_chat.c.

	.set CHAT_LOCKED, 2

Community_EventScript_Chat::
	lockall
	callnative CommunityChat_RequestRooms, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	callnative Community_ChooseRow, waitstate=1
	goto_if_eq VAR_RESULT, 0xFF, Community_EventScript_Done
	callnative CommunityChat_JoinRoom, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	goto_if_eq VAR_RESULT, CHAT_LOCKED, Community_EventScript_ChatLocked
Community_EventScript_ChatRoom:
	callnative CommunityChat_ShowRoom, waitstate=1
	goto_if_eq VAR_RESULT, FALSE, Community_EventScript_ChatLeave
	fadescreen FADE_TO_BLACK
	callnative CommunityChat_PickPhrase, waitstate=1
	callnative CommunityChat_Say, waitstate=1
	goto_if_eq VAR_RESULT, COMMUNITY_OFFLINE, Community_EventScript_Offline
	goto Community_EventScript_ChatRoom

Community_EventScript_ChatLeave:
	callnative CommunityChat_Leave
	releaseall
	end

Community_EventScript_ChatLocked:
	msgbox Community_Text_ChatLocked, MSGBOX_DEFAULT
	releaseall
	end

Community_Text_ChatLocked:
	.string "That room is only for trainers\n"
	.string "in another ladder tier.$"
