@ Legendary quests (src/world_legends.c). Legends_TryInteract starts these
@ with VAR_0x8004 = quest, VAR_0x8005 = species, STR_VAR_1 = its name.

World_EventScript_LegendFlee::
	lockall
	playmoncry VAR_0x8005, CRY_MODE_ENCOUNTER
	applymovement LOCALID_WORLD_LEGEND, World_Movement_LegendStartle
	waitmovement 0
	waitmoncry
	applymovement LOCALID_WORLD_LEGEND, World_Movement_LegendRunOff
	waitmovement 0
	callnative Legends_Flee
	msgbox gStringVar4, MSGBOX_DEFAULT
	releaseall
	end

World_EventScript_LegendBattle::
	lockall
	playmoncry VAR_0x8005, CRY_MODE_ENCOUNTER
	msgbox World_Text_LegendStands, MSGBOX_DEFAULT
	waitmoncry
	callnative Legends_SetupBattle
	dowildbattle
	callnative Legends_AfterBattle
	goto_if_eq VAR_RESULT, TRUE, World_EventScript_LegendCaught
	msgbox World_Text_LegendWillReturn, MSGBOX_DEFAULT
	releaseall
	end

World_EventScript_LegendCaught:
	releaseall
	end

World_Movement_LegendStartle:
	jump_in_place_down
	step_end

World_Movement_LegendRunOff:
	walk_faster_up
	walk_faster_up
	walk_faster_up
	set_invisible
	step_end

World_Text_LegendStands:
	.string "{STR_VAR_1} stands its ground!$"

World_Text_LegendWillReturn:
	.string "{STR_VAR_1} slipped away…\n"
	.string "It might come back here.$"
