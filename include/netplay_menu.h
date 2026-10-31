#ifndef GUARD_NETPLAY_MENU_H
#define GUARD_NETPLAY_MENU_H

// START > ONLINE. Each entry's callback runs with the player's controls
// locked and must unlock them (ScriptUnfreezeObjectEvents +
// UnlockPlayerFieldControls) when its screen closes.
struct NetplayMenuEntry
{
    const u8 *text;
    void (*callback)(void);
};

extern const struct NetplayMenuEntry gNetplayOnlineMenu[];

void ShowNetplayOnlineMenu(void);

#endif // GUARD_NETPLAY_MENU_H
