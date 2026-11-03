#ifndef GUARD_NPC_NAMES_H
#define GUARD_NPC_NAMES_H

// Everyone on the map has their own name, shown in the name box when they
// talk (src/npc_names.c, names from tools/npc_names/make_names.py).

struct NpcName
{
    u16 map; // group << 8 | num
    u8 localId;
    const u8 *name;
};

// The name of the person this object is, or NULL (Pokémon, items, ...).
const u8 *NpcNames_ForObject(u32 objectEventId);
// The player talked to / was spotted by this object: its name goes on the
// messages of the script that runs now.
void NpcNames_OnTalk(u32 objectEventId);
// Before a field message is drawn (`raw` is the text before expansion).
void NpcNames_ApplyToMessage(const u8 *raw);
void NpcNames_OnScriptEnd(void);

#endif // GUARD_NPC_NAMES_H
