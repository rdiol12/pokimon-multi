MetroOldLine_MapScripts::
	.byte 0

MetroOldLine_EventScript_Stamp::
	lockall
	goto_if_set FLAG_METRO_STAMP_4, Metro_EventScript_Stamped
	setflag FLAG_METRO_STAMP_4
	playse SE_BANG
	msgbox MetroOldLine_Text_Stamp, MSGBOX_DEFAULT
	releaseall
	end

MetroOldLine_EventScript_Board::
	msgbox MetroOldLine_Text_Board, MSGBOX_SIGN
	end

MetroOldLine_EventScript_Conductor::
	lock
	faceplayer
	msgbox MetroOldLine_Text_Conductor, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FAINT_ATTACK
	fadescreen FADE_TO_WHITE
	removeobject VAR_LAST_TALKED
	setflag FLAG_METRO_GHOST_SEEN
	fadescreen FADE_FROM_WHITE
	msgbox MetroOldLine_Text_ConductorGone, MSGBOX_DEFAULT
	release
	end

MetroOldLine_EventScript_Klink::
	lock
	faceplayer
	playmoncry SPECIES_KLINK, CRY_MODE_NORMAL
	msgbox MetroOldLine_Text_Klink, MSGBOX_DEFAULT
	waitmoncry
	release
	end

MetroOldLine_EventScript_Digby::
	trainerbattle_single TRAINER_METRO_DIGBY, MetroOldLine_Text_DigbyIntro, MetroOldLine_Text_DigbyDefeat
	msgbox MetroOldLine_Text_DigbyPost, MSGBOX_AUTOCLOSE
	end

MetroOldLine_EventScript_Gears::
	trainerbattle_single TRAINER_METRO_GEARS, MetroOldLine_Text_GearsIntro, MetroOldLine_Text_GearsDefeat
	msgbox MetroOldLine_Text_GearsPost, MSGBOX_AUTOCLOSE
	end

MetroOldLine_EventScript_Odette::
	trainerbattle_single TRAINER_METRO_ODETTE, MetroOldLine_Text_OdetteIntro, MetroOldLine_Text_OdetteDefeat
	msgbox MetroOldLine_Text_OdettePost, MSGBOX_AUTOCLOSE
	end

MetroOldLine_Text_Stamp:
	.string "{PLAYER} stamped the rally card!\n"
	.string "It's the GHOST STATION stamp.$"

MetroOldLine_Text_Board:
	.string "HOENN METRO\n"
	.string "…ST STATION. The rest has rusted off.$"

MetroOldLine_Text_Conductor:
	.string "Tickets, please…\p"
	.string "Ah, a rally card. Nobody's stamped\n"
	.string "it here since the last train left.\p"
	.string "That was… a long time ago now.\n"
	.string "Thank you for visiting.$"

MetroOldLine_Text_ConductorGone:
	.string "The conductor was gone. Only his\n"
	.string "cap was left on the platform…$"

MetroOldLine_Text_Klink:
	.string "KLINK is turning round and round,\n"
	.string "like a gear with nothing to drive.$"

MetroOldLine_Text_DigbyIntro:
	.string "These tunnels are full of old\n"
	.string "relics! Don't touch my finds!$"

MetroOldLine_Text_DigbyDefeat:
	.string "Buried!$"

MetroOldLine_Text_DigbyPost:
	.string "I found a ticket from fifty years\n"
	.string "ago. It still says GHOST STATION.$"

MetroOldLine_Text_GearsIntro:
	.string "The depot's machines still hum. My\n"
	.string "POKéMON love it here!$"

MetroOldLine_Text_GearsDefeat:
	.string "My gears… stripped!$"

MetroOldLine_Text_GearsPost:
	.string "The signal room still has power.\n"
	.string "Nobody knows where it comes from.$"

MetroOldLine_Text_OdetteIntro:
	.string "Fu fu fu… The last train never\n"
	.string "left this line…$"

MetroOldLine_Text_OdetteDefeat:
	.string "Fu fu… you're no ghost.$"

MetroOldLine_Text_OdettePost:
	.string "An old conductor still walks the\n"
	.string "GHOST STATION platform. Say hello.$"

MetroOldLine_EventScript_Item_HyperPotion::
	finditem ITEM_HYPER_POTION
	end

MetroOldLine_EventScript_Item_Magnet::
	finditem ITEM_MAGNET
	end

MetroOldLine_EventScript_Item_MetalCoat::
	finditem ITEM_METAL_COAT
	end

MetroOldLine_EventScript_Item_TmThunderbolt::
	finditem ITEM_TM_THUNDERBOLT
	end

MetroOldLine_EventScript_Item_SpellTag::
	finditem ITEM_SPELL_TAG
	end

