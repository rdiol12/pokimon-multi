GullwingPoint_PirateShip_MapScripts::
	.byte 0

GullwingPoint_PirateShip_EventScript_Captain::
	lock
	faceplayer
	goto_if_set FLAG_GULL_CAPTAIN_BEATEN, GullwingPoint_PirateShip_EventScript_CaptainAfter
	msgbox GullwingPoint_PirateShip_Text_CaptainIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_GULL_CAPTAIN, GullwingPoint_PirateShip_Text_CaptainDefeat
	setflag FLAG_GULL_CAPTAIN_BEATEN
	msgbox GullwingPoint_PirateShip_Text_CaptainGives, MSGBOX_DEFAULT
	release
	end

GullwingPoint_PirateShip_EventScript_CaptainAfter::
	msgbox GullwingPoint_PirateShip_Text_CaptainAfter, MSGBOX_DEFAULT
	release
	end

GullwingPoint_PirateShip_EventScript_Murkrow::
	lock
	faceplayer
	playmoncry SPECIES_MURKROW, CRY_MODE_NORMAL
	msgbox GullwingPoint_PirateShip_Text_Murkrow, MSGBOX_DEFAULT
	waitmoncry
	release
	end

GullwingPoint_PirateShip_EventScript_Peg::
	trainerbattle_single TRAINER_GULL_PEG, GullwingPoint_PirateShip_Text_PegIntro, GullwingPoint_PirateShip_Text_PegDefeat
	msgbox GullwingPoint_PirateShip_Text_PegPost, MSGBOX_AUTOCLOSE
	end

GullwingPoint_PirateShip_EventScript_Finn::
	trainerbattle_single TRAINER_GULL_FINN, GullwingPoint_PirateShip_Text_FinnIntro, GullwingPoint_PirateShip_Text_FinnDefeat
	msgbox GullwingPoint_PirateShip_Text_FinnPost, MSGBOX_AUTOCLOSE
	end

GullwingPoint_PirateShip_Text_CaptainIntro:
	.string "So YOU'RE the one who lit up my ship\n"
	.string "for the whole coast to see!\p"
	.string "I'm CAPTAIN BARNACLE, terror of the\n"
	.string "seven routes! Prepare to be boarded!$"

GullwingPoint_PirateShip_Text_CaptainDefeat:
	.string "Blast and barnacles! Beaten by a\n"
	.string "child with a flashlight!$"

GullwingPoint_PirateShip_Text_CaptainGives:
	.string "A deal's a deal. The treasure in the\n"
	.string "hold is yours, and we'll give the\l"
	.string "fishermen their things back.\p"
	.string "Maybe it's time we became honest\n"
	.string "sailors. Maybe.$"

GullwingPoint_PirateShip_Text_CaptainAfter:
	.string "Honest sailing is hard work. I miss\n"
	.string "the fog already.$"

GullwingPoint_PirateShip_Text_Murkrow:
	.string "MURKROW squawks: “Treasure! Treasure!”$"

GullwingPoint_PirateShip_Text_PegIntro:
	.string "Arr! Ye came up the gangplank? Then\n"
	.string "ye'll go down it in defeat!$"

GullwingPoint_PirateShip_Text_PegDefeat:
	.string "I'm the one going down…$"

GullwingPoint_PirateShip_Text_PegPost:
	.string "The CAPTAIN's at the helm. Good\n"
	.string "luck, ye'll need it.$"

GullwingPoint_PirateShip_Text_FinnIntro:
	.string "Stay away from the hatch! That's\n"
	.string "where the loot is!$"

GullwingPoint_PirateShip_Text_FinnDefeat:
	.string "Oops. I said too much.$"

GullwingPoint_PirateShip_Text_FinnPost:
	.string "Only the CAPTAIN can say who gets\n"
	.string "the treasure.$"

