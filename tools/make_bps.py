#!/usr/bin/env python3
"""Make a BPS patch that turns one ROM into another, so a release can ship
the patch instead of the game. Players apply it to their own Pokémon Emerald
with any BPS patcher (Flips, Rom Patcher JS, ...).

    python3 tools/make_bps.py base.gba pokeemerald.gba out.bps

Standard library only. Bytes equal to the base at the same offset are copied
from it, runs of one repeated byte (like the ROM's padding) are copied from
the target itself, and everything else is stored in the patch.
"""

import sys
import zlib


def encode_number(n):
    out = bytearray()
    while True:
        x = n & 0x7F
        n >>= 7
        if n == 0:
            out.append(0x80 | x)
            return out
        out.append(x)
        n -= 1


def decode_number(data, pos):
    n, shift = 0, 1
    while True:
        x = data[pos]
        pos += 1
        n += (x & 0x7F) * shift
        if x & 0x80:
            return n, pos
        shift <<= 7
        n += shift


SOURCE_READ, TARGET_READ, TARGET_COPY = 0, 1, 3
MIN_SOURCE_RUN = 4  # shorter matches cost more to describe than to store
MIN_FILL_RUN = 16


def make_patch(source, target):
    out = bytearray(b"BPS1")
    out += encode_number(len(source))
    out += encode_number(len(target))
    out += encode_number(0)  # no metadata

    def action(kind, length):
        out.extend(encode_number(((length - 1) << 2) | kind))

    target_rel = 0
    pos, literal_start = 0, 0

    def flush_literal(upto):
        if literal_start < upto:
            action(TARGET_READ, upto - literal_start)
            out.extend(target[literal_start:upto])

    while pos < len(target):
        run = 0
        while pos + run < len(target) and pos + run < len(source) and source[pos + run] == target[pos + run]:
            run += 1
        if run >= MIN_SOURCE_RUN:
            flush_literal(pos)
            action(SOURCE_READ, run)
            pos += run
            literal_start = pos
            continue
        fill = 1
        while pos + fill < len(target) and target[pos + fill] == target[pos]:
            fill += 1
        if fill >= MIN_FILL_RUN:
            # Store the byte once, then copy it forward from itself.
            flush_literal(pos + 1)
            delta = pos - target_rel
            action(TARGET_COPY, fill - 1)
            out.extend(encode_number((abs(delta) << 1) | (delta < 0)))
            target_rel = pos + fill - 1
            pos += fill
            literal_start = pos
            continue
        pos += max(run, 1)
    flush_literal(len(target))

    for blob in (source, target, out):
        out += zlib.crc32(blob).to_bytes(4, "little")
    return bytes(out)


def apply_patch(source, patch):
    """Enough of BPS to check our own patches (and Flips-made ones)."""
    if patch[:4] != b"BPS1" or zlib.crc32(patch[:-4]) != int.from_bytes(patch[-4:], "little"):
        raise ValueError("not a valid BPS patch")
    if zlib.crc32(source) != int.from_bytes(patch[-12:-8], "little"):
        raise ValueError("wrong base ROM")
    source_size, pos = decode_number(patch, 4)
    target_size, pos = decode_number(patch, pos)
    meta_size, pos = decode_number(patch, pos)
    pos += meta_size
    target = bytearray()
    source_rel = target_rel = 0
    end = len(patch) - 12
    while pos < end:
        data, pos = decode_number(patch, pos)
        kind, length = data & 3, (data >> 2) + 1
        if kind == 0:
            target += source[len(target):len(target) + length]
        elif kind == 1:
            target += patch[pos:pos + length]
            pos += length
        else:
            offset, pos = decode_number(patch, pos)
            offset = (-1 if offset & 1 else 1) * (offset >> 1)
            if kind == 2:
                source_rel += offset
                target += source[source_rel:source_rel + length]
                source_rel += length
            else:
                target_rel += offset
                for _ in range(length):
                    target.append(target[target_rel])
                    target_rel += 1
    if len(target) != target_size or zlib.crc32(target) != int.from_bytes(patch[-8:-4], "little"):
        raise ValueError("patch produced the wrong ROM")
    return bytes(target)


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    base_path, rom_path, out_path = sys.argv[1:]
    with open(base_path, "rb") as f:
        base = f.read()
    with open(rom_path, "rb") as f:
        rom = f.read()
    patch = make_patch(base, rom)
    if apply_patch(base, patch) != rom:
        sys.exit("the patch doesn't reproduce the ROM")
    with open(out_path, "wb") as f:
        f.write(patch)
    print(f"{out_path}: {len(patch)} bytes")


if __name__ == "__main__":
    main()
