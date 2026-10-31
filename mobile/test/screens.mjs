// Screenshots of the phone app in a phone-sized Chromium, with a second
// (headless) player online next to it. Runs the real netplay server on
// 127.0.0.1:7777 (the address a default build has) and mobile/dev/serve.py.
//
//   node mobile/test/screens.mjs BASE_ROM PATCH OUT_DIR
//
// BASE_ROM and PATCH stand in for the player's Emerald and the release's
// .bps (any pair made with tools/make_bps.py that gives pokeemerald.gba).
// Run `node scripts/assemble.mjs PATCH` first so dist/ has the patch.

import { spawn } from 'node:child_process';
import { createRequire } from 'node:module';
import { mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { MB, startPlayer, tick } from './headless.mjs';

const require = createRequire(import.meta.url);
const { chromium, devices } = require('playwright');
const mobile = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const repo = resolve(mobile, '..');
const [baseRom, , out] = process.argv.slice(2);
mkdirSync(out, { recursive: true });

const procs = [
    spawn('python3', [join(repo, 'server/netplay_server.py'), '--host', '127.0.0.1', '--port', '7777', '--db', ':memory:'], { stdio: 'ignore' }),
    spawn('python3', [join(mobile, 'dev/serve.py'), '--port', '8090'], { stdio: 'ignore' }),
];
const stop = () => procs.forEach((p) => p.kill());
process.on('exit', stop);
await new Promise((r) => setTimeout(r, 800));

// Player 2 plays headless at about 60 frames a second the whole time.
const p2 = await startPlayer(2, 7777, () => {});
let p2Running = true;
(async () => {
    while (p2Running) {
        const t = Date.now();
        p2.step();
        await tick();
        const wait = 16 - (Date.now() - t);
        if (wait > 0) await new Promise((r) => setTimeout(r, wait));
    }
})();

const browser = await chromium.launch();
const context = await browser.newContext({ ...devices['iPhone 13'] });
const page = await context.newPage();
page.on('console', (m) => { if (/netplay|error/i.test(m.text())) console.log('page:', m.text()); });
page.on('pageerror', (e) => console.log('page error:', e.message));
await page.goto('http://localhost:8090/');
await page.waitForSelector('#setup:not([hidden])');
await page.screenshot({ path: join(out, '01-setup.png') });

await page.setInputFiles('#rom-file', baseRom);
await page.waitForSelector('#game:not([hidden])', { timeout: 120000 });
await page.waitForSelector('#loading[hidden]', { state: 'attached', timeout: 60000 });
console.log('game started');

// Boot through the netplay test boot as TEST1, pressing START and A with
// real taps on the on-screen buttons.
const tapKey = async (key) => {
    const box = await page.locator(`[data-key="${key}"]`).boundingBox();
    await page.touchscreen.tap(box.x + box.width / 2, box.y + box.height / 2);
};
for (let i = 0; i < 60; i++) {
    await page.evaluate((mb) => {
        const emu = globalThis.pokimon?.emu;
        if (emu && emu.ewram >= 0 && emu.read32(mb) === 0x4E4D4B50) emu.write8(mb + 0x0D1, 0xB1);
    }, MB);
    await tapKey(i % 2 ? 'a' : 'start');
    await page.waitForTimeout(400);
    const online = await page.evaluate(() => document.querySelector('#status').classList.contains('online'));
    if (online && i > 10) break;
}
await page.waitForTimeout(3000);
const remotes = await page.evaluate((mb) => {
    const emu = globalThis.pokimon.emu;
    let n = 0;
    for (let s = 0; s < 8; s++) if (emu.read16(mb + 0x20 + s * 20) !== 0) n++;
    return n;
}, MB);
console.log('page sees', remotes, 'other player(s); player 2 sees', p2.remotes());
await page.screenshot({ path: join(out, '02-online-portrait.png') });

// Walk down a few steps with the D-pad (held by a touch).
const dpad = await page.locator('#dpad').boundingBox();
const cdp = await context.newCDPSession(page);
const touch = (type, x, y) => cdp.send('Input.dispatchTouchEvent', {
    type, touchPoints: type === 'touchEnd' ? [] : [{ x, y, id: 1 }],
});
await touch('touchStart', dpad.x + dpad.width / 2, dpad.y + dpad.height - 10);
await page.waitForTimeout(250);
await page.screenshot({ path: join(out, '03-dpad-held.png') });
await page.waitForTimeout(700);
await touch('touchEnd');
await page.waitForTimeout(800);

await page.locator('#menu-open').tap();
await page.waitForTimeout(300);
await page.screenshot({ path: join(out, '04-menu.png') });
await page.locator('[data-action="close"]').tap();

await page.setViewportSize({ width: 844, height: 390 });
await page.waitForTimeout(1500);
await page.screenshot({ path: join(out, '05-online-landscape.png') });

p2Running = false;
await browser.close();
stop();
console.log(remotes > 0 ? 'PASS' : 'FAIL: the page does not see player 2');
process.exit(remotes > 0 ? 0 : 1);
