#include "global.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "mechanics.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "constants/event_object_movement.h"
#include "data.h"
#include "follower_helper.h"

// Pokémon personalities: every Pokémon has one of eight, from its
// personality value (so it never changes and needs no save data). It sets
// what the Pokémon following you says and does: which moods it shows when
// you talk to it, lines of its own, and what it gets up to while you stand
// still.

extern const u8 EventScript_FollowerGeneric[];
extern const u8 EventScript_FollowerHopping[];
extern const u8 EventScript_FollowerLookAway[];
extern const u8 EventScript_FollowerFaceUp[];
extern const u8 EventScript_FollowerLookAround[];
extern const u8 EventScript_FollowerFastHopping[];
extern const u8 EventScript_FollowerCuddling[];
extern const u8 EventScript_FollowerLookAwayBark[];

struct Persona
{
    const u8 *name;
    u8 emotion;          // FOLLOWER_EMOTION_* shown with its own lines
    u8 moreOf[2];        // moods it shows more often
    const u8 *script;    // how it acts out its own lines
    const u8 *lines[3];  // {STR_VAR_1} is its nickname
};

static const struct Persona sPersonas[PERSONA_COUNT] =
{
    [PERSONA_PLAYFUL] =
    {
        .name = COMPOUND_STRING("PLAYFUL"),
        .emotion = FOLLOWER_EMOTION_MUSIC,
        .moreOf = {FOLLOWER_EMOTION_HAPPY, FOLLOWER_EMOTION_MUSIC},
        .script = EventScript_FollowerHopping,
        .lines = {
            COMPOUND_STRING("{STR_VAR_1} is bouncing around you.\nIt wants to play!"),
            COMPOUND_STRING("{STR_VAR_1} spins in a circle and\nlooks very pleased with itself."),
            COMPOUND_STRING("{STR_VAR_1} tagged you!\nIt's your turn to be it!"),
        },
    },
    [PERSONA_SHY] =
    {
        .name = COMPOUND_STRING("SHY"),
        .emotion = FOLLOWER_EMOTION_SURPRISE,
        .moreOf = {FOLLOWER_EMOTION_PENSIVE, FOLLOWER_EMOTION_SURPRISE},
        .script = EventScript_FollowerLookAway,
        .lines = {
            COMPOUND_STRING("{STR_VAR_1} looked away when you\nturned around…"),
            COMPOUND_STRING("{STR_VAR_1} is hiding behind you\nfrom everyone else."),
            COMPOUND_STRING("{STR_VAR_1} peeks at you, then\nquickly looks at its feet."),
        },
    },
    [PERSONA_PROUD] =
    {
        .name = COMPOUND_STRING("PROUD"),
        .emotion = FOLLOWER_EMOTION_HAPPY,
        .moreOf = {FOLLOWER_EMOTION_HAPPY, FOLLOWER_EMOTION_ANGRY},
        .script = EventScript_FollowerFaceUp,
        .lines = {
            COMPOUND_STRING("{STR_VAR_1} puffs out its chest.\nIt knows it looks good."),
            COMPOUND_STRING("{STR_VAR_1} holds its head high for\neveryone to see."),
            COMPOUND_STRING("{STR_VAR_1} seems to think it's the\nreal star of this team."),
        },
    },
    [PERSONA_LAZY] =
    {
        .name = COMPOUND_STRING("LAZY"),
        .emotion = FOLLOWER_EMOTION_NEUTRAL,
        .moreOf = {FOLLOWER_EMOTION_NEUTRAL, FOLLOWER_EMOTION_PENSIVE},
        .script = EventScript_FollowerGeneric,
        .lines = {
            COMPOUND_STRING("{STR_VAR_1} lets out a huge yawn."),
            COMPOUND_STRING("{STR_VAR_1} would like a nap.\nA long one."),
            COMPOUND_STRING("{STR_VAR_1} is dragging its feet.\nAre we there yet?"),
        },
    },
    [PERSONA_CURIOUS] =
    {
        .name = COMPOUND_STRING("CURIOUS"),
        .emotion = FOLLOWER_EMOTION_CURIOUS,
        .moreOf = {FOLLOWER_EMOTION_CURIOUS, FOLLOWER_EMOTION_SURPRISE},
        .script = EventScript_FollowerLookAround,
        .lines = {
            COMPOUND_STRING("{STR_VAR_1} is sniffing at\neverything around here."),
            COMPOUND_STRING("{STR_VAR_1} wants to know what's\nover there. And there. And there."),
            COMPOUND_STRING("{STR_VAR_1} found something!\nIt's… a pebble. A very nice pebble."),
        },
    },
    [PERSONA_BRAVE] =
    {
        .name = COMPOUND_STRING("BRAVE"),
        .emotion = FOLLOWER_EMOTION_ANGRY,
        .moreOf = {FOLLOWER_EMOTION_HAPPY, FOLLOWER_EMOTION_ANGRY},
        .script = EventScript_FollowerFastHopping,
        .lines = {
            COMPOUND_STRING("{STR_VAR_1} is fired up!\nIt wants a battle!"),
            COMPOUND_STRING("{STR_VAR_1} stands in front of you,\nready for anything."),
            COMPOUND_STRING("{STR_VAR_1} isn't scared of\nanything. Well, almost anything."),
        },
    },
    [PERSONA_GENTLE] =
    {
        .name = COMPOUND_STRING("GENTLE"),
        .emotion = FOLLOWER_EMOTION_LOVE,
        .moreOf = {FOLLOWER_EMOTION_LOVE, FOLLOWER_EMOTION_HAPPY},
        .script = EventScript_FollowerCuddling,
        .lines = {
            COMPOUND_STRING("{STR_VAR_1} nuzzles up against you."),
            COMPOUND_STRING("{STR_VAR_1} is watching out for\nthe little POKéMON nearby."),
            COMPOUND_STRING("{STR_VAR_1} gives you a soft,\nhappy look."),
        },
    },
    [PERSONA_GRUMPY] =
    {
        .name = COMPOUND_STRING("GRUMPY"),
        .emotion = FOLLOWER_EMOTION_UPSET,
        .moreOf = {FOLLOWER_EMOTION_UPSET, FOLLOWER_EMOTION_ANGRY},
        .script = EventScript_FollowerLookAwayBark,
        .lines = {
            COMPOUND_STRING("{STR_VAR_1} grumbles about\nsomething. Probably you."),
            COMPOUND_STRING("{STR_VAR_1} pretends not to care…\nbut it's still right behind you."),
            COMPOUND_STRING("{STR_VAR_1} huffs. It's not in the\nmood today. Or any day."),
        },
    },
};

