@ Shared by every town square.
Square_EventScript_Guide::
	lock
	faceplayer
	callnative HoennMulti_IsOnline
	goto_if_eq VAR_RESULT, FALSE, Square_EventScript_GuideOffline
	message Square_Text_GuideWelcome
	waitmessage
	dynmultipush Square_Text_Hub, 0
	dynmultipush Square_Text_Plaza, 1
	dynmultipush Online_Text_HubClose, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	closemessage
	release
	goto_if_eq VAR_RESULT, 0, Online_EventScript_Hub
	goto_if_eq VAR_RESULT, 1, NetplayExt_EventScript_Plaza
	end

Square_EventScript_GuideOffline:
	msgbox Square_Text_GuideOffline, MSGBOX_DEFAULT
	release
	end

Square_Text_GuideWelcome:
	.string "Welcome to the square! Trainers from\n"
	.string "all over meet up here.\p"
	.string "What would you like to see?$"

Square_Text_GuideOffline:
	.string "Welcome to the square! Trainers from\n"
	.string "all over meet up here.\p"
	.string "Choose JOIN ONLINE on the title screen\n"
	.string "and I can show you the HUB and PLAZA.$"

@ Rest stops on the long routes: a keeper who lets the party rest.
Square_EventScript_Healer::
	lock
	faceplayer
	msgbox Square_Text_HealerAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Square_EventScript_HealerNo
	closemessage
	fadescreen FADE_TO_BLACK
	special HealPlayerParty
	playfanfare MUS_HEAL
	waitfanfare
	fadescreen FADE_FROM_BLACK
	msgbox Square_Text_HealerDone, MSGBOX_DEFAULT
	release
	end

Square_EventScript_HealerNo:
	msgbox Square_Text_HealerNo, MSGBOX_DEFAULT
	release
	end

Square_Text_HealerAsk:
	.string "Welcome to the rest stop! It's a long\n"
	.string "road, and your POKéMON look tired.\p"
	.string "Would you like them to rest a while?$"

Square_Text_HealerDone:
	.string "There! Everyone is rested and ready.\n"
	.string "Safe travels, and good battles!$"

Square_Text_HealerNo:
	.string "Come back any time you need a rest.$"

Square_Text_Hub:
	.string "ONLINE HUB$"

Square_Text_Plaza:
	.string "PLAZA$"

@ Littleroot Town square (tools/map_upgrade/squares.py)
Square_Littleroot_EventScript_Board::
	msgbox Square_Littleroot_Text_Board, MSGBOX_SIGN
	end

Square_Littleroot_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Littleroot_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Littleroot_EventScript_Local1::
	msgbox Square_Littleroot_Text_Local1, MSGBOX_NPC
	end

Square_Littleroot_Text_Board:
	.string "LITTLEROOT GREEN\n"
	.string "Where every journey starts.\p"
	.string "Meet trainers from all over Hoenn.\n"
	.string "The guide has the HUB, the matchmaker\l"
	.string "finds you a battle or a trade.$"

Square_Littleroot_Text_Local1:
	.string "Mom says trainers from far away come\n"
	.string "all the way here just to battle!\p"
	.string "I want to be one of them someday.$"

@ Oldale Town square (tools/map_upgrade/squares.py)
Square_Oldale_EventScript_Board::
	msgbox Square_Oldale_Text_Board, MSGBOX_SIGN
	end

Square_Oldale_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Oldale_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Oldale_EventScript_Local1::
	msgbox Square_Oldale_Text_Local1, MSGBOX_NPC
	end

Square_Oldale_Text_Board:
	.string "OLDALE SQUARE\n"
	.string "Where all paths cross.\p"
	.string "The guide has the HUB and PLAZA. The\n"
	.string "matchmaker finds you a battle or trade.$"

Square_Oldale_Text_Local1:
	.string "When I was young, Oldale was just a\n"
	.string "stop on the way to somewhere else.\p"
	.string "Now trainers camp out on this square\n"
	.string "all night, trading stories.$"

@ Petalburg City square (tools/map_upgrade/squares.py)
Square_Petalburg_EventScript_Board::
	msgbox Square_Petalburg_Text_Board, MSGBOX_SIGN
	end

Square_Petalburg_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Petalburg_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Petalburg_EventScript_Local1::
	msgbox Square_Petalburg_Text_Local1, MSGBOX_NPC
	end

Square_Petalburg_Text_Board:
	.string "PETALBURG PLAZA\n"
	.string "A city of balance.\p"
	.string "Meet trainers from all over Hoenn.\n"
	.string "The guide has the HUB, the matchmaker\l"
	.string "finds you a battle or a trade.$"

Square_Petalburg_Text_Local1:
	.string "Norman runs a tight gym, but out here\n"
	.string "anyone can battle anyone.\p"
	.string "I come to watch. Best seat in town!$"

