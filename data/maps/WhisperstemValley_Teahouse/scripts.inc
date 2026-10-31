WhisperstemValley_Teahouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, WhisperstemValley_Teahouse_OnTransition
	.byte 0

WhisperstemValley_Teahouse_OnTransition:
	setflag FLAG_HIDE_BAMBOO_HOME_1
	setflag FLAG_HIDE_BAMBOO_HOME_2
	setflag FLAG_HIDE_BAMBOO_HOME_3
	goto_if_set FLAG_BAMBOO_TEA_DONE, WhisperstemValley_Teahouse_OnTransitionDone
	call_if_set FLAG_BAMBOO_PANCHAM_1, WhisperstemValley_Teahouse_EventScript_Home1
	call_if_set FLAG_BAMBOO_PANCHAM_2, WhisperstemValley_Teahouse_EventScript_Home2
	call_if_set FLAG_BAMBOO_PANCHAM_3, WhisperstemValley_Teahouse_EventScript_Home3
	end

@ once the quest is done, one of the three has gone with you
WhisperstemValley_Teahouse_OnTransitionDone:
	clearflag FLAG_HIDE_BAMBOO_HOME_1
	clearflag FLAG_HIDE_BAMBOO_HOME_3
	end

WhisperstemValley_Teahouse_EventScript_Home1::
	clearflag FLAG_HIDE_BAMBOO_HOME_1
	return

WhisperstemValley_Teahouse_EventScript_Home2::
	clearflag FLAG_HIDE_BAMBOO_HOME_2
	return

WhisperstemValley_Teahouse_EventScript_Home3::
	clearflag FLAG_HIDE_BAMBOO_HOME_3
	return

WhisperstemValley_Teahouse_EventScript_Master::
	lock
	faceplayer
	goto_if_set FLAG_BAMBOO_TEA_DONE, WhisperstemValley_Teahouse_EventScript_MasterDone
	goto_if_unset FLAG_BAMBOO_PANCHAM_1, WhisperstemValley_Teahouse_EventScript_MasterAsk
	goto_if_unset FLAG_BAMBOO_PANCHAM_2, WhisperstemValley_Teahouse_EventScript_MasterAsk
	goto_if_unset FLAG_BAMBOO_PANCHAM_3, WhisperstemValley_Teahouse_EventScript_MasterAsk
	msgbox WhisperstemValley_Teahouse_Text_MasterThanks, MSGBOX_DEFAULT
	givemon SPECIES_PANCHAM, 12
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, WhisperstemValley_Teahouse_EventScript_MasterFull
	setflag FLAG_BAMBOO_TEA_DONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox WhisperstemValley_Teahouse_Text_GotPancham, MSGBOX_DEFAULT
	waitfanfare
	msgbox WhisperstemValley_Teahouse_Text_MasterAfter, MSGBOX_DEFAULT
	release
	end

WhisperstemValley_Teahouse_EventScript_MasterAsk::
	msgbox WhisperstemValley_Teahouse_Text_MasterAsk, MSGBOX_DEFAULT
	release
	end

WhisperstemValley_Teahouse_EventScript_MasterFull::
	msgbox WhisperstemValley_Teahouse_Text_MasterFull, MSGBOX_DEFAULT
	release
	end

WhisperstemValley_Teahouse_EventScript_MasterDone::
	msgbox WhisperstemValley_Teahouse_Text_MasterDone, MSGBOX_DEFAULT
	release
	end

WhisperstemValley_Teahouse_EventScript_Pancham::
	lock
	faceplayer
	playmoncry SPECIES_PANCHAM, CRY_MODE_NORMAL
	msgbox WhisperstemValley_Teahouse_Text_Pancham, MSGBOX_DEFAULT
	waitmoncry
	release
	end

WhisperstemValley_Teahouse_EventScript_Guest::
	msgbox WhisperstemValley_Teahouse_Text_Guest, MSGBOX_NPC
	end

WhisperstemValley_Teahouse_Text_MasterAsk:
	.string "Welcome, welcome. Forgive the\n"
	.string "mess… my three PANCHAM ran off\l"
	.string "into the bamboo again.\p"
	.string "They hide where the paths end.\n"
	.string "Would you bring them home?$"

WhisperstemValley_Teahouse_Text_MasterThanks:
	.string "All three are home! Thank you.\p"
	.string "This little one keeps looking at\n"
	.string "you. I think it wants to go with\l"
	.string "you. Please, take it along.$"

WhisperstemValley_Teahouse_Text_GotPancham:
	.string "{PLAYER} received a PANCHAM!$"

WhisperstemValley_Teahouse_Text_MasterAfter:
	.string "Raise it well. It may grow into a\n"
	.string "fine PANGORO one day.$"

WhisperstemValley_Teahouse_Text_MasterFull:
	.string "Your party is full. Come back when\n"
	.string "you have room for one more.$"

WhisperstemValley_Teahouse_Text_MasterDone:
	.string "Have some tea before you go. It's\n"
	.string "brewed from young bamboo leaves.$"

WhisperstemValley_Teahouse_Text_Pancham:
	.string "PANCHAM is chewing on a leaf and\n"
	.string "trying to look tough.$"

WhisperstemValley_Teahouse_Text_Guest:
	.string "I come here for the quiet. Listen…\n"
	.string "you can hear the bamboo whisper.$"
