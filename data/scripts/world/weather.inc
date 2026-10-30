@ The weather watcher tells where the three roaming weather fronts are now
@ (src/world_weather.c).

World_EventScript_WeatherWatcher::
	lock
	faceplayer
	callnative RoamingWeather_GetForecast
	msgbox World_Text_WeatherForecast, MSGBOX_DEFAULT
	release
	end

World_Text_WeatherForecast:
	.string "I track the storms that roam HOENN!\p"
	.string "Right now, a rainstorm is over\n"
	.string "{STR_VAR_1}.\p"
	.string "A sandstorm is blowing over\n"
	.string "{STR_VAR_2}.\p"
	.string "And it's snowing over {STR_VAR_3}!\p"
	.string "They move on every hour. Rain, sand\n"
	.string "and snow each bring out Pokémon\l"
	.string "you won't find in clear weather!$"
