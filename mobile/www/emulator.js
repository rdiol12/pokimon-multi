// Runs the game on the mGBA core (vendor/mgba.js, one core->runFrame() per
// call) and gives the netplay bridge direct access to the GBA's EWRAM.

export const KEYS = { a: 0, b: 1, select: 2, start: 3, right: 4, left: 5, up: 6, down: 7, r: 8, l: 9 };

const EWRAM_START = 0x02000000;
const EWRAM_SIZE = 0x40000;
const STATE_EWRAM = 0x21000; // where a GBA savestate keeps EWRAM
const AUDIO_TARGET = 0.09; // seconds of audio kept queued
const MAX_FRAMES_PER_TICK = 4;

export async function loadCore(vendorUrl) {
    if (typeof globalThis.createMgbaModule !== 'function') {
        await new Promise((resolve, reject) => {
            const s = document.createElement('script');
            s.src = vendorUrl + 'mgba.js';
            s.onload = resolve;
            s.onerror = () => reject(new Error('cannot load the emulator'));
            document.head.appendChild(s);
        });
    }
    const mod = await globalThis.createMgbaModule({
        locateFile: (path) => vendorUrl + path,
    });
    mod._mgbawasm_init();
    mod._mgbawasm_set_log_level?.(1);
    return mod;
}

export class Emulator {
    constructor(mod) {
        this.mod = mod;
        this.ewram = -1; // EWRAM's offset in the wasm heap, once found
        this.frameHooks = [];
        this.keys = 0;
        this.tapped = 0; // pressed since the last frame, even if let go already
        this.sentKeys = -1;
        this.speed = 1;
        this.running = false;
        this.frame = 0;
        this.audio = null;
        this.audioPtr = 0;
        this.enqueued = 0;
        this.played = 0;
        this.playRate = 0; // the rate the audio player is told: the core's rate times speed
    }

    boot(rom) {
        const mod = this.mod;
        const ptr = mod._malloc(rom.length);
        mod.HEAPU8.set(rom, ptr);
        const ok = mod._mgbawasm_load(ptr, rom.length, 0, 0, 0, 0, 1);
        mod._free(ptr);
        if (!ok) throw new Error('The emulator could not start this ROM.');
        mod._mgbawasm_set_idle_optimization?.(1);
        this.ewram = -1;
        this.frame = 0;
        this.framerate = mod._mgbawasm_framerate_micro() / 1e6 || 59.7275;
        this.audioPtr ||= mod._malloc(4096 * 4);
    }

    // ------------------------------------------------------------ memory

    // EWRAM is a block of the wasm heap. Its offset isn't exported, so it is
    // found once by matching a savestate's copy of EWRAM against the heap.
    findEwram() {
        const mod = this.mod;
        const size = mod._mgbawasm_state_size();
        const ptr = mod._malloc(size);
        try {
            if (!mod._mgbawasm_state_save(ptr)) return false;
            const heap = mod.HEAPU8;
            const wram = heap.slice(ptr + STATE_EWRAM, ptr + STATE_EWRAM + EWRAM_SIZE);
            const probes = [];
            for (let i = 0; i < EWRAM_SIZE && probes.length < 64; i += 997) {
                if (wram[i] !== 0) probes.push(i);
            }
            if (probes.length < 16) return false; // the game hasn't started yet
            const matches = (base) => {
                for (const i of probes) if (heap[base + i] !== wram[i]) return false;
                for (let i = 0; i < EWRAM_SIZE; i += 64) if (heap[base + i] !== wram[i]) return false;
                return true;
            };
            // mGBA maps EWRAM with mmap, which Emscripten aligns to 64 KiB;
            // fall back to every word if that ever changes.
            for (const step of [0x10000, 4]) {
                for (let base = 0; base + EWRAM_SIZE <= heap.length; base += step) {
                    if (base >= ptr - EWRAM_SIZE && base < ptr + size) continue; // the state itself
                    if (heap[base + probes[0]] === wram[probes[0]] && matches(base)) {
                        this.ewram = base;
                        return true;
                    }
                }
            }
            return false;
        } finally {
            mod._free(ptr);
        }
    }

