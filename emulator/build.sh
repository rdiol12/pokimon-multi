#!/usr/bin/env bash
# Builds our fork of mGBA: upstream mGBA at a pinned commit plus emulator/patches/*.patch.
# The fork adds the Pokimon extended hardware described in emulator/README.md.
#
#   emulator/build.sh source            fetch + patch only -> $OUT/src (a git repo, patch committed)
#   emulator/build.sh headless          libmgba.so + mgba-headless (tests, e2e harness)
#   emulator/build.sh sdl               mgba (SDL desktop player, with Lua for the netplay bridge)
#   emulator/build.sh qt                mgba-qt (full desktop player, needs Qt 5/6 dev packages)
#   emulator/build.sh wasm              mgba.js + mgba.wasm for the phone app (needs emcc)
#
# OUT defaults to build/emulator. Nothing here is Nintendo code or data.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
OUT="${OUT:-$ROOT/build/emulator}"
SRC="$OUT/src"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"

MGBA_REPO="${MGBA_REPO:-https://github.com/mgba-emu/mgba.git}"
MGBA_COMMIT="$(tr -d ' \r\n' < "$HERE/MGBA_COMMIT")"
# The phone app's wasm build script comes from @wasm-gaming/mgba-wasm (MPL-2.0).
WASM_REPO="${WASM_REPO:-https://github.com/wasm-gaming/mGBA-wasm.git}"
WASM_COMMIT="$(tr -d ' \r\n' < "$HERE/WASM_COMMIT")"

fetch() { # repo commit dir
    if [ ! -d "$3/.git" ]; then
        mkdir -p "$3"
        git -C "$3" init -q
        git -C "$3" remote add origin "$1"
    fi
    git -C "$3" fetch -q --depth 1 origin "$2"
    git -C "$3" checkout -q -f FETCH_HEAD
    git -C "$3" clean -qfdx
}

source_tree() {
    fetch "$MGBA_REPO" "$MGBA_COMMIT" "$SRC"
    for p in "$HERE"/patches/*.patch; do
        git -C "$SRC" apply --whitespace=nowarn "$p"
    done
    git -C "$SRC" -c user.name=pokimon -c user.email=pokimon@invalid commit -q -am "Pokimon extended hardware"
    echo "patched mGBA source: $SRC ($(git -C "$SRC" rev-parse --short HEAD))"
}

cmake_build() { # build-dir target extra-cmake-args...
    local dir="$1" target="$2"
    shift 2
    cmake -S "$SRC" -B "$dir" -DCMAKE_BUILD_TYPE=Release \
        -DUSE_FFMPEG=OFF -DUSE_LIBZIP=OFF -DUSE_DISCORD_RPC=OFF -DUSE_EDITLINE=OFF -DBUILD_SUITE=OFF -DBUILD_CINEMA=OFF -DSKIP_GIT=ON \
        "$@" > "$dir.cmake.log" 2>&1 || { cat "$dir.cmake.log"; exit 1; }
    cmake --build "$dir" --target "$target" -j "$JOBS"
}

case "${1:-headless}" in
source)
    source_tree
    ;;
headless)
    source_tree
    cmake_build "$OUT/headless" mgba-headless -DBUILD_QT=OFF -DBUILD_SDL=OFF -DBUILD_HEADLESS=ON -DUSE_LUA=ON
    ls -l "$OUT/headless/mgba-headless" "$OUT"/headless/libmgba.so*
    ;;
sdl)
    source_tree
    cmake_build "$OUT/sdl" mgba-sdl -DBUILD_QT=OFF -DBUILD_SDL=ON -DUSE_LUA=ON
    ;;
qt)
    source_tree
    cmake_build "$OUT/qt" mgba-qt -DBUILD_QT=ON -DBUILD_SDL=OFF -DUSE_LUA=ON
    ;;
wasm)
    source_tree
    fetch "$WASM_REPO" "$WASM_COMMIT" "$OUT/wasm"
    # Our changes to its shim (patches/wasm): widescreen frames
    for p in "$HERE"/patches/wasm/*.patch; do
        git -C "$OUT/wasm" apply --whitespace=nowarn "$p"
    done
    MGBA_REPO="$SRC" MGBA_REF="$(git -C "$SRC" rev-parse HEAD)" bash "$OUT/wasm/scripts/build-mgba.sh"
    ls -l "$OUT/wasm/dist/mgba/mgba.js" "$OUT/wasm/dist/mgba/mgba.wasm"
    ;;
*)
    echo "usage: $0 source|headless|sdl|qt|wasm" >&2
    exit 2
    ;;
esac
