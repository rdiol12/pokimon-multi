@ The Haven Reserve (src/reserve.c, data/scripts/reserve.inc,
@ tools/reserve/make_reserve.py).

HavenReserve_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, HavenReserve_OnTransition
	.byte 0

HavenReserve_OnTransition:
	callnative Reserve_SetupMap
	end

