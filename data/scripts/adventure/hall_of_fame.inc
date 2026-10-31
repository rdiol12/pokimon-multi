@ The Hall of Fame wall (src/hall_of_fame_wall.c): the curator in the
@ POKéMON LEAGUE, and R > ADVENTURE > HALL OF FAME WALL.

Adventure_EventScript_HallOfFameCurator::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, Adventure_EventScript_HallOfFameCuratorChampion
	msgbox Adventure_Text_HallOfFameCurator, MSGBOX_DEFAULT
	goto Adventure_EventScript_HallOfFameCuratorShow
Adventure_EventScript_HallOfFameCuratorChampion:
	msgbox Adventure_Text_HallOfFameCuratorChampion, MSGBOX_DEFAULT
Adventure_EventScript_HallOfFameCuratorShow:
	closemessage
	callnative HallOfFameWall_Show, waitstate=1
	release
	end

Adventure_EventScript_HallOfFameWall::
	callnative HallOfFameWall_Show, waitstate=1
	releaseall
	end

Adventure_Text_HallOfFameCurator:
	.string "I look after the HALL OF FAME WALL.\p"
	.string "Every team that enters the HALL OF\n"
	.string "FAME goes up on it, from all over.\p"
	.string "Win through to the CHAMPION and your\n"
	.string "team will hang here too!$"

Adventure_Text_HallOfFameCuratorChampion:
	.string "Ah, a CHAMPION! Your team goes up on\n"
	.string "the wall once you're online.\p"
	.string "Have a look at who else is there!$"