@ Dewford Town square (tools/map_upgrade/squares.py)
Square_Dewford_EventScript_Board::
	msgbox Square_Dewford_Text_Board, MSGBOX_SIGN
	end

Square_Dewford_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Dewford_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Dewford_EventScript_Local1::
	msgbox Square_Dewford_Text_Local1, MSGBOX_NPC
	end

Square_Dewford_Text_Board:
	.string "DEWFORD BEACH SQUARE\n"
	.string "Waves, sand and battles.\p"
	.string "The guide has the HUB and PLAZA. The\n"
	.string "matchmaker finds you a battle or trade.$"

Square_Dewford_Text_Local1:
	.string "Boats bring trainers from every port\n"
	.string "to battle on this beach!\p"
	.string "Brawly says it's the best training a\n"
	.string "surfer could ask for.$"

@ Verdanturf Town square (tools/map_upgrade/squares.py)
Square_Verdanturf_EventScript_Board::
	msgbox Square_Verdanturf_Text_Board, MSGBOX_SIGN
	end

Square_Verdanturf_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Verdanturf_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Verdanturf_EventScript_Local1::
	msgbox Square_Verdanturf_Text_Local1, MSGBOX_NPC
	end

Square_Verdanturf_Text_Board:
	.string "VERDANTURF MEADOW\n"
	.string "Fresh air for every trainer.\p"
	.string "The guide has the HUB and PLAZA. The\n"
	.string "matchmaker finds you a battle or trade.$"

Square_Verdanturf_Text_Local1:
	.string "The air here is so clean that sick\n"
	.string "Pokémon come to rest.\p"
	.string "Gentle battles are good for them too,\n"
	.string "Wally told me so!$"

@ Mauville City square (tools/map_upgrade/squares.py)
Square_Mauville_EventScript_Board::
	msgbox Square_Mauville_Text_Board, MSGBOX_SIGN
	end

Square_Mauville_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Mauville_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Mauville_EventScript_Local1::
	msgbox Square_Mauville_Text_Local1, MSGBOX_NPC
	end

Square_Mauville_EventScript_Local2::
	msgbox Square_Mauville_Text_Local2, MSGBOX_NPC
	end

Square_Mauville_Text_Board:
	.string "MAUVILLE CROSSROADS SQUARE\n"
	.string "All of Hoenn passes through.\p"
	.string "The guide has the HUB and PLAZA. The\n"
	.string "matchmaker finds you a battle or trade.$"

Square_Mauville_Text_Local1:
	.string "Cycling Road, Route 111, Route 117...\n"
	.string "every road in Hoenn runs into Mauville.\p"
	.string "So this is where everyone meets up!$"

Square_Mauville_Text_Local2:
	.string "Wattson's gym is right up the road.\n"
	.string "Trainers warm up here before they go.$"

@ Fallarbor Town square (tools/map_upgrade/squares.py)
Square_Fallarbor_EventScript_Board::
	msgbox Square_Fallarbor_Text_Board, MSGBOX_SIGN
	end

Square_Fallarbor_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Fallarbor_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Fallarbor_EventScript_Local1::
	msgbox Square_Fallarbor_Text_Local1, MSGBOX_NPC
	end

Square_Fallarbor_Text_Board:
	.string "FALLARBOR ASH SQUARE\n"
	.string "Where the ash meets the crowd.\p"
	.string "The guide has the HUB and PLAZA. The\n"
	.string "matchmaker finds you a battle or trade.$"

Square_Fallarbor_Text_Local1:
	.string "Professor Cozmo says ash falls here\n"
	.string "like snow.\p"
	.string "Trainers sweep the square every\n"
	.string "morning before the battles start!$"

@ Lavaridge Town square (tools/map_upgrade/squares.py)
Square_Lavaridge_EventScript_Board::
	msgbox Square_Lavaridge_Text_Board, MSGBOX_SIGN
	end

Square_Lavaridge_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Lavaridge_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Lavaridge_EventScript_Local1::
	msgbox Square_Lavaridge_Text_Local1, MSGBOX_NPC
	end

Square_Lavaridge_Text_Board:
	.string "LAVARIDGE HOT SQUARE\n"
	.string "Warm sand, warm hearts.\p"
	.string "The guide has the HUB and PLAZA. The\n"
	.string "matchmaker finds you a battle or trade.$"

Square_Lavaridge_Text_Local1:
	.string "After a soak in the hot sand, there\n"
	.string "is nothing like a good battle.\p"
	.string "Flannery always says the heat brings\n"
	.string "out a trainer's fire!$"

@ Fortree City square (tools/map_upgrade/squares.py)
Square_Fortree_EventScript_Board::
	msgbox Square_Fortree_Text_Board, MSGBOX_SIGN
	end

Square_Fortree_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Fortree_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Fortree_EventScript_Local1::
	msgbox Square_Fortree_Text_Local1, MSGBOX_NPC
	end

