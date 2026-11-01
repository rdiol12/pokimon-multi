LaurelCity_LeagueShop_MapScripts::
	.byte 0

LaurelCity_LeagueShop_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart LaurelCity_LeagueShop_EventScript_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
LaurelCity_LeagueShop_EventScript_Items:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_SP_ATK
	.2byte ITEM_X_SP_DEF
	.2byte ITEM_GUARD_SPEC
	pokemartlistend

LaurelCity_LeagueShop_EventScript_Boy::
	msgbox LaurelCity_LeagueShop_EventScript_Boy_Text, MSGBOX_NPC
	end

LaurelCity_LeagueShop_EventScript_Boy_Text:
	.string "X items in a league match? Smart!\n"
	.string "Every little bit helps.$"
