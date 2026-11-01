#include "global.h"
#include "alive.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "data.h"
#include "follower_helper.h"
#include "injury.h"
#include "pokemon.h"
#include "random.h"
#include "rovers.h"
#include "save_ext.h"
#include "script.h"
#include "string_util.h"
#include "trainer_look.h"
#include "world.h"
#include "constants/event_objects.h"
#include "constants/map_types.h"
#include "constants/event_object_movement.h"

// Townsfolk who remember you, and who chat with each other (docs/alive.md).

// --- Remarks ---------------------------------------------------------------

static EWRAM_DATA const u8 *sResumeScript = NULL;
static EWRAM_DATA u8 sRemark[200] = {0};

static struct AliveMemory *FindMemory(struct ObjectEvent *obj, bool32 create)
{
    struct AliveSave *save = &gSaveBlockExt.alive;
    u32 i, oldest = 0;

    for (i = 0; i < ALIVE_MEMORIES; i++)
    {
        struct AliveMemory *m = &save->memories[i];
        if (m->times != 0 && m->mapGroup == gSaveBlock1Ptr->location.mapGroup && m->mapNum == gSaveBlock1Ptr->location.mapNum
         && m->localId == obj->localId)
            return m;
    }
    if (!create)
        return NULL;
    // Forget whoever was met longest ago (by day, then by fewest talks).
    for (i = 1; i < ALIVE_MEMORIES; i++)
    {
        struct AliveMemory *a = &save->memories[i], *b = &save->memories[oldest];
        if (a->times == 0)
        {
            oldest = i;
            break;
        }
        if (b->times != 0 && (u8)(Alive_Day() - a->lastDay) > (u8)(Alive_Day() - b->lastDay))
            oldest = i;
    }
    memset(&save->memories[oldest], 0, sizeof(save->memories[oldest]));
    return &save->memories[oldest];
}

static bool32 EvolvedFrom(u16 species, u16 old)
{
    u32 i;
    for (i = 0; i < 3 && species != SPECIES_NONE; i++)
    {
        species = GetSpeciesPreEvolution(species);
        if (species == old)
            return TRUE;
    }
    return FALSE;
}

static void Say(const u8 *fmt)
{
    StringExpandPlaceholders(sRemark, fmt);
}

static bool32 LeadHasScars(void)
{
    struct Pokemon *mon = GetFirstLiveMon();
    return mon != NULL && Injury_GetLooks(&mon->box) != 0;
}

static bool32 LeadIsShiny(void)
{
    struct Pokemon *mon = GetFirstLiveMon();
    return mon != NULL && GetMonData(mon, MON_DATA_IS_SHINY);
}

static bool32 HasDonated(void)
{
    u32 i;
    for (i = 0; i < ALIVE_PROJECT_COUNT; i++)
    {
        if (gSaveBlockExt.alive.donated[i] != 0)
            return TRUE;
    }
    return FALSE;
}

