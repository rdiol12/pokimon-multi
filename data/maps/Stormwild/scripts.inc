@ The Stormwild (src/stormwild.c, data/scripts/stormwild.inc).
Stormwild_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Stormwild_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, Stormwild_OnResume
	.byte 0

Stormwild_OnTransition:
	callnative Stormwild_SetupObjects
	end

Stormwild_OnResume:
	callnative Stormwild_HideTakenCaches
	end

