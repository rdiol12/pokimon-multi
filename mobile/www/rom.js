// The ROM the player picks: opens it from a .zip, and says what is wrong
// with a file the patch doesn't fit.

const REGIONS = { E: 'USA', P: 'European', D: 'German', F: 'French', I: 'Italian', S: 'Spanish', J: 'Japanese' };

function u16(b, at) { return b[at] | b[at + 1] << 8; }
function u32(b, at) { return (b[at] | b[at + 1] << 8 | b[at + 2] << 16 | b[at + 3] << 24) >>> 0; }

export function isZip(bytes) {
    return bytes.length > 4 && bytes[0] === 0x50 && bytes[1] === 0x4B && bytes[2] === 3 && bytes[3] === 4;
}

// The first .gba file in a zip, read through the central directory (the
// local headers may leave the sizes out).
export async function unzipRom(bytes) {
    let end = -1;
    for (let i = bytes.length - 22; i >= Math.max(0, bytes.length - 0x10016); i--) {
        if (u32(bytes, i) === 0x06054B50) { end = i; break; }
    }
    if (end < 0) throw new Error('This zip file is damaged.');
    let at = u32(bytes, end + 16);
    const count = u16(bytes, end + 10);
    for (let n = 0; n < count && u32(bytes, at) === 0x02014B50; n++) {
        const method = u16(bytes, at + 10);
        const size = u32(bytes, at + 20);
        const nameLen = u16(bytes, at + 28);
        const extraLen = u16(bytes, at + 30);
        const commentLen = u16(bytes, at + 32);
        const local = u32(bytes, at + 42);
        const name = new TextDecoder().decode(bytes.subarray(at + 46, at + 46 + nameLen));
        at += 46 + nameLen + extraLen + commentLen;
        if (!/\.gba$/i.test(name)) continue;
        const start = local + 30 + u16(bytes, local + 26) + u16(bytes, local + 28);
        const data = bytes.subarray(start, start + size);
        if (method === 0) return data.slice();
        if (method !== 8 || typeof DecompressionStream === 'undefined') {
            throw new Error('This zip can\'t be opened here. Unzip it in the Files app and pick the .gba inside.');
        }
        const stream = new Blob([data]).stream().pipeThrough(new DecompressionStream('deflate-raw'));
        return new Uint8Array(await new Response(stream).arrayBuffer());
    }
    throw new Error('There is no .gba file in this zip.');
}

// Why a ROM isn't the unmodified Pokémon Emerald (USA) the patch is for.
export function describeWrongRom(rom) {
    if (rom.length < 0xC0 || rom[0xB2] !== 0x96) return 'This is not a GBA ROM.';
    const text = (from, len) => String.fromCharCode(...rom.subarray(from, from + len)).replace(/\0+$/, '');
    const title = text(0xA0, 12);
    const code = text(0xAC, 4);
    if (code.startsWith('BPE') && code !== 'BPEE') {
        return `This is the ${REGIONS[code[3]] || 'wrong'} version of Pokémon Emerald. The app needs the USA version.`;
    }
    if (code !== 'BPEE') return `This is ${title || 'another game'}, not Pokémon Emerald.`;
    return 'This is Pokémon Emerald (USA), but a changed copy (a hack, translation, or a patch already applied). '
        + 'The app needs the unchanged original.';
}
