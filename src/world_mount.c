#include "global.h"
#include "bike.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "world.h"
#include "constants/event_objects.h"
#include "constants/songs.h"

// Mounts: ride a big Pokémon from your party instead of the bike. Riding is
// the Mach Bike underneath (same speed, same rules about where you can ride),
// but the player is drawn on foot, sitting on the Pokémon's overworld sprite,
// and the cycling music doesn't play. Other players online see the mount too
// (netplay_world.c sends its graphics id).

#define MOUNT_Y_OFFSET 6  // the mount sits this many pixels below the rider's center
#define RIDER_LIFT     7  // and the rider is drawn this much higher

// Pokémon big and strong enough to carry a trainer in the official games
// (ridden in Sun/Moon, Legends: Arceus or Scarlet/Violet, or shown carrying
// people in the anime and Pokédex).
static const u16 sRideableSpecies[] =
{
    SPECIES_RAPIDASH,
    SPECIES_PONYTA,
    SPECIES_MUDSDALE,
    SPECIES_GOGOAT,
    SPECIES_ARCANINE,
    SPECIES_STOUTLAND,
    SPECIES_TAUROS,
    SPECIES_RHYHORN,
    SPECIES_RHYDON,
    SPECIES_DODRIO,
    SPECIES_LUXRAY,
    SPECIES_MANECTRIC,
    SPECIES_MIGHTYENA,
    SPECIES_ZEBSTRIKA,
    SPECIES_SAWSBUCK,
    SPECIES_CAMERUPT,
    SPECIES_ABSOL,
    SPECIES_ENTEI,
    SPECIES_RAIKOU,
    SPECIES_SUICUNE,
    SPECIES_URSALUNA,
    SPECIES_WYRDEER,
    SPECIES_BASCULEGION,
};

static EWRAM_DATA u8 sMountSpriteId = 0;
static EWRAM_DATA u16 sMountGfx = 0;

#define sObjectId data[0]
#define sLastX    data[1]
#define sLastY    data[2]
#define sMoving   data[3]

bool32 Mount_IsRideable(u16 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sRideableSpecies); i++)
    {
        if (sRideableSpecies[i] == species)
            return TRUE;
    }
    return FALSE;
}

// The first rideable, non-fainted Pokémon in the party, or PARTY_SIZE.
static u32 FindMountInParty(void)
{
    u32 i;

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (!GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_HP) != 0
         && Mount_IsRideable(GetMonData(mon, MON_DATA_SPECIES)))
            return i;
    }
    return PARTY_SIZE;
}

static u16 MountGfxForMon(struct Pokemon *mon)
{
    u16 gfx = GetMonData(mon, MON_DATA_SPECIES) + OBJ_EVENT_MON;

    if (GetMonData(mon, MON_DATA_IS_SHINY))
        gfx += OBJ_EVENT_MON_SHINY;
    if (GetMonGender(mon) == MON_FEMALE)
        gfx += OBJ_EVENT_MON_FEMALE;
    return gfx;
}

bool32 Mount_IsRiding(void)
{
    return FlagGet(FLAG_WORLD_RIDING);
}

u16 Mount_GetGfx(void)
{
    return Mount_IsRiding() ? sMountGfx : 0;
}

static void SpriteCB_Mount(struct Sprite *sprite)
{
    struct ObjectEvent *obj = &gObjectEvents[sprite->sObjectId];
    struct Sprite *rider;
    bool32 moving;

    if (!obj->active)
    {
        sprite->invisible = TRUE;
        return;
    }
    rider = &gSprites[obj->spriteId];
    moving = rider->x != sprite->sLastX || rider->y != sprite->sLastY;
    sprite->sLastX = rider->x;
    sprite->sLastY = rider->y;

    sprite->x = rider->x;
    sprite->y = rider->y + MOUNT_Y_OFFSET;
    sprite->x2 = rider->x2;
    sprite->coordOffsetEnabled = rider->coordOffsetEnabled;
    rider->y2 = -RIDER_LIFT;
    sprite->invisible = rider->invisible;
    // Same priority split as the rider (objects draw their lower half under
    // tall grass and bridges), one step further back.
    sprite->oam.priority = rider->oam.priority;
    sprite->subspriteMode = rider->subspriteMode;
    sprite->subspriteTableNum = rider->subspriteTableNum;
    sprite->subpriority = rider->subpriority + 1;

    if (moving)
        StartSpriteAnimIfDifferent(sprite, GetMoveDirectionFastAnimNum(obj->facingDirection));
    else
        StartSpriteAnimIfDifferent(sprite, GetFaceDirectionAnimNum(obj->facingDirection));
}

