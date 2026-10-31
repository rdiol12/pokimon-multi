@ R > PASTIMES: the side activities that keep a log.

Pastime_EventScript_Menu::
	dynmultipush Pastime_Text_MenuLostFound, PASTIME_MENU_LOST_FOUND
	dynmultipush Pastime_Text_MenuMuseum, PASTIME_MENU_MUSEUM
	dynmultipush Pastime_Text_MenuDelivery, PASTIME_MENU_DELIVERY
	dynmultipush Pastime_Text_MenuMail, PASTIME_MENU_MAIL
	dynmultipush Pastime_Text_MenuQuiz, PASTIME_MENU_QUIZ
	dynmultipush Pastime_Text_MenuHotel, PASTIME_MENU_HOTEL
	dynmultipush Pastime_Text_MenuCafe, PASTIME_MENU_CAFE
	dynmultipush Pastime_Text_MenuCooking, PASTIME_MENU_COOKING
	dynmultipush Pastime_Text_MenuBugContest, PASTIME_MENU_BUG_CONTEST
	dynmultipush Pastime_Text_MenuTrading, PASTIME_MENU_TRADING
	dynmultipush Pastime_Text_MenuDojo, PASTIME_MENU_DOJO
	dynmultipush Pastime_Text_MenuRanger, PASTIME_MENU_RANGER
	dynmultipush Pastime_Text_MenuSchool, PASTIME_MENU_SCHOOL
	dynmultipush Pastime_Text_MenuGarden, PASTIME_MENU_GARDEN
	dynmultipush Pastime_Text_MenuClose, PASTIME_MENU_CLOSE
	dynmultistack 0, 0, FALSE, 6, FALSE, PASTIME_MENU_LOST_FOUND, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case PASTIME_MENU_LOST_FOUND, Pastime_EventScript_LostFound
	case PASTIME_MENU_MUSEUM, Pastime_EventScript_Museum
	case PASTIME_MENU_DELIVERY, Pastime_EventScript_Delivery
	case PASTIME_MENU_MAIL, Pastime_EventScript_Mail
	case PASTIME_MENU_QUIZ, Pastime_EventScript_Quiz
	case PASTIME_MENU_HOTEL, Pastime_EventScript_Hotel
	case PASTIME_MENU_CAFE, Pastime_EventScript_Cafe
	case PASTIME_MENU_COOKING, Pastime_EventScript_Cooking
	case PASTIME_MENU_BUG_CONTEST, Pastime_EventScript_BugContest
	case PASTIME_MENU_TRADING, Pastime_EventScript_Trading
	case PASTIME_MENU_DOJO, Pastime_EventScript_Dojo
	case PASTIME_MENU_RANGER, Pastime_EventScript_Ranger
	case PASTIME_MENU_SCHOOL, Pastime_EventScript_School
	case PASTIME_MENU_GARDEN, Pastime_EventScript_Garden
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

Pastime_EventScript_Cafe:
	callnative Cafe_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_Cooking:
	dynmultipush Pastime_Text_CookRecords, 0
	dynmultipush Pastime_Text_CookChamps, 1
	dynmultistack 0, 0, FALSE, 2, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Pastime_EventScript_CookChamps
	goto_if_ne VAR_RESULT, 0, Pastime_EventScript_MenuDone
	callnative Cook_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_CookChamps:
	setvar VAR_0x8004, WORLD_BOARD_COOKING
	callnative WorldOnline_ShowBoard, waitstate=1
	releaseall
	end

Pastime_EventScript_BugContest:
	dynmultipush Pastime_Text_CookRecords, 0
	dynmultipush Pastime_Text_BugChamps, 1
	dynmultistack 0, 0, FALSE, 2, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, Pastime_EventScript_BugChamps
	goto_if_ne VAR_RESULT, 0, Pastime_EventScript_MenuDone
	callnative BugContest_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_BugChamps:
	setvar VAR_0x8004, WORLD_BOARD_BUG_CONTEST
	callnative WorldOnline_ShowBoard, waitstate=1
	releaseall
	end

Pastime_EventScript_Trading:
	callnative Trading_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_Dojo:
	callnative Dojo_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_Ranger:
	callnative Ranger_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_School:
	callnative School_ShowList, waitstate=1
	releaseall
	end

Pastime_EventScript_Garden:
	callnative Garden_ShowList, waitstate=1
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
Pastime_Text_MenuCafe:
	.string "CAFE$"
Pastime_Text_MenuCooking:
	.string "COOK-OFF$"
Pastime_Text_CookRecords:
	.string "MY RECORDS$"
Pastime_Text_CookChamps:
	.string "COOK-OFF CHAMPS$"
Pastime_Text_MenuBugContest:
	.string "BUG CONTEST$"
Pastime_Text_BugChamps:
	.string "BUG CONTEST BOARD$"
Pastime_Text_MenuTrading:
	.string "TRADING POST$"
Pastime_Text_MenuDojo:
	.string "BATTLE DOJO$"
Pastime_Text_MenuRanger:
	.string "RANGER$"
Pastime_Text_MenuSchool:
	.string "TRAINER SCHOOL$"
Pastime_Text_MenuGarden:
	.string "GARDEN$"
Pastime_Text_MenuClose:
	.string "CLOSE$"
