StarfallIsle_Cave_MapScripts::
	.byte 0

StarfallIsle_Cave_EventScript_Jirachi::
	lock
	faceplayer
	goto_if_unset FLAG_SYS_GAME_CLEAR, StarfallIsle_Cave_EventScript_JirachiAsleep
	msgbox StarfallIsle_Cave_Text_Glow, MSGBOX_DEFAULT
	closemessage
	waitse
	playmoncry SPECIES_JIRACHI, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_JIRACHI, 50
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	dowildbattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, StarfallIsle_Cave_EventScript_JirachiDefeated
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, StarfallIsle_Cave_EventScript_JirachiRan
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, StarfallIsle_Cave_EventScript_JirachiRan
	setflag FLAG_STARFALL_JIRACHI_DONE
	release
	end

StarfallIsle_Cave_EventScript_JirachiDefeated::
	setflag FLAG_STARFALL_JIRACHI_DONE
	goto Common_EventScript_RemoveStaticPokemon
	end

StarfallIsle_Cave_EventScript_JirachiRan::
	setvar VAR_0x8004, SPECIES_JIRACHI
	goto Common_EventScript_LegendaryFlewAway
	end

StarfallIsle_Cave_Text_Glow:
	.string "A tiny POKéMON is floating in a\nsoft, starry glow...$"

@ The island opens at eight badges; JIRACHI only wakes for the CHAMPION.
StarfallIsle_Cave_EventScript_JirachiAsleep:
	msgbox StarfallIsle_Cave_Text_JirachiAsleep, MSGBOX_DEFAULT
	release
	end

StarfallIsle_Cave_Text_JirachiAsleep:
	.string "JIRACHI is fast asleep, curled up in\n"
	.string "the starlight.\p"
	.string "It stirs a little… Maybe it will wake\n"
	.string "for the CHAMPION of HOENN.$"
