@ Weather lab (src/pastime_weather.c), at the Route 119 Weather Institute.

Weather_EventScript_Researcher::
	lock
	faceplayer
	callnative Weather_GetState
	goto_if_eq VAR_RESULT, TRUE, Weather_EventScript_Hello
	msgbox Weather_Text_Welcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Weather_EventScript_Bye
	callnative Weather_Join
	msgbox Weather_Text_Joined, MSGBOX_DEFAULT
	goto Weather_EventScript_Menu

Weather_EventScript_Hello:
	msgbox Weather_Text_Hello, MSGBOX_DEFAULT
Weather_EventScript_Menu:
	message Weather_Text_WhatToDo
	waitmessage
	dynmultipush Weather_Text_Forecast, WEATHER_MENU_FORECAST
	dynmultipush Weather_Text_Report, WEATHER_MENU_REPORT
	dynmultipush Weather_Text_Cancel, WEATHER_MENU_CANCEL
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, WEATHER_MENU_FORECAST, Weather_EventScript_Forecast
	goto_if_eq VAR_RESULT, WEATHER_MENU_REPORT, Weather_EventScript_Report
	goto Weather_EventScript_Bye

Weather_EventScript_Forecast:
	closemessage
	callnative Weather_ShowForecast, waitstate=1
	goto Weather_EventScript_Menu

Weather_EventScript_Report:
	callnative Weather_Report
	goto_if_eq VAR_RESULT, 0, Weather_EventScript_NoReadings
	playse SE_SUCCESS
	msgbox Weather_Text_Reported, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8006, FALSE, Weather_EventScript_CheckRank
	playfanfare MUS_OBTAIN_ITEM
	message Weather_Text_FullSet
	waitmessage
	waitfanfare
	waitbuttonpress
Weather_EventScript_CheckRank:
	goto_if_eq VAR_0x8005, ITEM_NONE, Weather_EventScript_Menu
	playfanfare MUS_LEVEL_UP
	message Weather_Text_RankUp
	waitmessage
	waitfanfare
	waitbuttonpress
	giveitem VAR_0x8005
	goto Weather_EventScript_Menu

Weather_EventScript_NoReadings:
	msgbox Weather_Text_NoReadings, MSGBOX_DEFAULT
	goto Weather_EventScript_Menu

Weather_EventScript_Bye:
	msgbox Weather_Text_Bye, MSGBOX_DEFAULT
	release
	end

Weather_Text_Welcome:
	.string "I track the storms that drift\n"
	.string "across HOENN, one place an hour.\p"
	.string "I can forecast where they'll be,\n"
	.string "but I need readings from inside!\p"
	.string "Will you chase the weather for\n"
	.string "the lab?$"

Weather_Text_Joined:
	.string "Wonderful! Just stand in a storm's\n"
	.string "weather and your POKéNAV takes a\l"
	.string "reading, one an hour.\p"
	.string "Rain, thunderstorms, sandstorms\n"
	.string "and snow! Bring them to me.$"

Weather_Text_Hello:
	.string "Ah, our {STR_VAR_1}! Out\n"
	.string "chasing storms?$"

Weather_Text_WhatToDo:
	.string "What do you need?$"

Weather_Text_Forecast:
	.string "FORECAST$"

Weather_Text_Report:
	.string "REPORT READINGS$"

Weather_Text_Cancel:
	.string "CANCEL$"

Weather_Text_Reported:
	.string "Readings handed in: {STR_VAR_1}.\n"
	.string "That's {STR_VAR_2} Research Points!$"

Weather_Text_FullSet:
	.string "And you've logged every kind of\n"
	.string "weather! 10 bonus points!$"

Weather_Text_RankUp:
	.string "Your lab rank is now\n"
	.string "{STR_VAR_3}! Take this, too.$"

Weather_Text_NoReadings:
	.string "No readings yet? Check the\n"
	.string "FORECAST and go find a storm!$"

Weather_Text_Bye:
	.string "Watch the skies!$"
