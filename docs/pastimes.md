# Pastimes: minigames and side activities

Single-player activities around Hoenn. Each one has its own file in
`src/pastime_*.c` and `data/scripts/pastime/`, its saved state in
`struct PastimeSaveData` (`include/pastime_save.h`, at the end of
SaveBlock3), and a log under **R > PASTIMES** when it has one.

## How activities place NPCs

Activities don't edit `map.json` or use event flags. Each lists its NPCs
and objects as `struct PastimeSpot` (`include/pastime_spots.h`): map,
position, graphics, script, and an `isShown(arg)` check. `src/pastime.c`
spawns the spots of the current map while they are in view and their check
passes, with local ids 170 to 189, and talking to one runs its script with
`VAR_0x8004 = arg`. Add a new list to `sSpotLists` in `src/pastime.c`.

Hooks into shared code: `Pastime_Update`, `Pastime_TryInteract`,
`Pastime_OnCatch` and `Pastime_OnTrainerBattleEnd` from `src/world.c`, the
talk script in `src/field_control_avatar.c`, and the PASTIMES row of the R
menu (`data/scripts/world/menu.inc`).

## Screenshots

`tools/pastime_shots/run.py <scenario.lua> <out dir>` boots the ROM headless
and runs a scenario (same driver as `tools/world_shots/`), with helpers that
write a scene straight into `struct PastimeSaveData`.

## Lost and found

![Lost and found](images/pastime-lost-found.png)

Eight people in Hoenn towns lost something on a nearby route. The lost things
lie in plain sight as item balls; pick one up and bring it back to its owner
for a reward. A thing found before meeting its owner says whose it is.

| Owner | Town | Lost | Where | Reward |
|---|---|---|---|---|
| Lucy | Petalburg City | Hair Ribbon | Route 104, flower beds | Soothe Bell |
| Evan | Rustboro City | Notebook | Petalburg Woods | Lucky Egg |
| Sailor Hank | Slateport City | Old Compass | Route 109 beach | Mystic Water |
| Timmy | Mauville City | Toy Car | Route 110 | Magnet |
| Gramps Otto | Fallarbor Town | Reading Glasses | Route 113 | Wise Glasses |
| Wren | Fortree City | Bird Whistle | Route 119 | Sharp Beak |
| Mrs. Diane | Lilycove City | Wedding Ring | Route 121 | Big Nugget |
| Kai | Dewford Town | Shell Charm | Route 104, north | Shell Bell |

**R > PASTIMES > LOST & FOUND** lists the owners met and the things found,
with where each was lost. Screenshots: `tools/pastime_shots/lost_found.lua`.

## Museum donations

![Museum donations](images/pastime-museum.png)

The Lilycove Museum opens a natural history wing. Its curator (by the
stairs on 1F) takes three kinds of donations:

- **Fossils**: any of the 15 fossil items (Root, Claw, Helix, Dome, Old
  Amber, Skull, Armor, Cover, Plume, Jaw, Sail and the four Galar
  fossilized parts). The fossil leaves the bag.
- **Rare Pokémon**: show a legendary, mythical, Ultra Beast, Paradox,
  shiny or pseudo-legendary (Dragonite, Tyranitar, Salamence, Metagross,
  ...). It stays in the party; the museum records up to 12.
- **Photos** taken with R > PHOTO; the photo wall grows at 5, 15 and 30.

The first four fossils and first three rare Pokémon stand in the hall as
life-size models (overworld Pokémon sprites), each with a plaque. Every
fossil, rare Pokémon and photo wall section is an exhibit point; at 1, 3,
5, 8, 12, 16 and 22 points the curator gives a Nugget, Rare Candy, PP Max,
Big Nugget, Comet Shard, Ability Patch and finally a Master Ball.
**R > PASTIMES > MUSEUM** lists the exhibits. Screenshots:
`tools/pastime_shots/museum.lua`.

## Delivery jobs

![Delivery jobs](images/pastime-delivery.png)

HOENN EXPRESS has a PARCEL DESK outside the Pokémon Center in Oldale,
Petalburg, Rustboro, Slateport, Mauville, Fallarbor, Fortree and Lilycove
(plus a VIGOROTH mover at the Slateport head office). A desk offers a
parcel for another of those towns you've already visited. The time limit is
90 seconds plus 20 per step of distance on the region map, counted in play
time; pay is ¥300 plus ¥100 per step.

- Delivered in half the time: **EXPRESS**, +50% pay.
- In time: full pay. Late: half pay (a notice pops up when time runs out).
- Every fifth parcel also earns an item (Great Ball, Hyper Potion, Rare
  Candy, PP Up, Nugget, Ultra Ball, Big Nugget, in turn).

**R > PASTIMES > DELIVERY** shows the parcel, where it goes, the time left
and the parcels delivered. Talking to another desk while carrying one lets
you give the job up. Screenshots: `tools/pastime_shots/delivery.lua`.
