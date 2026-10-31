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
