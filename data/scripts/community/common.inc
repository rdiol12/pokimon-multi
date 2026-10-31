@ Shared by the community features.

Community_EventScript_Offline::
	msgbox Community_Text_Offline, MSGBOX_DEFAULT
	releaseall
	end

Community_EventScript_Done::
	releaseall
	end

Community_Text_Offline::
	.string "You need to be online for that.\n"
	.string "Pick JOIN ONLINE on the title screen.$"
