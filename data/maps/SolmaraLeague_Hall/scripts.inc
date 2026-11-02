SolmaraLeague_Hall_MapScripts::
	.byte 0

SolmaraLeague_Hall_EventScript_Keeper::
	lock
	faceplayer
	goto_if_unset FLAG_SOL_CHAMPION_THIS_RUN, SolmaraLeague_Hall_EventScript_NotYet
	msgbox SolmaraLeague_Hall_EventScript_Text_Keeper, MSGBOX_DEFAULT
	closemessage
	warp MAP_SOLMARA_LEAGUE_LOBBY, 9, 10
	waitstate
	release
	end

SolmaraLeague_Hall_EventScript_NotYet::
	msgbox SolmaraLeague_Hall_EventScript_Text_NotYet, MSGBOX_DEFAULT
	release
	end

SolmaraLeague_Hall_EventScript_Text_Keeper:
	.string "Your team's names are written in\n"
	.string "the SOLMARA HALL forever.\p"
	.string "Your friends will see them on\n"
	.string "the HALL OF FAME wall online.\l"
	.string "Back to the lobby you go!$"

SolmaraLeague_Hall_EventScript_Text_NotYet:
	.string "Only a Champion may stand here.$"
