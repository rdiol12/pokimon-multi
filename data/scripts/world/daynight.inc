@ Night-only visitors. Their objects use FLAG_HIDE_*_NIGHT_* flags, which
@ DayNight_UpdateFlags (src/world_daynight.c) sets by day and clears at night.

World_EventScript_OldaleStoryteller::
	lock
	faceplayer
	msgbox World_Text_StorytellerIntro, MSGBOX_DEFAULT
	goto_if_set FLAG_RECEIVED_NIGHT_SPELL_TAG, World_EventScript_StorytellerDone
	msgbox World_Text_StorytellerGift, MSGBOX_DEFAULT
	giveitem ITEM_SPELL_TAG
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_NIGHT_SPELL_TAG
World_EventScript_StorytellerDone:
	msgbox World_Text_StorytellerBye, MSGBOX_DEFAULT
	release
	end

World_EventScript_FallarborStargazer::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_STARGAZER_MOON_STONE, World_EventScript_StargazerDone
	msgbox World_Text_StargazerIntro, MSGBOX_DEFAULT
	giveitem ITEM_MOON_STONE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_STARGAZER_MOON_STONE
	release
	end
World_EventScript_StargazerDone:
	msgbox World_Text_StargazerDone, MSGBOX_DEFAULT
	release
	end

World_EventScript_NightMarket::
	lock
	faceplayer
	msgbox World_Text_NightMarketIntro, MSGBOX_DEFAULT
	pokemart World_NightMarket_Items
	msgbox World_Text_NightMarketBye, MSGBOX_DEFAULT
	release
	end

	.align 2
World_NightMarket_Items:
	.2byte ITEM_DUSK_BALL
	.2byte ITEM_REAPER_CLOTH
	.2byte ITEM_MOON_STONE
	.2byte ITEM_DUSK_STONE
	.2byte ITEM_SPELL_TAG
	.2byte ITEM_BLACK_GLASSES
	pokemartlistend

World_Text_StorytellerIntro:
	.string "Hehehe… You're out late too?\p"
	.string "When the sun goes down, Pokémon you\n"
	.string "never see by day come out.\p"
	.string "HOOTHOOT hoot in the trees, SPINARAK\n"
	.string "spin webs in the grass…\p"
	.string "and the ghosts of MT. PYRE wander\n"
	.string "as far as ROUTE 123.$"

World_Text_StorytellerGift:
	.string "Take this charm. It keeps the\n"
	.string "spirits friendly…$"

World_Text_StorytellerBye:
	.string "Night is short. Go and look!$"

World_Text_StargazerIntro:
	.string "On clear nights the stars over\n"
	.string "FALLARBOR are so bright…\p"
	.string "A piece of a falling star landed\n"
	.string "right here. It's yours, if you\l"
	.string "promise to come and look up too.$"

World_Text_StargazerDone:
	.string "They say CLEFAIRY dance under the\n"
	.string "moon when nobody is watching.$"

World_Text_NightMarketIntro:
	.string "Welcome to the NIGHT MARKET!\n"
	.string "Open from dusk till dawn only.$"

World_Text_NightMarketBye:
	.string "Come back after dark!$"
