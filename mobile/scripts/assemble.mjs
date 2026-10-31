// Builds mobile/dist/, the web root both phone apps load: the app's own
// files from www/, the mGBA core and the Lua VM from node_modules, the
// netplay bridge scripts, and (when given) the game's .bps patch.
//
//   node scripts/assemble.mjs [path/to/pokimon-multi.bps]
//
// Without a patch the app asks the player for the .bps file too.

import { cpSync, existsSync, mkdirSync, rmSync, writeFileSync, readFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const repo = resolve(root, '..');
const dist = join(root, 'dist');
const modules = join(root, 'node_modules');

rmSync(dist, { recursive: true, force: true });
cpSync(join(root, 'www'), dist, { recursive: true });

const vendor = join(dist, 'vendor');
mkdirSync(vendor, { recursive: true });
const copy = (from, to) => {
    if (!existsSync(from)) throw new Error(`missing ${from} (run npm install in mobile/)`);
    cpSync(from, join(vendor, to));
};
copy(join(modules, '@wasm-gaming/mgba-wasm/dist/mgba/mgba.js'), 'mgba.js');
copy(join(modules, '@wasm-gaming/mgba-wasm/dist/mgba/mgba.wasm'), 'mgba.wasm');
copy(join(modules, 'wasmoon/dist/index.js'), 'wasmoon.js');
copy(join(modules, 'wasmoon/dist/glue.wasm'), 'glue.wasm');

const lua = join(dist, 'lua');
mkdirSync(lua, { recursive: true });
for (const name of ['netplay_world.lua', 'netplay_bridge.lua', 'netplay_ext.lua']) {
    cpSync(join(repo, 'tools/netplay_bridge', name), join(lua, name));
}

const patch = process.argv[2] || process.env.POKIMON_PATCH;
if (patch) {
    cpSync(patch, join(dist, 'game.bps'));
}
const version = process.env.POKIMON_VERSION || 'dev';
writeFileSync(join(dist, 'build.json'), JSON.stringify({ version, patch: !!patch }) + '\n');
console.log(`mobile/dist ready (version ${version}, ${patch ? 'patch bundled' : 'no patch'})`);

// Keep the licenses of what we ship next to it.
writeFileSync(join(vendor, 'NOTICE.txt'), [
    'mgba.js, mgba.wasm: mGBA (https://mgba.io) built by @wasm-gaming/mgba-wasm,',
    'Mozilla Public License 2.0. Source: https://github.com/mgba-emu/mgba and',
    'https://www.npmjs.com/package/@wasm-gaming/mgba-wasm',
    '',
    'wasmoon.js, glue.wasm: wasmoon (Lua 5.4 in WebAssembly), MIT license:',
    '',
    readFileSync(join(modules, 'wasmoon/LICENSE'), 'utf8'),
].join('\n'));
