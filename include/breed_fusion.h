#ifndef GUARD_BREED_FUSION_H
#define GUARD_BREED_FUSION_H

// Fusion breeding (docs/breed_fusion.md): two Pokémon of different kinds
// at the Day Care make a fusion egg. The baby has its mother's shape and
// its father's colors, a name made from both, one type from each, and the
// average of both parents' base stats. The father's species is stored in
// the Pokémon (MON_DATA_FUSION_PARTNER, spare bits), so a fusion trades,
// evolves and battles online like any other Pokémon.
//
// The server builds each fusion (server/community_fusion.py): when the
// party holds a fusion whose colors the server hasn't built yet, the game
// sends it the two species and saves the colors it answers with
// (SaveBlock3 breedFusion). Until then, or offline, the game builds them
// itself. Types, stats and the name come from the same rules on both
// sides, so link battles agree even before a game has synced, and the
// server's legality checks (server/community_legal.py) know fusions.

u32 BreedFusion_GetPartner(struct BoxPokemon *boxMon);
bool32 BreedFusion_CanFuse(u32 motherSpecies, u32 fatherSpecies);
bool32 BreedFusion_CanFuseDaycare(struct DayCare *daycare);
void BreedFusion_MakeEgg(struct Pokemon *egg, struct BoxPokemon *father);
u32 BreedFusion_GetBaseStat(struct BoxPokemon *boxMon, u32 species, u32 stat);
void BreedFusion_GetTypes(u32 species, u32 partner, enum Type *types);
void BreedFusion_SetBattlerTypes(struct BoxPokemon *boxMon, u32 species, enum Type *types);
void BreedFusion_BufferName(u8 *dest, u32 species, u32 partner);
void BreedFusion_SetHatchedName(struct Pokemon *mon);
u8 *BreedFusion_BufferSummaryLine(struct BoxPokemon *boxMon, u8 *dest);
const u16 *BreedFusion_ApplyPalette(const u16 *palette, u32 species, bool32 isShiny, u32 personality);
struct ObjectEventGraphicsInfo;
const u16 *BreedFusion_GetFollowerPalette(const u16 *palette, u32 species, bool32 isShiny, const struct ObjectEventGraphicsInfo *info, u16 *tag);
u32 BreedFusion_GetIconPalIndex(u32 species, u32 personality, u32 palIndex);
u32 BreedFusion_FindPartner(u32 personality);
void BreedFusion_OverworldUpdate(void);

#endif // GUARD_BREED_FUSION_H
