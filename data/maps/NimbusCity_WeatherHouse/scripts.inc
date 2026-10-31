NimbusCity_WeatherHouse_MapScripts::
	.byte 0

NimbusCity_WeatherHouse_EventScript_Watcher::
	msgbox NimbusCity_WeatherHouse_Text_Watcher, MSGBOX_NPC
	end

NimbusCity_WeatherHouse_EventScript_Castform::
	lock
	faceplayer
	playmoncry SPECIES_CASTFORM, CRY_MODE_NORMAL
	msgbox NimbusCity_WeatherHouse_Text_Castform, MSGBOX_DEFAULT
	waitmoncry
	release
	end

NimbusCity_WeatherHouse_Text_Watcher:
	.string "I watch the winds of the isles.\p"
	.string "Two streams blow between the west\n"
	.string "and east islands: east on the\l"
	.string "upper one, west on the lower.\p"
	.string "A third once blew up to the summit,\n"
	.string "but it's been still since the storm.$"

NimbusCity_WeatherHouse_Text_Castform:
	.string "CASTFORM changes shape with the\n"
	.string "weather. Today it's sunny!$"
