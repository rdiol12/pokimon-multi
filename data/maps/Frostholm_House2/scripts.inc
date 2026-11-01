Frostholm_House2_MapScripts::
	.byte 0

Frostholm_House2_EventScript_Boy::
	msgbox Frostholm_House2_EventScript_Boy_Text, MSGBOX_NPC
	end

Frostholm_House2_EventScript_Boy_Text:
	.string "My SNOM eats the frost off the\n"
	.string "windows. It's very helpful!$"

Frostholm_House2_EventScript_Snom::
	msgbox Frostholm_House2_EventScript_Snom_Text, MSGBOX_NPC
	end

Frostholm_House2_EventScript_Snom_Text:
	.string "SNOM: Snom!$"
