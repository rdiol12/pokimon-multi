#include "global.h"
#include "community.h"
#include "script.h"

// Team check (server/community_legal.py): START > ONLINE > COMMUNITY >
// CHECK TEAM asks the server to look over the party for Pokémon that
// couldn't have come from the game.

extern const u8 Community_EventScript_LegalCheck[];

void Community_OpenLegalCheck(void)
{
    ScriptContext_SetupScript(Community_EventScript_LegalCheck);
}
