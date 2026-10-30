EverGrandeCity_PokemonLeague_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, EverGrandeCity_PokemonLeague_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

EverGrandeCity_PokemonLeague_1F_OnTransition:
	setrespawn HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE
	setflag FLAG_LANDMARK_POKEMON_LEAGUE
	call_if_unset FLAG_ENTERED_ELITE_FOUR, EverGrandeCity_PokemonLeague_1F_EventScript_GuardsBlockDoor
	end

EverGrandeCity_PokemonLeague_1F_EventScript_GuardsBlockDoor::
	setobjectxyperm LOCALID_LEAGUE_GUARD_1, 9, 2
	setobjectxyperm LOCALID_LEAGUE_GUARD_2, 10, 2
	return

EverGrandeCity_PokemonLeague_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_LEAGUE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

EverGrandeCity_PokemonLeague_1F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart EverGrandeCity_PokemonLeague_1F_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
EverGrandeCity_PokemonLeague_1F_Pokemart:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REPEL
	pokemartlistend

@ The door guards only check for FLAG_BADGE06_GET because Winonas badge is the only one that can be skipped
@ Its assumed the player has the remaining badges
EverGrandeCity_PokemonLeague_1F_EventScript_DoorGuard::
	lockall
	goto_if_set FLAG_ENTERED_ELITE_FOUR, EverGrandeCity_PokemonLeague_1F_EventScript_GoForth
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_ge VAR_TEMP_0, 11, EverGrandeCity_PokemonLeague_1F_EventScript_PlayerMoveToFrontFromRight
	call_if_le VAR_TEMP_0, 8, EverGrandeCity_PokemonLeague_1F_EventScript_PlayerMoveToFrontFromLeft
	message EverGrandeCity_PokemonLeague_1F_Text_MustHaveAllGymBadges
	waitmessage
	delay 120
	goto_if_unset FLAG_BADGE06_GET, EverGrandeCity_PokemonLeague_1F_EventScript_NotAllBadges
	closemessage
	applymovement LOCALID_LEAGUE_GUARD_1, EverGrandeCity_PokemonLeague_1F_Movement_LeftGuardOutOfWay
	applymovement LOCALID_LEAGUE_GUARD_2, EverGrandeCity_PokemonLeague_1F_Movement_RightGuardOutOfWay
	waitmovement 0
	delay 10
	playfanfare MUS_OBTAIN_BADGE
	message EverGrandeCity_PokemonLeague_1F_Text_GoForth
	waitmessage
	waitfanfare
	closemessage
	copyobjectxytoperm LOCALID_LEAGUE_GUARD_1
	copyobjectxytoperm LOCALID_LEAGUE_GUARD_2
	setflag FLAG_ENTERED_ELITE_FOUR
	releaseall
	end

EverGrandeCity_PokemonLeague_1F_EventScript_PlayerMoveToFrontFromRight::
	applymovement LOCALID_PLAYER, EverGrandeCity_PokemonLeague_1F_Movement_MoveToFrontFromRight
	waitmovement 0
	return

EverGrandeCity_PokemonLeague_1F_EventScript_PlayerMoveToFrontFromLeft::
	applymovement LOCALID_PLAYER, EverGrandeCity_PokemonLeague_1F_Movement_MoveToFrontFromLeft
	waitmovement 0
	return

EverGrandeCity_PokemonLeague_1F_EventScript_NotAllBadges::
	playse SE_FAILURE
	msgbox EverGrandeCity_PokemonLeague_1F_Text_HaventObtainedAllBadges, MSGBOX_DEFAULT
	releaseall
	end

EverGrandeCity_PokemonLeague_1F_EventScript_GoForth::
	applymovement VAR_LAST_TALKED, Common_Movement_FacePlayer
	waitmovement 0
	msgbox EverGrandeCity_PokemonLeague_1F_Text_GoForth, MSGBOX_DEFAULT
	closemessage
	applymovement VAR_LAST_TALKED, Common_Movement_FaceOriginalDirection
	waitmovement 0
	releaseall
	end

