StarfallIsle_ExplorersHouse_MapScripts::
	.byte 0

StarfallIsle_ExplorersHouse_EventScript_Explorer::
	lock
	faceplayer
	goto_if_set FLAG_STARFALL_EXPLORER_REWARD, StarfallIsle_ExplorersHouse_EventScript_Done
	goto_if_set FLAG_STARFALL_JIRACHI_DONE, StarfallIsle_ExplorersHouse_EventScript_Reward
	msgbox StarfallIsle_ExplorersHouse_Text_Hint, MSGBOX_DEFAULT
	release
	end

StarfallIsle_ExplorersHouse_EventScript_Reward::
	msgbox StarfallIsle_ExplorersHouse_Text_YouFoundIt, MSGBOX_DEFAULT
	giveitem ITEM_COMET_SHARD
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_STARFALL_EXPLORER_REWARD
	release
	end

StarfallIsle_ExplorersHouse_EventScript_Done::
	msgbox StarfallIsle_ExplorersHouse_Text_Done, MSGBOX_DEFAULT
	release
	end

StarfallIsle_ExplorersHouse_Text_Hint:
	.string "I'm IRIS, an explorer! I came here\nchasing a legend.\pA POKéMON that grants wishes sleeps\nat the heart of STARFALL CAVE.\pAnd one more secret: the forest west\nof the village hides a narrow path.\pFollow it and you'll find a clearing\nnobody else knows about!$"

StarfallIsle_ExplorersHouse_Text_YouFoundIt:
	.string "You met the wishing POKéMON?!\nThe legend is true!\pHere, take this. I found it on the\nbeach the night the star fell.$"

StarfallIsle_ExplorersHouse_Text_Done:
	.string "Next I'm off to search for the\nlegends of other regions!$"