// What a townsperson says, remembering last time if there was one.
static void MakeRemark(struct AliveMemory *m, bool32 met)
{
    u16 lead = Alive_LeadSpecies();
    u32 badges = Alive_Badges();

    StringCopy(gStringVar1, GetSpeciesName(lead));
    ConvertIntToDecimalStringN(gStringVar2, badges, STR_CONV_MODE_LEFT_ALIGN, 1);
    if (met)
    {
        StringCopy(gStringVar3, GetSpeciesName(m->lead));
        if (badges > m->badges)
        {
            ConvertIntToDecimalStringN(gStringVar3, m->badges, STR_CONV_MODE_LEFT_ALIGN, 1);
            Say(COMPOUND_STRING("Oh, {PLAYER}! {STR_VAR_2} BADGES now?\nLast time you only had {STR_VAR_3}!"));
        }
        else if (lead != m->lead && EvolvedFrom(lead, m->lead))
            Say(COMPOUND_STRING("{PLAYER}! Your {STR_VAR_3} evolved\ninto {STR_VAR_1}! It's grown so much!"));
        else if (lead != m->lead && m->lead != SPECIES_NONE)
            Say(COMPOUND_STRING("You're with {STR_VAR_1} today?\nLast time it was {STR_VAR_3}."));
        else if (m->times >= 4)
            Say(COMPOUND_STRING("{PLAYER}, you come by so often,\nyou're practically a local now!"));
        else
            Say(COMPOUND_STRING("Back again, {PLAYER}?\nYour {STR_VAR_1} looks well."));
        return;
    }
    if (FlagGet(FLAG_SYS_GAME_CLEAR) && Random() % 2)
        Say(COMPOUND_STRING("Wait… you're {PLAYER}, the CHAMPION!\nCan I shake your hand?"));
    else if (LeadIsShiny())
        Say(COMPOUND_STRING("Whoa, your {STR_VAR_1} is an unusual\ncolor! I've never seen one like it."));
    else if (LeadHasScars())
        Say(COMPOUND_STRING("Your {STR_VAR_1} has scars… It's been\nthrough some tough battles, hasn't it?"));
    else if (gSaveBlockExt.alive.helped >= 3 && Random() % 2)
        Say(COMPOUND_STRING("Aren't you the TRAINER who keeps\nhelping people on the routes?"));
    else if (HasDonated() && Random() % 2)
        Say(COMPOUND_STRING("You gave to the town project,\ndidn't you? Thank you, {PLAYER}!"));
    else if ((TrainerLook_Get() & LOOK_ACCESSORY_MASK) != 0 && Random() % 2)
        Say(COMPOUND_STRING("I like your outfit!\nDid you pick it yourself?"));
    else if (badges >= 6)
        Say(COMPOUND_STRING("{STR_VAR_2} BADGES! You'll be at the\nPOKéMON LEAGUE before long."));
    else if (badges >= 1)
        Say(COMPOUND_STRING("Oh, a TRAINER with {STR_VAR_2} BADGE(S)!\nAnd that must be your {STR_VAR_1}."));
    else
        Say(COMPOUND_STRING("A new TRAINER? And a {STR_VAR_1}!\nGood luck on your journey."));
}

static bool32 WantsToRemark(struct ObjectEvent *obj)
{
    if (!Alive_IsTownsperson(obj) || Alive_IsQuiet() || Alive_LeadSpecies() == SPECIES_NONE)
        return FALSE;
    // Townsfolk at home or in town, not on routes or in caves.
    return Alive_IsTown() || gMapHeader.mapType == MAP_TYPE_INDOOR;
}

const u8 *AliveTalk_GetRemarkScript(struct ObjectEvent *obj, const u8 *script)
{
    extern const u8 Alive_EventScript_Remark[];
    struct AliveMemory *m;
    bool32 met;
    u8 today = Alive_Day();

    if (script == NULL || !WantsToRemark(obj))
        return script;
    // Shop clerks and nurses get straight down to business.
    if (obj->graphicsId == OBJ_EVENT_GFX_MART_EMPLOYEE || obj->graphicsId == OBJ_EVENT_GFX_NURSE)
        return script;
    m = FindMemory(obj, FALSE);
    met = (m != NULL);
    // Once a day each, unless a screenshot scene wants one now.
    if (met && m->lastDay == today && gAliveShowcase.scene != ALIVE_SHOW_REMARK)
        return script;
    if (!met && gAliveShowcase.scene != ALIVE_SHOW_REMARK && Random() % 2)
        return script; // not everyone has something to say
    if (gAliveShowcase.scene == ALIVE_SHOW_REMARK)
        gAliveShowcase.scene = ALIVE_SHOW_NONE;
    if (m == NULL)
        m = FindMemory(obj, TRUE);
    MakeRemark(m, met && m->times != 0);
    m->mapGroup = gSaveBlock1Ptr->location.mapGroup;
    m->mapNum = gSaveBlock1Ptr->location.mapNum;
    m->localId = obj->localId;
    if (m->times < 255)
        m->times++;
    m->lead = Alive_LeadSpecies();
    m->badges = Alive_Badges();
    m->lastDay = today;
    sResumeScript = script;
    return Alive_EventScript_Remark;
}

