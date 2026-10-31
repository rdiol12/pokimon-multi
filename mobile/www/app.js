// The phone app: first-run setup (patch the player's own ROM), then the
// game with touch controls, saves kept on the phone, and the netplay bridge.

import { applyBps, crc32, patchInfo, PatchError } from './bps.js';
import { storage, keepData } from './storage.js';
import { Emulator, loadCore } from './emulator.js';
import { attachControls } from './controls.js';
import { startBridge } from './bridge.js';

const $ = (sel) => document.querySelector(sel);
const SAVE_EVERY_MS = 3000;
const STATUS_TEXT = ['Offline', 'Connecting…', 'Online'];
const STATUS_CLASS = ['offline', 'connecting', 'online'];

let build = { version: 'dev', patch: false };
let emu = null;
let bridge = null;
let lastSaveCrc = 0;

function toast(text, ms = 2200) {
    const el = document.createElement('div');
    el.className = 'toast';
    el.textContent = text;
    document.body.appendChild(el);
    setTimeout(() => el.remove(), ms);
}

const readFile = (file) => file.arrayBuffer().then((b) => new Uint8Array(b));

async function bundledPatch() {
    if (!build.patch) return null;
    const res = await fetch('game.bps');
    return res.ok ? new Uint8Array(await res.arrayBuffer()) : null;
}

// The patched game for the stored ROM, patching again when the app brings a
// new patch.
async function gameRom() {
    const patch = (await bundledPatch()) || (await storage.get('patch'));
    const rom = await storage.get('rom');
    if (!rom || !patch) return null;
    const target = patchInfo(patch).target;
    if ((await storage.get('gameCrc')) === target) {
        const game = await storage.get('game');
        if (game) return game;
    }
    let game;
    if (crc32(rom) === target) {
        game = rom; // the player picked an already patched ROM
    } else {
        game = applyBps(rom, patch);
    }
    await storage.set('game', game);
    await storage.set('gameCrc', target);
    return game;
}

// ---------------------------------------------------------------- setup

function showSetup(message = '', isError = false) {
    $('#loading').hidden = true;
    $('#game').hidden = true;
    $('#setup').hidden = false;
    $('#patch-step').hidden = build.patch;
    const msg = $('#setup-msg');
    msg.textContent = message;
    msg.classList.toggle('error', isError);
}

async function onRomPicked(file) {
    const msg = $('#setup-msg');
    msg.classList.remove('error');
    msg.textContent = 'Checking your ROM…';
    try {
        const rom = await readFile(file);
        const patch = (await bundledPatch()) || (await storage.get('patch'));
        if (!patch) {
            await storage.set('rom', rom);
            $('#patch-pick').hidden = false;
            $('#rom-pick').hidden = true;
            msg.textContent = 'Now pick the .bps patch.';
            return;
        }
        await installRom(rom, patch);
    } catch (e) {
        showSetup(e instanceof PatchError ? e.message : 'Something went wrong: ' + e.message, true);
    }
}

async function onPatchPicked(file) {
    try {
        const patch = await readFile(file);
        patchInfo(patch);
        await installRom(await storage.get('rom'), patch);
        await storage.set('patch', patch);
    } catch (e) {
        showSetup(e instanceof PatchError ? e.message : 'Something went wrong: ' + e.message, true);
    }
}

async function installRom(rom, patch) {
    $('#setup-msg').textContent = 'Patching… (this takes a few seconds)';
    await new Promise((r) => setTimeout(r, 30)); // let the message paint
    const target = patchInfo(patch).target;
    const game = crc32(rom) === target ? rom : applyBps(rom, patch);
    await storage.set('rom', rom);
    await storage.set('game', game);
    await storage.set('gameCrc', target);
    location.reload();
}

// ---------------------------------------------------------------- saves

async function persistSave(force = false) {
    if (!emu) return;
    const sram = emu.getSave();
    if (!sram) return;
    const crc = crc32(sram);
    if (!force && crc === lastSaveCrc) return;
    lastSaveCrc = crc;
    await storage.set('sram', sram);
}

function nativeShare() {
    if (globalThis.PokimonNative?.saveFile) {
        return (name, bytes) => globalThis.PokimonNative.saveFile(name, toBase64(bytes));
    }
    if (globalThis.webkit?.messageHandlers?.pokimonFiles) {
        return (name, bytes) => globalThis.webkit.messageHandlers.pokimonFiles.postMessage({ name, data: toBase64(bytes) });
    }
    return null;
}

