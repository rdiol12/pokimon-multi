@ The Rift Below (src/dungeon.c): dug out anew on every floor.

AdventureDungeon_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, AdventureDungeon_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, AdventureDungeon_OnLoad
	map_script MAP_SCRIPT_ON_RESUME, AdventureDungeon_OnLoad
	.byte 0

AdventureDungeon_OnTransition:
	callnative Dungeon_SetupObjects
	end

AdventureDungeon_OnLoad:
	callnative Dungeon_DrawMap
	end

AdventureDungeon_EventScript_Item::
	callnative Dungeon_GetItem
	callstd STD_FIND_ITEM
	end

AdventureDungeon_EventScript_Ladder::
	lockall
	callnative Dungeon_GetNextFloor
	msgbox AdventureDungeon_Text_Ladder, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, AdventureDungeon_EventScript_LadderNo
	closemessage
	playse SE_EXIT
	callnative Dungeon_Descend
	warp MAP_ADVENTURE_DUNGEON, 16, 27
	waitstate
	releaseall
	end

AdventureDungeon_EventScript_LadderNo:
	releaseall
	end

AdventureDungeon_Text_Ladder:
	.string "A ladder leads down into the dark.\n"
	.string "Climb down to B{STR_VAR_1}F?$"
