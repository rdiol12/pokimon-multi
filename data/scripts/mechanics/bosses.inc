@ Cave bosses (src/mechanics_bosses.c): a Terastal or Dynamax Pokémon in
@ five caves. Beat it for its treasure or catch it.
Mechanics_EventScript_BossGranite::
	setvar VAR_0x8004, BOSS_GRANITE
	goto Mechanics_EventScript_Boss

Mechanics_EventScript_BossMeteor::
	setvar VAR_0x8004, BOSS_METEOR
	goto Mechanics_EventScript_Boss

Mechanics_EventScript_BossShoal::
	setvar VAR_0x8004, BOSS_SHOAL
	goto Mechanics_EventScript_Boss

Mechanics_EventScript_BossSeafloor::
	setvar VAR_0x8004, BOSS_SEAFLOOR
	goto Mechanics_EventScript_Boss

Mechanics_EventScript_BossVictory::
	setvar VAR_0x8004, BOSS_VICTORY
Mechanics_EventScript_Boss:
	lockall
	callnative Mechanics_SetupBoss
	playmoncry VAR_0x8006, CRY_MODE_ENCOUNTER
	msgbox gStringVar4, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	dowildbattle
	callnative Mechanics_EndBoss
	goto_if_eq VAR_RESULT, 1, Mechanics_EventScript_BossBeaten
	goto_if_eq VAR_RESULT, 2, Mechanics_EventScript_BossCaught
	msgbox Mechanics_Text_BossStill, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_EventScript_BossBeaten:
	msgbox Mechanics_Text_BossBeaten, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	releaseall
	end

Mechanics_EventScript_BossCaught:
	msgbox Mechanics_Text_BossCaught, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_Text_BossStill:
	.string "The huge POKéMON is still here,\n"
	.string "watching you.$"

Mechanics_Text_BossBeaten:
	.string "The boss fled deeper into the cave!\n"
	.string "It left something shiny behind…$"

Mechanics_Text_BossCaught:
	.string "The cave is quiet now. The boss is\n"
	.string "on your team!$"
