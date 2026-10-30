@ The player's house. Furniture is placed and drawn by src/world_house.c.

StarfallIsle_PlayerHouse_MapScripts::
	.byte 0

StarfallIsle_PlayerHouse_EventScript_Catalog::
	lockall
	callnative House_GetCatalogState
	goto_if_eq VAR_RESULT, 2, StarfallIsle_PlayerHouse_EventScript_Visiting
	message StarfallIsle_PlayerHouse_Text_WhatToDo
	waitmessage
	dynmultipush StarfallIsle_PlayerHouse_Text_Decorate, 0
	goto_if_eq VAR_RESULT, 1, StarfallIsle_PlayerHouse_EventScript_PushClose
	dynmultipush StarfallIsle_PlayerHouse_Text_Invite, 1
	goto StarfallIsle_PlayerHouse_EventScript_PushCancel
StarfallIsle_PlayerHouse_EventScript_PushClose:
	dynmultipush StarfallIsle_PlayerHouse_Text_CloseHouse, 1
StarfallIsle_PlayerHouse_EventScript_PushCancel:
	dynmultipush StarfallIsle_PlayerHouse_Text_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 1, StarfallIsle_PlayerHouse_EventScript_ToggleOpen
	goto_if_ne VAR_RESULT, 0, StarfallIsle_PlayerHouse_EventScript_CatalogClose
	msgbox StarfallIsle_PlayerHouse_Text_Catalog, MSGBOX_DEFAULT
	dynmultipush StarfallIsle_PlayerHouse_Text_Plant, FURNITURE_PLANT
	dynmultipush StarfallIsle_PlayerHouse_Text_Bookcase, FURNITURE_BOOKCASE
	dynmultipush StarfallIsle_PlayerHouse_Text_Table, FURNITURE_TABLE
	dynmultipush StarfallIsle_PlayerHouse_Text_Stool, FURNITURE_STOOL
	dynmultipush StarfallIsle_PlayerHouse_Text_Bed, FURNITURE_BED
	dynmultipush StarfallIsle_PlayerHouse_Text_Tv, FURNITURE_TV
	dynmultipush StarfallIsle_PlayerHouse_Text_Lamp, FURNITURE_LAMP
	dynmultipush StarfallIsle_PlayerHouse_Text_Aquarium, FURNITURE_AQUARIUM
	dynmultipush StarfallIsle_PlayerHouse_Text_Cushion, FURNITURE_CUSHION
	dynmultipush StarfallIsle_PlayerHouse_Text_Trophies, FURNITURE_TROPHIES
	dynmultipush StarfallIsle_PlayerHouse_Text_Telescope, FURNITURE_TELESCOPE
	dynmultipush StarfallIsle_PlayerHouse_Text_Cancel, FURNITURE_NONE
	dynmultistack 0, 0, FALSE, 6, FALSE, FURNITURE_PLANT, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, FURNITURE_NONE, StarfallIsle_PlayerHouse_EventScript_CatalogClose
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, StarfallIsle_PlayerHouse_EventScript_CatalogClose
	copyvar VAR_0x8004, VAR_RESULT
	callnative House_StartPlacing
	goto_if_eq VAR_RESULT, HOUSE_FULL, StarfallIsle_PlayerHouse_EventScript_Full
	msgbox StarfallIsle_PlayerHouse_Text_HowToPlace, MSGBOX_DEFAULT
StarfallIsle_PlayerHouse_EventScript_CatalogClose:
	releaseall
	end

StarfallIsle_PlayerHouse_EventScript_ToggleOpen:
	callnative House_ToggleOpen
	callnative House_GetCatalogState
	goto_if_eq VAR_RESULT, 1, StarfallIsle_PlayerHouse_EventScript_Opened
	msgbox StarfallIsle_PlayerHouse_Text_Closed, MSGBOX_DEFAULT
	releaseall
	end

