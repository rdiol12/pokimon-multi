@ The guild hall: every guild has its own (the server only shows members of
@ the same guild to each other here). See data/scripts/online/guilds.inc.

OnlineGuildHall_MapScripts::
	.byte 0

OnlineGuildHall_EventScript_Clerk::
	lock
	faceplayer
	goto Online_EventScript_GuildClerk
	end

OnlineGuildHall_EventScript_WarOfficer::
	lock
	faceplayer
	goto Online_EventScript_GuildWar
	end

OnlineGuildHall_EventScript_RaidCaptain::
	lock
	faceplayer
	goto Online_EventScript_GuildRaids
	end
