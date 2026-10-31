// End-to-end test of the phone app's game code without a phone: two
// headless games on the app's emulator driver and bridge (test/headless.mjs)
// connect to a real server over TCP and must see each other in Oldale Town.
//
//   node mobile/test/e2e_two_players.mjs [--shots DIR]
//
// Needs a built pokeemerald.gba and `npm install` in mobile/.

import { spawn } from 'node:child_process';
import { mkdirSync, writeFileSync } from 'node:fs';
import { createServer } from 'node:net';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { png, startPlayer, tick } from './headless.mjs';

const repo = resolve(dirname(fileURLToPath(import.meta.url)), '..', '..');
const shotsArg = process.argv.indexOf('--shots');
const shots = shotsArg > 0 ? process.argv[shotsArg + 1] : null;
if (shots) mkdirSync(shots, { recursive: true });

const port = await new Promise((res) => {
    const s = createServer().listen(0, '127.0.0.1', () => { const p = s.address().port; s.close(() => res(p)); });
});
const server = spawn('python3', [join(repo, 'server/netplay_server.py'), '--host', '127.0.0.1',
    '--port', String(port), '--db', ':memory:'], { stdio: 'ignore' });
await new Promise((r) => setTimeout(r, 700));

const players = [await startPlayer(1, port), await startPlayer(2, port)];
const step = async () => {
    for (const p of players) p.step();
    await tick(); // let the sockets deliver
};

const t0 = Date.now();
let ok = false;
let frames = 0;
for (; frames < 9000; frames++) {
    await step();
    if (frames % 60 === 0 && players.every((p) => p.bridge.status() === 2 && p.remotes() > 0)) {
        for (let i = 0; i < 90; i++) await step(); // let the sprites settle
        ok = true;
        break;
    }
}
console.log(`frames ${frames}, ${((Date.now() - t0) / frames).toFixed(2)} ms per frame for two games`);
for (const p of players) {
    console.log(`p${p.n}: status ${p.bridge.status()}, ewram at heap 0x${p.emu.ewram.toString(16)}, sees ${p.remotes()} player(s)`);
    if (shots) writeFileSync(join(shots, `p${p.n}.png`), png(p.emu));
}
server.kill();
console.log(ok ? 'PASS' : 'FAIL');
process.exit(ok ? 0 : 1);
