@ Berry picking minigame (src/world_records.c): runs before a tree is picked.

World_EventScript_BerryPickingGame::
	message World_Text_PickCarefully
	waitmessage
	callnative BerryGame_Start, waitstate=1
	closemessage
	return

World_Text_PickCarefully:
	.string "Pick carefully! Stop the needle in\n"
	.string "the green for an extra BERRY.$"
