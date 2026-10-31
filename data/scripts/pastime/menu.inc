@ R > PASTIMES: the side activities that keep a log.

Pastime_EventScript_Menu::
	dynmultipush Pastime_Text_MenuLostFound, PASTIME_MENU_LOST_FOUND
	dynmultipush Pastime_Text_MenuClose, PASTIME_MENU_CLOSE
	dynmultistack 0, 0, FALSE, 6, FALSE, PASTIME_MENU_LOST_FOUND, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case PASTIME_MENU_LOST_FOUND, Pastime_EventScript_LostFound
Pastime_EventScript_MenuDone::
	releaseall
	end

Pastime_EventScript_LostFound:
	callnative LostFound_ShowList, waitstate=1
	releaseall
	end

Pastime_Text_MenuLostFound:
	.string "LOST & FOUND$"
Pastime_Text_MenuClose:
	.string "CLOSE$"
