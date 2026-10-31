#include "global.h"
#include "data.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/trainers.h"

// Trainers of the new areas (docs/areas.md), generated from
// src/data/area_trainers.party.
const struct Trainer gAreaTrainers[DIFFICULTY_COUNT][AREA_TRAINERS_COUNT] =
{
#include "data/area_trainers.h"
};
