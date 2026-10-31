GullwingPoint_PirateHold_MapScripts::
	.byte 0

GullwingPoint_PirateHold_EventScript_Guard::
	lock
	faceplayer
	goto_if_set FLAG_GULL_CAPTAIN_BEATEN, GullwingPoint_PirateHold_EventScript_GuardOk
	msgbox GullwingPoint_PirateHold_Text_GuardNo, MSGBOX_DEFAULT
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, GullwingPoint_PirateHold_Movement_Out
	waitmovement 0
	warp MAP_GULLWING_POINT_PIRATE_SHIP, 9, 10
	waitstate
	release
	end

GullwingPoint_PirateHold_EventScript_GuardOk::
	msgbox GullwingPoint_PirateHold_Text_GuardOk, MSGBOX_DEFAULT
	release
	end

GullwingPoint_PirateHold_Movement_Out:
	walk_up
	step_end

GullwingPoint_PirateHold_EventScript_BigNugget::
	finditem ITEM_BIG_NUGGET
	end

GullwingPoint_PirateHold_EventScript_KingsRock::
	finditem ITEM_KINGS_ROCK
	end

GullwingPoint_PirateHold_EventScript_PearlString::
	finditem ITEM_PEARL_STRING
	end

GullwingPoint_PirateHold_Text_GuardNo:
	.string "Oi! Nobody touches the loot without\n"
	.string "the CAPTAIN's say-so! Out!$"

GullwingPoint_PirateHold_Text_GuardOk:
	.string "The CAPTAIN says it's yours. Take it\n"
	.string "before I change my mind.$"
