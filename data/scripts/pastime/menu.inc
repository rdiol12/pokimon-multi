@ R > PASTIMES: the side activities that keep a log.

Pastime_EventScript_Menu::
	dynmultipush Pastime_Text_MenuLostFound, PASTIME_MENU_LOST_FOUND
	dynmultipush Pastime_Text_MenuMuseum, PASTIME_MENU_MUSEUM
	dynmultipush Pastime_Text_MenuDelivery, PASTIME_MENU_DELIVERY
	dynmultipush Pastime_Text_MenuMail, PASTIME_MENU_MAIL
	dynmultipush Pastime_Text_MenuQuiz, PASTIME_MENU_QUIZ
	dynmultipush Pastime_Text_MenuHotel, PASTIME_MENU_HOTEL
	dynmultipush Pastime_Text_MenuClose, PASTIME_MENU_CLOSE
	dynmultistack 0, 0, FALSE, 6, FALSE, PASTIME_MENU_LOST_FOUND, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case PASTIME_MENU_LOST_FOUND, Pastime_EventScript_LostFound
	case PASTIME_MENU_MUSEUM, Pastime_EventScript_Museum
	case PASTIME_MENU_DELIVERY, Pastime_EventScript_Delivery
	case PASTIME_MENU_MAIL, Pastime_EventScript_Mail
	case PASTIME_MENU_QUIZ, Pastime_EventScript_Quiz
	case PASTIME_MENU_HOTEL, Pastime_EventScript_Hotel
Pastime_EventScript_MenuDone::
	releaseall
	end

Pastime_EventScript_LostFound:
	callnative LostFound_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_Museum:
	callnative Museum_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_Delivery:
	callnative Delivery_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_Mail:
	callnative Mail_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_Quiz:
	dynmultipush Pastime_Text_QuizRecords, 0
	dynmultipush Pastime_Text_QuizChamps, 1
	dynmultistack 0, 0, FALSE, 2, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Pastime_EventScript_QuizChamps
	goto_if_ne VAR_RESULT, 0, Pastime_EventScript_MenuDone
	callnative Quiz_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_QuizChamps:
	setvar VAR_0x8004, WORLD_BOARD_QUIZ
	callnative WorldOnline_ShowBoard, waitstate=1
	releaseall
	end

Pastime_EventScript_Hotel:
	callnative Hotel_ShowList, waitstate=1
	releaseall
	end

Pastime_Text_MenuLostFound:
	.string "LOST & FOUND$"
Pastime_Text_MenuMuseum:
	.string "MUSEUM$"
Pastime_Text_MenuDelivery:
	.string "DELIVERY$"
Pastime_Text_MenuMail:
	.string "MAIL ROUTE$"
Pastime_Text_MenuQuiz:
	.string "QUIZ HALL$"
Pastime_Text_QuizRecords:
	.string "MY RECORDS$"
Pastime_Text_QuizChamps:
	.string "QUIZ CHAMPS$"
Pastime_Text_MenuHotel:
	.string "HOTEL$"
Pastime_Text_MenuClose:
	.string "CLOSE$"