void AliveTalk_BufferRemark(void)
{
    StringCopy(gStringVar4, sRemark);
}

// Script: on to what the townsperson always says.
void Alive_ResumeTalk(struct ScriptContext *ctx)
{
    if (sResumeScript != NULL)
        ScriptJump(ctx, sResumeScript);
    sResumeScript = NULL;
}

// --- Chatter ---------------------------------------------------------------

struct Chat
{
    const u8 *first;
    const u8 *reply;
};

static const struct Chat sChats[] =
{
    {COMPOUND_STRING("Did you hear? {STR_VAR_1} came\nthrough here with a strong team."),
     COMPOUND_STRING("{STR_VAR_1}? They battled half\nthe route yesterday!")},
    {COMPOUND_STRING("That TRAINER {PLAYER} has\n{STR_VAR_2} BADGE(S) now, you know."),
     COMPOUND_STRING("I saw them with a {STR_VAR_3}.\nThey'll go far!")},
    {COMPOUND_STRING("My ZIGZAGOON dug up a NUGGET\nonce. I've been rich ever since!"),
     COMPOUND_STRING("Ha! It was a rock painted gold.")},
    {COMPOUND_STRING("They say there's a hidden grove\non STARFALL ISLE."),
     COMPOUND_STRING("Only after the LEAGUE, though.\nThe ferry won't go before.")},
    {COMPOUND_STRING("Bring a Fire-type into dark caves.\nIt lights the way!"),
     COMPOUND_STRING("A Water-type is handy on ice,\ntoo. Just saying.")},
    {COMPOUND_STRING("The town project is coming along!\nHave you seen the foreman?"),
     COMPOUND_STRING("Everyone's chipping in.\nI gave my lunch money.")},
    {COMPOUND_STRING("I heard a herd of wild POKéMON\nstampeded across a route."),
     COMPOUND_STRING("Wild ones travel in packs.\nKeep your eyes open!")},
    {COMPOUND_STRING("The wild POKéMON fall asleep in\nthe grass at night, you know."),
     COMPOUND_STRING("Sneak up on one and it won't\neven wake up for the battle!")},
    {COMPOUND_STRING("The MART closes at ten.\nI always forget."),
     COMPOUND_STRING("Then buy your POTIONS early!")},
    {COMPOUND_STRING("Did you read the news this\nmorning? Ask the town crier."),
     COMPOUND_STRING("I only read the sports page.\nLadder battles!")},
    {COMPOUND_STRING("When it rains, the low paths\nflood. Wear boots!"),
     COMPOUND_STRING("And in a dry spell the ponds\ndry up. People find things!")},
    {COMPOUND_STRING("I found a crate washed up on\nthe beach once."),
     COMPOUND_STRING("Lucky! All I find is seaweed.")},
    {COMPOUND_STRING("A child got lost on the route\nyesterday."),
     COMPOUND_STRING("A kind TRAINER walked them\nhome. Wasn't that nice?")},
    {COMPOUND_STRING("Have you been to the HAVEN\nRESERVE? They care for POKéMON."),
     COMPOUND_STRING("I left my old SURSKIT there.\nIt's so happy now.")},
    {COMPOUND_STRING("The ROVER LEAGUE is on this\nweek in LAUREL CITY!"),
     COMPOUND_STRING("My money's on {STR_VAR_1}.")},
    {COMPOUND_STRING("Lovely weather today, isn't it?"),
     COMPOUND_STRING("Perfect for a POKéMON battle!")},
    {COMPOUND_STRING("My TAILLOW won't stop pecking\nat the berry trees."),
     COMPOUND_STRING("Plant more, then! Share the\nharvest.")},
    {COMPOUND_STRING("I hear the GYM LEADER has been\ntraining every night."),
     COMPOUND_STRING("They must have heard about\n{PLAYER}!")},
};

