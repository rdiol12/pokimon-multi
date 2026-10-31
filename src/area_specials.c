#include "global.h"
#include "pokemon.h"
#include "script_pokemon_util.h"

// Script helpers for the new areas (docs/areas.md), called with callnative.

// A soak in the Ember Springs: heals the party and makes each Pokémon a
// little friendlier.
void Area_SoakInHotSpring(void)
{
    u32 i;

    HealPlayerParty();
    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 friendship;

        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        friendship = GetMonData(mon, MON_DATA_FRIENDSHIP) + 10;
        if (friendship > MAX_FRIENDSHIP)
            friendship = MAX_FRIENDSHIP;
        SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
    }
}
