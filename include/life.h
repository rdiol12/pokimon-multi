#ifndef GUARD_LIFE_H
#define GUARD_LIFE_H

#include "constants/life.h"

// Fifth batch of features (docs/life.md).

// Called once per overworld frame. Returns TRUE if it took over input.
bool32 Life_OverworldUpdate(void);
// Called after the map header's weather is loaded, before the map's
// ON_TRANSITION script runs.
void Life_OnMapTransition(void);

// Bonding (life_bond.c): opens the bonding screen for party slot
// VAR_0x8004 (a script calls it after ChoosePartyMon).
void LifeBond_Open(void);

// Screenshot showcase settings (life.c), written by tools/life_shots/.
struct LifeShowcase
{
    u8 pending;
    u8 friendship; // the lead's friendship, if not 0
    u8 season;     // SEASON_* + 1 overrides the clock, 0 uses it
    u8 finishJobs; // 1: every Pokémon job is done
    u16 photoSpecies; // photo safari: puts this Pokémon in front of the player
    u8 photoDistance; // tiles away
    u8 photoFacing;   // DIR_*, or 0: looking at the player
    u8 photoCount;    // how many of them, side by side
    u8 eclipseStep;   // Team Eclipse: game clear, and this chapter step + 1
    u8 moneyThousands; // sets the player's money to this many thousand
    u8 ghostNow;       // 1: ask the server for a ghost trainer right away
};
extern struct LifeShowcase gLifeShowcase;

// Lets this batch swap in another species for a wild encounter.
enum Species Life_ModifyWildSpecies(enum Species species, u8 level);

// Seasons (life_seasons.c).
u32 Seasons_Get(void);
const u8 *Seasons_GetName(void);
void Seasons_TintPrimaryPalettes(bool32 skipFaded);
void Seasons_OnMapTransition(void);
void Seasons_Update(void);
enum Species Seasons_ModifyWildSpecies(enum Species species, u8 level);

// Pokémon jobs (life_jobs.c).
void LifeJobs_Describe(void);
void LifeJobs_HasFreeSlot(void);
void LifeJobs_Send(void);
void LifeJobs_Collect(void);
void LifeJobs_ShowList(void);
void LifeJobs_FinishAll(void);

// Underground mining (life_mining.c).
void LifeMining_CheckSpot(void);
void LifeMining_Start(void);
void LifeMining_GiveKit(void);

// Battles against trainers that aren't in gTrainers (life_battle.c).
struct Trainer;
void LifeBattle_Start(const struct Trainer *trainer, const u8 *defeatText);

// Team Eclipse (life_eclipse.c).
void LifeAuction_OpenFromMenu(void);

// Ghost trainers (life_ghosts.c). One Pokémon of a ghost's team as it goes
// over the wire (server/life_ghosts.py MON_SIZE).
struct LifeGhostMon
{
    /*0x00*/ u16 species;
    /*0x02*/ u16 heldItem;
    /*0x04*/ u16 moves[MAX_MON_MOVES];
    /*0x0C*/ u16 ability;
    /*0x0E*/ u8 level;
    /*0x0F*/ u8 nature;
    /*0x10*/ u8 ivs[NUM_STATS];
    /*0x16*/ u8 evs[NUM_STATS]; // HP, Atk, Def, SpAtk, SpDef, Speed (struct TrainerMon's order)
    /*0x1C*/ u8 shiny;
    /*0x1D*/ u8 gender; // TRAINER_MON_MALE, _FEMALE or _RANDOM_GENDER (genderless)
    /*0x1E*/ u8 friendship;
    /*0x1F*/ u8 padding;
    /*0x20*/ u8 nickname[POKEMON_NAME_LENGTH]; // EOS-terminated unless it's 12 letters
}; // size 0x2C

void LifeGhost_OnMapTransition(void);
void LifeGhost_Update(void);
bool32 LifeGhost_IsGhost(struct ObjectEvent *objectEvent);
void LifeGhost_OnBattleEnd(const struct Trainer *trainer, bool32 playerWon);
extern const u8 Life_EventScript_Ghost[];
void LifeEclipse_UpdateObjects(void);
void LifeEclipse_ApplyShowcase(u32 step);
void LifeEclipse_BufferIntro(void);
void LifeEclipse_StartBattle(void);
void LifeEclipse_Defeated(void);
void LifeEclipse_Scientist(void);

// Photo safari (life_photo.c).
bool32 LifePhoto_IsActive(void);
bool32 LifePhoto_IsPhotoMon(struct ObjectEvent *objectEvent);
extern const u8 Life_EventScript_PhotoMon[];
extern const u8 Life_EventScript_Auction[];
bool32 LifePhoto_TrySnap(void);
void LifePhoto_Update(void);
void LifePhoto_OnMapTransition(void);
void LifePhoto_ShowPhoto(void);
void LifePhoto_RangerState(void);
void LifePhoto_Start(void);
void LifePhoto_ShowAlbum(void);

#endif // GUARD_LIFE_H
