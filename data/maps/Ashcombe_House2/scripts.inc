Ashcombe_House2_MapScripts::
	.byte 0

Ashcombe_House2_EventScript_Girl::
	msgbox Ashcombe_House2_EventScript_Girl_Text, MSGBOX_NPC
	end

Ashcombe_House2_EventScript_Girl_Text:
	.string "My NUMEL sleeps by the stove.\n"
	.string "It says it's too cold outside!$"

Ashcombe_House2_EventScript_Numel::
	msgbox Ashcombe_House2_EventScript_Numel_Text, MSGBOX_NPC
	end

Ashcombe_House2_EventScript_Numel_Text:
	.string "NUMEL: Nuuumel...$"
