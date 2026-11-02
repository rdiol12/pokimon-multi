InteractWithOverworldWildEncounter::
    lock
    @ Online, the first player to battle it claims it (src/shared_map.c).
    callnative SharedMap_ClaimOwn, waitstate=1
    goto_if_eq VAR_RESULT, FALSE, SharedMap_EventScript_Taken
    overworldwildencounterapproach
    applymovement VAR_LAST_TALKED, Common_Movement_ExclamationMark
    facetogether
    getoverworldwildencounterspecies VAR_0x8005
    playmoncry VAR_0x8005, CRY_MODE_DOUBLES
    waitmoncry
    tryoverworldwildencounter
    waitstate
    end

