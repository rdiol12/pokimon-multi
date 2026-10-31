#include "global.h"
#include "battle_main.h"
#include "event_data.h"
#include "item.h"
#include "malloc.h"
#include "pastime.h"
#include "pastime_spots.h"
#include "pokedex.h"
#include "pokemon.h"
#include "rtc.h"
#include "script_menu.h"
#include "string_util.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/pokedex.h"

// Trading post: the trader at the end of the Slateport market row barters
// items (three offers a day from a pool, each once a day), and once a day
// swaps a Pokémon of a wanted type for one from another region.
// R > PASTIMES > TRADING POST shows today's offers.

extern const u8 Trading_EventScript_Trader[];

struct Barter
{
    u16 giveItem;
    u8 giveCount;
    u16 getItem;
    u8 getCount;
};

static const struct Barter sBarters[] =
{
    {ITEM_ORAN_BERRY, 5, ITEM_SITRUS_BERRY, 1},
    {ITEM_PECHA_BERRY, 4, ITEM_LUM_BERRY, 1},
    {ITEM_TINY_MUSHROOM, 2, ITEM_BIG_MUSHROOM, 1},
    {ITEM_PEARL, 3, ITEM_BIG_PEARL, 1},
    {ITEM_STARDUST, 3, ITEM_STAR_PIECE, 1},
    {ITEM_NUGGET, 1, ITEM_PP_UP, 1},
    {ITEM_POKE_BALL, 5, ITEM_GREAT_BALL, 2},
    {ITEM_GREAT_BALL, 5, ITEM_ULTRA_BALL, 2},
    {ITEM_POTION, 3, ITEM_SUPER_POTION, 1},
    {ITEM_SUPER_POTION, 2, ITEM_HYPER_POTION, 1},
    {ITEM_REPEL, 2, ITEM_SUPER_REPEL, 1},
    {ITEM_SUPER_REPEL, 2, ITEM_MAX_REPEL, 1},
    {ITEM_FRESH_WATER, 3, ITEM_MOOMOO_MILK, 1},
    {ITEM_BIG_PEARL, 1, ITEM_WATER_STONE, 1},
    {ITEM_STAR_PIECE, 1, ITEM_THUNDER_STONE, 1},
    {ITEM_BIG_MUSHROOM, 1, ITEM_LEAF_STONE, 1},
    {ITEM_HEART_SCALE, 2, ITEM_FIRE_STONE, 1},
    {ITEM_SHOAL_SALT, 3, ITEM_HEART_SCALE, 1},
};

struct Swap
{
    u8 type;          // the Pokémon wanted
    u16 species;      // the one given in return
    u8 level;
    u16 heldItem;
    const u8 *nickname;
};

static const struct Swap sSwaps[] =
{
    {TYPE_FIRE,     SPECIES_SNEASEL,   20, ITEM_QUICK_CLAW,    COMPOUND_STRING("FROSTY")},
    {TYPE_WATER,    SPECIES_GROWLITHE, 20, ITEM_CHARCOAL,      COMPOUND_STRING("EMBER")},
    {TYPE_GRASS,    SPECIES_SHINX,     20, ITEM_MAGNET,        COMPOUND_STRING("SPARKY")},
    {TYPE_BUG,      SPECIES_EEVEE,     20, ITEM_SOOTHE_BELL,   COMPOUND_STRING("BUTTONS")},
    {TYPE_NORMAL,   SPECIES_RIOLU,     20, ITEM_BLACK_BELT,    COMPOUND_STRING("POW")},
    {TYPE_ELECTRIC, SPECIES_VULPIX,    20, ITEM_FIRE_STONE,    COMPOUND_STRING("KIT")},
    {TYPE_POISON,   SPECIES_SNOVER,    20, ITEM_NEVER_MELT_ICE, COMPOUND_STRING("FLURRY")},
    {TYPE_FLYING,   SPECIES_DRATINI,   20, ITEM_DRAGON_FANG,   COMPOUND_STRING("NOODLE")},
    {TYPE_ROCK,     SPECIES_TOGEPI,    20, ITEM_SHINY_STONE,   COMPOUND_STRING("PUFF")},
};

#define TRADER_OT_ID 51735

static const u8 sTraderName[] = _("TRADER");

