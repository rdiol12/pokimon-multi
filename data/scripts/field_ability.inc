@ Pokémon abilities in the overworld (src/field_ability.c, docs/field_abilities.md).
@ The C code sets VAR_0x8004 to the spot's index before these run.

FieldAbility_EventScript_Ice::
	lockall
	callnative FieldAbility_FindHelper
	goto_if_eq VAR_RESULT, PARTY_SIZE, FieldAbility_EventScript_IceNoHelp
	setfieldeffectargument 0, VAR_RESULT
	msgbox FieldAbility_Text_IceAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FieldAbility_EventScript_Cancel
	closemessage
	dofieldeffect FLDEFF_USE_STRENGTH
	waitstate
	playse SE_M_FLAME_WHEEL
	waitse
	playse SE_ICE_BREAK
	callnative FieldAbility_Solve
	msgbox FieldAbility_Text_IceMelted, MSGBOX_DEFAULT
	finditem VAR_0x8005
	releaseall
	end

FieldAbility_EventScript_IceNoHelp:
	msgbox FieldAbility_Text_IceNoHelp, MSGBOX_DEFAULT
	releaseall
	end

FieldAbility_EventScript_Generator::
	lockall
	callnative FieldAbility_FindHelper
	goto_if_eq VAR_RESULT, PARTY_SIZE, FieldAbility_EventScript_GeneratorNoHelp
	setfieldeffectargument 0, VAR_RESULT
	msgbox FieldAbility_Text_GeneratorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FieldAbility_EventScript_Cancel
	closemessage
	dofieldeffect FLDEFF_USE_STRENGTH
	waitstate
	playse SE_M_THUNDERBOLT
	waitse
	callnative FieldAbility_Solve
	animateflash 0
	setflashlevel 0
	msgbox FieldAbility_Text_GeneratorOn, MSGBOX_DEFAULT
	finditem VAR_0x8005
	releaseall
	end

FieldAbility_EventScript_GeneratorNoHelp:
	msgbox FieldAbility_Text_GeneratorNoHelp, MSGBOX_DEFAULT
	releaseall
	end

FieldAbility_EventScript_GeneratorOn::
	lockall
	playse SE_M_THUNDER_WAVE
	msgbox FieldAbility_Text_GeneratorHums, MSGBOX_DEFAULT
	releaseall
	end

FieldAbility_EventScript_Dig::
	lockall
	callnative FieldAbility_FindHelper
	goto_if_eq VAR_RESULT, PARTY_SIZE, FieldAbility_EventScript_DigNoHelp
	setfieldeffectargument 0, VAR_RESULT
	msgbox FieldAbility_Text_DigAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FieldAbility_EventScript_Cancel
	closemessage
	dofieldeffect FLDEFF_USE_STRENGTH
	waitstate
	playse SE_M_DIG
	waitse
	callnative FieldAbility_Solve
	msgbox FieldAbility_Text_DugUp, MSGBOX_DEFAULT
	finditem VAR_0x8005
	releaseall
	end

FieldAbility_EventScript_DigNoHelp:
	msgbox FieldAbility_Text_DigNoHelp, MSGBOX_DEFAULT
	releaseall
	end

FieldAbility_EventScript_BoulderPrize::
	lockall
	msgbox FieldAbility_Text_UnderBoulder, MSGBOX_DEFAULT
	finditem VAR_0x8005
	releaseall
	end

@ From EventScript_StrengthBoulder: VAR_RESULT is a Fighting type (or Guts,
@ Huge Power, Pure Power) that can shove boulders without the move.
FieldAbility_EventScript_StrongArms::
	setfieldeffectargument 0, VAR_RESULT
	msgbox FieldAbility_Text_StrongAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FieldAbility_EventScript_Cancel
	closemessage
	dofieldeffect FLDEFF_USE_STRENGTH
	waitstate
	setflag FLAG_SYS_USE_STRENGTH
	msgbox FieldAbility_Text_StrongDone, MSGBOX_DEFAULT
	releaseall
	end

@ Berry trees: says so when a Harvest / Grass-type lead adds berries.
@ Keeps VAR_RESULT, which the berry tree script uses next.
FieldAbility_EventScript_BerryHelp::
	copyvar VAR_0x8009, VAR_RESULT
	callnative FieldAbility_BerryHelper
	goto_if_eq VAR_RESULT, FALSE, FieldAbility_EventScript_BerryHelpEnd
	msgbox FieldAbility_Text_BerryHelp, MSGBOX_DEFAULT
