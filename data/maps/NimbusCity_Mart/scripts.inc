NimbusCity_Mart_MapScripts::
	.byte 0

NimbusCity_Mart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart NimbusCity_Mart_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
NimbusCity_Mart_Pokemart:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_REVIVE
	.2byte ITEM_PRETTY_FEATHER
	.2byte ITEM_SHARP_BEAK
	pokemartlistend

NimbusCity_Mart_EventScript_Woman::
	msgbox NimbusCity_Mart_Text_Woman, MSGBOX_NPC
	end

NimbusCity_Mart_Text_Woman:
	.string "Everything here comes up by\n"
	.string "balloon. Even the shop's floor!$"