u8 Mechanics_GetPersona(u32 personality)
{
    // Mixed so it doesn't follow the nature (personality % 25).
    return ((personality * 0x2545F491u) >> 29) % PERSONA_COUNT;
}

const u8 *Mechanics_GetPersonaName(u8 persona)
{
    return sPersonas[persona].name;
}

// GetFollowerAction: shows the moods this personality has more often.
void Mechanics_AdjustFollowerMoods(struct Pokemon *mon, u8 *weights)
{
    const struct Persona *persona = &sPersonas[Mechanics_GetPersona(GetMonData(mon, MON_DATA_PERSONALITY))];

    weights[persona->moreOf[0]] += 20;
    weights[persona->moreOf[1]] += 10;
}

// GetFollowerAction: 2 times in 5, the Pokémon says one of its own lines
// instead. Returns TRUE if it did.
bool32 Mechanics_TryFollowerPersonaLine(struct ScriptContext *ctx, struct Pokemon *mon, struct ObjectEvent *follower)
{
    const struct Persona *persona = &sPersonas[Mechanics_GetPersona(GetMonData(mon, MON_DATA_PERSONALITY))];

    if (Random() % 5 >= 2 || follower == NULL)
        return FALSE;
    {
    u32 line = Random() % (ARRAY_COUNT(persona->lines) + 1);

    ObjectEventEmote(follower, persona->emotion);
    StringCopy(gStringVar2, persona->name);
    if (line < ARRAY_COUNT(persona->lines))
        ctx->data[0] = (u32)persona->lines[line];
    else
        ctx->data[0] = (u32)COMPOUND_STRING("{STR_VAR_1} has a {STR_VAR_2}\npersonality. You can just tell!");
    ScriptCall(ctx, persona->script);
    }
    return TRUE;
}

