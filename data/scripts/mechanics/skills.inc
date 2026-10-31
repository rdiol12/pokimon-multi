@ The trainer skills teacher in the Rustboro Pokémon School
@ (src/mechanics_skills.c): a skill for every second badge, one on at a time.
Mechanics_EventScript_SkillTeacher::
	lockall
	faceplayer
	goto_if_set FLAG_MECH_MET_SKILL_TEACHER, Mechanics_EventScript_SkillMenu
	setflag FLAG_MECH_MET_SKILL_TEACHER
	msgbox Mechanics_Text_SkillIntro, MSGBOX_DEFAULT
Mechanics_EventScript_SkillMenu:
	callnative Mechanics_CountSkills
	goto_if_eq VAR_RESULT, 0, Mechanics_EventScript_SkillNoBadges
	msgbox Mechanics_Text_SkillCurrent, MSGBOX_DEFAULT
	message Mechanics_Text_SkillWhich
	waitmessage
	callnative Mechanics_PushSkillRows
	dynmultipush Mechanics_Text_RefereeNotNow, 0
	dynmultistack 0, 0, FALSE, 5, FALSE, SKILL_FIELD_MEDIC, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, Mechanics_EventScript_SkillBye
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Mechanics_EventScript_SkillBye
	copyvar VAR_0x8005, VAR_RESULT
	callnative Mechanics_GetSkillHelp
	msgbox gStringVar4, MSGBOX_DEFAULT
	msgbox Mechanics_Text_SkillUse, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Mechanics_EventScript_SkillMenu
	callnative Mechanics_SetSkill
	playse SE_SUCCESS
	msgbox Mechanics_Text_SkillOn, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_EventScript_SkillNoBadges:
	msgbox Mechanics_Text_SkillNoBadges, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_EventScript_SkillBye:
	msgbox Mechanics_Text_SkillBye, MSGBOX_DEFAULT
	releaseall
	end

Mechanics_Text_SkillIntro:
	.string "Welcome! I teach TRAINER SKILLS,\n"
	.string "the tricks good TRAINERS use.\p"
	.string "Every second GYM BADGE you earn,\n"
	.string "I have a new skill for you.\p"
	.string "You can only keep one in mind at a\n"
	.string "time, but come back to switch.$"

Mechanics_Text_SkillNoBadges:
	.string "Come back with a GYM BADGE and I'll\n"
	.string "teach you your first skill!$"

Mechanics_Text_SkillCurrent:
	.string "Skill on right now: {STR_VAR_1}.$"

Mechanics_Text_SkillWhich:
	.string "Which skill do you want to use?$"

Mechanics_Text_SkillUse:
	.string "Use {STR_VAR_1}?$"

Mechanics_Text_SkillOn:
	.string "{STR_VAR_1} is on! Good luck out\n"
	.string "there.$"

Mechanics_Text_SkillBye:
	.string "Study hard, train harder!$"
