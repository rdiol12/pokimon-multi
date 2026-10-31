Abyssia_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Abyssia_OnLoad
	.byte 0

Abyssia_OnLoad:
	call_if_set FLAG_ABYSS_LAMP_1, Abyssia_OnLoadLamp1
	call_if_set FLAG_ABYSS_LAMP_2, Abyssia_OnLoadLamp2
	call_if_set FLAG_ABYSS_LAMP_3, Abyssia_OnLoadLamp3
	call_if_set FLAG_ABYSS_LAMP_4, Abyssia_OnLoadLamp4
	call_if_set FLAG_ABYSS_TEMPLE_OPEN, Abyssia_OpenTemple
	end

Abyssia_OnLoadLamp1:
	setmetatile 18, 15, 790, TRUE
	return

Abyssia_OnLoadLamp2:
	setmetatile 37, 15, 790, TRUE
	return

Abyssia_OnLoadLamp3:
	setmetatile 18, 31, 790, TRUE
	return

Abyssia_OnLoadLamp4:
	setmetatile 37, 31, 790, TRUE
	return

Abyssia_OpenTemple:
	setmetatile 27, 6, 851, TRUE
	setmetatile 28, 6, 852, TRUE
	setmetatile 27, 7, 859, TRUE
	setmetatile 28, 7, 860, TRUE
	setmetatile 27, 8, 867, FALSE
	setmetatile 28, 8, 868, FALSE
	return

Abyssia_EventScript_Lamp1::
	lockall
	goto_if_set FLAG_ABYSS_LAMP_1, Abyssia_EventScript_LampLit
	msgbox Abyssia_Text_LampAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Abyssia_EventScript_LampLeave
	setflag FLAG_ABYSS_LAMP_1
	setmetatile 18, 15, 790, TRUE
	special DrawWholeMapView
	playse SE_M_CHARGE
	waitse
	msgbox Abyssia_Text_LampGlows, MSGBOX_DEFAULT
	goto Abyssia_EventScript_CheckLamps

Abyssia_EventScript_Lamp2::
	lockall
	goto_if_set FLAG_ABYSS_LAMP_2, Abyssia_EventScript_LampLit
	msgbox Abyssia_Text_LampAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Abyssia_EventScript_LampLeave
	setflag FLAG_ABYSS_LAMP_2
	setmetatile 37, 15, 790, TRUE
	special DrawWholeMapView
	playse SE_M_CHARGE
	waitse
	msgbox Abyssia_Text_LampGlows, MSGBOX_DEFAULT
	goto Abyssia_EventScript_CheckLamps

Abyssia_EventScript_Lamp3::
	lockall
	goto_if_set FLAG_ABYSS_LAMP_3, Abyssia_EventScript_LampLit
	msgbox Abyssia_Text_LampAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Abyssia_EventScript_LampLeave
	setflag FLAG_ABYSS_LAMP_3
	setmetatile 18, 31, 790, TRUE
	special DrawWholeMapView
	playse SE_M_CHARGE
	waitse
	msgbox Abyssia_Text_LampGlows, MSGBOX_DEFAULT
	goto Abyssia_EventScript_CheckLamps

Abyssia_EventScript_Lamp4::
	lockall
	goto_if_set FLAG_ABYSS_LAMP_4, Abyssia_EventScript_LampLit
	msgbox Abyssia_Text_LampAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Abyssia_EventScript_LampLeave
	setflag FLAG_ABYSS_LAMP_4
	setmetatile 37, 31, 790, TRUE
	special DrawWholeMapView
	playse SE_M_CHARGE
	waitse
	msgbox Abyssia_Text_LampGlows, MSGBOX_DEFAULT
	goto Abyssia_EventScript_CheckLamps

Abyssia_EventScript_LampLit::
	msgbox Abyssia_Text_LampLit, MSGBOX_DEFAULT
	releaseall
	end

Abyssia_EventScript_LampLeave::
	releaseall
	end

Abyssia_EventScript_CheckLamps::
	goto_if_unset FLAG_ABYSS_LAMP_1, Abyssia_EventScript_LampsLeft
	goto_if_unset FLAG_ABYSS_LAMP_2, Abyssia_EventScript_LampsLeft
	goto_if_unset FLAG_ABYSS_LAMP_3, Abyssia_EventScript_LampsLeft
	goto_if_unset FLAG_ABYSS_LAMP_4, Abyssia_EventScript_LampsLeft
	setflag FLAG_ABYSS_TEMPLE_OPEN
	playse SE_M_EARTHQUAKE
	special ShakeCamera
	call Abyssia_OpenTemple
	special DrawWholeMapView
	msgbox Abyssia_Text_TempleOpens, MSGBOX_DEFAULT
	releaseall
	end

Abyssia_EventScript_LampsLeft::
	msgbox Abyssia_Text_LampsLeft, MSGBOX_DEFAULT
	releaseall
	end

