// Starts the netplay bridge (host.lua and tools/netplay_bridge/*.lua) in a
// Lua 5.4 VM (wasmoon) and runs it once per emulated frame, the way mGBA's
// "frame" callback does on desktop.

import { net as defaultNet } from './net.js';

export async function startBridge(emu, { base = './', log = console.log } = {}) {
    if (!globalThis.wasmoon) {
        await new Promise((resolve, reject) => {
            const s = document.createElement('script');
            s.src = base + 'vendor/wasmoon.js';
            s.onload = resolve;
            s.onerror = () => reject(new Error('cannot load Lua'));
            document.head.appendChild(s);
        });
    }
    const text = async (path) => {
        const res = await fetch(base + path);
        if (!res.ok) throw new Error(`cannot load ${path}`);
        return res.text();
    };
    const [world, bridge, ext, host] = await Promise.all([
        text('lua/netplay_world.lua'), text('lua/netplay_bridge.lua'),
        text('lua/netplay_ext.lua'), text('host.lua'),
    ]);
    return runBridge(emu, { world, bridge, ext, host, log, wasmUrl: base + 'vendor/glue.wasm' });
}

// Split out so the tests can run it under Node with the same sources.
export async function runBridge(emu, { world, bridge, ext, host, log, wasmUrl, wasmoon = globalThis.wasmoon, net = defaultNet }) {
    const factory = new wasmoon.LuaFactory(wasmUrl);
    const lua = await factory.createEngine();
    const fs = (await factory.getLuaModule()).module.FS;
    try { fs.mkdir('/bridge'); } catch { /* exists */ }

    const g = lua.global;
    g.set('SOURCES', { world, bridge, ext });
    g.set('js_read8', (a) => emu.read8(a));
    g.set('js_read16', (a) => emu.read16(a));
    g.set('js_read32', (a) => emu.read32(a));
    g.set('js_write8', (a, v) => emu.write8(a, v));
    g.set('js_write16', (a, v) => emu.write16(a, v));
    g.set('js_find_word', (v) => emu.findWord(v).join(','));
    g.set('js_connect', (h, p) => net.connect(h, p));
    g.set('js_send', (line) => net.send(line));
    // undefined, not null, is Lua's nil here (wasmoon trips over null).
    g.set('js_receive', () => net.receive() ?? undefined);
    g.set('js_close', () => net.close());
    g.set('js_log', (msg) => log(String(msg)));
    // Photos: the world features save a PNG and read it back to upload it.
    g.set('js_screenshot', (path) => fs.writeFile(path, emu.screenshotPng()));
    await lua.doString(host);

    const frame = g.get('PokimonFrame');
    const reset = g.get('PokimonReset');
    const status = g.get('PokimonStatus');
    let failed = false;
    const hook = {
        onFrame() {
            if (failed || emu.ewram < 0) return;
            try {
                frame();
            } catch (e) {
                failed = true;
                log('netplay bridge stopped: ' + e);
            }
        },
        onReset() { if (!failed) reset(); },
    };
    emu.frameHooks.push(hook);
    log('netplay bridge loaded; it connects after JOIN ONLINE');
    return {
        status: () => (failed ? 0 : status()),
        close: () => { emu.frameHooks.splice(emu.frameHooks.indexOf(hook), 1); lua.global.close(); },
    };
}
