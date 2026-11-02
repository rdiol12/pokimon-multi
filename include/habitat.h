#ifndef GUARD_HABITAT_H
#define GUARD_HABITAT_H

// Keeps wild Pokémon in their own areas (src/habitat.c).

// A species on this map's own wild tables (any time of day).
bool32 Habitat_IsNative(enum Species species);
// May `species` show up here although it isn't on this map's tables
// (weather and season visitors, migrants)? Never in the starting area; else
// only if it shares a type with a species that lives here.
bool32 Habitat_VisitorFits(enum Species species);
// The routes around Littleroot: their own Pokémon only.
bool32 Habitat_IsStartingArea(void);
// A random Pokémon of `type` that lives here (or, in a town, on a route next
// to it), awake at this hour; preferWater picks one that is also Water type
// when there is one. SPECIES_NONE if none.
enum Species Habitat_PickNativeOfType(enum Type type, bool32 night, bool32 preferWater);

#endif // GUARD_HABITAT_H
