// Headless games for the tests: the phone app's emulator driver
// (www/emulator.js) and bridge (www/host.lua in wasmoon) under Node, with
// a plain TCP socket in place of the app's native connection.

import { createRequire } from 'node:module';
import { readFileSync } from 'node:fs';
import { createConnection } from 'node:net';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { deflateSync } from 'node:zlib';

const here = dirname(fileURLToPath(import.meta.url));
const mobile = resolve(here, '..');
const repo = resolve(mobile, '..');
const require = createRequire(import.meta.url);

const { Emulator } = await import(join(mobile, 'www/emulator.js'));
const { runBridge } = await import(join(mobile, 'www/bridge.js'));
const wasmoon = require(join(mobile, 'node_modules/wasmoon/dist/index.js'));

export const MB = parseInt(readFileSync(join(repo, 'pokeemerald.map'), 'utf8')
    .match(/^\s+0x([0-9a-f]+)\s+gNetplayMailbox\s*$/m)[1], 16);

export function loadCoreNode() {
    const dir = join(mobile, 'node_modules/@wasm-gaming/mgba-wasm/dist/mgba/');
    const src = readFileSync(join(dir, 'mgba.js'), 'utf8');
    const factory = new Function('require', '__dirname', src + '\nreturn createMgbaModule;')(require, dir);
    return factory({ locateFile: (p) => join(dir, p) }).then((mod) => { mod._mgbawasm_init(); return mod; });
}

// A TCP connection with the interface www/net.js gives the bridge.
export function tcpNet() {
    let sock = null;
    let inbox = '';
    let closed = false;
    return {
        connect(host, port) {
            inbox = '';
            closed = false;
            sock = createConnection({ host, port });
            sock.setEncoding('latin1');
            sock.on('data', (d) => { inbox += d; });
            sock.on('close', () => { closed = true; });
            sock.on('error', () => { closed = true; });
            return true;
        },
        send(line) { sock?.write(line + '\n', 'latin1'); },
        receive() {
            if (inbox) { const d = inbox; inbox = ''; return d; }
            return closed ? false : null;
        },
        close() { sock?.destroy(); sock = null; },
    };
}

export function png(emu) {
    const mod = emu.mod;
    const w = mod._mgbawasm_video_width(), h = mod._mgbawasm_video_height();
    const px = mod.HEAPU8.subarray(mod._mgbawasm_video_ptr(), mod._mgbawasm_video_ptr() + w * h * 4);
    const raw = Buffer.alloc((w * 3 + 1) * h);
    for (let y = 0; y < h; y++) {
        for (let x = 0; x < w; x++) {
            const s = (y * w + x) * 4, d = y * (w * 3 + 1) + 1 + x * 3;
            raw[d] = px[s]; raw[d + 1] = px[s + 1]; raw[d + 2] = px[s + 2];
        }
    }
    const crcTable = Array.from({ length: 256 }, (_, n) => {
        let c = n;
        for (let k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320 ^ (c >>> 1) : c >>> 1;
        return c >>> 0;
    });
    const crc = (buf) => {
        let c = 0xFFFFFFFF;
        for (const b of buf) c = crcTable[(c ^ b) & 0xFF] ^ (c >>> 8);
        return (c ^ 0xFFFFFFFF) >>> 0;
    };
    const chunk = (type, data) => {
        const len = Buffer.alloc(4); len.writeUInt32BE(data.length);
        const td = Buffer.concat([Buffer.from(type), data]);
        const c = Buffer.alloc(4); c.writeUInt32BE(crc(td));
        return Buffer.concat([len, td, c]);
    };
    const ihdr = Buffer.alloc(13);
    ihdr.writeUInt32BE(w, 0); ihdr.writeUInt32BE(h, 4); ihdr[8] = 8; ihdr[9] = 2;
    return Buffer.concat([Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]), chunk('IHDR', ihdr),
        chunk('IDAT', deflateSync(raw)), chunk('IEND', Buffer.alloc(0))]);
}

const rom = new Uint8Array(readFileSync(join(repo, 'pokeemerald.gba')));
const lua = (name) => readFileSync(join(repo, 'tools/netplay_bridge', name), 'utf8');
const sources = {
    world: lua('netplay_world.lua'), bridge: lua('netplay_bridge.lua'), ext: lua('netplay_ext.lua'),
    host: readFileSync(join(mobile, 'www/host.lua'), 'utf8'),
};
const KEY = { a: 1, start: 8 };

// Player n (TESTn through the netplay test boot), pointed at 127.0.0.1:port.
export async function startPlayer(n, port, log = (m) => console.log(`p${n}: ${m}`)) {
    const emu = new Emulator(await loadCoreNode());
    emu.boot(rom);
    emu.screenshotPng = () => png(emu);
    const bridge = await runBridge(emu, { ...sources, wasmoon, net: tcpNet(), log });
    // The ROM has the real server's address built in; point the bridge here.
    emu.frameHooks.unshift({
        onFrame() {
            if (emu.read32(MB) !== 0x4E4D4B50) return;
            const host = '127.0.0.1';
            for (let i = 0; i < 40; i++) emu.write8(MB + 0x35C + i, i < host.length ? host.charCodeAt(i) : 0);
            emu.write16(MB + 0x384, port);
        },
    });
    const player = { n, emu, bridge, frame: 0, keys: 0 };
    // One frame; the first 3000 boot straight into the overworld as TESTn
    // (the netplay test boot in src/netplay.c).
    player.step = () => {
        const f = ++player.frame;
        if (f < 3000) {
            if (emu.ewram >= 0 && emu.read32(MB) === 0x4E4D4B50) emu.write8(MB + 0x0D1, 0xB0 | n);
            emu.keys = f % 40 < 4 ? (f % 80 < 40 ? KEY.start : KEY.a) : 0;
        } else {
            emu.keys = player.keys;
        }
        emu.runFrame();
        emu.drainAudio(false);
    };
    player.remotes = () => {
        let count = 0;
        for (let s = 0; s < 8; s++) if (emu.read16(MB + 0x20 + s * 20) !== 0) count++;
        return count;
    };
    return player;
}

export const tick = () => new Promise((r) => setImmediate(r));
