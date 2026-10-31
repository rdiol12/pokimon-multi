MtFrostcrown_MapScripts::
	.byte 0

MtFrostcrown_EventScript_Ranger::
	lock
	faceplayer
	msgbox MtFrostcrown_Text_RangerBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MtFrostcrown_EventScript_RangerStay
	msgbox MtFrostcrown_Text_RangerGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_LILYCOVE_CITY, 20, 17
	waitstate
	release
	end

MtFrostcrown_EventScript_RangerStay::
	msgbox MtFrostcrown_Text_RangerStay, MSGBOX_DEFAULT
	release
	end

MtFrostcrown_EventScript_Snorunt::
	lock
	faceplayer
	playmoncry SPECIES_SNORUNT, CRY_MODE_NORMAL
	msgbox MtFrostcrown_Text_Snorunt, MSGBOX_DEFAULT
	waitmoncry
	release
	end

MtFrostcrown_EventScript_Hermit::
	msgbox MtFrostcrown_Text_Hermit, MSGBOX_NPC
	end

MtFrostcrown_EventScript_Sign::
	msgbox MtFrostcrown_Text_Sign, MSGBOX_SIGN
	end

MtFrostcrown_EventScript_Climber1::
	lock
	faceplayer
	msgbox MtFrostcrown_Text_Climber1, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_FROST_CLIMBER_1
	fadescreen FADE_FROM_BLACK
	msgbox MtFrostcrown_Text_ClimberGone, MSGBOX_DEFAULT
	release
	end

MtFrostcrown_Text_Climber1:
	.string "Brr… I've been stuck in this grass\n"
	.string "since the blizzard. My legs are\l"
	.string "frozen stiff!\p"
	.string "Thanks for finding me. I'll make my\n"
	.string "way down to the ranger's camp.$"
MtFrostcrown_EventScript_Climber2::
	lock
	faceplayer
	msgbox MtFrostcrown_Text_Climber2, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_FROST_CLIMBER_2
	fadescreen FADE_FROM_BLACK
	msgbox MtFrostcrown_Text_ClimberGone, MSGBOX_DEFAULT
	release
	end

MtFrostcrown_Text_Climber2:
	.string "That rock rolled down and shut me\n"
	.string "in! I thought I'd never get out.\p"
	.string "Phew… I'm heading back down while\n"
	.string "the weather's clear.$"
MtFrostcrown_EventScript_Climber3::
	lock
	faceplayer
	msgbox MtFrostcrown_Text_Climber3, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_FROST_CLIMBER_3
	fadescreen FADE_FROM_BLACK
	msgbox MtFrostcrown_Text_ClimberGone, MSGBOX_DEFAULT
	release
	end

MtFrostcrown_Text_Climber3:
	.string "I took shelter under the pines and\n"
	.string "lost the trail in the snow.\p"
	.string "You found me! I'll follow your\n"
	.string "tracks back down.$"
MtFrostcrown_Text_ClimberGone:
	.string "The climber headed down the mountain.\n"
	.string "Better tell the MOUNTAIN RESCUE.$"

MtFrostcrown_EventScript_Bram::
	trainerbattle_single TRAINER_FROST_BRAM, MtFrostcrown_Text_BramIntro, MtFrostcrown_Text_BramDefeat
	msgbox MtFrostcrown_Text_BramPost, MSGBOX_AUTOCLOSE
	end

MtFrostcrown_Text_BramIntro:
	.string "This mountain is my backyard! Show\nme your climbing legs!$"

MtFrostcrown_Text_BramDefeat:
	.string "My legs gave out first…$"

MtFrostcrown_Text_BramPost:
	.string "The trail zigzags all the way up.\nTake it one shelf at a time.$"

MtFrostcrown_EventScript_Maren::
	trainerbattle_single TRAINER_FROST_MAREN, MtFrostcrown_Text_MarenIntro, MtFrostcrown_Text_MarenDefeat
	msgbox MtFrostcrown_Text_MarenPost, MSGBOX_AUTOCLOSE
	end

MtFrostcrown_Text_MarenIntro:
	.string "The cold sharpens the mind. And my\nPOKéMON.$"

MtFrostcrown_Text_MarenDefeat:
	.string "Your mind is sharper still.$"

MtFrostcrown_Text_MarenPost:
	.string "Some climbers never came back from\nthe blizzard. Keep your eyes open.$"

MtFrostcrown_EventScript_Roald::
	trainerbattle_single TRAINER_FROST_ROALD, MtFrostcrown_Text_RoaldIntro, MtFrostcrown_Text_RoaldDefeat
	msgbox MtFrostcrown_Text_RoaldPost, MSGBOX_AUTOCLOSE
	end

MtFrostcrown_Text_RoaldIntro:
	.string "You made it to the pines? Let's see\nif you can make it past me!$"

MtFrostcrown_Text_RoaldDefeat:
	.string "Snowed under!$"

MtFrostcrown_Text_RoaldPost:
	.string "The pines hide all sorts of things.\nEven people, sometimes.$"

MtFrostcrown_EventScript_Elsa::
	trainerbattle_single TRAINER_FROST_ELSA, MtFrostcrown_Text_ElsaIntro, MtFrostcrown_Text_ElsaDefeat
	msgbox MtFrostcrown_Text_ElsaPost, MSGBOX_AUTOCLOSE
	end

MtFrostcrown_Text_ElsaIntro:
	.string "Nearly at the top! But the last\nstretch is the coldest.$"

MtFrostcrown_Text_ElsaDefeat:
	.string "I'm iced!$"

MtFrostcrown_Text_ElsaPost:
	.string "Snowdrift Village is just above.\nTry the sled run on the way down!$"

MtFrostcrown_Text_RangerBack:
	.string "I run the ranger's camp. Want a\n"
	.string "lift back to LILYCOVE CITY?$"

MtFrostcrown_Text_RangerGo:
	.string "Hold on, it's a bumpy ride!$"

MtFrostcrown_Text_RangerStay:
	.string "Stay warm up there. And if you see\n"
	.string "any lost climbers, send them down!$"

MtFrostcrown_Text_Snorunt:
	.string "SNORUNT is rolling in the snow.$"

MtFrostcrown_Text_Hermit:
	.string "I've lived on this ledge for forty\n"
	.string "winters.\p"
	.string "They say a bird of ice roosts by the\n"
	.string "summit cairn. Its wings make the\l"
	.string "snow fall.$"

MtFrostcrown_Text_Sign:
	.string "MT. FROSTCROWN TRAIL\n"
	.string "SNOWDRIFT VILLAGE: straight up!$"

MtFrostcrown_EventScript_Item_MaxRepel::
	finditem ITEM_MAX_REPEL
	end

MtFrostcrown_EventScript_Item_UltraBall::
	finditem ITEM_ULTRA_BALL
	end

MtFrostcrown_EventScript_Item_TmHail::
	finditem ITEM_TM_HAIL
	end

MtFrostcrown_EventScript_Item_NeverMeltIce::
	finditem ITEM_NEVER_MELT_ICE
	end

MtFrostcrown_EventScript_Item_MaxPotion::
	finditem ITEM_MAX_POTION
	end