#define sSave (gSaveBlock3Ptr->pastime)

const struct PastimeSpot gTradingSpots[1] =
{
    {MAP_SLATEPORT_CITY, 32, 27, OBJ_EVENT_GFX_POKEFAN_M, MOVEMENT_TYPE_FACE_DOWN, 0, Trading_EventScript_Trader},
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

// Today's barters: TRADING_OFFERS different ones, picked by the day.
static u32 GetBarter(u32 offer)
{
    u32 day = Today();
    return (day * 5 + offer * 7) % ARRAY_COUNT(sBarters);
}

static const struct Swap *GetSwap(void)
{
    return &sSwaps[(Today() * 4) % ARRAY_COUNT(sSwaps)];
}

// Starts a new day's offers.
static void CheckDay(void)
{
    if (sSave.tradeDay != Today() + 1) // + 1, so 0 means never
    {
        sSave.tradeDay = Today() + 1;
        sSave.tradeDone = 0;
    }
}

static u8 *AppendItems(u8 *dest, u16 item, u32 count)
{
    dest = ConvertIntToDecimalStringN(dest, count, STR_CONV_MODE_LEFT_ALIGN, 2);
    *dest++ = CHAR_SPACE;
    CopyItemNameHandlePlural(item, dest, count);
    return dest + StringLength(dest);
}

static u8 *AppendBarter(u8 *dest, u32 barter)
{
    dest = AppendItems(dest, sBarters[barter].giveItem, sBarters[barter].giveCount);
    dest = StringCopy(dest, COMPOUND_STRING(" > "));
    return AppendItems(dest, sBarters[barter].getItem, sBarters[barter].getCount);
}

static void PushMenuItem(const u8 *name, u32 id)
{
    struct ListMenuItem item;
    u8 *buffer = Alloc(StringLength(name) + 1);

    StringCopy(buffer, name);
    item.name = buffer;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

// Script: pushes today's offers onto the dynmultichoice stack.
void Trading_PushMenu(void)
{
    u8 text[48];
    u32 offer;

    CheckDay();
    for (offer = 0; offer < TRADING_OFFERS; offer++)
    {
        u8 *p = text;
        if (sSave.tradeDone & (1 << offer))
            p = StringCopy(p, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
        AppendBarter(p, GetBarter(offer));
        PushMenuItem(text, offer);
    }
    PushMenuItem(COMPOUND_STRING("POKéMON SWAP"), TRADING_MENU_SWAP);
    PushMenuItem(COMPOUND_STRING("CANCEL"), TRADING_MENU_CANCEL);
}

// Script: VAR_0x8005 = the offer picked. VAR_RESULT = TRADING_OK,
// TRADING_DONE (already today) or TRADING_MISSING. gPastimeText = the offer.
void Trading_CheckBarter(void)
{
    u32 offer = gSpecialVar_0x8005, barter = GetBarter(offer);
    u8 *p = gPastimeText;

    CheckDay();
    p = StringCopy(p, COMPOUND_STRING("Your "));
    p = AppendItems(p, sBarters[barter].giveItem, sBarters[barter].giveCount);
    p = StringCopy(p, COMPOUND_STRING("\nfor my "));
    p = AppendItems(p, sBarters[barter].getItem, sBarters[barter].getCount);
    StringCopy(p, COMPOUND_STRING("?"));
    if (sSave.tradeDone & (1 << offer))
        gSpecialVar_Result = TRADING_DONE;
    else if (!CheckBagHasItem(sBarters[barter].giveItem, sBarters[barter].giveCount))
        gSpecialVar_Result = TRADING_MISSING;
    else
        gSpecialVar_Result = TRADING_OK;
}

// Script: makes the barter (VAR_0x8005). VAR_0x8006, VAR_0x8007 = the item
// and count to give, for giveitem.
void Trading_DoBarter(void)
{
    u32 offer = gSpecialVar_0x8005, barter = GetBarter(offer);

    RemoveBagItem(sBarters[barter].giveItem, sBarters[barter].giveCount);
    sSave.tradeDone |= 1 << offer;
    if (sSave.tradeBarters < 0xFFFF)
        sSave.tradeBarters++;
    gSpecialVar_0x8006 = sBarters[barter].getItem;
    gSpecialVar_0x8007 = sBarters[barter].getCount;
}

// Script: today's swap. VAR_RESULT = FALSE if it's done for today.
// gStringVar1 = the type wanted, gStringVar2 = the Pokémon offered.
void Trading_GetSwap(void)
{
    const struct Swap *swap = GetSwap();

    CheckDay();
    StringCopy(gStringVar1, gTypesInfo[swap->type].name);
    StringCopy(gStringVar2, GetSpeciesName(swap->species));
    gSpecialVar_Result = !(sSave.tradeDone & TRADING_SWAP_DONE);
}

// Script: after ChoosePartyMon (VAR_0x8004 = the slot). VAR_RESULT =
// TRADING_OK, TRADING_MISSING (wrong type) or TRADING_EGG.
// gStringVar3 = the Pokémon's name.
void Trading_CheckSwapMon(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 type = GetSwap()->type;

    StringCopy(gStringVar1, gTypesInfo[type].name); // the party menu used it
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar3);
    StringGet_Nickname(gStringVar3);
    if (GetMonData(mon, MON_DATA_IS_EGG))
        gSpecialVar_Result = TRADING_EGG;
    else if (gSpeciesInfo[species].types[0] != type && gSpeciesInfo[species].types[1] != type)
        gSpecialVar_Result = TRADING_MISSING;
    else
        gSpecialVar_Result = TRADING_OK;
}

// Script: swaps the Pokémon in slot VAR_0x8004 for today's.
void Trading_DoSwap(void)
{
    const struct Swap *swap = GetSwap();
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u32 dexNum = SpeciesToNationalPokedexNum(swap->species);

    CreateMon(mon, swap->species, swap->level,
        GetMonPersonality(swap->species, MON_GENDER_RANDOM, NATURE_RANDOM, RANDOM_UNOWN_LETTER),
        OTID_STRUCT_PRESET(TRADER_OT_ID));
    SetMonData(mon, MON_DATA_NICKNAME, swap->nickname);
    SetMonData(mon, MON_DATA_OT_NAME, sTraderName);
    SetMonData(mon, MON_DATA_HELD_ITEM, &swap->heldItem);
    CalculateMonStats(mon);
    GetSetPokedexFlag(dexNum, FLAG_SET_SEEN);
    GetSetPokedexFlag(dexNum, FLAG_SET_CAUGHT);
    sSave.tradeDone |= TRADING_SWAP_DONE;
    if (sSave.tradeSwaps < 0xFFFF)
        sSave.tradeSwaps++;
    StringCopy(gStringVar2, swap->nickname);
}

// R > PASTIMES > TRADING POST ---------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    bool32 done;

    CheckDay();
    if (index < TRADING_OFFERS)
    {
        done = sSave.tradeDone & (1 << index);
        if (done)
            dest = StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
        AppendBarter(dest, GetBarter(index));
        return;
    }
    if (index == TRADING_OFFERS)
    {
        const struct Swap *swap = GetSwap();
        if (sSave.tradeDone & TRADING_SWAP_DONE)
            dest = StringCopy(dest, COMPOUND_STRING("{COLOR LIGHT_GRAY}{SHADOW WHITE}"));
        dest = StringCopy(dest, gTypesInfo[swap->type].name);
        dest = StringCopy(dest, COMPOUND_STRING(" type > "));
        StringCopy(dest, GetSpeciesName(swap->species));
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Trades made{CLEAR_TO 128}"));
    ConvertIntToDecimalStringN(dest, sSave.tradeBarters + sSave.tradeSwaps, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static void GetDetail(u16 index, u8 *dest)
{
    if (index < TRADING_OFFERS)
        StringCopy(dest, (sSave.tradeDone & (1 << index)) ? COMPOUND_STRING("Done today") : COMPOUND_STRING("Today's barter, once a day"));
    else if (index == TRADING_OFFERS)
        StringCopy(dest, COMPOUND_STRING("Today's POKéMON SWAP, once a day"));
    else
        StringCopy(dest, COMPOUND_STRING("The trader is in the Slateport market"));
}

void Trading_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("TRADING POST"), TRADING_OFFERS + 2, GetLine, GetDetail);
}
