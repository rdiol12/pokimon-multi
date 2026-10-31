// Compiled by tools/areas/shots/run.py to learn struct offsets for the Lua driver.
#include "global.h"
const int kSaveBlock1Flags = offsetof(struct SaveBlock1, flags);
const int kSaveBlock3Areas = offsetof(struct SaveBlock3, areas);
const int kSaveBlock3Size = sizeof(struct SaveBlock3);
const int kObjectEventSize = sizeof(struct ObjectEvent);
const int kObjectEventCoords = offsetof(struct ObjectEvent, currentCoords);
const int kPlayerAvatarObjectId = offsetof(struct PlayerAvatar, objectEventId);
