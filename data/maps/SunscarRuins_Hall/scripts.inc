SunscarRuins_Hall_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SunscarRuins_Hall_OnLoad
	.byte 0

@ Once the third riddle is answered the vault stays open.
SunscarRuins_Hall_OnLoad:
	call_if_set FLAG_SUNSCAR_RIDDLE_3, SunscarRuins_Hall_EventScript_OpenDoor
	end

SunscarRuins_Hall_EventScript_OpenDoor::
	setmetatile 11, 1, 554, TRUE
	setmetatile 12, 1, 555, TRUE
	setmetatile 13, 1, 556, TRUE
	setmetatile 11, 2, 562, TRUE
	setmetatile 12, 2, 563, FALSE
	setmetatile 13, 2, 564, TRUE
	return

SunscarRuins_Hall_EventScript_OpenVault::
	msgbox SunscarRuins_Hall_Text_Rumble, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 5
	special ShakeCamera
	waitstate
	playse SE_BANG
	call SunscarRuins_Hall_EventScript_OpenDoor
	special DrawWholeMapView
	delay 40
	msgbox SunscarRuins_Hall_Text_Opened, MSGBOX_DEFAULT
	releaseall
	end

SunscarRuins_Hall_EventScript_Nothing::
	msgbox SunscarRuins_Hall_Text_Nothing, MSGBOX_DEFAULT
SunscarRuins_Hall_EventScript_GlyphLeave::
	releaseall
	end

SunscarRuins_Hall_EventScript_StatueSolved::
	msgbox SunscarRuins_Hall_Text_StatueSolved, MSGBOX_DEFAULT
	release
	end

SunscarRuins_Hall_EventScript_StatueSilent::
	msgbox SunscarRuins_Hall_Text_StatueSilent, MSGBOX_DEFAULT
	release
	end

SunscarRuins_Hall_EventScript_GlyphSun::
	lockall
	msgbox SunscarRuins_Hall_Text_GlyphSun, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunscarRuins_Hall_EventScript_GlyphLeave
	goto_if_set FLAG_SUNSCAR_RIDDLE_1, SunscarRuins_Hall_EventScript_Nothing
	setflag FLAG_SUNSCAR_RIDDLE_1
	playse SE_M_REFLECT
	waitse
	msgbox SunscarRuins_Hall_Text_StatueGlows, MSGBOX_DEFAULT
	releaseall
	end

SunscarRuins_Hall_EventScript_GlyphMoon::
	lockall
	msgbox SunscarRuins_Hall_Text_GlyphMoon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunscarRuins_Hall_EventScript_GlyphLeave
	goto_if_unset FLAG_SUNSCAR_RIDDLE_1, SunscarRuins_Hall_EventScript_Nothing
	goto_if_set FLAG_SUNSCAR_RIDDLE_2, SunscarRuins_Hall_EventScript_Nothing
	setflag FLAG_SUNSCAR_RIDDLE_2
	playse SE_M_REFLECT
	waitse
	msgbox SunscarRuins_Hall_Text_StatueGlows, MSGBOX_DEFAULT
	releaseall
	end

SunscarRuins_Hall_EventScript_GlyphWave::
	lockall
	msgbox SunscarRuins_Hall_Text_GlyphWave, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunscarRuins_Hall_EventScript_GlyphLeave
	goto_if_unset FLAG_SUNSCAR_RIDDLE_2, SunscarRuins_Hall_EventScript_Nothing
	goto_if_set FLAG_SUNSCAR_RIDDLE_3, SunscarRuins_Hall_EventScript_Nothing
	setflag FLAG_SUNSCAR_RIDDLE_3
	playse SE_M_REFLECT
	waitse
	msgbox SunscarRuins_Hall_Text_StatueGlows, MSGBOX_DEFAULT
	goto SunscarRuins_Hall_EventScript_OpenVault

SunscarRuins_Hall_EventScript_GlyphFlame::
	lockall
	msgbox SunscarRuins_Hall_Text_GlyphFlame, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunscarRuins_Hall_EventScript_GlyphLeave
	goto SunscarRuins_Hall_EventScript_Nothing

SunscarRuins_Hall_EventScript_Statue1::
	lock
	goto_if_set FLAG_SUNSCAR_RIDDLE_1, SunscarRuins_Hall_EventScript_StatueSolved
	playmoncry SPECIES_UNOWN, CRY_MODE_NORMAL
	msgbox SunscarRuins_Hall_Text_Riddle1, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunscarRuins_Hall_Text_Riddle1:
	.string "The UNOWN statue whispers…\p"
	.string "Each dawn I climb the eastern stair\n"
	.string "and warm the sand and burn the air.$"

SunscarRuins_Hall_EventScript_Statue2::
	lock
	goto_if_set FLAG_SUNSCAR_RIDDLE_2, SunscarRuins_Hall_EventScript_StatueSolved
	goto_if_unset FLAG_SUNSCAR_RIDDLE_1, SunscarRuins_Hall_EventScript_StatueSilent
	playmoncry SPECIES_UNOWN, CRY_MODE_NORMAL
	msgbox SunscarRuins_Hall_Text_Riddle2, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunscarRuins_Hall_Text_Riddle2:
	.string "The UNOWN statue whispers…\p"
	.string "I wear the light of one who goes\n"
	.string "and rule the sky when daylight closes.$"

SunscarRuins_Hall_EventScript_Statue3::
	lock
	goto_if_set FLAG_SUNSCAR_RIDDLE_3, SunscarRuins_Hall_EventScript_StatueSolved
	goto_if_unset FLAG_SUNSCAR_RIDDLE_2, SunscarRuins_Hall_EventScript_StatueSilent
	playmoncry SPECIES_UNOWN, CRY_MODE_NORMAL
	msgbox SunscarRuins_Hall_Text_Riddle3, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunscarRuins_Hall_Text_Riddle3:
	.string "The UNOWN statue whispers…\p"
	.string "Without me every flower dies.\n"
	.string "The desert dreams of me, and sighs.$"

SunscarRuins_Hall_Text_GlyphSun:
	.string "A glyph of the RISING SUN.\n"
	.string "Press your hand to it?$"

SunscarRuins_Hall_Text_GlyphMoon:
	.string "A glyph of the CRESCENT MOON.\n"
	.string "Press your hand to it?$"

SunscarRuins_Hall_Text_GlyphWave:
	.string "A glyph of the ROLLING WAVE.\n"
	.string "Press your hand to it?$"

SunscarRuins_Hall_Text_GlyphFlame:
	.string "A glyph of the DANCING FLAME.\n"
	.string "Press your hand to it?$"

SunscarRuins_Hall_Text_StatueGlows:
	.string "One of the UNOWN statues began to\n"
	.string "glow!$"

SunscarRuins_Hall_Text_Nothing:
	.string "The glyph is cold. Nothing happens.$"

SunscarRuins_Hall_Text_StatueSolved:
	.string "The UNOWN statue glows softly.$"

SunscarRuins_Hall_Text_StatueSilent:
	.string "The UNOWN statue is silent. Maybe\n"
	.string "another one speaks first.$"

SunscarRuins_Hall_Text_Rumble:
	.string "All three statues are glowing!\n"
	.string "The ruins begin to rumble…$"

SunscarRuins_Hall_Text_Opened:
	.string "The sealed door to the north has\n"
	.string "opened!$"