StarfallIsle_PlayerHouse_EventScript_Opened:
	msgbox StarfallIsle_PlayerHouse_Text_Opened, MSGBOX_DEFAULT
	releaseall
	end

StarfallIsle_PlayerHouse_EventScript_Visiting:
	msgbox StarfallIsle_PlayerHouse_Text_Visiting, MSGBOX_DEFAULT
	releaseall
	end

StarfallIsle_PlayerHouse_EventScript_Full:
	msgbox StarfallIsle_PlayerHouse_Text_Full, MSGBOX_DEFAULT
	releaseall
	end

StarfallIsle_PlayerHouse_EventScript_PutAway::
	lockall
	msgbox StarfallIsle_PlayerHouse_Text_PutAway, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, StarfallIsle_PlayerHouse_EventScript_CatalogClose
	callnative House_PutAway
	playse SE_CLICK
	releaseall
	end

StarfallIsle_PlayerHouse_EventScript_Placed::
	playse SE_SHOP
	waitse
	end

StarfallIsle_PlayerHouse_EventScript_CantPlace::
	lockall
	msgbox StarfallIsle_PlayerHouse_Text_CantPlace, MSGBOX_DEFAULT
	releaseall
	end

StarfallIsle_PlayerHouse_Text_Catalog:
	.string "It's a furniture catalog.\n"
	.string "Pick something for your house!$"

StarfallIsle_PlayerHouse_Text_WhatToDo:
	.string "It's your house. What would you\n"
	.string "like to do?$"

StarfallIsle_PlayerHouse_Text_Decorate:
	.string "DECORATE$"
StarfallIsle_PlayerHouse_Text_Invite:
	.string "INVITE VISITORS$"
StarfallIsle_PlayerHouse_Text_CloseHouse:
	.string "STOP VISITORS$"

StarfallIsle_PlayerHouse_Text_Opened:
	.string "Your house is open! Players online\n"
	.string "who come in while you're here will\l"
	.string "see your furniture.$"

StarfallIsle_PlayerHouse_Text_Closed:
	.string "Your house is closed to visitors.$"

StarfallIsle_PlayerHouse_Text_Visiting:
	.string "This is {STR_VAR_1}'s house!\n"
	.string "Make yourself at home.$"

StarfallIsle_PlayerHouse_Text_HowToPlace:
	.string "Face an empty spot and press A\n"
	.string "to put the {STR_VAR_1} there.$"

StarfallIsle_PlayerHouse_Text_Full:
	.string "There's no room for more furniture.\n"
	.string "Put something away first.$"

StarfallIsle_PlayerHouse_Text_PutAway:
	.string "Put away the {STR_VAR_1}?$"

StarfallIsle_PlayerHouse_Text_CantPlace:
	.string "The {STR_VAR_1} won't fit there.$"

StarfallIsle_PlayerHouse_Text_Plant:
	.string "POTTED PLANT$"
StarfallIsle_PlayerHouse_Text_Bookcase:
	.string "BOOKCASE$"
StarfallIsle_PlayerHouse_Text_Table:
	.string "TABLE$"
StarfallIsle_PlayerHouse_Text_Stool:
	.string "STOOL$"
StarfallIsle_PlayerHouse_Text_Bed:
	.string "BED$"
StarfallIsle_PlayerHouse_Text_Tv:
	.string "TV$"
StarfallIsle_PlayerHouse_Text_Lamp:
	.string "LAMP$"
StarfallIsle_PlayerHouse_Text_Aquarium:
	.string "AQUARIUM$"
StarfallIsle_PlayerHouse_Text_Cushion:
	.string "BALL CUSHION$"
StarfallIsle_PlayerHouse_Text_Trophies:
	.string "TROPHY CASE$"
StarfallIsle_PlayerHouse_Text_Telescope:
	.string "TELESCOPE$"
StarfallIsle_PlayerHouse_Text_Cancel:
	.string "CANCEL$"
