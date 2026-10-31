// Touch controls: every finger is tracked on its own and may slide from one
// button to another, and the D-pad reads the direction from where the finger
// is relative to its centre (diagonals included). The keyboard works
// too, for testing on a computer.

const DIRS = ['right', 'down', 'left', 'up'];
const KEYBOARD = {
    ArrowUp: 'up', ArrowDown: 'down', ArrowLeft: 'left', ArrowRight: 'right',
    KeyX: 'a', KeyZ: 'b', KeyA: 'l', KeyS: 'r', Enter: 'start', Backspace: 'select', ShiftRight: 'select',
};

export function attachControls(pad, emu, { onPress } = {}) {
    const dpad = pad.querySelector('#dpad');
    const arms = Object.fromEntries(DIRS.map((d) => [d, dpad.querySelector('.' + d)]));
    const pointers = new Map(); // pointer id -> set of keys it holds
    const held = new Map(); // key -> count of sources holding it

    const press = (key, down) => {
        const n = (held.get(key) || 0) + (down ? 1 : -1);
        held.set(key, Math.max(0, n));
        const on = n > 0;
        emu.setKey(key, on);
        const el = arms[key] || pad.querySelector(`[data-key="${key}"]`);
        el?.classList.toggle(arms[key] ? 'down-on' : 'down', on);
        if (on && down && n === 1) onPress?.(key);
    };

    const keysAt = (x, y) => {
        const r = dpad.getBoundingClientRect();
        const cx = r.left + r.width / 2, cy = r.top + r.height / 2;
        const dx = x - cx, dy = y - cy;
        const reach = r.width * 0.75;
        if (Math.abs(dx) < reach && Math.abs(dy) < reach) {
            if (Math.hypot(dx, dy) < r.width * 0.1) return [];
            const angle = Math.atan2(dy, dx); // 0 = right, clockwise
            const sector = Math.round(angle / (Math.PI / 4)); // -4..4
            const map = {
                0: ['right'], 1: ['right', 'down'], 2: ['down'], 3: ['down', 'left'],
                4: ['left'], [-4]: ['left'], [-3]: ['left', 'up'], [-2]: ['up'], [-1]: ['up', 'right'],
            };
            return map[sector];
        }
        const el = document.elementFromPoint(x, y)?.closest('[data-key]');
        return el && pad.contains(el) ? [el.dataset.key] : [];
    };

    const update = (id, keys) => {
        const before = pointers.get(id) || new Set();
        const after = new Set(keys);
        for (const k of before) if (!after.has(k)) press(k, false);
        for (const k of after) if (!before.has(k)) press(k, true);
        if (after.size) pointers.set(id, after); else pointers.delete(id);
    };

    pad.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        pad.setPointerCapture?.(e.pointerId);
        update(e.pointerId, keysAt(e.clientX, e.clientY));
    });
    pad.addEventListener('pointermove', (e) => {
        if (!pointers.has(e.pointerId) && e.buttons === 0) return;
        update(e.pointerId, keysAt(e.clientX, e.clientY));
    });
    const lift = (e) => update(e.pointerId, []);
    pad.addEventListener('pointerup', lift);
    pad.addEventListener('pointercancel', lift);
    pad.addEventListener('contextmenu', (e) => e.preventDefault());

    window.addEventListener('keydown', (e) => {
        const key = KEYBOARD[e.code];
        if (!key || e.repeat) return;
        e.preventDefault();
        press(key, true);
    });
    window.addEventListener('keyup', (e) => {
        const key = KEYBOARD[e.code];
        if (!key) return;
        e.preventDefault();
        press(key, false);
    });

    // Lets go of everything, e.g. when the app goes to the background.
    return function releaseAll() {
        for (const id of [...pointers.keys()]) update(id, []);
        for (const [key, n] of held) for (let i = 0; i < n; i++) press(key, false);
    };
}
