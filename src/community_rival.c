#include "global.h"
#include "community.h"
#include "event_data.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/event_objects.h"
#include "constants/opponents.h"

// The postgame rival (data/scripts/community/rival.inc): after the Hall of
// Fame, BRENDAN or MAY waits at one of four places. Each time you win they
// move on to the next one with one more POKéMON, up to a full team, always
// at your level and led by the evolved starter that beats yours.

#define RIVAL_SPOTS     4
#define RIVAL_MIN_TEAM  2 // before their starter
#define RIVAL_MAX_TEAM  5

static const u16 sSpotFlags[RIVAL_SPOTS] =
{
    FLAG_COMMUNITY_RIVAL_1, FLAG_COMMUNITY_RIVAL_2, FLAG_COMMUNITY_RIVAL_3, FLAG_COMMUNITY_RIVAL_4,
};

static const u8 *const sSpotNames[RIVAL_SPOTS] =
{
    COMPOUND_STRING("EVER GRANDE CITY"),
    COMPOUND_STRING("FORTREE CITY"),
    COMPOUND_STRING("MOSSDEEP CITY"),
    COMPOUND_STRING("PACIFIDLOG TOWN"),
};

// The rival's starter beats the player's (VAR_STARTER_MON: Treecko, Torchic, Mudkip).
static const u16 sRivalAces[] = {SPECIES_BLAZIKEN, SPECIES_SWAMPERT, SPECIES_SCEPTILE};

static u32 GetRivalTrainer(void)
{
    return gSaveBlock2Ptr->playerGender == MALE ? TRAINER_MAY_PLACEHOLDER : TRAINER_BRENDAN_PLACEHOLDER;
}

// Script (each spot's map, on transition): the rival's look, and which spot
// they're at.
void CommunityRival_Setup(void)
{
    u32 i, spot = VarGet(VAR_COMMUNITY_RIVAL_WINS) % RIVAL_SPOTS;

    VarSet(VAR_OBJ_GFX_ID_E, gSaveBlock2Ptr->playerGender == MALE ? OBJ_EVENT_GFX_RIVAL_MAY_NORMAL : OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL);
    for (i = 0; i < RIVAL_SPOTS; i++)
    {
        if (i == spot && FlagGet(FLAG_SYS_GAME_CLEAR))
            FlagClear(sSpotFlags[i]);
        else
            FlagSet(sSpotFlags[i]);
    }
}

// Script: the next battle against the rival. VAR_RESULT: their team size.
void CommunityRival_Prepare(void)
{
    u32 trainer = GetRivalTrainer();
    u32 size = min(RIVAL_MIN_TEAM + VarGet(VAR_COMMUNITY_RIVAL_WINS), RIVAL_MAX_TEAM);

    FlagClear(TRAINER_FLAGS_START + trainer);
    gCommunityTrainerScale.trainerId = trainer;
    gCommunityTrainerScale.partySize = size;
    gCommunityTrainerScale.levelBonus = 0;
    gCommunityTrainerScale.aceSpecies = sRivalAces[VarGet(VAR_STARTER_MON) % ARRAY_COUNT(sRivalAces)];
    gSpecialVar_Result = size + 1;
}

// Script: after a win. They move on; STR_VAR_1 = where to.
void CommunityRival_Won(void)
{
    u32 wins = VarGet(VAR_COMMUNITY_RIVAL_WINS);

    FlagSet(sSpotFlags[wins % RIVAL_SPOTS]);
    VarSet(VAR_COMMUNITY_RIVAL_WINS, wins + 1);
    StringCopy(gStringVar1, sSpotNames[(wins + 1) % RIVAL_SPOTS]);
    ConvertIntToDecimalStringN(gStringVar2, min(RIVAL_MIN_TEAM + wins + 1, RIVAL_MAX_TEAM) + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
}