static bool32 IsMountSprite(u8 spriteId)
{
    return spriteId < MAX_SPRITES && gSprites[spriteId].inUse && gSprites[spriteId].callback == SpriteCB_Mount;
}

// Draws `gfx` under the object event `objectId` until Mount_Detach. Used for
// the player and for other players online.
u8 Mount_Attach(u8 objectId, u16 gfx)
{
    struct Sprite *rider = &gSprites[gObjectEvents[objectId].spriteId];
    u8 spriteId = CreateObjectGraphicsSprite(gfx, SpriteCB_Mount, rider->x, rider->y + MOUNT_Y_OFFSET, rider->subpriority + 1);

    if (spriteId == MAX_SPRITES)
        return MAX_SPRITES;
    gSprites[spriteId].sObjectId = objectId;
    gSprites[spriteId].oam.priority = rider->oam.priority;
    SpriteCB_Mount(&gSprites[spriteId]);
    return spriteId;
}

void Mount_Detach(u8 spriteId)
{
    if (!IsMountSprite(spriteId))
        return;
    gSprites[gObjectEvents[gSprites[spriteId].sObjectId].spriteId].y2 = 0;
    DestroySprite(&gSprites[spriteId]);
}

static void Dismount(void)
{
    FlagClear(FLAG_WORLD_RIDING);
    Mount_Detach(sMountSpriteId);
    sMountSpriteId = MAX_SPRITES;
    UpdateFollowingPokemon();
}

// Called every overworld frame: keeps the mount sprite alive across map
// changes, and ends the ride when something took the player off the "bike"
// (a building, surfing, a cutscene).
void Mount_Update(void)
{
    if (!Mount_IsRiding())
        return;
    if (!TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE))
    {
        Dismount();
        return;
    }
    if (!IsMountSprite(sMountSpriteId))
    {
        if (sMountGfx == 0)
        {
            u32 slot = FindMountInParty();
            if (slot == PARTY_SIZE)
            {
                Dismount();
                SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_ON_FOOT);
                return;
            }
            sMountGfx = MountGfxForMon(&gPlayerParty[slot]);
        }
        sMountSpriteId = Mount_Attach(gPlayerAvatar.objectEventId, sMountGfx);
    }
}

// Script: VAR_RESULT = 0 rode off, 1 got off, 2 nothing to ride,
// 3 can't ride here. The mount's name is put in STR_VAR_1.
void Mount_Toggle(void)
{
    u32 slot;

    if (Mount_IsRiding())
    {
        Dismount();
        SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_ON_FOOT);
        gSpecialVar_Result = 1;
        return;
    }
    slot = FindMountInParty();
    if (slot == PARTY_SIZE)
    {
        gSpecialVar_Result = 2;
        return;
    }
    GetMonData(&gPlayerParty[slot], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    if (!gMapHeader.allowCycling || TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER)
     || IsBikingDisallowedByPlayer())
    {
        gSpecialVar_Result = 3;
        return;
    }
    sMountGfx = MountGfxForMon(&gPlayerParty[slot]);
    FlagSet(FLAG_WORLD_RIDING);
    SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_MACH_BIKE);
    sMountSpriteId = Mount_Attach(gPlayerAvatar.objectEventId, sMountGfx);
    UpdateFollowingPokemon();
    PlayCry_Normal(GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES), 0);
    gSpecialVar_Result = 0;
}
