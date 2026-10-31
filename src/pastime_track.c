#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "pastime.h"
#include "rtc.h"
#include "string_util.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/items.h"

// Monthly reward track: everything the player does earns Track Points (a
// visit each day, catching Pokémon, beating trainers), and each of the
// TRACK_TIERS tiers of the month's track pays out as it's reached, straight
// to the bag with a toast. A month is TRACK_MONTH_DAYS days of the game
// clock; when it turns, the points start over on the next month's track
// (three tracks take turns).

struct TrackReward
{
    u16 item;
    u8 count;
};

static const u16 sTierPoints[TRACK_TIERS] = {10, 25, 45, 70, 100, 135, 175, 220, 270, 325};

static const struct TrackReward sTracks[TRACK_COUNT][TRACK_TIERS] =
{
    { // The explorer's track
        {ITEM_POKE_BALL, 5}, {ITEM_POTION, 5}, {ITEM_GREAT_BALL, 5}, {ITEM_SUPER_REPEL, 3}, {ITEM_ULTRA_BALL, 5},
        {ITEM_RARE_CANDY, 1}, {ITEM_PP_UP, 1}, {ITEM_NUGGET, 1}, {ITEM_ABILITY_CAPSULE, 1}, {ITEM_MASTER_BALL, 1},
    },
    { // The battler's track
        {ITEM_POTION, 5}, {ITEM_X_ATTACK, 2}, {ITEM_SUPER_POTION, 5}, {ITEM_PROTEIN, 1}, {ITEM_HYPER_POTION, 3},
        {ITEM_RARE_CANDY, 2}, {ITEM_CALCIUM, 1}, {ITEM_FULL_RESTORE, 2}, {ITEM_PP_MAX, 1}, {ITEM_BOTTLE_CAP, 2},
    },
    { // The collector's track
        {ITEM_ORAN_BERRY, 5}, {ITEM_HEART_SCALE, 2}, {ITEM_STARDUST, 3}, {ITEM_NEST_BALL, 5}, {ITEM_PEARL, 3},
        {ITEM_RARE_CANDY, 1}, {ITEM_STAR_PIECE, 2}, {ITEM_BIG_NUGGET, 1}, {ITEM_ABILITY_PATCH, 1}, {ITEM_GOLD_BOTTLE_CAP, 1},
    },
};

static const u8 *const sTrackNames[TRACK_COUNT] =
{
    COMPOUND_STRING("EXPLORER'S TRACK"), COMPOUND_STRING("BATTLER'S TRACK"), COMPOUND_STRING("COLLECTOR'S TRACK"),
};

#define sSave (gSaveBlock3Ptr->pastime)

static u32 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static u32 ThisMonth(void)
{
    return Today() / TRACK_MONTH_DAYS;
}

static const struct TrackReward *ThisTrack(void)
{
    return sTracks[ThisMonth() % TRACK_COUNT];
}

// Starts the month's track over if the month turned.
static void CheckMonth(void)
{
    if (sSave.trackMonth == ThisMonth() + 1) // + 1, so 0 means never
        return;
    sSave.trackMonth = ThisMonth() + 1;
    sSave.trackPoints = 0;
    sSave.trackTier = 0;
}

// Pays out every tier reached (as long as the bag has room), one toast for
// the last one.
static void PayTiers(void)
{
    const struct TrackReward *track = ThisTrack();
    bool32 paid = FALSE;
    u8 *p;

    while (sSave.trackTier < TRACK_TIERS && sSave.trackPoints >= sTierPoints[sSave.trackTier])
    {
        const struct TrackReward *reward = &track[sSave.trackTier];
        if (!AddBagItem(reward->item, reward->count))
            break; // the bag is full: try again later
        sSave.trackTier++;
        paid = TRUE;
    }
    if (!paid)
        return;
    p = StringCopy(gStringVar4, COMPOUND_STRING("Tier "));
    p = ConvertIntToDecimalStringN(p, sSave.trackTier, STR_CONV_MODE_LEFT_ALIGN, 2);
    p = StringCopy(p, COMPOUND_STRING(": "));
    p = ConvertIntToDecimalStringN(p, track[sSave.trackTier - 1].count, STR_CONV_MODE_LEFT_ALIGN, 2);
    *p++ = CHAR_SPACE;
    CopyItemNameHandlePlural(track[sSave.trackTier - 1].item, p, track[sSave.trackTier - 1].count);
    World_ShowToast(COMPOUND_STRING("REWARD TRACK"), gStringVar4);
}

static void AddPoints(u32 points)
{
    CheckMonth();
    sSave.trackPoints = min(sSave.trackPoints + points, 0xFFFF);
}

// Pastime_Update: the first visit of each day, and any tiers waiting for
// room in the bag.
void Track_Update(void)
{
    CheckMonth();
    if (sSave.trackDay != Today() + 1)
    {
        sSave.trackDay = Today() + 1;
        AddPoints(TRACK_DAILY_POINTS);
    }
    PayTiers();
}

void Track_OnCatch(void)
{
    AddPoints(TRACK_CATCH_POINTS);
}

void Track_OnTrainerBattleEnd(void)
{
    if (gBattleOutcome == B_OUTCOME_WON)
        AddPoints(TRACK_TRAINER_POINTS);
}

// R > PASTIMES > REWARD TRACK -------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    const struct TrackReward *reward;

    CheckMonth();
    if (index == 0)
    {
        dest = StringCopy(dest, sTrackNames[ThisMonth() % TRACK_COUNT]);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 144}"));
        dest = ConvertIntToDecimalStringN(dest, sSave.trackPoints, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringCopy(dest, COMPOUND_STRING(" pts"));
        return;
    }
    reward = &ThisTrack()[index - 1];
    dest = ConvertIntToDecimalStringN(dest, sTierPoints[index - 1], STR_CONV_MODE_RIGHT_ALIGN, 3);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 32}"));
    dest = ConvertIntToDecimalStringN(dest, reward->count, STR_CONV_MODE_LEFT_ALIGN, 2);
    *dest++ = CHAR_SPACE;
    CopyItemNameHandlePlural(reward->item, dest, reward->count);
    dest += StringLength(dest);
    dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 160}"));
    StringCopy(dest, sSave.trackTier >= index ? COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}GOT") : COMPOUND_STRING("-"));
}

static void GetDetail(u16 index, u8 *dest)
{
    u32 left = TRACK_MONTH_DAYS - Today() % TRACK_MONTH_DAYS;
    u8 *p;

    p = StringCopy(dest, COMPOUND_STRING("A new track in "));
    p = ConvertIntToDecimalStringN(p, left, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringCopy(p, left == 1 ? COMPOUND_STRING(" day") : COMPOUND_STRING(" days"));
}

void Track_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("REWARD TRACK"), TRACK_TIERS + 1, GetLine, GetDetail);
}