    offset(addr) {
        const off = addr - EWRAM_START;
        if (this.ewram < 0 || off < 0 || off >= EWRAM_SIZE) return -1;
        return this.ewram + off;
    }

    read8(addr) {
        const at = this.offset(addr);
        return at < 0 ? 0 : this.mod.HEAPU8[at];
    }

    read16(addr) {
        const at = this.offset(addr);
        if (at < 0) return 0;
        const h = this.mod.HEAPU8;
        return h[at] | h[at + 1] << 8;
    }

    read32(addr) {
        const at = this.offset(addr);
        if (at < 0) return 0;
        const h = this.mod.HEAPU8;
        return (h[at] | h[at + 1] << 8 | h[at + 2] << 16 | h[at + 3] << 24) >>> 0;
    }

    write8(addr, v) {
        const at = this.offset(addr);
        if (at >= 0) this.mod.HEAPU8[at] = v;
    }

    write16(addr, v) {
        const at = this.offset(addr);
        if (at < 0) return;
        const h = this.mod.HEAPU8;
        h[at] = v;
        h[at + 1] = v >> 8;
    }

    // Every EWRAM address (word-aligned) holding this 32-bit value.
    findWord(value) {
        if (this.ewram < 0) return [];
        const h = this.mod.HEAPU8;
        const b0 = value & 0xFF, b1 = (value >>> 8) & 0xFF, b2 = (value >>> 16) & 0xFF, b3 = value >>> 24;
        const out = [];
        for (let i = this.ewram, end = this.ewram + EWRAM_SIZE - 4; i <= end; i += 4) {
            if (h[i] === b0 && h[i + 1] === b1 && h[i + 2] === b2 && h[i + 3] === b3) {
                out.push(EWRAM_START + i - this.ewram);
            }
        }
        return out;
    }

    // ------------------------------------------------------------- saves

    getSave() {
        const mod = this.mod;
        const size = mod._mgbawasm_sram_save();
        const ptr = size ? mod._mgbawasm_sram_ptr() : 0;
        if (!ptr) return null;
        return mod.HEAPU8.slice(ptr, ptr + size);
    }

    putSave(bytes) {
        const mod = this.mod;
        const ptr = mod._malloc(bytes.length);
        mod.HEAPU8.set(bytes, ptr);
        mod._mgbawasm_sram_load(ptr, bytes.length);
        mod._free(ptr);
    }

    reset() {
        this.mod._mgbawasm_reset();
        this.frame = 0;
        for (const hook of this.frameHooks) hook.onReset?.();
    }

    // ------------------------------------------------------------- input

    setKey(name, down) {
        const bit = 1 << KEYS[name];
        this.keys = down ? this.keys | bit : this.keys & ~bit;
        if (down) this.tapped |= bit;
    }

    // A quick tap can start and end between two frames; it still counts.
    pushKeys() {
        const keys = this.keys | this.tapped;
        this.tapped = 0;
        if (keys !== this.sentKeys) {
            this.sentKeys = keys;
            this.mod._mgbawasm_set_keys(keys);
        }
    }

    // ------------------------------------------------------------- frames

    runFrame() {
        this.pushKeys();
        this.mod._mgbawasm_run_frame();
        this.frame++;
        if (this.ewram < 0 && this.frame % 30 === 0) this.findEwram();
        for (const hook of this.frameHooks) hook.onFrame(this.frame);
    }

    // ------------------------------------------------------------- video

    attachCanvas(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.image = null;
    }

    render() {
        const mod = this.mod;
        const w = mod._mgbawasm_video_width();
        const h = mod._mgbawasm_video_height();
        if (w <= 0 || h <= 0 || !this.ctx) return;
        if (!this.image || this.image.width !== w || this.image.height !== h) {
            this.canvas.width = w;
            this.canvas.height = h;
            this.image = this.ctx.createImageData(w, h);
        }
        const ptr = mod._mgbawasm_video_ptr();
        this.image.data.set(mod.HEAPU8.subarray(ptr, ptr + w * h * 4));
        this.ctx.putImageData(this.image, 0, 0);
    }

