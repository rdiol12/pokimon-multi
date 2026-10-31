// Plays the core's audio: interleaved 16-bit stereo chunks at the core's
// rate, resampled to the device rate. Reports how many source frames it has
// played so the emulator can keep a small amount queued.

class GameAudio extends AudioWorkletProcessor {
    constructor() {
        super();
        this.queue = [];
        this.chunk = null;
        this.pos = 0; // fractional position in the current chunk, in frames
        this.rate = 32768;
        this.played = 0;
        this.port.onmessage = (e) => {
            if (e.data.rate) this.rate = e.data.rate;
            else this.queue.push(e.data);
        };
    }

    process(inputs, outputs) {
        const [left, right] = outputs[0];
        const step = this.rate / sampleRate;
        let consumed = 0;
        for (let i = 0; i < left.length; i++) {
            while (this.chunk === null || this.pos >= this.chunk.length / 2) {
                if (this.chunk !== null) {
                    this.pos -= this.chunk.length / 2;
                    this.chunk = null;
                }
                if (!this.queue.length) break;
                this.chunk = this.queue.shift();
            }
            if (this.chunk === null) {
                left[i] = 0;
                right[i] = 0;
                continue;
            }
            const f = Math.floor(this.pos) * 2;
            left[i] = this.chunk[f] / 32768;
            right[i] = this.chunk[f + 1] / 32768;
            this.pos += step;
            consumed += step;
        }
        this.played += consumed;
        this.port.postMessage(Math.floor(this.played));
        return true;
    }
}

registerProcessor('game-audio', GameAudio);
