// A small key-value store in IndexedDB: the player's ROM, the patched game,
// the battery save and settings. Everything stays on the phone.

const DB_NAME = 'pokimon-multi';
const STORE = 'files';

let opening = null;

function db() {
    if (!opening) {
        opening = new Promise((resolve, reject) => {
            const req = indexedDB.open(DB_NAME, 1);
            req.onupgradeneeded = () => req.result.createObjectStore(STORE);
            req.onsuccess = () => resolve(req.result);
            req.onerror = () => reject(req.error);
        });
    }
    return opening;
}

async function run(mode, fn) {
    const d = await db();
    return new Promise((resolve, reject) => {
        const tx = d.transaction(STORE, mode);
        const req = fn(tx.objectStore(STORE));
        tx.oncomplete = () => resolve(req.result);
        tx.onerror = () => reject(tx.error);
        tx.onabort = () => reject(tx.error);
    });
}

export const storage = {
    get: (key) => run('readonly', (s) => s.get(key)),
    set: (key, value) => run('readwrite', (s) => s.put(value, key)),
    remove: (key) => run('readwrite', (s) => s.delete(key)),
};

// Asks the browser not to evict our data when the phone is low on space.
export function keepData() {
    navigator.storage?.persist?.().catch(() => {});
}
