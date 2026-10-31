@ Legend sightings: where players last met each roaming Pokémon or legend.

Online_EventScript_Sightings::
	call Online_EventScript_Connecting
	onl_request_or_fail ONL_REQ_SIGHT_BOARD
	closemessage
	onl_board
	goto Online_EventScript_End
