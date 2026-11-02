#include "global.h"
#include "online.h"
#include "random.h"
#include "wild_encounter.h"

// Living ecosystem (server/online_eco.py, docs/online.md). Online, the
// server's status for the map the player is on ("S|ECO|group.num|...") says
// which species are thinned out here (sp:population, 100 is full) and which
// have migrated or fled here (sp:percent of the encounters). The wild
// encounter code asks Eco_PickSlot to pass over thinned species (the more
// thinned, the likelier) and Eco_ModifyWildSpecies to swap in migrants.

#define ECO_REROLLS 4

static bool32 IsHere(void)
{
    const char *m = Online_Status("ECO", 0);
    s32 group = 0, num = 0;

    if (m[0] == 0)
        return FALSE;
    while (*m >= '0' && *m <= '9')
        group = group * 10 + *m++ - '0';
    if (*m++ != '.')
        return FALSE;
    while (*m >= '0' && *m <= '9')
        num = num * 10 + *m++ - '0';
    return group == gSaveBlock1Ptr->location.mapGroup && num == gSaveBlock1Ptr->location.mapNum;
}

// Reads "a:b" from s; returns the text after it (past a ',' if any).
static const char *ReadPair(const char *s, s32 *a, s32 *b)
{
    *a = *b = 0;
    while (*s >= '0' && *s <= '9')
        *a = *a * 10 + *s++ - '0';
    if (*s == ':')
        s++;
    while (*s >= '0' && *s <= '9')
        *b = *b * 10 + *s++ - '0';
    if (*s == ',')
        s++;
    return s;
}

// The population (0-100) of a species on this map; 100 if it isn't thinned.
static u32 Population(u32 species)
{
    u32 field;

    for (field = 1; field <= 2; field++)
    {
        const char *s = Online_Status("ECO", field);
        while (*s != 0)
        {
            s32 sp, pop;
            s = ReadPair(s, &sp, &pop);
            if (sp == 0)
                break;
            if ((u32)sp == species)
                return pop;
        }
    }
    return 100;
}

u32 Eco_PickSlot(const struct WildPokemon *mons, u32 index, u32 (*choose)(u32 arg), u32 arg)
{
    u32 i;

    if (!IsHere())
        return index;
    for (i = 0; i < ECO_REROLLS; i++)
    {
        u32 pop = Population(mons[index].species);
        if (pop >= 100 || Random() % 100 < pop)
            break;
        index = choose(arg);
    }
    return index;
}

enum Species Eco_ModifyWildSpecies(enum Species species)
{
    u32 field;

    if (!IsHere())
        return species;
    for (field = 3; field <= 4; field++)
    {
        s32 sp, rate;
        ReadPair(Online_Status("ECO", field), &sp, &rate);
        if (sp > 0 && sp < NUM_SPECIES && Random() % 100 < (u32)rate)
            return sp;
    }
    return species;
}
