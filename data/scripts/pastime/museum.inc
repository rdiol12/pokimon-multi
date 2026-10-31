@ Museum donations (src/pastime_museum.c), in the Lilycove Museum.

Museum_EventScript_Curator::
	lock
	faceplayer
	msgbox Museum_Text_Welcome, MSGBOX_DEFAULT
Museum_EventScript_CuratorMenu:
	message Museum_Text_WhatToDo
	waitmessage
	dynmultipush Museum_Text_MenuFossil, 0
	dynmultipush Museum_Text_MenuRare, 1
	dynmultipush Museum_Text_MenuPhotos, 2
	dynmultipush Museum_Text_MenuBye, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Museum_EventScript_Fossil
	case 1, Museum_EventScript_Rare
	case 2, Museum_EventScript_Photos
	msgbox Museum_Text_Bye, MSGBOX_DEFAULT
	release
	end

Museum_EventScript_Fossil:
	callnative Museum_FindFossil
	goto_if_eq VAR_RESULT, FALSE, Museum_EventScript_NoFossil
	msgbox Museum_Text_DonateFossil, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Museum_EventScript_CuratorMenu
	callnative Museum_DonateFossil
	playfanfare MUS_OBTAIN_ITEM
	message Museum_Text_FossilDonated
	waitmessage
	waitfanfare
	msgbox Museum_Text_FossilModel, MSGBOX_DEFAULT
	goto Museum_EventScript_Reward

Museum_EventScript_NoFossil:
	msgbox Museum_Text_NoFossil, MSGBOX_DEFAULT
	goto Museum_EventScript_CuratorMenu

Museum_EventScript_Rare:
	msgbox Museum_Text_ShowRare, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, Museum_EventScript_CuratorMenu
	callnative Museum_ShowRare
	switch VAR_RESULT
	case MUSEUM_RARE_NOT_RARE, Museum_EventScript_NotRare
	case MUSEUM_RARE_ALREADY, Museum_EventScript_AlreadyRare
	case MUSEUM_RARE_FULL, Museum_EventScript_RareFull
	case MUSEUM_RARE_EGG, Museum_EventScript_RareEgg
	playfanfare MUS_OBTAIN_ITEM
	message Museum_Text_RareRecorded
	waitmessage
	waitfanfare
	msgbox Museum_Text_RareModel, MSGBOX_DEFAULT
	goto Museum_EventScript_Reward

Museum_EventScript_NotRare:
	msgbox Museum_Text_NotRare, MSGBOX_DEFAULT
	goto Museum_EventScript_CuratorMenu

Museum_EventScript_AlreadyRare:
	msgbox Museum_Text_AlreadyRare, MSGBOX_DEFAULT
	goto Museum_EventScript_CuratorMenu

Museum_EventScript_RareFull:
	msgbox Museum_Text_RareFull, MSGBOX_DEFAULT
	goto Museum_EventScript_CuratorMenu

Museum_EventScript_RareEgg:
	msgbox Museum_Text_RareEgg, MSGBOX_DEFAULT
	goto Museum_EventScript_CuratorMenu

Museum_EventScript_Photos:
	callnative Museum_DonatePhotos
	goto_if_eq VAR_RESULT, 0, Museum_EventScript_NoPhotos
	msgbox Museum_Text_PhotosDonated, MSGBOX_DEFAULT
	goto Museum_EventScript_Reward

Museum_EventScript_NoPhotos:
	msgbox Museum_Text_NoPhotos, MSGBOX_DEFAULT
	goto Museum_EventScript_CuratorMenu

Museum_EventScript_Reward:
	callnative Museum_CheckReward
	goto_if_eq VAR_RESULT, FALSE, Museum_EventScript_CuratorMenu
	msgbox Museum_Text_Reward, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	goto Museum_EventScript_Reward

Museum_EventScript_Statue::
	lock
	callnative Museum_DescribeStatue
	msgbox gPastimeText, MSGBOX_DEFAULT
	release
	end

Museum_Text_Welcome:
	.string "Welcome to the NATURAL HISTORY\n"
	.string "WING! I'm the curator.\p"
	.string "We're building a hall of fossils\n"
	.string "and rare Pokémon. Can you help?$"

Museum_Text_WhatToDo:
	.string "What have you brought for us?$"

Museum_Text_MenuFossil:
	.string "DONATE FOSSIL$"
Museum_Text_MenuRare:
	.string "SHOW RARE POKéMON$"
Museum_Text_MenuPhotos:
	.string "DONATE PHOTOS$"
Museum_Text_MenuBye:
	.string "BYE$"

Museum_Text_Bye:
	.string "Come back with more discoveries!$"

Museum_Text_DonateFossil:
	.string "Oh! Is that a {STR_VAR_1}?\n"
	.string "May the museum keep it?$"

Museum_Text_FossilDonated:
	.string "{PLAYER} donated the\n{STR_VAR_1}!$"

Museum_Text_FossilModel:
	.string "Wonderful! We'll build a model of\n"
	.string "{STR_VAR_2} from it for the hall.$"

Museum_Text_NoFossil:
	.string "You don't have a fossil we don't\n"
	.string "already have.\p"
	.string "They turn up in caves, digs and\n"
	.string "deep underground.$"

Museum_Text_ShowRare:
	.string "Show me a rare Pokémon: a legend,\n"
	.string "a shiny, an ULTRA BEAST…\p"
	.string "Don't worry, it stays with you.\n"
	.string "We only take notes and sketches!$"

Museum_Text_RareRecorded:
	.string "The curator sketched {STR_VAR_1}\n"
	.string "for the rare Pokémon hall!$"

Museum_Text_RareModel:
	.string "Magnificent! Our model makers will\n"
	.string "get to work right away.$"

Museum_Text_NotRare:
	.string "A fine {STR_VAR_1}, but not a rare\n"
	.string "one, I'm afraid.$"

Museum_Text_AlreadyRare:
	.string "We already have {STR_VAR_1} in\n"
	.string "the hall. Thank you, though!$"

Museum_Text_RareFull:
	.string "The rare Pokémon hall is full!\n"
	.string "What a collection!$"

Museum_Text_RareEgg:
	.string "An EGG? We'll wait until it hatches,\n"
	.string "thank you.$"

Museum_Text_PhotosDonated:
	.string "{STR_VAR_1} new photos! We'll hang them\n"
	.string "on the photo wall.$"

Museum_Text_NoPhotos:
	.string "Take photos of Pokémon with\n"
	.string "R > PHOTO, then bring them here!$"

Museum_Text_Reward:
	.string "The museum is growing thanks to\n"
	.string "you. Please accept this.$"
