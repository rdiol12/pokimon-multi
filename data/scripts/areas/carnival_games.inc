@ Sunfair Carnival game hosts (tools/areas/carnival.py). The minigames
@ branch (claude/pastime-carnival) replaces each body with
@ goto Pastime_EventScript_<game>.
Carnival_EventScript_Bowling::
	msgbox Carnival_Text_BowlingSoon, MSGBOX_NPC
	end

Carnival_EventScript_Golf::
	msgbox Carnival_Text_GolfSoon, MSGBOX_NPC
	end

Carnival_EventScript_Soccer::
	msgbox Carnival_Text_SoccerSoon, MSGBOX_NPC
	end

Carnival_EventScript_FerrisWheel::
	msgbox Carnival_Text_FerrisWheelSoon, MSGBOX_NPC
	end

Carnival_Text_BowlingSoon:
	.string "The bowling lanes are being polished.\n"
	.string "Coming soon!$"

Carnival_Text_GolfSoon:
	.string "Mini golf opens soon! We're still\n"
	.string "mowing the green.$"

Carnival_Text_SoccerSoon:
	.string "We're picking teams for a POKéMON\n"
	.string "soccer match. Coming soon!$"

Carnival_Text_FerrisWheelSoon:
	.string "The Ferris wheel is getting its last\n"
	.string "safety check. Rides start soon!$"
