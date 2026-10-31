@ Hardcore (Nuzlocke) mode: the Challenge Clerk in Oldale Town, and the
@ messages after a battle (src/world_hardcore.c).

World_EventScript_ChallengeClerk::
	lock
	faceplayer
	callnative Hardcore_GetStatus
	goto_if_eq VAR_RESULT, HARDCORE_ON, World_EventScript_ClerkRunning
	goto_if_eq VAR_RESULT, HARDCORE_FAILED, World_EventScript_ClerkFailed
	callnative Hardcore_CanStart
	goto_if_eq VAR_RESULT, FALSE, World_EventScript_ClerkTooLate
	msgbox World_Text_ClerkRules, MSGBOX_DEFAULT
	msgbox World_Text_ClerkAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, World_EventScript_ClerkNo
	msgbox World_Text_ClerkSure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, World_EventScript_ClerkNo
	callnative Hardcore_Start
	playfanfare MUS_OBTAIN_BADGE
	msgbox World_Text_ClerkStarted, MSGBOX_DEFAULT
	waitfanfare
	release
	end

World_EventScript_ClerkNo:
	msgbox World_Text_ClerkNo, MSGBOX_DEFAULT
	release
	end

World_EventScript_ClerkTooLate:
	msgbox World_Text_ClerkTooLate, MSGBOX_DEFAULT
	release
	end

World_EventScript_ClerkRunning:
	msgbox World_Text_ClerkRunning, MSGBOX_DEFAULT
	release
	end

World_EventScript_ClerkFailed:
	msgbox World_Text_ClerkFailed, MSGBOX_DEFAULT
	release
	end

World_EventScript_HardcoreLosses::
	lockall
	goto_if_gt VAR_0x8004, 1, World_EventScript_HardcoreLossesMany
	msgbox World_Text_LostOne, MSGBOX_DEFAULT
	releaseall
	end
World_EventScript_HardcoreLossesMany:
	msgbox World_Text_LostMany, MSGBOX_DEFAULT
	releaseall
	end

World_EventScript_HardcoreOver::
	lockall
	msgbox World_Text_RunOver, MSGBOX_DEFAULT
	releaseall
	end

World_Text_ClerkRules:
	.string "I run the HARDCORE CHALLENGE.\p"
	.string "The rules are simple, and they're\n"
	.string "forever:\p"
	.string "One. In each area, only the first\n"
	.string "wild Pokémon you meet can be caught.\p"
	.string "Two. A Pokémon that faints is gone.\n"
	.string "It leaves your party for good.\p"
	.string "Three. If your whole party faints,\n"
	.string "your run is over.\p"
	.string "Online, runs go on the HARDCORE\n"
	.string "LADDER. Most badges, fewest losses!$"

World_Text_ClerkAsk:
	.string "Will you take the challenge?$"

World_Text_ClerkSure:
	.string "It can never be turned off.\n"
	.string "Are you sure?$"

World_Text_ClerkStarted:
	.string "Your HARDCORE run starts now!\n"
	.string "Good luck out there…$"

World_Text_ClerkNo:
	.string "Come back if you change your mind,\n"
	.string "before your first badge.$"

World_Text_ClerkTooLate:
	.string "The challenge has to start before\n"
	.string "your first Gym Badge. Sorry!$"

World_Text_ClerkRunning:
	.string "You're on a HARDCORE run.\n"
	.string "Pokémon lost so far: {STR_VAR_1}.\p"
	.string "Remember: first encounter only!$"

World_Text_ClerkFailed:
	.string "Your HARDCORE run ended…\n"
	.string "Pokémon lost: {STR_VAR_1}.\p"
	.string "You can keep playing as normal.$"

World_Text_LostOne:
	.string "{STR_VAR_1} fainted in battle…\p"
	.string "Under HARDCORE rules, it has left\n"
	.string "your party for good.$"

World_Text_LostMany:
	.string "{STR_VAR_1} and {STR_VAR_2} more fainted\n"
	.string "in battle…\p"
	.string "Under HARDCORE rules, they have left\n"
	.string "your party for good.$"

World_Text_RunOver:
	.string "Your whole party fainted…\p"
	.string "Your HARDCORE run is over.\n"
	.string "The game goes on as normal.$"