    // The current picture as PNG bytes (photos from the world features).
    screenshotPng() {
        const url = this.canvas.toDataURL('image/png');
        const bin = atob(url.slice(url.indexOf(',') + 1));
        const out = new Uint8Array(bin.length);
        for (let i = 0; i < bin.length; i++) out[i] = bin.charCodeAt(i);
        return out;
    }

    // ------------------------------------------------------------- audio

    async startAudio(workletUrl) {
        // iOS plays Web Audio as "ambient" sound, which the silent switch
        // mutes, whatever audio session the app itself asked for. iOS 16.4+
        // lets the page say it is playback, like a music or game app.
        try {
            if (globalThis.navigator?.audioSession) globalThis.navigator.audioSession.type = 'playback';
        } catch (e) { /* not supported */ }
        let ctx;
        try {
            ctx = new AudioContext({ latencyHint: 'interactive' });
        } catch (e) {
            console.warn('no audio', e);
            return;
        }
        let node;
        try {
            await ctx.audioWorklet.addModule(workletUrl);
            node = new AudioWorkletNode(ctx, 'game-audio', { numberOfInputs: 0, outputChannelCount: [2] });
        } catch (e) {
            // No AudioWorklet (an insecure context, older WebViews).
            node = scriptProcessorSink(ctx);
        }
        node.connect(ctx.destination);
        node.port.onmessage = (e) => { this.played = e.data; };
        this.audio = { ctx, node };
    }

    // Call from a tap. iOS starts audio only from inside a user gesture, and
    // only once something has actually been played in one.
    resumeAudio() {
        if (!this.audio) return;
        const { ctx } = this.audio;
        if (ctx.state === 'running') return;
        try {
            const src = ctx.createBufferSource();
            src.buffer = ctx.createBuffer(1, 1, ctx.sampleRate);
            src.connect(ctx.destination);
            src.start(0);
        } catch (e) { /* closed context */ }
        ctx.resume().catch(() => {});
        playbackKeepAlive(true);
    }

    // Mute: stop the context and the iOS keep-alive.
    suspendAudio() {
        if (!this.audio) return;
        this.audio.ctx.suspend().catch(() => {});
        playbackKeepAlive(false);
    }

    drainAudio(keep) {
        const mod = this.mod;
        for (;;) {
            const frames = mod._mgbawasm_read_audio(this.audioPtr, 4096);
            if (frames <= 0) return;
            if (keep && this.audio) {
                const start = this.audioPtr >> 1;
                const chunk = mod.HEAP16.slice(start, start + frames * 2);
                this.audio.node.port.postMessage(chunk, [chunk.buffer]);
                this.enqueued += frames;
            }
            if (frames < 4096) return;
        }
    }

    // ------------------------------------------------------------- loop

    start() {
        if (this.running) return;
        this.running = true;
        this.wallStart = 0;
        const tick = (now) => {
            if (!this.running) return;
            requestAnimationFrame(tick);
            this.tick(now);
        };
        requestAnimationFrame(tick);
    }

    stop() {
        this.running = false;
    }

    // Paced by the audio clock while sound plays, by the wall clock
    // otherwise (no audio device, or sound off). Fast forward plays the
    // sound sped up (higher pitched) instead of dropping it: the player
    // consumes it `speed` times faster, so the game runs that much faster.
    tick(now) {
        const mod = this.mod;
        const rate = mod._mgbawasm_sample_rate();
        const playRate = rate * this.speed;
        if (this.audio && rate > 0 && playRate !== this.playRate) {
            this.playRate = playRate;
            this.audio.node.port.postMessage({ rate: playRate });
        }
        let frames;
        const audioLive = this.audio && this.audio.ctx.state === 'running' && rate > 0;
        if (audioLive) {
            const queued = this.enqueued - this.played;
            const deficit = playRate * AUDIO_TARGET - queued;
            frames = deficit > 0 ? Math.ceil(deficit / (rate / this.framerate)) : 0;
            this.wallStart = 0;
        } else {
            if (!this.wallStart) {
                this.wallStart = now;
                this.wallFrames = 0;
            }
            const due = Math.floor((now - this.wallStart) / 1000 * this.framerate * this.speed);
            frames = due - this.wallFrames;
            this.wallFrames = due;
        }
        frames = Math.max(0, Math.min(MAX_FRAMES_PER_TICK * this.speed, frames));
        for (let i = 0; i < frames; i++) {
            this.runFrame();
            this.drainAudio(audioLive);
        }
        if (frames > 0) this.render();
    }
}

