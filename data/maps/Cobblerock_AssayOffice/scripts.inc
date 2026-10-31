Cobblerock_AssayOffice_MapScripts::
	.byte 0

Cobblerock_AssayOffice_EventScript_Assayer::
	lock
	faceplayer
	goto_if_set FLAG_COBBLE_ASSAY_DONE, Cobblerock_AssayOffice_EventScript_Done
	goto_if_unset FLAG_COBBLE_ORE_1, Cobblerock_AssayOffice_EventScript_Ask
	goto_if_unset FLAG_COBBLE_ORE_2, Cobblerock_AssayOffice_EventScript_Ask
	goto_if_unset FLAG_COBBLE_ORE_3, Cobblerock_AssayOffice_EventScript_Ask
	msgbox Cobblerock_AssayOffice_Text_Thanks, MSGBOX_DEFAULT
	givemon SPECIES_ROLYCOLY, 15
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, Cobblerock_AssayOffice_EventScript_Full
	setflag FLAG_COBBLE_ASSAY_DONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox Cobblerock_AssayOffice_Text_GotRolycoly, MSGBOX_DEFAULT
	waitfanfare
	msgbox Cobblerock_AssayOffice_Text_Shaft, MSGBOX_DEFAULT
	release
	end

Cobblerock_AssayOffice_EventScript_Ask::
	msgbox Cobblerock_AssayOffice_Text_Ask, MSGBOX_DEFAULT
	release
	end

Cobblerock_AssayOffice_EventScript_Full::
	msgbox Cobblerock_AssayOffice_Text_Full, MSGBOX_DEFAULT
	release
	end

Cobblerock_AssayOffice_EventScript_Done::
	msgbox Cobblerock_AssayOffice_Text_Done, MSGBOX_DEFAULT
	release
	end

Cobblerock_AssayOffice_EventScript_Clerk::
	msgbox Cobblerock_AssayOffice_Text_Clerk, MSGBOX_NPC
	end

Cobblerock_AssayOffice_Text_Ask:
	.string "I'm the ASSAYER. I test ore to see\n"
	.string "what the mine is worth.\p"
	.string "I need three fresh samples from\n"
	.string "the veins in COBBLEROCK MINE. One\l"
	.string "on 1F, two on B1F. Look for red\l"
	.string "and blue sparkles in the rock!$"

Cobblerock_AssayOffice_Text_Thanks:
	.string "Three samples! Let me see…\p"
	.string "Iron, copper… and traces of\n"
	.string "crystal! Just like the old shaft.\p"
	.string "Here, take this ROLYCOLY. It sniffs\n"
	.string "out coal better than any miner.$"

Cobblerock_AssayOffice_Text_GotRolycoly:
	.string "{PLAYER} received a ROLYCOLY!$"

Cobblerock_AssayOffice_Text_Shaft:
	.string "I'll tell the FOREMAN to move the\n"
	.string "boulder off the old shaft. With\l"
	.string "crystal like this, it must be worth\l"
	.string "a look!$"

Cobblerock_AssayOffice_Text_Full:
	.string "Your party's full. Make some room,\n"
	.string "then come back for your reward.$"

Cobblerock_AssayOffice_Text_Done:
	.string "The old shaft is open now. Crystal\n"
	.string "like that only grows where a\l"
	.string "CARBINK lives, you know.$"

Cobblerock_AssayOffice_Text_Clerk:
	.string "Every cart of ore that leaves\n"
	.string "COBBLEROCK gets weighed in here.$"
