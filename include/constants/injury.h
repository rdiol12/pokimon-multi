#ifndef GUARD_CONSTANTS_INJURY_H
#define GUARD_CONSTANTS_INJURY_H

// Body parts a Pokémon can injure or lose (MON_DATA_INJURY_PART).
#define INJURY_PART_NONE   0
#define INJURY_PART_EYE    1
#define INJURY_PART_LIMB   2
#define INJURY_PART_TAIL   3
#define INJURY_PART_WING   4
#define INJURY_PART_HORN   5
#define INJURY_PART_COUNT  6

#define INJURY_PART_BIT(part) (1 << ((part) - 1)) // MON_DATA_LOST_PARTS bit

// MON_DATA_INJURY_FLAGS
#define INJURY_FLAG_SCAR    (1 << 0) // Cosmetic scar, permanent.
#define INJURY_FLAG_NOTIFY  (1 << 1) // Tell the trainer about the injury on the next step.
#define INJURY_FLAG_NEW_SCAR (1 << 2) // Tell the trainer about a new scar on the next step.

// MON_DATA_INJURY_DAMAGE is stored in 2% units.
#define INJURY_DAMAGE_UNIT  2
#define INJURY_DAMAGE_MAX   (100 / INJURY_DAMAGE_UNIT)

// Pokédex body shapes (src/data/injury_shapes.h).
#define BODY_SHAPE_NONE       0
#define BODY_SHAPE_BALL       1
#define BODY_SHAPE_SQUIGGLE   2
#define BODY_SHAPE_FISH       3
#define BODY_SHAPE_ARMS       4
#define BODY_SHAPE_BLOB       5
#define BODY_SHAPE_UPRIGHT    6
#define BODY_SHAPE_LEGS       7
#define BODY_SHAPE_QUADRUPED  8
#define BODY_SHAPE_WINGS      9
#define BODY_SHAPE_TENTACLES  10
#define BODY_SHAPE_HEADS      11
#define BODY_SHAPE_HUMANOID   12
#define BODY_SHAPE_BUG_WINGS  13
#define BODY_SHAPE_ARMOR      14

#endif // GUARD_CONSTANTS_INJURY_H