Abyssia_EventScript_Marina::
	lock
	faceplayer
	goto_if_set FLAG_ABYSS_MARINA_REWARD, Abyssia_EventScript_MarinaDone
	goto_if_set FLAG_ABYSS_TEMPLE_OPEN, Abyssia_EventScript_MarinaReward
	setflag FLAG_ABYSS_MARINA_TALKED
	msgbox Abyssia_Text_MarinaAsk, MSGBOX_DEFAULT
	release
	end

Abyssia_EventScript_MarinaReward::
	msgbox Abyssia_Text_MarinaThanks, MSGBOX_DEFAULT
	giveitem ITEM_DEEP_SEA_SCALE
	giveitem ITEM_DEEP_SEA_TOOTH
	setflag FLAG_ABYSS_MARINA_REWARD
	msgbox Abyssia_Text_MarinaAfter, MSGBOX_DEFAULT
	release
	end

Abyssia_EventScript_MarinaDone::
	msgbox Abyssia_Text_MarinaDone, MSGBOX_DEFAULT
	release
	end

Abyssia_EventScript_Lanturn::
	lock
	faceplayer
	playmoncry SPECIES_LANTURN, CRY_MODE_NORMAL
	msgbox Abyssia_Text_Lanturn, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Abyssia_EventScript_Mantine::
	lock
	faceplayer
	playmoncry SPECIES_MANTINE, CRY_MODE_NORMAL
	msgbox Abyssia_Text_Mantine, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Abyssia_EventScript_Alomomola::
	lock
	faceplayer
	playmoncry SPECIES_ALOMOMOLA, CRY_MODE_NORMAL
	msgbox Abyssia_Text_Alomomola, MSGBOX_DEFAULT
	waitmoncry
	special HealPlayerParty
	msgbox Abyssia_Text_Healed, MSGBOX_DEFAULT
	release
	end

Abyssia_EventScript_Lumineon::
	lock
	faceplayer
	playmoncry SPECIES_LUMINEON, CRY_MODE_NORMAL
	msgbox Abyssia_Text_Lumineon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Abyssia_EventScript_Triton::
	trainerbattle_single TRAINER_ABYSS_TRITON, Abyssia_Text_TritonIntro, Abyssia_Text_TritonDefeat
	msgbox Abyssia_Text_TritonPost, MSGBOX_AUTOCLOSE
	end

Abyssia_Text_LampAsk:
	.string "An old lamp pillar. There's a crank\n"
	.string "on its side. Turn it?$"

Abyssia_Text_LampGlows:
	.string "The crank creaked… and the lamp\n"
	.string "began to glow!$"

Abyssia_Text_LampLit:
	.string "The lamp is glowing softly.$"

Abyssia_Text_LampsLeft:
	.string "Somewhere in the city, something is\n"
	.string "still waiting in the dark.$"

Abyssia_Text_TempleOpens:
	.string "All four lamps are shining!\p"
	.string "The temple doors are grinding open!$"

Abyssia_Text_MarinaAsk:
	.string "A visitor, down here? I'm MARINA.\n"
	.string "I study this sunken city, ABYSSIA.\p"
	.string "Its people built four lamp pillars\n"
	.string "around the temple.\p"
	.string "The carvings say: “When the four\n"
	.string "lights shine, the temple wakes.”\p"
	.string "My arms aren't strong enough for\n"
	.string "the cranks. Could you try?$"

Abyssia_Text_MarinaThanks:
	.string "The temple is open! Decades of study\n"
	.string "and you did it in an afternoon!\p"
	.string "Please, take these. I found them in\n"
	.string "the ruins.$"

Abyssia_Text_MarinaAfter:
	.string "Something ancient sleeps inside. Be\n"
	.string "gentle with it.$"

Abyssia_Text_MarinaDone:
	.string "Every carving here tells of a fish\n"
	.string "older than the city itself.$"

Abyssia_Text_Lanturn:
	.string "LANTURN's light flickers. It seems\n"
	.string "to miss the old city lamps.$"

Abyssia_Text_Mantine:
	.string "MANTINE glides slowly over the\n"
	.string "rooftops.$"

Abyssia_Text_Alomomola:
	.string "ALOMOMOLA wrapped your POKéMON in\n"
	.string "its caring fins…$"

Abyssia_Text_Healed:
	.string "Your POKéMON are fully healed!$"

Abyssia_Text_Lumineon:
	.string "LUMINEON's fins glow in the dark\n"
	.string "water.$"

Abyssia_Text_TritonIntro:
	.string "I've been treasure hunting down\n"
	.string "here for weeks. You're not taking\l"
	.string "my treasure!$"

Abyssia_Text_TritonDefeat:
	.string "Fine, fine…$"

Abyssia_Text_TritonPost:
	.string "The real treasure is in the temple.\n"
	.string "But it's sealed tight.$"

Abyssia_EventScript_Item_MysticWater::
	finditem ITEM_MYSTIC_WATER
	end

Abyssia_EventScript_Item_PrismScale::
	finditem ITEM_PRISM_SCALE
	end

