@ Living ecosystem (server/online_eco.py): this area's wild populations,
@ then every migration going on now.
Online_EventScript_Wildlife::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_ECO_AREA
	closemessage
	goto_if_ne VAR_0x800B, 0, Online_EventScript_WildlifeNone
	onl_board
	goto Online_EventScript_WildlifeMoves
Online_EventScript_WildlifeNone::
	msgbox gStringVar4, MSGBOX_DEFAULT
Online_EventScript_WildlifeMoves::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_ECO_MOVES
	closemessage
	onl_board
	goto Online_EventScript_End

@ Research Pokédex leaders (server/online_research.py; the research itself
@ is in the save, src/research.c).
Online_EventScript_ResearchLeaders::
	call Online_EventScript_Connecting
	callnative Research_GetCompletedCount
	onl_request_or_fail ONL_REQ_RESEARCH_BOARD
	closemessage
	onl_board
	goto Online_EventScript_End
