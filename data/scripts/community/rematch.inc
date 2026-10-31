@ Gym rematches at your level and the postgame aces.
@
@ After the Hall of Fame each gym leader offers a rematch: a double battle
@ with their strongest team, a level above your best Pokémon
@ (src/community_rematch.c). Each leader's script calls its
@ Community_EventScript_Rematch<Name> once the game is clear.
@
@ Seven aces (Cooltrainers who specialise in the gym's type) wait by the gym
@ entrances. Before the Hall of Fame they won't battle; after it they do,
@ once, at your level, and give a type item.

.macro leader_rematch name:req, trainer:req
	.global Community_EventScript_Rematch\name
Community_EventScript_Rematch\name:
	msgbox Community_Text_RematchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Community_EventScript_RematchDeclined
	setvar VAR_0x8004, \trainer
	callnative CommunityRematch_Prepare
	goto_if_eq VAR_RESULT, FALSE, Community_EventScript_RematchNeedTwo
	trainerbattle_double \trainer, Community_Text_RematchIntro, Community_Text_RematchDefeat, Community_Text_RematchNeedTwo, Community_EventScript_RematchWon, NO_MUSIC
	end
.endm

	leader_rematch Roxanne, TRAINER_ROXANNE_5
	leader_rematch Brawly, TRAINER_BRAWLY_5
	leader_rematch Wattson, TRAINER_WATTSON_5
	leader_rematch Flannery, TRAINER_FLANNERY_5
	leader_rematch Norman, TRAINER_NORMAN_5
	leader_rematch Winona, TRAINER_WINONA_5
	leader_rematch TateAndLiza, TRAINER_TATE_AND_LIZA_5
	leader_rematch Juan, TRAINER_JUAN_5

Community_EventScript_RematchDeclined::
	msgbox Community_Text_RematchDeclined, MSGBOX_DEFAULT
	release
	end

Community_EventScript_RematchNeedTwo::
	msgbox Community_Text_RematchNeedTwo, MSGBOX_DEFAULT
	release
	end

Community_EventScript_RematchWon::
	callnative CommunityRematch_Finish
	msgbox Community_Text_RematchWon, MSGBOX_DEFAULT
	release
	end

Community_Text_RematchAsk::
	.string "Now that you're the CHAMPION…\n"
	.string "Will you face my best team, at your\l"
	.string "full strength, in a double battle?$"

Community_Text_RematchDeclined::
	.string "Come back whenever you're ready.\n"
	.string "My team keeps up with yours.$"

Community_Text_RematchIntro::
	.string "No holding back. Let's go!$"

Community_Text_RematchDefeat::
	.string "You really are the CHAMPION.$"

Community_Text_RematchNeedTwo::
	.string "This is a double battle.\n"
	.string "Bring at least two POKéMON.$"

Community_Text_RematchWon::
	.string "That was a great battle.\n"
	.string "My team will be stronger next time.$"

@ --- The aces ------------------------------------------------------------

.macro gym_ace name:req, trainer:req, item:req
	.global Community_EventScript_Ace\name
Community_EventScript_Ace\name:
	goto_if_unset FLAG_SYS_GAME_CLEAR, Community_EventScript_AceWaiting
	setvar VAR_0x8004, \trainer
	setvar VAR_0x8005, 0
	setvar VAR_0x8006, 1
	callnative Community_SetTrainerScale
	trainerbattle_single \trainer, Community_Text_Ace\name\()Intro, Community_Text_AceDefeat, Community_EventScript_Ace\name\()Reward
	msgbox Community_Text_Ace\name\()After, MSGBOX_AUTOCLOSE
	end

Community_EventScript_Ace\name\()Reward:
	msgbox Community_Text_AceReward, MSGBOX_DEFAULT
	giveitem \item
	msgbox Community_Text_Ace\name\()After, MSGBOX_DEFAULT
	release
	end
.endm

	gym_ace Rustboro, TRAINER_DUDLEY, ITEM_HARD_STONE
	gym_ace Dewford, TRAINER_KAYLEE, ITEM_BLACK_BELT
	gym_ace Mauville, TRAINER_TERRY, ITEM_MAGNET
	gym_ace Lavaridge, TRAINER_CINDY_2, ITEM_CHARCOAL
	gym_ace Fortree, TRAINER_LUCAS_2, ITEM_SHARP_BEAK
	gym_ace Mossdeep, TRAINER_MIKE_1, ITEM_TWISTED_SPOON
	gym_ace Sootopolis, TRAINER_GRUNT_UNUSED, ITEM_MYSTIC_WATER

Community_EventScript_AceWaiting::
	msgbox Community_Text_AceWaiting, MSGBOX_NPC
	end

Community_Text_AceWaiting::
	.string "I'm training here for one battle:\n"
	.string "the CHAMPION's.\p"
	.string "Come back once you're in the\n"
	.string "HALL OF FAME.$"

Community_Text_AceDefeat::
	.string "So that's a CHAMPION's strength!$"

Community_Text_AceReward::
	.string "You earned this. Take it!$"

Community_Text_AceRustboroIntro::
	.string "ROCK-type POKéMON don't crumble.\n"
	.string "CHAMPION, let's test that!$"
Community_Text_AceRustboroAfter::
	.string "I'll be back at full strength.\n"
	.string "Like the stone in this gym.$"

Community_Text_AceDewfordIntro::
	.string "Every punch I've trained for\n"
	.string "was for this moment!$"
Community_Text_AceDewfordAfter::
	.string "A loss is just more training.\n"
	.string "Thanks, CHAMPION!$"

Community_Text_AceMauvilleIntro::
	.string "My team runs at a million volts.\n"
	.string "Try not to get shocked!$"
Community_Text_AceMauvilleAfter::
	.string "Bzzt… I'm all out of charge.$"

Community_Text_AceLavaridgeIntro::
	.string "The hot springs keep my team warm.\n"
	.string "Now they're burning to battle!$"
Community_Text_AceLavaridgeAfter::
	.string "Phew… I need a long soak.$"

Community_Text_AceFortreeIntro::
	.string "Up here, we see every move\n"
	.string "coming. Ready, CHAMPION?$"
Community_Text_AceFortreeAfter::
	.string "Even birds get grounded sometimes.$"

Community_Text_AceMossdeepIntro::
	.string "I foresaw this battle.\n"
	.string "I didn't foresee the ending…$"
Community_Text_AceMossdeepAfter::
	.string "Next time I'll read your mind\n"
	.string "a little faster.$"

Community_Text_AceSootopolisIntro::
	.string "The water here is calm.\n"
	.string "My team is not!$"
Community_Text_AceSootopolisAfter::
	.string "You flow like water, CHAMPION.$"