// While the player stands still, the follower does something now and then.
static EWRAM_DATA s16 sIdleFrames = 0;
static EWRAM_DATA s16 sLastX = 0;
static EWRAM_DATA s16 sLastY = 0;

static const u8 sFaceActions[] =
{
    MOVEMENT_ACTION_FACE_DOWN, MOVEMENT_ACTION_FACE_UP, MOVEMENT_ACTION_FACE_LEFT, MOVEMENT_ACTION_FACE_RIGHT,
};

static const u8 sJumpActions[] =
{
    [DIR_SOUTH - 1] = MOVEMENT_ACTION_JUMP_IN_PLACE_DOWN,
    [DIR_NORTH - 1] = MOVEMENT_ACTION_JUMP_IN_PLACE_UP,
    [DIR_WEST - 1] = MOVEMENT_ACTION_JUMP_IN_PLACE_LEFT,
    [DIR_EAST - 1] = MOVEMENT_ACTION_JUMP_IN_PLACE_RIGHT,
};

static void DoIdleAction(struct ObjectEvent *follower, u8 persona)
{
    u8 dir = follower->facingDirection;

    if (dir < DIR_SOUTH || dir > DIR_EAST)
        dir = DIR_SOUTH;
    switch (persona)
    {
    case PERSONA_PLAYFUL:
    case PERSONA_BRAVE:
        ObjectEventSetHeldMovement(follower, sJumpActions[dir - 1]);
        if (persona == PERSONA_PLAYFUL)
            ObjectEventEmote(follower, FOLLOWER_EMOTION_MUSIC);
        break;
    case PERSONA_CURIOUS:
        ObjectEventSetHeldMovement(follower, sFaceActions[Random() % ARRAY_COUNT(sFaceActions)]);
        ObjectEventEmote(follower, FOLLOWER_EMOTION_CURIOUS);
        break;
    case PERSONA_SHY:
        // Turns its back on the player.
        ObjectEventSetHeldMovement(follower, sFaceActions[Random() % ARRAY_COUNT(sFaceActions)]);
        break;
    case PERSONA_LAZY:
        ObjectEventEmote(follower, FOLLOWER_EMOTION_NEUTRAL);
        break;
    case PERSONA_GENTLE:
        ObjectEventEmote(follower, FOLLOWER_EMOTION_LOVE);
        break;
    case PERSONA_GRUMPY:
        ObjectEventEmote(follower, FOLLOWER_EMOTION_UPSET);
        break;
    case PERSONA_PROUD:
        ObjectEventSetHeldMovement(follower, MOVEMENT_ACTION_FACE_DOWN);
        ObjectEventEmote(follower, FOLLOWER_EMOTION_HAPPY);
        break;
    }
}

void Mechanics_UpdatePersonaFollower(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *follower = GetFollowerObject();
    struct Pokemon *mon = GetFirstLiveMon();

    if (player->currentCoords.x != sLastX || player->currentCoords.y != sLastY || follower == NULL || mon == NULL
     || follower->invisible || ArePlayerFieldControlsLocked())
    {
        sLastX = player->currentCoords.x;
        sLastY = player->currentCoords.y;
        sIdleFrames = 0;
        return;
    }
    // After 4 seconds standing still, then every 5 seconds.
    if (++sIdleFrames < 240)
        return;
    sIdleFrames = -60;
    if (!ObjectEventClearHeldMovementIfFinished(follower) && follower->heldMovementActive)
        return;
    DoIdleAction(follower, Mechanics_GetPersona(GetMonData(mon, MON_DATA_PERSONALITY)));
}
