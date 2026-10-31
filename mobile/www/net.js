// One TCP connection to the netplay server, as the bridge sees it:
// connect / send a line / receive what has arrived / close.
//
// In the phone apps the connection is a real TCP socket made by the app
// (Android: globalThis.PokimonNative, iOS: webkit.messageHandlers.pokimonNet),
// which pushes what it reads back through window.PokimonNet. In a desktop
// browser, for testing, it goes through the dev server's WebSocket proxy
// (mobile/dev/serve.py).

class Connection {
    constructor() {
        this.id = 0;
        this.inbox = '';
        this.closed = false;
        this.open = false;
        this.outbox = [];
    }
}

const connections = new Map();
let nextId = 1;
let current = null;

// Called by the apps' native code.
globalThis.PokimonNet = {
    opened(id) {
        const c = connections.get(id);
        if (!c) return;
        c.open = true;
        for (const line of c.outbox) transport().send(id, line);
        c.outbox = [];
    },
    data(id, b64) {
        const c = connections.get(id);
        if (c) c.inbox += atob(b64); // the protocol is ASCII lines
    },
    closed(id, reason) {
        const c = connections.get(id);
        if (c) {
            c.closed = true;
            if (reason) console.log(`netplay: connection closed (${reason})`);
        }
    },
};

const android = {
    connect: (id, host, port) => globalThis.PokimonNative.connect(id, host, port),
    send: (id, text) => globalThis.PokimonNative.send(id, text),
    close: (id) => globalThis.PokimonNative.close(id),
};

const ios = {
    post: (msg) => globalThis.webkit.messageHandlers.pokimonNet.postMessage(msg),
    connect(id, host, port) { this.post({ op: 'connect', id, host, port }); },
    send(id, text) { this.post({ op: 'send', id, text }); },
    close(id) { this.post({ op: 'close', id }); },
};

const sockets = new Map();
const websocket = {
    connect(id, host, port) {
        const scheme = location.protocol === 'https:' ? 'wss' : 'ws';
        const ws = new WebSocket(`${scheme}://${location.host}/net?host=${encodeURIComponent(host)}&port=${port}`);
        sockets.set(id, ws);
        ws.onopen = () => globalThis.PokimonNet.opened(id);
        ws.onmessage = (e) => {
            const c = connections.get(id);
            if (c) c.inbox += e.data;
        };
        ws.onclose = () => globalThis.PokimonNet.closed(id, 'websocket closed');
    },
    send(id, text) { sockets.get(id)?.send(text); },
    close(id) { sockets.get(id)?.close(); sockets.delete(id); },
};

export function transport() {
    if (globalThis.PokimonNative) return android;
    if (globalThis.webkit?.messageHandlers?.pokimonNet) return ios;
    return websocket;
}

export const net = {
    // The socket opens in the background; lines sent before that wait. A
    // failed connect shows up as a closed connection on the next receive.
    connect(host, port) {
        if (current) net.close();
        const c = new Connection();
        c.id = nextId++;
        connections.set(c.id, c);
        current = c;
        try {
            transport().connect(c.id, String(host), Number(port));
        } catch (e) {
            console.log('netplay: cannot connect: ' + e);
            connections.delete(c.id);
            current = null;
            return false;
        }
        console.log(`netplay: connecting to ${host}:${port}`);
        return true;
    },
    send(line) {
        const c = current;
        if (!c || c.closed) return;
        if (c.open) transport().send(c.id, line + '\n');
        else c.outbox.push(line + '\n');
    },
    // A string with what arrived, null when nothing did, false once closed.
    receive() {
        const c = current;
        if (!c) return false;
        if (c.inbox.length) {
            const data = c.inbox;
            c.inbox = '';
            return data;
        }
        return c.closed ? false : null;
    },
    close() {
        const c = current;
        if (!c) return;
        current = null;
        connections.delete(c.id);
        try { transport().close(c.id); } catch { /* already gone */ }
    },
    status() {
        if (!current) return 'offline';
        if (current.closed) return 'offline';
        return current.open ? 'online' : 'connecting';
    },
};