FieldAbility_EventScript_BerryHelpEnd:
	copyvar VAR_RESULT, VAR_0x8009
	return

FieldAbility_EventScript_Cancel:
	closemessage
	releaseall
	end

@ VAR_0x8004: TRUE if the cave was dark until now.
FieldAbility_EventScript_Lantern::
	lockall
	goto_if_eq VAR_0x8004, FALSE, FieldAbility_EventScript_LanternMsg
	playse SE_M_EMBER
	animateflash 1
	setflashlevel 1
FieldAbility_EventScript_LanternMsg:
	msgbox FieldAbility_Text_Lantern, MSGBOX_DEFAULT
	releaseall
	end

FieldAbility_EventScript_Healer::
	lockall
	playse SE_M_HEAL_BELL
	msgbox FieldAbility_Text_Healer, MSGBOX_DEFAULT
	releaseall
	end

FieldAbility_EventScript_Frisk::
	lockall
	playfanfare MUS_OBTAIN_ITEM
	message FieldAbility_Text_Frisk
	waitfanfare
	waitmessage
	waitbuttonpress
	releaseall
	end

FieldAbility_Text_IceAsk:
	.string "A block of ice. Something is frozen\n"
	.string "inside it!\p"
	.string "{STR_VAR_1}'s {STR_VAR_2} could melt it.\n"
	.string "Should {STR_VAR_1} try?$"

FieldAbility_Text_IceMelted:
	.string "The ice melted away in a puff\n"
	.string "of steam!$"

FieldAbility_Text_IceNoHelp:
	.string "A block of ice. Something is frozen\n"
	.string "inside it!\p"
	.string "A FIRE-type POKéMON might be able\n"
	.string "to melt it.$"

FieldAbility_Text_GeneratorAsk:
	.string "An old generator. It's out of power.\p"
	.string "{STR_VAR_1}'s {STR_VAR_2} could get it\n"
	.string "running. Give it a jolt?$"

FieldAbility_Text_GeneratorOn:
	.string "The generator roared to life!\p"
	.string "Lights flickered on all through\n"
	.string "the cave!\p"
	.string "Something rattled out of the\n"
	.string "generator's panel…$"

FieldAbility_Text_GeneratorNoHelp:
	.string "An old generator. It's out of power.\p"
	.string "An ELECTRIC-type POKéMON might be\n"
	.string "able to get it running.$"

FieldAbility_Text_GeneratorHums:
	.string "The generator hums along.\n"
	.string "The cave stays lit.$"

FieldAbility_Text_DigAsk:
	.string "The sand here is sparkling.\n"
	.string "Something might be buried!\p"
	.string "{STR_VAR_1}'s {STR_VAR_2} could dig\n"
	.string "it up. Should {STR_VAR_1} dig?$"

FieldAbility_Text_DugUp:
	.string "{STR_VAR_1} dug up something!$"

FieldAbility_Text_DigNoHelp:
	.string "The sand here is sparkling.\n"
	.string "Something might be buried!\p"
	.string "A GROUND-type POKéMON might be\n"
	.string "able to dig it up.$"

FieldAbility_Text_UnderBoulder:
	.string "There was something under\n"
	.string "the boulder!$"

FieldAbility_Text_StrongAsk:
	.string "It's a big boulder.\p"
	.string "{STR_VAR_1}'s {STR_VAR_2} could shove\n"
	.string "it around. Want {STR_VAR_1} to push?$"

FieldAbility_Text_StrongDone:
	.string "{STR_VAR_1} braced itself. Boulders\n"
	.string "can be moved now!$"

FieldAbility_Text_BerryHelp:
	.string "{STR_VAR_3} gave the tree a shake.\n"
	.string "A few more BERRIES fell!$"

FieldAbility_Text_Lantern:
	.string "{STR_VAR_1}'s glow lights up\n"
	.string "the dark cave!$"

FieldAbility_Text_Healer:
	.string "{STR_VAR_1}'s {STR_VAR_2} soothed\n"
	.string "{STR_VAR_3}.\p"
	.string "{STR_VAR_3} is feeling better!$"

FieldAbility_Text_Frisk:
	.string "{STR_VAR_1} frisked the wild {STR_VAR_2}\n"
	.string "and pocketed its {STR_VAR_3}!$"
