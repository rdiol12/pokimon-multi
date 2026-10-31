IronmoorPrison_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, IronmoorPrison_OnLoad
	.byte 0

IronmoorPrison_OnLoad:
	call_if_set FLAG_IRONMOOR_CORRIDOR, IronmoorPrison_EventScript_CorridorOpen
	call_if_set FLAG_IRONMOOR_CELLS, IronmoorPrison_EventScript_CellsOpen
	call_if_set FLAG_IRONMOOR_SPIRIT_1, IronmoorPrison_EventScript_SpiritGone1
	call_if_set FLAG_IRONMOOR_SPIRIT_2, IronmoorPrison_EventScript_SpiritGone2
	call_if_set FLAG_IRONMOOR_SPIRIT_3, IronmoorPrison_EventScript_SpiritGone3
	call_if_set FLAG_IRONMOOR_SPIRIT_4, IronmoorPrison_EventScript_SpiritGone4
	end

IronmoorPrison_EventScript_CellsOpen::
	setmetatile 6, 3, 609, FALSE
	setmetatile 6, 4, 610, FALSE
	setmetatile 10, 3, 609, FALSE
	setmetatile 10, 4, 610, FALSE
	setmetatile 14, 3, 609, FALSE
	setmetatile 14, 4, 610, FALSE
	setmetatile 18, 3, 609, FALSE
	setmetatile 18, 4, 610, FALSE
	setmetatile 22, 3, 609, FALSE
	setmetatile 22, 4, 610, FALSE
	setmetatile 3, 9, 609, FALSE
	setmetatile 3, 10, 610, FALSE
	setmetatile 8, 9, 609, FALSE
	setmetatile 8, 10, 610, FALSE
	setmetatile 20, 9, 609, FALSE
	setmetatile 20, 10, 610, FALSE
	setmetatile 25, 9, 609, FALSE
	setmetatile 25, 10, 610, FALSE
	setmetatile 2, 3, 550, TRUE
	setmetatile 3, 3, 551, TRUE
	setmetatile 2, 4, 552, TRUE
	setmetatile 3, 4, 553, TRUE
	return

IronmoorPrison_EventScript_CorridorOpen::
	setmetatile 14, 9, 609, FALSE
	setmetatile 14, 10, 610, FALSE
	setmetatile 15, 9, 609, FALSE
	setmetatile 15, 10, 610, FALSE
	return

IronmoorPrison_EventScript_SpiritGone1::
	setmetatile 10, 1, 742, FALSE
	return

IronmoorPrison_EventScript_SpiritGone2::
	setmetatile 22, 2, 742, FALSE
	return

IronmoorPrison_EventScript_SpiritGone3::
	setmetatile 3, 12, 742, FALSE
	return

IronmoorPrison_EventScript_SpiritGone4::
	setmetatile 25, 11, 742, FALSE
	return

IronmoorPrison_EventScript_CorridorGate::
	lockall
	goto_if_set FLAG_IRONMOOR_CORRIDOR, IronmoorPrison_EventScript_Leave
	goto_if_unset FLAG_IRONMOOR_KEY, IronmoorPrison_EventScript_CorridorLocked
	msgbox IronmoorPrison_Text_CorridorKey, MSGBOX_DEFAULT
	playse SE_UNLOCK
	call IronmoorPrison_EventScript_CorridorOpen
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_CORRIDOR
	delay 20
	msgbox IronmoorPrison_Text_CorridorOpen, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_CorridorLocked::
	msgbox IronmoorPrison_Text_CorridorLocked, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_Leave::
	releaseall
	end

IronmoorPrison_EventScript_CellGate::
	goto_if_set FLAG_IRONMOOR_CELLS, IronmoorPrison_EventScript_Leave
	msgbox IronmoorPrison_Text_CellLocked, MSGBOX_SIGN
	end

