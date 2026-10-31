#include "global.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "life.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "constants/battle_ai.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/trainers.h"

// Team Eclipse (docs/life.md): a postgame chapter. After the player becomes
// Champion, Team Eclipse raids the Mossdeep Space Center: they want to use
// its rocket to put a shade in orbit and leave Hoenn in endless night.
// The player chases them through Meteor Falls (moon rock), the Mt. Pyre
// summit (Admin Vesper) and back to the Space Center (Leader Nyx).
//
// VAR_LIFE_ECLIPSE is the step. Each step has two trainers; beating both
// moves on to the next. The trainers' objects are in the maps with
// FLAG_LIFE_ECLIPSE_HIDE_* flags that this file sets on every map load, so
// each one shows only during its step and until it's beaten.

#define IVS(n) TRAINER_PARTY_IVS(n, n, n, n, n, n)
#define MON(sp, level, ivs, item)                                            \
    {.species = sp, .lvl = level, .iv = IVS(ivs), .heldItem = item,          \
     .gender = TRAINER_MON_RANDOM_GENDER, .ball = POKEBALL_COUNT,             \
     .nature = NATURE_HARDY, .dynamaxLevel = MAX_DYNAMAX_LEVEL}

static const struct TrainerMon sFallsGruntM[] =
{
    MON(SPECIES_MURKROW, 58, 20, ITEM_NONE),
    MON(SPECIES_LUNATONE, 58, 20, ITEM_NONE),
    MON(SPECIES_MIGHTYENA, 60, 20, ITEM_NONE),
};

static const struct TrainerMon sFallsGruntF[] =
{
    MON(SPECIES_SABLEYE, 59, 20, ITEM_NONE),
    MON(SPECIES_SOLROCK, 59, 20, ITEM_NONE),
    MON(SPECIES_LIEPARD, 60, 20, ITEM_NONE),
};

static const struct TrainerMon sPyreGrunt[] =
{
    MON(SPECIES_CRAWDAUNT, 61, 22, ITEM_NONE),
    MON(SPECIES_ABSOL, 61, 22, ITEM_NONE),
    MON(SPECIES_CROBAT, 62, 22, ITEM_NONE),
};

static const struct TrainerMon sVesper[] =
{
    MON(SPECIES_HONCHKROW, 64, 26, ITEM_SITRUS_BERRY),
    MON(SPECIES_GENGAR, 64, 26, ITEM_FOCUS_SASH),
    MON(SPECIES_BISHARP, 65, 26, ITEM_BLACK_GLASSES),
    MON(SPECIES_WEAVILE, 66, 26, ITEM_LIFE_ORB),
};

static const struct TrainerMon sCenterGrunt[] =
{
    MON(SPECIES_COFAGRIGUS, 63, 24, ITEM_NONE),
    MON(SPECIES_MISMAGIUS, 63, 24, ITEM_NONE),
    MON(SPECIES_DRAPION, 64, 24, ITEM_NONE),
};

static const struct TrainerMon sNyx[] =
{
    MON(SPECIES_UMBREON, 68, 31, ITEM_LEFTOVERS),
    MON(SPECIES_MALAMAR, 68, 31, ITEM_SITRUS_BERRY),
    MON(SPECIES_SPIRITOMB, 68, 31, ITEM_LEFTOVERS),
    MON(SPECIES_ABSOL, 69, 31, ITEM_SCOPE_LENS),
    MON(SPECIES_TYRANITAR, 69, 31, ITEM_CHOICE_BAND),
    MON(SPECIES_HYDREIGON, 70, 31, ITEM_LIFE_ORB),
};

#define ECLIPSE_TRAINER(class, pic, female, name, mons, ai) \
{                                                         \
    .trainerClass = class,                                \
    .trainerPic = pic,                                    \
    .gender = female ? TRAINER_GENDER_FEMALE : TRAINER_GENDER_MALE,                                     \
    .trainerName = _(name),                               \
    .party = mons,                                        \
    .partySize = ARRAY_COUNT(mons),                       \
    .aiFlags = ai,                                        \
}

