@ Shared wild spawns (docs/shared_map.md, src/shared_map.c).

@ This game's own wild Pokémon, which someone else is battling.
SharedMap_EventScript_Taken::
	msgbox SharedMap_Text_Taken, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, SharedMap_EventScript_Watch
	releaseall
	end

@ Another player's wild Pokémon.
SharedMap_EventScript_Remote::
	lock
	callnative SharedMap_BufferRemote
	goto_if_eq VAR_RESULT, 1, SharedMap_EventScript_RemoteTaken
	goto_if_eq VAR_RESULT, 2, SharedMap_EventScript_Gone
	applymovement VAR_LAST_TALKED, Common_Movement_ExclamationMark
	waitmovement 0
	playmoncry VAR_0x8005, CRY_MODE_DOUBLES
	waitmoncry
	callnative SharedMap_ClaimRemote, waitstate=1
	goto_if_eq VAR_RESULT, FALSE, SharedMap_EventScript_LostIt
	callnative SharedMap_StartRemoteBattle
	waitstate
	releaseall
	end

SharedMap_EventScript_RemoteTaken:
	msgbox SharedMap_Text_Taken, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, SharedMap_EventScript_Watch
	releaseall
	end

SharedMap_EventScript_LostIt:
	goto_if_eq VAR_0x8006, 2, SharedMap_EventScript_TooLate
SharedMap_EventScript_Gone:
	msgbox SharedMap_Text_Gone, MSGBOX_DEFAULT
	releaseall
	end

SharedMap_EventScript_TooLate:
	msgbox SharedMap_Text_TooLate, MSGBOX_DEFAULT
	releaseall
	end

SharedMap_EventScript_Watch:
	closemessage
	callnative HoennMulti_WatchLive
	waitstate
	releaseall
	end

SharedMap_Text_Taken:
	.string "{STR_VAR_1} is battling this\n"
	.string "{STR_VAR_2}!\p"
	.string "Watch the battle?$"

SharedMap_Text_TooLate:
	.string "Too late! {STR_VAR_1} got to it first.$"

SharedMap_Text_Gone:
	.string "It ran off before you could\n"
	.string "battle it…$"