IronmoorPrison_EventScript_Console::
	lockall
	goto_if_set FLAG_IRONMOOR_CELLS, IronmoorPrison_EventScript_ConsoleDone
	msgbox IronmoorPrison_Text_ConsoleAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IronmoorPrison_EventScript_Leave
	playse SE_PIN
	delay 20
	playse SE_DOOR
	call IronmoorPrison_EventScript_CellsOpen
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_CELLS
	delay 30
	msgbox IronmoorPrison_Text_ConsoleDone, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_ConsoleDone::
	msgbox IronmoorPrison_Text_ConsoleIsDone, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_Spirit1::
	lockall
	goto_if_set FLAG_IRONMOOR_SPIRIT_1, IronmoorPrison_EventScript_Leave
	msgbox IronmoorPrison_Text_Spirit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IronmoorPrison_EventScript_Leave
	playse SE_M_CONFUSE_RAY
	delay 40
	call IronmoorPrison_EventScript_SpiritGone1
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_SPIRIT_1
	goto IronmoorPrison_EventScript_SpiritFree

IronmoorPrison_EventScript_Spirit2::
	lockall
	goto_if_set FLAG_IRONMOOR_SPIRIT_2, IronmoorPrison_EventScript_Leave
	msgbox IronmoorPrison_Text_Spirit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IronmoorPrison_EventScript_Leave
	playse SE_M_CONFUSE_RAY
	delay 40
	call IronmoorPrison_EventScript_SpiritGone2
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_SPIRIT_2
	goto IronmoorPrison_EventScript_SpiritFree

IronmoorPrison_EventScript_Spirit3::
	lockall
	goto_if_set FLAG_IRONMOOR_SPIRIT_3, IronmoorPrison_EventScript_Leave
	msgbox IronmoorPrison_Text_Spirit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IronmoorPrison_EventScript_Leave
	playse SE_M_CONFUSE_RAY
	delay 40
	call IronmoorPrison_EventScript_SpiritGone3
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_SPIRIT_3
	goto IronmoorPrison_EventScript_SpiritFree

IronmoorPrison_EventScript_Spirit4::
	lockall
	goto_if_set FLAG_IRONMOOR_SPIRIT_4, IronmoorPrison_EventScript_Leave
	msgbox IronmoorPrison_Text_Spirit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IronmoorPrison_EventScript_Leave
	playse SE_M_CONFUSE_RAY
	delay 40
	call IronmoorPrison_EventScript_SpiritGone4
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_SPIRIT_4
	goto IronmoorPrison_EventScript_SpiritFree

IronmoorPrison_EventScript_SpiritFree::
	goto_if_unset FLAG_IRONMOOR_SPIRIT_1, IronmoorPrison_EventScript_SpiritMore
	goto_if_unset FLAG_IRONMOOR_SPIRIT_2, IronmoorPrison_EventScript_SpiritMore
	goto_if_unset FLAG_IRONMOOR_SPIRIT_3, IronmoorPrison_EventScript_SpiritMore
	goto_if_unset FLAG_IRONMOOR_SPIRIT_4, IronmoorPrison_EventScript_SpiritMore
	msgbox IronmoorPrison_Text_SpiritsAll, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_SpiritMore::
	msgbox IronmoorPrison_Text_SpiritOne, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_Desk::
	lockall
	goto_if_set FLAG_IRONMOOR_KEY, IronmoorPrison_EventScript_DeskDone
	msgbox IronmoorPrison_Text_Desk, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	msgbox IronmoorPrison_Text_GotKey, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_IRONMOOR_KEY
	msgbox IronmoorPrison_Text_KeyHint, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_DeskDone::
	msgbox IronmoorPrison_Text_DeskDone, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_Rack::
	lockall
	goto_if_set FLAG_IRONMOOR_HONEDGE, IronmoorPrison_EventScript_RackEmpty
	msgbox IronmoorPrison_Text_Rack, MSGBOX_DEFAULT
	playmoncry SPECIES_HONEDGE, CRY_MODE_NORMAL
	msgbox IronmoorPrison_Text_RackMoves, MSGBOX_DEFAULT
	waitmoncry
	givemon SPECIES_HONEDGE, 25
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, IronmoorPrison_EventScript_RackFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox IronmoorPrison_Text_GotHonedge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_IRONMOOR_HONEDGE
	releaseall
	end

IronmoorPrison_EventScript_RackFull::
	msgbox IronmoorPrison_Text_RackFull, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_RackEmpty::
	msgbox IronmoorPrison_Text_RackEmpty, MSGBOX_DEFAULT
	releaseall
	end

IronmoorPrison_EventScript_Shuppet::
	lock
	faceplayer
	playmoncry SPECIES_SHUPPET, CRY_MODE_NORMAL
	msgbox IronmoorPrison_Text_Shuppet, MSGBOX_DEFAULT
	waitmoncry
	release
	end