static const struct Chat sNightChats[] =
{
    {COMPOUND_STRING("Shh… everyone's asleep."),
     COMPOUND_STRING("Except the HOOTHOOT. Listen!")},
    {COMPOUND_STRING("Can't sleep either?"),
     COMPOUND_STRING("Too many stars out tonight.")},
    {COMPOUND_STRING("The town is so quiet at night."),
     COMPOUND_STRING("Just the way I like it.")},
};

static const struct Chat sMorningChats[] =
{
    {COMPOUND_STRING("Morning! Off to work already?"),
     COMPOUND_STRING("Same as always. See you tonight!")},
    {COMPOUND_STRING("Up early to catch POKéMON?"),
     COMPOUND_STRING("The early TAILLOW gets the\nWURMPLE, they say.")},
};

enum { CHAT_IDLE, CHAT_APPROACH, CHAT_FIRST, CHAT_REPLY };

static EWRAM_DATA struct
{
    u16 timer;
    u8 state;
    u8 steps;
    u8 a, b;    // local ids
    u8 typeA, typeB; // their movement types before they stopped to chat
    const struct Chat *chat;
} sChat = {0};

void AliveTalk_OnMapTransition(void)
{
    memset(&sChat, 0, sizeof(sChat));
    sChat.timer = 240 + Random() % 300;
}

static bool32 Ready(struct ObjectEvent *obj)
{
    return Alive_IsTownsperson(obj) && !Alive_Busy(obj) && Alive_PlayerDistance(obj->currentCoords.x, obj->currentCoords.y) <= 9;
}