// The same player as audio-worklet.js on a ScriptProcessorNode, with the
// worklet's port interface.
function scriptProcessorSink(ctx) {
    const proc = ctx.createScriptProcessor(1024, 0, 2);
    const queue = [];
    let chunk = null, pos = 0, rate = 32768, played = 0;
    const port = {
        onmessage: null,
        postMessage(data) {
            if (data.rate) rate = data.rate;
            else queue.push(data);
        },
    };
    proc.onaudioprocess = (e) => {
        const left = e.outputBuffer.getChannelData(0);
        const right = e.outputBuffer.getChannelData(1);
        const step = rate / ctx.sampleRate;
        for (let i = 0; i < left.length; i++) {
            while (chunk === null || pos >= chunk.length / 2) {
                if (chunk !== null) { pos -= chunk.length / 2; chunk = null; }
                if (!queue.length) break;
                chunk = queue.shift();
            }
            if (chunk === null) { left[i] = right[i] = 0; continue; }
            const f = Math.floor(pos) * 2;
            left[i] = chunk[f] / 32768;
            right[i] = chunk[f + 1] / 32768;
            pos += step;
            played += step;
        }
        port.onmessage?.({ data: Math.floor(played) });
    };
    return { port, connect: (dest) => proc.connect(dest) };
}

// Before iOS 16.4 the only way to make iOS treat the page's audio as playback
// (heard with the silent switch on) is to have an HTML media element playing
// too. This loops one second of silence for that, on iOS only, started from
// a tap like the AudioContext.
let keepAlive = null;
function playbackKeepAlive(on) {
    const doc = globalThis.document;
    const nav = globalThis.navigator;
    if (!doc || !nav || nav.audioSession) return;
    const ios = /iPhone|iPad|iPod/.test(nav.userAgent) || (nav.platform === 'MacIntel' && nav.maxTouchPoints > 1);
    if (!ios) return;
    if (!on) {
        keepAlive?.pause();
        return;
    }
    if (!keepAlive) {
        keepAlive = doc.createElement('audio');
        keepAlive.src = silentWav(8000);
        keepAlive.loop = true;
        keepAlive.setAttribute('playsinline', '');
        keepAlive.setAttribute('x-webkit-airplay', 'deny');
    }
    if (keepAlive.paused) keepAlive.play().catch(() => {});
}

// A data: URL of n samples of 8 kHz mono 8-bit silence.
function silentWav(n) {
    const bytes = new Uint8Array(44 + n);
    const view = new DataView(bytes.buffer);
    const text = (at, s) => { for (let i = 0; i < s.length; i++) bytes[at + i] = s.charCodeAt(i); };
    text(0, 'RIFF'); view.setUint32(4, 36 + n, true); text(8, 'WAVE');
    text(12, 'fmt '); view.setUint32(16, 16, true); view.setUint16(20, 1, true); view.setUint16(22, 1, true);
    view.setUint32(24, 8000, true); view.setUint32(28, 8000, true); view.setUint16(32, 1, true); view.setUint16(34, 8, true);
    text(36, 'data'); view.setUint32(40, n, true);
    bytes.fill(128, 44);
    let bin = '';
    for (let i = 0; i < bytes.length; i++) bin += String.fromCharCode(bytes[i]);
    return 'data:audio/wav;base64,' + btoa(bin);
}