static const struct
{
    struct Trainer trainer;
    u8 step;
    const u8 *intro;
    const u8 *defeat; // in battle
    const u8 *after;
} sEclipse[LIFE_ECLIPSE_TRAINERS] =
{
    {
        ECLIPSE_TRAINER(TRAINER_CLASS_TEAM_ECLIPSE, TRAINER_PIC_ECLIPSE_GRUNT_M, FALSE, "GRUNT", sFallsGruntM, AI_FLAG_BASIC_TRAINER),
        LIFE_ECLIPSE_METEOR_FALLS,
        COMPOUND_STRING("Hey! This moon rock belongs to\n"
                        "TEAM ECLIPSE now!\p"
                        "When the sun goes out, it's the\n"
                        "POKéMON of the night that'll rule!$"),
        COMPOUND_STRING("The night… didn't fall for me…$"),
        COMPOUND_STRING("Fine, keep your rocks! We've got\n"
                        "plenty for the launch already.$"),
    },
    {
        ECLIPSE_TRAINER(TRAINER_CLASS_TEAM_ECLIPSE, TRAINER_PIC_ECLIPSE_GRUNT_F, TRUE, "GRUNT", sFallsGruntF, AI_FLAG_BASIC_TRAINER),
        LIFE_ECLIPSE_METEOR_FALLS,
        COMPOUND_STRING("You're the CHAMPION? Ha! Champions\n"
                        "shine. We put out what shines.$"),
        COMPOUND_STRING("Too bright… too bright!$"),
        COMPOUND_STRING("Admin VESPER is up on MT. PYRE,\n"
                        "gathering the dusk from the graves.\p"
                        "You'll never stop her!$"),
    },
    {
        ECLIPSE_TRAINER(TRAINER_CLASS_TEAM_ECLIPSE, TRAINER_PIC_ECLIPSE_GRUNT_M, FALSE, "GRUNT", sPyreGrunt, AI_FLAG_BASIC_TRAINER),
        LIFE_ECLIPSE_MT_PYRE,
        COMPOUND_STRING("The ADMIN is not to be disturbed.\n"
                        "Turn around, CHAMPION!$"),
        COMPOUND_STRING("Ugh! Sorry, Admin…$"),
        COMPOUND_STRING("Even the spirits here are on our\n"
                        "side. They hate the light too.$"),
    },
    {
        ECLIPSE_TRAINER(TRAINER_CLASS_ECLIPSE_ADMIN, TRAINER_PIC_ECLIPSE_ADMIN, TRUE, "VESPER", sVesper, AI_FLAG_SMART_TRAINER),
        LIFE_ECLIPSE_MT_PYRE,
        COMPOUND_STRING("I am VESPER, ADMIN of TEAM ECLIPSE.\p"
                        "Hoenn burns under its sun and drowns\n"
                        "under its rain. We'll give it peace:\l"
                        "one long, quiet night.\p"
                        "You stopped MAGMA and AQUA. You\n"
                        "won't stop the dark!$"),
        COMPOUND_STRING("The evening star… sets.$"),
        COMPOUND_STRING("It doesn't matter. Our leader, NYX,\n"
                        "is already at the SPACE CENTER.\p"
                        "When the rocket launches tonight,\n"
                        "the shade goes up with it!$"),
    },
    {
        ECLIPSE_TRAINER(TRAINER_CLASS_TEAM_ECLIPSE, TRAINER_PIC_ECLIPSE_GRUNT_F, TRUE, "GRUNT", sCenterGrunt, AI_FLAG_BASIC_TRAINER),
        LIFE_ECLIPSE_SPACE_CENTER,
        COMPOUND_STRING("The countdown has started! Nobody\n"
                        "gets near the launch controls!$"),
        COMPOUND_STRING("No! The countdown…$"),
        COMPOUND_STRING("Go on, then. NYX will finish you.$"),
    },
    {
        ECLIPSE_TRAINER(TRAINER_CLASS_ECLIPSE_LEADER, TRAINER_PIC_ECLIPSE_LEADER, TRUE, "NYX", sNyx, AI_FLAG_SMART_TRAINER),
        LIFE_ECLIPSE_SPACE_CENTER,
        COMPOUND_STRING("So you're the CHAMPION. I'm NYX.\p"
                        "MAXIE wanted more land. ARCHIE wanted\n"
                        "more sea. Both nearly woke the gods.\p"
                        "I only want the sun to rest. No\n"
                        "droughts, no storms. Just the night.\p"
                        "Step aside, or be the last light\n"
                        "I put out!$"),
        COMPOUND_STRING("…So this is what dawn feels like.$"),
        COMPOUND_STRING("The launch is off. Without the shade\n"
                        "there's no eclipse.\p"
                        "Maybe Hoenn needs its sun after all.\n"
                        "TEAM ECLIPSE is finished.$"),
    },
};

// The trainer being talked to. The objects' scripts set VAR_0x8004
// (data/scripts/life/eclipse.inc); it's kept here because the battle
// reuses the special vars.
static EWRAM_DATA u8 sTrainerId = 0;

