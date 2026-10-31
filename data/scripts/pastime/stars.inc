@ Star map and meteor shower nights (src/pastime_stars.c).

Star_EventScript_Astronomer::
	lock
	faceplayer
	callnative Star_TalkToAstronomer
	msgbox Star_Text_Astronomer, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8006, TRUE, Star_EventScript_ShowerTonight
	msgbox Star_Text_NextShower, MSGBOX_DEFAULT
	goto Star_EventScript_AstronomerPrize

Star_EventScript_ShowerTonight:
	msgbox Star_Text_ShowerTonight, MSGBOX_DEFAULT
Star_EventScript_AstronomerPrize:
	goto_if_eq VAR_0x8005, ITEM_NONE, Star_EventScript_AstronomerBye
	msgbox Star_Text_Prize, MSGBOX_DEFAULT
	giveitem VAR_0x8005
Star_EventScript_AstronomerBye:
	release
	end

Star_EventScript_Stargazer::
	lock
	faceplayer
	callnative Star_Look
	goto_if_eq VAR_0x8006, TRUE, Star_EventScript_StargazerMeteors
	msgbox Star_Text_Stargazer, MSGBOX_DEFAULT
	goto Star_EventScript_Telescope

Star_EventScript_StargazerMeteors:
	msgbox Star_Text_StargazerMeteors, MSGBOX_DEFAULT
Star_EventScript_Telescope:
	callnative Star_ShowChart
	msgbox Star_Text_Chart, MSGBOX_DEFAULT
	callnative Star_HideChart
	setvar VAR_0x8006, 0
	setvar VAR_0x8007, 0
	callnative Pastime_ShowMonPic
	msgbox Star_Text_LooksLike, MSGBOX_DEFAULT
	callnative Pastime_HideMonPic
	goto_if_eq VAR_RESULT, FALSE, Star_EventScript_StargazerBye
	playfanfare MUS_OBTAIN_ITEM
	message Star_Text_Logged
	waitmessage
	waitfanfare
	waitbuttonpress
Star_EventScript_StargazerBye:
	release
	end

Star_EventScript_Meteorite::
	lock
	msgbox Star_Text_Meteorite, MSGBOX_DEFAULT
	callnative Star_PickMeteorite
	giveitem VAR_0x8005
	release
	end

Star_Text_Astronomer:
	.string "The night sky over HOENN is full of\n"
	.string "constellations shaped like Pokémon.\p"
	.string "Stargazers come out at night all\n"
	.string "over HOENN. Ask to look through\l"
	.string "their telescopes!\p"
	.string "Your STAR MAP has {STR_VAR_1} of 8\n"
	.string "so far.$"

Star_Text_NextShower:
	.string "My charts say the next meteor\n"
	.string "shower comes in {STR_VAR_2} night(s).\p"
	.string "Meteorites fall around METEOR\n"
	.string "FALLS on those nights!$"

Star_Text_ShowerTonight:
	.string "And tonight there's a meteor\n"
	.string "shower! Look for meteorites\l"
	.string "around METEOR FALLS!$"

Star_Text_Prize:
	.string "Your STAR MAP is filling up.\n"
	.string "Here, a gift from the stars!$"

Star_Text_Stargazer:
	.string "What a clear night! Want to look\n"
	.string "through my telescope?$"

Star_Text_StargazerMeteors:
	.string "Look, shooting stars everywhere!\n"
	.string "Here, take a look!$"

Star_Text_Chart:
	.string "That's {STR_VAR_1}!$"

Star_Text_LooksLike:
	.string "They say it's a {STR_VAR_2}\n"
	.string "watching over HOENN.$"

Star_Text_Logged:
	.string "{STR_VAR_1} was added to\n"
	.string "the STAR MAP!$"

Star_Text_Meteorite:
	.string "A small meteorite, still warm!\n"
	.string "Something sparkles inside…$"
