#ifndef GUARD_TRAINER_LOOK_H
#define GUARD_TRAINER_LOOK_H

// The trainer look (docs/trainer_look.md): skin, hair and clothes colours
// and accessories for Brendan and May, picked on the Look screen when a
// new game starts (and later from START > STYLE). Other players online see
// it: it travels with the world LOOK line (src/world_online.c).
//
// A look is a u32 of eight 4-bit fields, 0 meaning "as drawn".
enum
{
    LOOK_SKIN,
    LOOK_HAT,     // Brendan's hat, May's hair
    LOOK_BAND,
    LOOK_CLOTHES,
    LOOK_DARK,    // Brendan's hair, May's shorts
    LOOK_COLOUR_PARTS,
    LOOK_HAIRSTYLE = LOOK_COLOUR_PARTS,
    LOOK_HEADWEAR,
    LOOK_FACE,
    LOOK_FIELDS,
};
#define LOOK_ACCESSORY_PARTS (LOOK_FIELDS - LOOK_COLOUR_PARTS)

#define LOOK_FIELD(look, field) (((look) >> ((field) * 4)) & 0xF)
#define LOOK_COLOUR_MASK        0x000FFFFF
#define LOOK_ACCESSORY_MASK     0xFFF00000

struct ObjectEvent;
struct Sprite;

u32 TrainerLook_Get(void);
void TrainerLook_Set(u32 look);

// Recolours an object event if it is the player or another player online.
void TrainerLook_ApplyToObjectEvent(struct ObjectEvent *objectEvent);
// Called every overworld frame after the sprites animate: keeps the
// accessory layers on the player and other players in step.
void TrainerLook_UpdateOverlays(void);

// Recolours a Brendan or May trainer picture's 16 colours at palette
// offset `offset` (0-511) for look. Other pictures are left alone.
void TrainerLook_RecolorTrainerPic(u16 offset, u16 trainerPic, u32 look);

// The Look screen. Goes to returnCallback when done.
void TrainerLook_OpenScreen(void (*returnCallback)(void), bool32 newGame);

// Implemented in src/world_online.c: another player's look, by netplay slot.
u32 WorldOnline_GetRemoteTrainerLook(u32 slot);

#endif // GUARD_TRAINER_LOOK_H
