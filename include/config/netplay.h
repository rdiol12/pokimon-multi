#ifndef GUARD_CONFIG_NETPLAY_H
#define GUARD_CONFIG_NETPLAY_H

// The server JOIN ONLINE on the title menu connects to. Players still load
// tools/netplay_bridge/netplay_bridge.lua in mGBA, which reads the address
// from the game. Change it to your server's name or IP and rebuild.
#define NETPLAY_SERVER_HOST "127.0.0.1"
#define NETPLAY_SERVER_PORT 7777

#endif // GUARD_CONFIG_NETPLAY_H
