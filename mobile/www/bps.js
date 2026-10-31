// BPS patching (the format tools/make_bps.py writes), done on the phone so
// the app never carries a ROM: the player's own Emerald is patched here.

const CRC_TABLE = (() => {
    const table = new Uint32Array(256);
    for (let n = 0; n < 256; n++) {
        let c = n;
        for (let k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320 ^ (c >>> 1) : c >>> 1;
        table[n] = c >>> 0;
    }
    return table;
})();

export function crc32(bytes, start = 0, end = bytes.length) {
    let c = 0xFFFFFFFF;
    for (let i = start; i < end; i++) c = CRC_TABLE[(c ^ bytes[i]) & 0xFF] ^ (c >>> 8);
    return (c ^ 0xFFFFFFFF) >>> 0;
}

function u32(bytes, at) {
    return (bytes[at] | bytes[at + 1] << 8 | bytes[at + 2] << 16 | bytes[at + 3] << 24) >>> 0;
}

export class PatchError extends Error {
    constructor(code, message) {
        super(message);
        this.code = code;
    }
}

// Reads the three checksums at the end of a patch.
export function patchInfo(patch) {
    if (patch.length < 16 || String.fromCharCode(...patch.subarray(0, 4)) !== 'BPS1') {
        throw new PatchError('not-bps', 'This is not a BPS patch.');
    }
    const end = patch.length - 12;
    return { source: u32(patch, end), target: u32(patch, end + 4), patch: u32(patch, end + 8) };
}

export function applyBps(source, patch) {
    const info = patchInfo(patch);
    if (crc32(patch, 0, patch.length - 4) !== info.patch) {
        throw new PatchError('bad-patch', 'The patch file is damaged.');
    }
    if (crc32(source) !== info.source) {
        throw new PatchError('wrong-rom', 'This ROM is not Pokémon Emerald (USA).');
    }
    let at = 4;
    const number = () => {
        let value = 0;
        let shift = 1;
        for (;;) {
            const b = patch[at++];
            value += (b & 0x7F) * shift;
            if (b & 0x80) return value;
            shift *= 128;
            value += shift;
        }
    };
    const sourceSize = number();
    const targetSize = number();
    const metaSize = number();
    at += metaSize;
    if (sourceSize !== source.length) {
        throw new PatchError('wrong-rom', 'This ROM is not Pokémon Emerald (USA).');
    }
    const target = new Uint8Array(targetSize);
    const end = patch.length - 12;
    let out = 0;
    let sourceRel = 0;
    let targetRel = 0;
    while (at < end) {
        const data = number();
        const length = (data >> 2) + 1;
        const mode = data & 3;
        if (mode === 0) { // SourceRead
            target.set(source.subarray(out, out + length), out);
            out += length;
        } else if (mode === 1) { // TargetRead
            target.set(patch.subarray(at, at + length), out);
            at += length;
            out += length;
        } else {
            const word = number();
            const offset = (word & 1 ? -1 : 1) * Math.floor(word / 2);
            if (mode === 2) { // SourceCopy
                sourceRel += offset;
                target.set(source.subarray(sourceRel, sourceRel + length), out);
                sourceRel += length;
                out += length;
            } else { // TargetCopy: may overlap itself, so byte by byte
                targetRel += offset;
                for (let i = 0; i < length; i++) target[out++] = target[targetRel++];
            }
        }
    }
    if (crc32(target) !== info.target) {
        throw new PatchError('bad-result', 'Patching went wrong (checksum mismatch).');
    }
    return target;
}