IronmoorPrison_EventScript_Vesper::
	trainerbattle_single TRAINER_IRONMOOR_VESPER, IronmoorPrison_Text_VesperIntro, IronmoorPrison_Text_VesperDefeat
	msgbox IronmoorPrison_Text_VesperPost, MSGBOX_AUTOCLOSE
	end

IronmoorPrison_Text_VesperIntro:
	.string "I came to speak with the spirits.\n"
	.string "They told me you'd come.$"

IronmoorPrison_Text_VesperDefeat:
	.string "They didn't tell me this part…$"

IronmoorPrison_Text_VesperPost:
	.string "Four spirits are bound in the cells.\n"
	.string "They long to be free.$"


IronmoorPrison_Text_CorridorLocked:
	.string "A heavy iron gate. It's locked.\p"
	.string "The warden must have kept the key\n"
	.string "somewhere.$"

IronmoorPrison_Text_CorridorKey:
	.string "{PLAYER} tried the WARDEN'S KEY…$"

IronmoorPrison_Text_CorridorOpen:
	.string "Click! The iron gate swung open.$"

IronmoorPrison_Text_CellLocked:
	.string "The cell is locked. A console must\n"
	.string "open the cells.$"

IronmoorPrison_Text_ConsoleAsk:
	.string "A rusty console. Its brass plate\n"
	.string "reads CELLS: RELEASE ALL.\p"
	.string "Pull the lever?$"

IronmoorPrison_Text_ConsoleDone:
	.string "Every cell door clanged open at once!$"

IronmoorPrison_Text_ConsoleIsDone:
	.string "The lever is pulled. The cells stand\n"
	.string "open.$"

IronmoorPrison_Text_Spirit:
	.string "A cold blue flame flickers in the\n"
	.string "cell. It seems to be held here\l"
	.string "against its will…\p"
	.string "Reach out to it?$"

IronmoorPrison_Text_SpiritOne:
	.string "The flame brushed past you and\n"
	.string "drifted away, down toward solitary.$"

IronmoorPrison_Text_SpiritsAll:
	.string "The last flame drifted down toward\n"
	.string "solitary.\p"
	.string "From below, something cracked like\n"
	.string "splitting stone…$"

IronmoorPrison_Text_Desk:
	.string "The warden's desk. His journal is\n"
	.string "open on it:\p"
	.string "“The spirits grow restless. I have\n"
	.string "bound them to the cells and sealed\l"
	.string "their master in the keystone below.”\p"
	.string "There's a key in the drawer.$"

IronmoorPrison_Text_GotKey:
	.string "{PLAYER} found the WARDEN'S KEY!$"

IronmoorPrison_Text_KeyHint:
	.string "It's stamped “CORRIDOR GATE”.$"

IronmoorPrison_Text_DeskDone:
	.string "The warden's journal. The last page\n"
	.string "is torn out.$"

IronmoorPrison_Text_Rack:
	.string "An old sword hangs on the armoury\n"
	.string "rack, its blade still bright…$"

IronmoorPrison_Text_RackMoves:
	.string "The sword moved on its own!\p"
	.string "Its blue cloth wound round\n"
	.string "{PLAYER}'s arm. It wants to come\l"
	.string "along!$"

IronmoorPrison_Text_GotHonedge:
	.string "{PLAYER} received a HONEDGE!$"

IronmoorPrison_Text_RackFull:
	.string "Your party is full. The sword settles\n"
	.string "back onto the rack to wait.$"

IronmoorPrison_Text_RackEmpty:
	.string "The armoury rack is empty.$"

IronmoorPrison_Text_Shuppet:
	.string "SHUPPET: Shuppa!\p"
	.string "It's rattling the cell bars for fun.$"

IronmoorPrison_EventScript_Item_RareCandy::
	finditem ITEM_RARE_CANDY
	end

IronmoorPrison_EventScript_Item_ReaperCloth::
	finditem ITEM_REAPER_CLOTH
	end

IronmoorPrison_EventScript_Item_DuskStone::
	finditem ITEM_DUSK_STONE
	end

IronmoorPrison_EventScript_Item_MaxEther::
	finditem ITEM_MAX_ETHER
	end

