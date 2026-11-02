// The phone app: first-run setup (patch the player's own ROM), then the
// game with touch controls, saves kept on the phone, and the netplay bridge.

import { applyBps, crc32, patchInfo, PatchError } from './bps.js';
import { describeWrongRom, isZip, unzipRom } from './rom.js';
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

// A yes/no question in the app's own dialog: iOS WebViews answer the
// browser's confirm() with "no" without showing it.
function ask(question) {
    const dialog = $('#ask');
    $('#ask-text').textContent = question;
    return new Promise((resolve) => {
        const answer = (e) => {
            const value = e.target.closest('[data-answer]')?.dataset.answer;
            if (!value) return;
            dialog.removeEventListener('click', answer);
            dialog.close();
            resolve(value === 'yes');
        };
        dialog.addEventListener('click', answer);
        dialog.showModal();
    });
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
        let rom = await readFile(file);
        if (isZip(rom)) rom = await unzipRom(rom);
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
    let game = rom;
    if (crc32(rom) !== target) {
        try {
            game = applyBps(rom, patch);
        } catch (e) {
            if (e.code === 'wrong-rom') throw new PatchError('wrong-rom', describeWrongRom(rom));
            throw e;
        }
    }
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
    if (!(await ask('Replace your current save with this file? The game restarts.'))) return;
    await storage.set('sram', bytes);
    location.reload();
}

// ---------------------------------------------------------------- settings

// Settings live with the save in the app's storage; read once at start.
const prefs = {
    widescreen: true,
    // Phone notifications for online play (docs/notifications.md)
    notify: { on: true, trade: true, friend: true, tour: true, fishing: false },
};

async function loadPrefs() {
    const saved = await storage.get('prefs').catch(() => null);
    const notify = { ...prefs.notify, ...(saved?.notify || {}) };
    Object.assign(prefs, saved || {}, { notify });
}

// The apps' native code shows the notifications, also while the page isn't
// running, so it keeps its own copy of these settings.
function pushNotifySettings() {
    const settings = { ...prefs.notify };
    if (globalThis.PokimonNative?.setNotifySettings) {
        globalThis.PokimonNative.setNotifySettings(JSON.stringify(settings));
    } else {
        globalThis.webkit?.messageHandlers?.pokimonNotify?.postMessage(settings);
    }
}

function setupNotifyDialog() {
    const dialog = $('#notify');
    const boxes = [...dialog.querySelectorAll('input[data-kind]')];
    const show = () => {
        for (const box of boxes) box.checked = !!prefs.notify[box.dataset.kind];
        dialog.dataset.off = String(!prefs.notify.on);
    };
    for (const box of boxes) {
        box.addEventListener('change', async () => {
            await setPref('notify', { ...prefs.notify, [box.dataset.kind]: box.checked });
            pushNotifySettings();
            show();
        });
    }
    dialog.addEventListener('click', (e) => {
        if (e.target.closest('[data-action="close"]')) dialog.close();
    });
    return () => { show(); dialog.showModal(); };
}

async function setPref(key, value) {
    prefs[key] = value;
    await storage.set('prefs', { ...prefs });
}

// Widescreen: the overworld fills a 320x160 screen. On unless turned off.
function widescreenOn() {
    return prefs.widescreen !== false;
}

// ---------------------------------------------------------------- game

async function startGame(game) {
    $('#setup').hidden = true;
    $('#game').hidden = false;
    const mod = await loadCore('vendor/');
    emu = new Emulator(mod);
    globalThis.pokimon = { emu }; // for debugging from the console
    await loadPrefs();
    const wide = widescreenOn();
    emu.boot(game, { wide });
    $('#screen').classList.toggle('wide', wide);
    const sram = await storage.get('sram');
    if (sram) {
        emu.putSave(sram);
        lastSaveCrc = crc32(sram);
    }
    emu.attachCanvas($('#screen'));
    let muted = false; // the sound button's mute, set before any tap handler reads it
    await emu.startAudio('audio-worklet.js');
    // Browsers start sound only from a tap; iOS wants it from the end of
    // one, so try on every kind of touch until the audio is running.
    for (const type of ['pointerdown', 'touchend', 'click', 'keydown']) {
        document.addEventListener(type, () => { if (!muted) emu.resumeAudio(); }, { capture: true, passive: true });
    }
    const release = attachControls($('#pad'), emu, {
        onPress: () => { if (!muted) emu.resumeAudio(); globalThis.navigator.vibrate?.(8); },
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
    const pause = () => { release(); persistSave(true); emu.stop(); emu.suspendAudio(); };
    document.addEventListener('visibilitychange', () => {
        if (document.hidden) pause();
        else { emu.start(); if (!muted) emu.resumeAudio(); }
    });
    window.addEventListener('pagehide', pause);

    // Shows whether sound is playing; a tap starts it (a sure user gesture
    // for iOS) or mutes it.
    const sound = $('#sound');
    sound.addEventListener('click', () => {
        const ctx = emu.audio?.ctx;
        if (!ctx) return toast('Sound is not available on this device');
        muted = ctx.state === 'running';
        if (muted) emu.suspendAudio();
        else emu.resumeAudio();
    });
    setInterval(() => {
        const on = emu.audio?.ctx.state === 'running';
        sound.textContent = on ? '🔊' : '🔇';
        // Keep it going unless the player muted it.
        if (!on && !muted && !document.hidden) emu.resumeAudio();
    }, 500);

    const ff = $('#ff');
    ff.addEventListener('click', () => {
        emu.speed = emu.speed === 1 ? 3 : 1;
        ff.setAttribute('aria-pressed', String(emu.speed > 1));
    });
    const menu = $('#menu');
    $('#widescreen-state').textContent = widescreenOn() ? 'on' : 'off';
    pushNotifySettings();
    const openNotify = setupNotifyDialog();
    $('#about').textContent = `Version ${build.version}. mGBA (MPL-2.0) and Lua (wasmoon, MIT) inside.`;
    $('#menu-open').addEventListener('click', () => { release(); persistSave(true); menu.showModal(); });
    menu.addEventListener('click', async (e) => {
        const action = e.target.closest('[data-action]')?.dataset.action;
        if (action === 'close') menu.close();
        if (action === 'export') exportSave();
        if (action === 'notify') { menu.close(); openNotify(); }
        if (action === 'widescreen') {
            const on = !widescreenOn();
            if (await ask(`Turn widescreen ${on ? 'on' : 'off'}? The game restarts, so save in the game first.`)) {
                await setPref('widescreen', on);
                location.reload();
            }
        }
        if (action === 'reset' && await ask('Restart the game? Unsaved progress is lost.')) {
            emu.reset();
            menu.close();
        }
        if (action === 'change-rom' && await ask('Pick a different ROM? Your save stays.')) {
            await storage.remove('game');
            await storage.remove('gameCrc');
            await storage.remove('rom');
            await storage.remove('patch');
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
