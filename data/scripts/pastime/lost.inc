@ Lost and found (src/pastime_lost.c). VAR_0x8004 = which owner or thing.

LostFound_EventScript_Owner::
	lock
	faceplayer
	callnative LostFound_TalkToOwner
	goto_if_eq VAR_RESULT, LOST_FOUND_CARRYING, LostFound_EventScript_GiveBack
	msgbox gPastimeText, MSGBOX_DEFAULT
	release
	end

LostFound_EventScript_GiveBack:
	msgbox LostFound_Text_HandOver, MSGBOX_DEFAULT
	msgbox gPastimeText, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	callnative LostFound_Return
	release
	end

LostFound_EventScript_Item::
	lock
	callnative LostFound_PickUp
	removeobject VAR_LAST_TALKED
	playfanfare MUS_OBTAIN_ITEM
	message LostFound_Text_Found
	waitmessage
	waitfanfare
	goto_if_eq VAR_RESULT, TRUE, LostFound_EventScript_KnownOwner
	msgbox LostFound_Text_UnknownOwner, MSGBOX_DEFAULT
	release
	end

LostFound_EventScript_KnownOwner:
	msgbox LostFound_Text_KnownOwner, MSGBOX_DEFAULT
	release
	end

LostFound_Text_HandOver:
	.string "{PLAYER} handed over the\n{STR_VAR_1}.$"

LostFound_Text_Found:
	.string "{PLAYER} found a {STR_VAR_1}!$"

LostFound_Text_UnknownOwner:
	.string "There's a name on it: {STR_VAR_2},\n"
	.string "{STR_VAR_3}.\p"
	.string "Someone there must be missing it.$"

LostFound_Text_KnownOwner:
	.string "It's {STR_VAR_2}'s {STR_VAR_1}!\n"
	.string "Better take it back to {STR_VAR_3}.$"