static bool32 FindPair(u8 *a, u8 *b)
{
    u32 i, j;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *p = &gObjectEvents[i];
        if (!Ready(p))
            continue;
        for (j = i + 1; j < OBJECT_EVENTS_COUNT; j++)
        {
            struct ObjectEvent *q = &gObjectEvents[j];
            u32 d;
            if (!Ready(q))
                continue;
            d = abs(p->currentCoords.x - q->currentCoords.x) + abs(p->currentCoords.y - q->currentCoords.y);
            if (d >= 1 && d <= 8)
            {
                *a = p->localId;
                *b = q->localId;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void FaceEachOther(struct ObjectEvent *p, struct ObjectEvent *q)
{
    s16 dx = q->currentCoords.x - p->currentCoords.x, dy = q->currentCoords.y - p->currentCoords.y;
    u32 dir = abs(dx) >= abs(dy) ? (dx > 0 ? DIR_EAST : DIR_WEST) : (dy > 0 ? DIR_SOUTH : DIR_NORTH);
    static const u8 sOpp[] = {[DIR_SOUTH] = DIR_NORTH, [DIR_NORTH] = DIR_SOUTH, [DIR_WEST] = DIR_EAST, [DIR_EAST] = DIR_WEST};

    // Wanderers stop for a chat.
    SetTrainerMovementType(p, GetTrainerFacingDirectionMovementType(dir));
    SetTrainerMovementType(q, GetTrainerFacingDirectionMovementType(sOpp[dir]));
    ObjectEventTurn(p, dir);
    ObjectEventTurn(q, sOpp[dir]);
}

static void BufferChatVars(void)
{
    // A rover's name, the player's badges and lead.
    StringCopy(gStringVar1, gRoverCast[(Alive_Day() * 13 + gSaveBlock1Ptr->location.mapNum) % ROVER_COUNT].name);
    ConvertIntToDecimalStringN(gStringVar2, Alive_Badges(), STR_CONV_MODE_LEFT_ALIGN, 1);
    StringCopy(gStringVar3, GetSpeciesName(Alive_LeadSpecies()));
}

static void Line(u8 localId, const u8 *fmt)
{
    struct ObjectEvent *obj = Alive_Obj(localId);
    if (obj == NULL)
        return;
    BufferChatVars();
    StringExpandPlaceholders(gStringVar4, fmt);
    Alive_ShowCaption(Alive_SpeakerName(obj), gStringVar4);
}

void AliveTalk_UpdateChatter(void)
{
    struct ObjectEvent *p, *q;

    if (Alive_IsQuiet() || !(Alive_IsTown() || gMapHeader.mapType == MAP_TYPE_INDOOR || Alive_IsRoute()))
        return;
    if (gAliveShowcase.scene == ALIVE_SHOW_CHATTER && sChat.state == CHAT_IDLE)
        sChat.timer = 0;
    if (sChat.timer != 0)
    {
        sChat.timer--;
        return;
    }
    switch (sChat.state)
    {
    case CHAT_IDLE:
        if (Alive_IsCaptionShown() || !FindPair(&sChat.a, &sChat.b))
        {
            sChat.timer = 120;
            return;
        }
        if (gAliveShowcase.scene == ALIVE_SHOW_CHATTER)
            gAliveShowcase.scene = ALIVE_SHOW_NONE;
        if (Alive_IsNight())
            sChat.chat = &sNightChats[Random() % ARRAY_COUNT(sNightChats)];
        else if (Alive_Hour() < 9)
            sChat.chat = &sMorningChats[Random() % ARRAY_COUNT(sMorningChats)];
        else
            sChat.chat = &sChats[Random() % ARRAY_COUNT(sChats)];
        p = Alive_Obj(sChat.a);
        q = Alive_Obj(sChat.b);
        sChat.typeA = p->movementType;
        sChat.typeB = q->movementType;
        // A stops; B walks over to them.
        SetTrainerMovementType(p, GetTrainerFacingDirectionMovementType(p->facingDirection));
        SetTrainerMovementType(q, GetTrainerFacingDirectionMovementType(q->facingDirection));
        Alive_Emote(q, FOLLOWER_EMOTION_SURPRISE);
        sChat.state = CHAT_APPROACH;
        sChat.steps = 0;
        break;
    case CHAT_APPROACH:
        p = Alive_Obj(sChat.a);
        q = Alive_Obj(sChat.b);
        if (p == NULL || q == NULL)
        {
            sChat.state = CHAT_REPLY; // to the end
            break;
        }
        if (Alive_Busy(q))
            break;
        if (abs(p->currentCoords.x - q->currentCoords.x) + abs(p->currentCoords.y - q->currentCoords.y) > 1)
        {
            if (++sChat.steps > 10 || !Alive_StepToward(q, p->currentCoords.x, p->currentCoords.y, FALSE))
            {
                if (abs(p->currentCoords.x - q->currentCoords.x) + abs(p->currentCoords.y - q->currentCoords.y) > 3)
                {
                    sChat.state = CHAT_REPLY; // can't get there: never mind
                    break;
                }
            }
            else
            {
                break;
            }
        }
        FaceEachOther(p, q);
        Alive_Emote(p, FOLLOWER_EMOTION_MUSIC);
        Line(sChat.a, sChat.chat->first);
        sChat.state = CHAT_FIRST;
        sChat.timer = 150;
        break;
    case CHAT_FIRST:
        q = Alive_Obj(sChat.b);
        Alive_Emote(q, Random() & 1 ? FOLLOWER_EMOTION_HAPPY : FOLLOWER_EMOTION_SURPRISE);
        Line(sChat.b, sChat.chat->reply);
        sChat.state = CHAT_REPLY;
        sChat.timer = 150;
        break;
    default:
        // Back to what they were doing.
        p = Alive_Obj(sChat.a);
        q = Alive_Obj(sChat.b);
        if (p != NULL && !p->invisible && p->movementType != MOVEMENT_TYPE_NONE)
            SetTrainerMovementType(p, sChat.typeA);
        if (q != NULL && !q->invisible && q->movementType != MOVEMENT_TYPE_NONE)
            SetTrainerMovementType(q, sChat.typeB);
        sChat.state = CHAT_IDLE;
        sChat.timer = 600 + Random() % 900;
        break;
    }
}