Square_Fortree_Text_Board:
	.string "FORTREE CLEARING\n"
	.string "The one place on the ground.\p"
	.string "The guide has the HUB and PLAZA. The\n"
	.string "matchmaker finds you a battle or trade.$"

Square_Fortree_Text_Local1:
	.string "Everyone lives up in the trees, so the\n"
	.string "ground is the only place big enough\l"
	.string "for a crowd!\p"
	.string "Winona flies over to watch sometimes.$"

@ Rustboro City square (tools/map_upgrade/squares.py)
Square_Rustboro_EventScript_Board::
	msgbox Square_Rustboro_Text_Board, MSGBOX_SIGN
	end

Square_Rustboro_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Rustboro_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Rustboro_EventScript_Local1::
	msgbox Square_Rustboro_Text_Local1, MSGBOX_NPC
	end

Square_Rustboro_EventScript_Local2::
	msgbox Square_Rustboro_Text_Local2, MSGBOX_NPC
	end

Square_Rustboro_Text_Board:
	.string "RUSTBORO STONE PARK\n"
	.string "Where trainers meet in the city.\p"
	.string "The guide has the HUB and PLAZA. The\n"
	.string "matchmaker finds you a battle or trade.$"

Square_Rustboro_Text_Local1:
	.string "The Trainer's School teaches the\n"
	.string "basics, but this park is where you\l"
	.string "really learn: battling real people!$"

Square_Rustboro_Text_Local2:
	.string "Devon makes the newest gadgets for\n"
	.string "trainers who play together online.\p"
	.string "I test them out right here!$"

@ Route 102 square (tools/map_upgrade/squares.py)
Square_Route102_EventScript_Board::
	msgbox Square_Route102_Text_Board, MSGBOX_SIGN
	end

Square_Route102_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Route102_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Route102_EventScript_Local1::
	msgbox Square_Route102_Text_Local1, MSGBOX_NPC
	end

Square_Route102_Text_Board:
	.string "ROUTE 102 REST STOP\n"
	.string "Rest, meet and battle.\p"
	.string "The keeper lets your POKéMON rest.\n"
	.string "The matchmaker finds you a battle or trade.$"

Square_Route102_Text_Local1:
	.string "I battle everyone who passes by on\n"
	.string "their way to Petalburg.\p"
	.string "Then I rest up here and do it again!$"

@ Route 111 square (tools/map_upgrade/squares.py)
Square_Route111_EventScript_Board::
	msgbox Square_Route111_Text_Board, MSGBOX_SIGN
	end

Square_Route111_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Route111_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Route111_EventScript_Local1::
	msgbox Square_Route111_Text_Local1, MSGBOX_NPC
	end

Square_Route111_Text_Board:
	.string "DESERT REST CAMP\n"
	.string "Shelter from the sandstorm.\p"
	.string "The keeper lets your POKéMON rest.\n"
	.string "The matchmaker finds you a battle or trade.$"

Square_Route111_Text_Local1:
	.string "The sandstorm stops at these rocks.\n"
	.string "It's the only calm spot in the desert!\p"
	.string "Trainers wait here for the wind to\n"
	.string "die down, and battle while they wait.$"

@ Route 117 square (tools/map_upgrade/squares.py)
Square_Route117_EventScript_Board::
	msgbox Square_Route117_Text_Board, MSGBOX_SIGN
	end

Square_Route117_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Route117_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Route117_EventScript_Local1::
	msgbox Square_Route117_Text_Local1, MSGBOX_NPC
	end

Square_Route117_Text_Board:
	.string "ROUTE 117 REST STOP\n"
	.string "Between Verdanturf and Mauville.\p"
	.string "The keeper lets your POKéMON rest.\n"
	.string "The matchmaker finds you a battle or trade.$"

Square_Route117_Text_Local1:
	.string "The Day Care up the road raises\n"
	.string "POKéMON for trainers from all over.\p"
	.string "Sometimes they meet up right here to\n"
	.string "swap stories about their EGGS.$"

@ Route 123 square (tools/map_upgrade/squares.py)
Square_Route123_EventScript_Board::
	msgbox Square_Route123_Text_Board, MSGBOX_SIGN
	end

Square_Route123_EventScript_Guide::
	goto Square_EventScript_Guide

Square_Route123_EventScript_Matchmaker::
	goto HoennMulti_EventScript_QuickMatchDesk

Square_Route123_EventScript_Local1::
	msgbox Square_Route123_Text_Local1, MSGBOX_NPC
	end

Square_Route123_Text_Board:
	.string "ROUTE 123 REST STOP\n"
	.string "The last rest before Mt. Pyre.\p"
	.string "The keeper lets your POKéMON rest.\n"
	.string "The matchmaker finds you a battle or trade.$"

Square_Route123_Text_Local1:
	.string "The Berry Master's garden is just up\n"
	.string "the road. I come here to rest after.\p"
	.string "Picking berries is hard work!$"