EverGrandeCity_PokemonLeague_1F_Movement_MoveToFrontFromRight:
	walk_down
	walk_left
	walk_in_place_faster_up
	step_end

EverGrandeCity_PokemonLeague_1F_Movement_MoveToFrontFromLeft:
	walk_down
	walk_right
	walk_in_place_faster_up
	step_end

EverGrandeCity_PokemonLeague_1F_Movement_LeftGuardOutOfWay:
	walk_left
	walk_in_place_faster_down
	step_end

EverGrandeCity_PokemonLeague_1F_Movement_RightGuardOutOfWay:
	walk_right
	walk_in_place_faster_down
	step_end

EverGrandeCity_PokemonLeague_1F_Text_MustHaveAllGymBadges:
	.string "Beyond this point, only those TRAINERS\n"
	.string "who have collected all the GYM BADGES\l"
	.string "are permitted to enter.\p"
	.string "TRAINER, let us confirm that you have\n"
	.string "all the GYM BADGES.$"

EverGrandeCity_PokemonLeague_1F_Text_HaventObtainedAllBadges:
	.string "You haven't obtained all the BADGES.\p"
	.string "If you're bound for the POKéMON\n"
	.string "LEAGUE, you must return with them.$"

EverGrandeCity_PokemonLeague_1F_Text_GoForth:
	.string "TRAINER! Believe in yourself and your\n"
	.string "POKéMON, and go forth!$"


EverGrandeCity_PokemonLeague_1F_EventScript_BattleLeagueGuide::
	lock
	faceplayer
	goto_if_unset FLAG_SYS_GAME_CLEAR, EverGrandeCity_PokemonLeague_1F_EventScript_BattleLeagueNotYet
	buffernumberstring STR_VAR_1, VAR_BATTLE_LEAGUE_CLEARS
	msgbox EverGrandeCity_PokemonLeague_1F_Text_BattleLeagueWelcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EverGrandeCity_PokemonLeague_1F_EventScript_BattleLeagueDeclined
	cleartrainerflag TRAINER_LEAGUE_KAI
	cleartrainerflag TRAINER_LEAGUE_NOVA
	cleartrainerflag TRAINER_LEAGUE_RYU
	cleartrainerflag TRAINER_LEAGUE_MORGANA
	cleartrainerflag TRAINER_LEAGUE_REN
	special HealPlayerParty
	msgbox EverGrandeCity_PokemonLeague_1F_Text_BattleLeagueGoodLuck, MSGBOX_DEFAULT
	closemessage
	warp MAP_BATTLE_LEAGUE_ROOM1, 6, 7
	waitstate
	release
	end

EverGrandeCity_PokemonLeague_1F_EventScript_BattleLeagueNotYet::
	msgbox EverGrandeCity_PokemonLeague_1F_Text_BattleLeagueNotYet, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_PokemonLeague_1F_EventScript_BattleLeagueDeclined::
	msgbox EverGrandeCity_PokemonLeague_1F_Text_BattleLeagueDeclined, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_PokemonLeague_1F_Text_BattleLeagueNotYet:
	.string "Beyond this point lies the new\n"
	.string "BATTLE LEAGUE.\p"
	.string "Only CHAMPIONS of HOENN may\n"
	.string "enter. Come back once you've\l"
	.string "entered the HALL OF FAME!$"

EverGrandeCity_PokemonLeague_1F_Text_BattleLeagueWelcome:
	.string "Welcome to the BATTLE LEAGUE!\p"
	.string "Four LEAGUE MASTERS and their\n"
	.string "CHAMPION use POKéMON from far-off\l"
	.string "regions. You've won {STR_VAR_1} time(s).\p"
	.string "Your POKéMON will be healed.\n"
	.string "Will you take the challenge?$"

EverGrandeCity_PokemonLeague_1F_Text_BattleLeagueGoodLuck:
	.string "Then step through! Good luck!$"

EverGrandeCity_PokemonLeague_1F_Text_BattleLeagueDeclined:
	.string "Come back when you're ready.$"