// Every map load (Life_OnMapTransition): show each trainer only during its
// step until it's beaten, and the scientist only after the game is beaten.
void LifeEclipse_UpdateObjects(void)
{
    u32 i, step = VarGet(VAR_LIFE_ECLIPSE);
    bool32 postgame = FlagGet(FLAG_SYS_GAME_CLEAR);

    for (i = 0; i < LIFE_ECLIPSE_TRAINERS; i++)
    {
        if (postgame && sEclipse[i].step == step && !FlagGet(FLAG_LIFE_ECLIPSE_BEATEN_0 + i))
            FlagClear(FLAG_LIFE_ECLIPSE_HIDE_0 + i);
        else
            FlagSet(FLAG_LIFE_ECLIPSE_HIDE_0 + i);
    }
    if (postgame)
        FlagClear(FLAG_LIFE_ECLIPSE_HIDE_SCIENTIST);
    else
        FlagSet(FLAG_LIFE_ECLIPSE_HIDE_SCIENTIST);
}

// For screenshots: the game beaten (with all eight badges) and the chapter
// at `step`. The story's
// own visitors to these maps are gone, as they are by the postgame.
void LifeEclipse_ApplyShowcase(u32 step)
{
    static const u16 sStoryFlags[] =
    {
        FLAG_HIDE_METEOR_FALLS_TEAM_MAGMA, FLAG_HIDE_METEOR_FALLS_TEAM_AQUA, FLAG_HIDE_METEOR_FALLS_1F_1R_COZMO,
        FLAG_HIDE_MT_PYRE_SUMMIT_ARCHIE, FLAG_HIDE_MT_PYRE_SUMMIT_MAXIE, FLAG_HIDE_MT_PYRE_SUMMIT_TEAM_AQUA,
        FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_1F_TEAM_MAGMA, FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_TEAM_MAGMA,
        FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_1F_STEVEN, FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_STEVEN,
        FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_MAGMA_NOTE,
    };
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sStoryFlags); i++)
        FlagSet(sStoryFlags[i]);
    FlagSet(FLAG_SYS_GAME_CLEAR);
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        FlagSet(i);
    // No "Will you switch?" between the opponent's Pokémon, and a party
    // that's ready to fight.
    gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SET;
    HealPlayerParty();
    VarSet(VAR_LIFE_ECLIPSE, step);
    for (i = 0; i < LIFE_ECLIPSE_TRAINERS; i++)
    {
        if (sEclipse[i].step < step)
            FlagSet(FLAG_LIFE_ECLIPSE_BEATEN_0 + i);
        else
            FlagClear(FLAG_LIFE_ECLIPSE_BEATEN_0 + i);
    }
}

void LifeEclipse_BufferIntro(void)
{
    sTrainerId = min(gSpecialVar_0x8004, LIFE_ECLIPSE_TRAINERS - 1);
    StringExpandPlaceholders(gStringVar4, sEclipse[sTrainerId].intro);
}

void LifeEclipse_StartBattle(void)
{
    LifeBattle_Start(&sEclipse[sTrainerId].trainer, sEclipse[sTrainerId].defeat);
}

// After a win: VAR_RESULT is TRUE if that was the step's last trainer, and
// VAR_0x8004 is the trainer again.
void LifeEclipse_Defeated(void)
{
    u32 i, id = sTrainerId, step = sEclipse[id].step;

    gSpecialVar_0x8004 = id;

    FlagSet(FLAG_LIFE_ECLIPSE_BEATEN_0 + id);
    FlagSet(FLAG_LIFE_ECLIPSE_HIDE_0 + id);
    StringExpandPlaceholders(gStringVar4, sEclipse[id].after);
    gSpecialVar_Result = TRUE;
    for (i = 0; i < LIFE_ECLIPSE_TRAINERS; i++)
    {
        if (sEclipse[i].step == step && !FlagGet(FLAG_LIFE_ECLIPSE_BEATEN_0 + i))
            gSpecialVar_Result = FALSE;
    }
    if (gSpecialVar_Result && VarGet(VAR_LIFE_ECLIPSE) == step)
        VarSet(VAR_LIFE_ECLIPSE, step + 1);
}

// The Space Center scientist: starts the chapter and gives the reward.
void LifeEclipse_Scientist(void)
{
    gSpecialVar_Result = VarGet(VAR_LIFE_ECLIPSE);
    if (gSpecialVar_Result == LIFE_ECLIPSE_NOT_STARTED)
        VarSet(VAR_LIFE_ECLIPSE, LIFE_ECLIPSE_METEOR_FALLS);
}
