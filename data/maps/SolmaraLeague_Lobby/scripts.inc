SolmaraLeague_Lobby_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SolmaraLeague_Lobby_OnTransition
	.byte 0

@ Walking into the lobby starts a fresh run at the Elite Four.
SolmaraLeague_Lobby_OnTransition:
	clearflag FLAG_SOL_E4_VESPER
	clearflag FLAG_SOL_E4_NETTLE
	clearflag FLAG_SOL_E4_DUNE
	clearflag FLAG_SOL_E4_ORION
	end

SolmaraLeague_Lobby_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_SOLMARA_LEAGUE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

SolmaraLeague_Lobby_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart SolmaraLeague_Lobby_EventScript_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
SolmaraLeague_Lobby_EventScript_Items:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_MAX_POTION
	.2byte ITEM_MAX_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_MAX_ETHER
	.2byte ITEM_MAX_REPEL
	pokemartlistend

SolmaraLeague_Lobby_EventScript_Flight::
	setvar VAR_0x8008, 255
	goto Solmara_EventScript_FlightDesk

@ Live PvP battles and the ladder, on the lobby's big screens.
SolmaraLeague_Lobby_EventScript_Screens::
	lock
	faceplayer
	message SolmaraLeague_Lobby_EventScript_Text_Screens
	waitmessage
	multichoice 18, 6, MULTI_YESNO, FALSE
	goto_if_eq VAR_RESULT, 0, SolmaraLeague_Lobby_EventScript_Watch
	msgbox SolmaraLeague_Lobby_EventScript_Text_Ladder, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, SolmaraLeague_Lobby_EventScript_Ladder
	release
	end

SolmaraLeague_Lobby_EventScript_Watch::
	closemessage
	lockall
	callnative ShowNetplayWatch
	end

SolmaraLeague_Lobby_EventScript_Ladder::
	closemessage
	lockall
	callnative ShowNetplayLadder
	end

SolmaraLeague_Lobby_EventScript_Text_Screens:
	.string "Welcome to the SOLMARA LEAGUE!\n"
	.string "Want to watch the live PvP\l"
	.string "battles on the big screen?$"

SolmaraLeague_Lobby_EventScript_Text_Ladder:
	.string "Then how about the PvP LADDER?$"

SolmaraLeague_Lobby_EventScript_Fan::
	msgbox SolmaraLeague_Lobby_EventScript_Fan_Text, MSGBOX_NPC
	end

SolmaraLeague_Lobby_EventScript_Fan_Text:
	.string "When it's busy, there's a crowd\n"
	.string "watching the big screen and\l"
	.string "cheering for their friends!$"
