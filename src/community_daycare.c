#include "global.h"
#include "community.h"
#include "daycare.h"
#include "event_data.h"
#include "malloc.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/daycare.h"
#include "constants/party_menu.h"

// The shared daycare (server/community_daycare.py): START > ONLINE >
// COMMUNITY > DAYCARE pairs you with another trainer. Each of you signs up
// one Pokémon as a parent (it stays in your party); once both are in, each
// of you gets an egg made from the two parents by the game's own breeding
// rules (src/daycare.c).

extern const u8 Community_EventScript_Daycare[];

#define BOX_MON_SIZE sizeof(struct BoxPokemon)
#define NICK_SIZE 12

// CommunityDaycare_MakeEgg results.
#define EGG_MADE         0
#define EGG_INCOMPATIBLE 2
#define EGG_PARTY_FULL   3

void Community_OpenDaycare(void)
{
    ScriptContext_SetupScript(Community_EventScript_Daycare);
}

// Script, waitstate: VAR_0x8004 = party slot (from ChoosePartyMon),
// VAR_0x8005 = the trainer picked from the list. Signs that Pokémon up.
void CommunityDaycare_Leave(void)
{
    u8 data[BOX_MON_SIZE + 2 + NICK_SIZE];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u16 species = GetMonData(mon, MON_DATA_SPECIES);

    memcpy(data, &mon->box, BOX_MON_SIZE);
    data[BOX_MON_SIZE] = species;
    data[BOX_MON_SIZE + 1] = species >> 8;
    memset(data + BOX_MON_SIZE + 2, EOS, NICK_SIZE);
    GetMonData(mon, MON_DATA_NICKNAME, data + BOX_MON_SIZE + 2);
    data[BOX_MON_SIZE + 2 + NICK_SIZE - 1] = EOS;
    Community_RequestAndWait(COMMUNITY_REQ_DAYCARE_LEAVE, gSpecialVar_0x8005, 0, 0, data, sizeof(data));
}

// Script: after a DAYCARE_EGG answer with both parents, makes the egg.
// VAR_RESULT: EGG_MADE, EGG_INCOMPATIBLE or EGG_PARTY_FULL.
void CommunityDaycare_MakeEgg(void)
{
    struct DayCare *daycare = AllocZeroed(sizeof(struct DayCare));

    memcpy(&daycare->mons[0].mon, gCommunityMailbox.respData, BOX_MON_SIZE);
    memcpy(&daycare->mons[1].mon, gCommunityMailbox.respData + BOX_MON_SIZE, BOX_MON_SIZE);
    daycare->offspringPersonality = Random32();
    if (GetDaycareCompatibilityScore(daycare) == PARENTS_INCOMPATIBLE)
        gSpecialVar_Result = EGG_INCOMPATIBLE;
    else if (gPartiesCount[B_TRAINER_PLAYER] >= PARTY_SIZE)
        gSpecialVar_Result = EGG_PARTY_FULL;
    else
    {
        GiveEggFromParents(daycare);
        gSpecialVar_Result = EGG_MADE;
    }
    Free(daycare);
}
