@ The Solmara flight desk (tools/solmara/flight.py).
Solmara_EventScript_FlightDesk::
	lock
	faceplayer
	msgbox Solmara_Text_FlightAsk, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8008, 0, Solmara_EventScript_FlightSkip0
	goto_if_unset FLAG_SOL_VISITED_PORT_SOLACE, Solmara_EventScript_FlightSkip0
	dynmultipush Solmara_Text_Town0, 0
Solmara_EventScript_FlightSkip0:
	goto_if_eq VAR_0x8008, 1, Solmara_EventScript_FlightSkip1
	goto_if_unset FLAG_SOL_VISITED_GEARHOLT, Solmara_EventScript_FlightSkip1
	dynmultipush Solmara_Text_Town1, 1
Solmara_EventScript_FlightSkip1:
	goto_if_eq VAR_0x8008, 2, Solmara_EventScript_FlightSkip2
	goto_if_unset FLAG_SOL_VISITED_ASHCOMBE, Solmara_EventScript_FlightSkip2
	dynmultipush Solmara_Text_Town2, 2
Solmara_EventScript_FlightSkip2:
	goto_if_eq VAR_0x8008, 3, Solmara_EventScript_FlightSkip3
	goto_if_unset FLAG_SOL_VISITED_SILKWOOD, Solmara_EventScript_FlightSkip3
	dynmultipush Solmara_Text_Town3, 3
Solmara_EventScript_FlightSkip3:
	goto_if_eq VAR_0x8008, 4, Solmara_EventScript_FlightSkip4
	goto_if_unset FLAG_SOL_VISITED_ROSEMEAD, Solmara_EventScript_FlightSkip4
	dynmultipush Solmara_Text_Town4, 4
Solmara_EventScript_FlightSkip4:
	goto_if_eq VAR_0x8008, 5, Solmara_EventScript_FlightSkip5
	goto_if_unset FLAG_SOL_VISITED_DUSKMERE, Solmara_EventScript_FlightSkip5
	dynmultipush Solmara_Text_Town5, 5
Solmara_EventScript_FlightSkip5:
	goto_if_eq VAR_0x8008, 6, Solmara_EventScript_FlightSkip6
	goto_if_unset FLAG_SOL_VISITED_FROSTHOLM, Solmara_EventScript_FlightSkip6
	dynmultipush Solmara_Text_Town6, 6
Solmara_EventScript_FlightSkip6:
	goto_if_eq VAR_0x8008, 7, Solmara_EventScript_FlightSkip7
	goto_if_unset FLAG_SOL_VISITED_DRAKESPIRE, Solmara_EventScript_FlightSkip7
	dynmultipush Solmara_Text_Town7, 7
Solmara_EventScript_FlightSkip7:
	dynmultipush Solmara_Text_FlightCancel, 127
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 127, Solmara_EventScript_FlightNo
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Solmara_EventScript_FlightNo
	goto_if_eq VAR_RESULT, 0, Solmara_EventScript_Fly0
	goto_if_eq VAR_RESULT, 1, Solmara_EventScript_Fly1
	goto_if_eq VAR_RESULT, 2, Solmara_EventScript_Fly2
	goto_if_eq VAR_RESULT, 3, Solmara_EventScript_Fly3
	goto_if_eq VAR_RESULT, 4, Solmara_EventScript_Fly4
	goto_if_eq VAR_RESULT, 5, Solmara_EventScript_Fly5
	goto_if_eq VAR_RESULT, 6, Solmara_EventScript_Fly6
	goto_if_eq VAR_RESULT, 7, Solmara_EventScript_Fly7
Solmara_EventScript_FlightNo::
	msgbox Solmara_Text_FlightNo, MSGBOX_DEFAULT
	release
	end

Solmara_EventScript_Fly0::
	msgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FLY
	fadescreen FADE_TO_WHITE
	warpsilent MAP_PORT_SOLACE, 9, 21
	waitstate
	release
	end

Solmara_EventScript_Fly1::
	msgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FLY
	fadescreen FADE_TO_WHITE
	warpsilent MAP_GEARHOLT, 8, 9
	waitstate
	release
	end

Solmara_EventScript_Fly2::
	msgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FLY
	fadescreen FADE_TO_WHITE
	warpsilent MAP_ASHCOMBE, 9, 14
	waitstate
	release
	end

Solmara_EventScript_Fly3::
	msgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FLY
	fadescreen FADE_TO_WHITE
	warpsilent MAP_SILKWOOD, 7, 9
	waitstate
	release
	end

Solmara_EventScript_Fly4::
	msgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FLY
	fadescreen FADE_TO_WHITE
	warpsilent MAP_ROSEMEAD, 8, 25
	waitstate
	release
	end

Solmara_EventScript_Fly5::
	msgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FLY
	fadescreen FADE_TO_WHITE
	warpsilent MAP_DUSKMERE, 27, 9
	waitstate
	release
	end

Solmara_EventScript_Fly6::
	msgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FLY
	fadescreen FADE_TO_WHITE
	warpsilent MAP_FROSTHOLM, 10, 14
	waitstate
	release
	end

Solmara_EventScript_Fly7::
	msgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FLY
	fadescreen FADE_TO_WHITE
	warpsilent MAP_DRAKESPIRE, 28, 9
	waitstate
	release
	end

Solmara_Text_Town0:
	.string "PORT SOLACE$"

Solmara_Text_Town1:
	.string "GEARHOLT$"

Solmara_Text_Town2:
	.string "ASHCOMBE$"

Solmara_Text_Town3:
	.string "SILKWOOD$"

Solmara_Text_Town4:
	.string "ROSEMEAD$"

Solmara_Text_Town5:
	.string "DUSKMERE$"

Solmara_Text_Town6:
	.string "FROSTHOLM$"

Solmara_Text_Town7:
	.string "DRAKESPIRE$"

Solmara_Text_FlightCancel:
	.string "CANCEL$"

Solmara_Text_FlightAsk:
	.string "SWANNA AIR, at your service!\n"
	.string "Where shall we fly today?$"

Solmara_Text_FlightGo:
	.string "Hold on tight!$"

Solmara_Text_FlightNo:
	.string "Come fly with us any time!$"
