MirageOasis_Inn_MapScripts::
	.byte 0

MirageOasis_Inn_EventScript_Keeper::
	lock
	faceplayer
	msgbox MirageOasis_Inn_Text_Keeper, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MirageOasis_Inn_EventScript_KeeperNo
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox MirageOasis_Inn_Text_KeeperDone, MSGBOX_DEFAULT
	release
	end

MirageOasis_Inn_EventScript_KeeperNo::
	msgbox MirageOasis_Inn_Text_KeeperNo, MSGBOX_DEFAULT
	release
	end

MirageOasis_Inn_Text_Keeper:
	.string "Welcome to the oasis inn! Rest in\n"
	.string "the shade a while?$"

MirageOasis_Inn_Text_KeeperDone:
	.string "Refreshed? The desert is kinder to\n"
	.string "those who rest.$"

MirageOasis_Inn_Text_KeeperNo:
	.string "The shade is always here.$"
