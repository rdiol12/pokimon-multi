SolmaraVictoryRoad_MapScripts::
	.byte 0

SolmaraVictoryRoad_EventScript_Ace_Kai::
	trainerbattle_single TRAINER_SOL_VR_ACE_KAI, SolmaraVictoryRoad_EventScript_Ace_Kai_Intro, SolmaraVictoryRoad_EventScript_Ace_Kai_Defeat
	msgbox SolmaraVictoryRoad_EventScript_Ace_Kai_Post, MSGBOX_AUTOCLOSE
	end

SolmaraVictoryRoad_EventScript_Ace_Kai_Intro:
	.string "Only the best get this far.\n"
	.string "Are you one of them?$"

SolmaraVictoryRoad_EventScript_Ace_Kai_Defeat:
	.string "You are one of them.$"

SolmaraVictoryRoad_EventScript_Ace_Kai_Post:
	.string "The LEAGUE is just ahead. Rest\n"
	.string "up before you go in.$"

SolmaraVictoryRoad_EventScript_Ace_Lyra::
	trainerbattle_single TRAINER_SOL_VR_ACE_LYRA, SolmaraVictoryRoad_EventScript_Ace_Lyra_Intro, SolmaraVictoryRoad_EventScript_Ace_Lyra_Defeat
	msgbox SolmaraVictoryRoad_EventScript_Ace_Lyra_Post, MSGBOX_AUTOCLOSE
	end

SolmaraVictoryRoad_EventScript_Ace_Lyra_Intro:
	.string "Only the best get this far.\n"
	.string "Are you one of them?$"

SolmaraVictoryRoad_EventScript_Ace_Lyra_Defeat:
	.string "You are one of them.$"

SolmaraVictoryRoad_EventScript_Ace_Lyra_Post:
	.string "The LEAGUE is just ahead. Rest\n"
	.string "up before you go in.$"

SolmaraVictoryRoad_EventScript_Ace_Rolf::
	trainerbattle_single TRAINER_SOL_VR_ACE_ROLF, SolmaraVictoryRoad_EventScript_Ace_Rolf_Intro, SolmaraVictoryRoad_EventScript_Ace_Rolf_Defeat
	msgbox SolmaraVictoryRoad_EventScript_Ace_Rolf_Post, MSGBOX_AUTOCLOSE
	end

SolmaraVictoryRoad_EventScript_Ace_Rolf_Intro:
	.string "Only the best get this far.\n"
	.string "Are you one of them?$"

SolmaraVictoryRoad_EventScript_Ace_Rolf_Defeat:
	.string "You are one of them.$"

SolmaraVictoryRoad_EventScript_Ace_Rolf_Post:
	.string "The LEAGUE is just ahead. Rest\n"
	.string "up before you go in.$"

SolmaraVictoryRoad_EventScript_Ace_Nova::
	trainerbattle_single TRAINER_SOL_VR_ACE_NOVA, SolmaraVictoryRoad_EventScript_Ace_Nova_Intro, SolmaraVictoryRoad_EventScript_Ace_Nova_Defeat
	msgbox SolmaraVictoryRoad_EventScript_Ace_Nova_Post, MSGBOX_AUTOCLOSE
	end

SolmaraVictoryRoad_EventScript_Ace_Nova_Intro:
	.string "Only the best get this far.\n"
	.string "Are you one of them?$"

SolmaraVictoryRoad_EventScript_Ace_Nova_Defeat:
	.string "You are one of them.$"

SolmaraVictoryRoad_EventScript_Ace_Nova_Post:
	.string "The LEAGUE is just ahead. Rest\n"
	.string "up before you go in.$"

SolmaraVictoryRoad_EventScript_Item_FullRestore::
	finditem ITEM_FULL_RESTORE
	end

SolmaraVictoryRoad_EventScript_Item_MaxElixir::
	finditem ITEM_MAX_ELIXIR
	end

SolmaraVictoryRoad_EventScript_Item_RareCandy::
	finditem ITEM_RARE_CANDY
	end

SolmaraVictoryRoad_EventScript_Item_TmEarthquake::
	finditem ITEM_TM_EARTHQUAKE
	end

SolmaraVictoryRoad_EventScript_Item_MaxRevive::
	finditem ITEM_MAX_REVIVE
	end

