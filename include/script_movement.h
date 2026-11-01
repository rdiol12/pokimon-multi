#ifndef GUARD_SCRIPT_MOVEMENT_H
#define GUARD_SCRIPT_MOVEMENT_H

bool8 ScriptMovement_StartObjectMovementScript(u8 localId, u8 mapNum, u8 mapGroup, const u8 *movementScript);
bool8 ScriptMovement_IsObjectMovementFinished(u8 localId, u8 mapNum, u8 mapGroup);
bool32 ScriptMovement_IsAllObjectMovementFinished(void);
bool32 ScriptMovement_IsMovingOtherThan(u8 objEventIdA, u8 objEventIdB);
void ScriptMovement_UnfreezeObjectEvents(void);

#endif // GUARD_SCRIPT_MOVEMENT_H