function toBase64(bytes) {
    let bin = '';
    for (let i = 0; i < bytes.length; i += 0x8000) bin += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
    return btoa(bin);
}

function exportSave() {
    const sram = emu.getSave();
    if (!sram) return toast('No save yet');
    const name = 'pokimon-multi.sav';
    const share = nativeShare();
    if (share) return share(name, sram);
    const a = document.createElement('a');
    a.href = URL.createObjectURL(new Blob([sram]));
    a.download = name;
    a.click();
    setTimeout(() => URL.revokeObjectURL(a.href), 1000);
}

async function importSave(file) {
    const bytes = await readFile(file);
    if (bytes.length < 0x8000) return toast('That is not a GBA save file');
    if (!confirm('Replace your current save with this file? The game restarts.')) return;
    await storage.set('sram', bytes);
    location.reload();
}

// ---------------------------------------------------------------- game

async function startGame(game) {
    $('#setup').hidden = true;
    $('#game').hidden = false;
    const mod = await loadCore('vendor/');
    emu = new Emulator(mod);
    globalThis.pokimon = { emu }; // for debugging from the console
    emu.boot(game);
    const sram = await storage.get('sram');
    if (sram) {
        emu.putSave(sram);
        lastSaveCrc = crc32(sram);
    }
    emu.attachCanvas($('#screen'));
    await emu.startAudio('audio-worklet.js');
    const release = attachControls($('#pad'), emu, {
        onPress: () => { emu.resumeAudio(); globalThis.navigator.vibrate?.(8); },
    });
    try {
        bridge = await startBridge(emu, { log: (m) => console.log(m) });
    } catch (e) {
        console.warn('online play unavailable', e);
    }
    emu.start();
    $('#loading').hidden = true;

    setInterval(() => persistSave(), SAVE_EVERY_MS);
    let shown = -1;
    setInterval(() => {
        const s = bridge ? bridge.status() : 0;
        if (s === shown) return;
        shown = s;
        const el = $('#status');
        el.className = 'status ' + STATUS_CLASS[s];
        el.querySelector('span').textContent = STATUS_TEXT[s];
    }, 500);

    // Phones suspend apps in the background: save first, and pause.
    const pause = () => { release(); persistSave(true); emu.stop(); emu.audio?.ctx.suspend(); };
    document.addEventListener('visibilitychange', () => {
        if (document.hidden) pause();
        else emu.start();
    });
    window.addEventListener('pagehide', pause);

    const ff = $('#ff');
    ff.addEventListener('click', () => {
        emu.speed = emu.speed === 1 ? 3 : 1;
        ff.setAttribute('aria-pressed', String(emu.speed > 1));
    });
    const menu = $('#menu');
    $('#about').textContent = `Version ${build.version}. mGBA (MPL-2.0) and Lua (wasmoon, MIT) inside.`;
    $('#menu-open').addEventListener('click', () => { release(); persistSave(true); menu.showModal(); });
    menu.addEventListener('click', async (e) => {
        const action = e.target.closest('[data-action]')?.dataset.action;
        if (action === 'close') menu.close();
        if (action === 'export') exportSave();
        if (action === 'reset' && confirm('Restart the game? Unsaved progress is lost.')) {
            emu.reset();
            menu.close();
        }
        if (action === 'change-rom' && confirm('Pick a different ROM? Your save stays.')) {
            await storage.remove('game');
            await storage.remove('gameCrc');
            await storage.remove('rom');
            location.reload();
        }
    });
    $('#save-file').addEventListener('change', (e) => e.target.files[0] && importSave(e.target.files[0]));
}

async function main() {
    try {
        const res = await fetch('build.json');
        if (res.ok) build = await res.json();
    } catch { /* dev without assemble */ }
    keepData();
    $('#rom-file').addEventListener('change', (e) => e.target.files[0] && onRomPicked(e.target.files[0]));
    $('#patch-file').addEventListener('change', (e) => e.target.files[0] && onPatchPicked(e.target.files[0]));
    let game = null;
    try {
        game = await gameRom();
    } catch (e) {
        return showSetup(e instanceof PatchError ? e.message : 'Something went wrong: ' + e.message, true);
    }
    if (!game) return showSetup();
    try {
        await startGame(game);
    } catch (e) {
        console.error(e);
        showSetup('The game could not start: ' + e.message, true);
    }
}

main();
