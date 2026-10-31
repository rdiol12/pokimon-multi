CorallineReef_Wreck_MapScripts::
	.byte 0

CorallineReef_Wreck_EventScript_Dhelmise::
	lock
	faceplayer
	playmoncry SPECIES_DHELMISE, CRY_MODE_ENCOUNTER
	msgbox CorallineReef_Wreck_Text_Dhelmise, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_DHELMISE, 44
	dowildbattle
	setflag FLAG_ABYSS_DHELMISE_DONE
	removeobject VAR_LAST_TALKED
	release
	end

CorallineReef_Wreck_EventScript_Nugget::
	finditem ITEM_NUGGET
	end

CorallineReef_Wreck_EventScript_RareCandy::
	finditem ITEM_RARE_CANDY
	end

CorallineReef_Wreck_EventScript_Log::
	msgbox CorallineReef_Wreck_Text_Log, MSGBOX_SIGN
	end

CorallineReef_Wreck_Text_Dhelmise:
	.string "The old anchor came to life!$"

CorallineReef_Wreck_Text_Log:
	.string "A waterlogged captain's log…\p"
	.string "“Day 40. We found the lights of a\n"
	.string "city under the sea, then lost our\l"
	.string "anchor in a storm. The crew say\l"
	.string "it swam away on its own.”$"
