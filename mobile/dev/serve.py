#!/usr/bin/env python3
"""Serves mobile/dist/ to a desktop browser, for trying the phone app
without a phone. Browsers can't open TCP sockets, so the app's connection to
the netplay server goes through a WebSocket here (/net?host=..&port=..),
which this script relays to the server. The phone apps don't need this:
they open the TCP socket themselves.

    python3 mobile/dev/serve.py [--port 8090] [--dist mobile/dist]

Standard library only.
"""

import argparse
import base64
import hashlib
import http.server
import os
import socket
import struct
import threading
import urllib.parse

WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


def ws_recv(sock):
    """One text frame's payload, or None when the browser closed."""
    def exact(n):
        data = b""
        while len(data) < n:
            chunk = sock.recv(n - len(data))
            if not chunk:
                raise ConnectionError
            data += chunk
        return data

    message = b""
    while True:
        b0, b1 = exact(2)
        opcode, length = b0 & 0x0F, b1 & 0x7F
        if length == 126:
            length = struct.unpack(">H", exact(2))[0]
        elif length == 127:
            length = struct.unpack(">Q", exact(8))[0]
        mask = exact(4) if b1 & 0x80 else b"\0\0\0\0"
        payload = bytes(c ^ mask[i % 4] for i, c in enumerate(exact(length)))
        if opcode == 8:
            return None
        if opcode in (0, 1, 2):
            message += payload
            if b0 & 0x80:
                return message
        # ping/pong frames are ignored


def ws_send(sock, data):
    header = bytes([0x81])
    if len(data) < 126:
        header += bytes([len(data)])
    elif len(data) < 65536:
        header += bytes([126]) + struct.pack(">H", len(data))
    else:
        header += bytes([127]) + struct.pack(">Q", len(data))
    sock.sendall(header + data)


class Handler(http.server.SimpleHTTPRequestHandler):
    extensions_map = {**http.server.SimpleHTTPRequestHandler.extensions_map,
                      ".wasm": "application/wasm", ".js": "text/javascript", ".lua": "text/plain"}

    def log_message(self, fmt, *args):
        pass

    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def do_GET(self):
        url = urllib.parse.urlparse(self.path)
        if url.path == "/net" and self.headers.get("Upgrade", "").lower() == "websocket":
            return self.relay(urllib.parse.parse_qs(url.query))
        return super().do_GET()

    def relay(self, query):
        host = query.get("host", ["127.0.0.1"])[0]
        port = int(query.get("port", ["7777"])[0])
        try:
            upstream = socket.create_connection((host, port), timeout=5)
            upstream.settimeout(None)
        except OSError:
            self.send_error(502, "cannot reach the netplay server")
            return
        accept = base64.b64encode(hashlib.sha1((self.headers["Sec-WebSocket-Key"] + WS_GUID).encode()).digest())
        self.send_response(101)
        self.send_header("Upgrade", "websocket")
        self.send_header("Connection", "Upgrade")
        self.send_header("Sec-WebSocket-Accept", accept.decode())
        self.end_headers()
        browser = self.connection

        def upstream_to_browser():
            try:
                while True:
                    data = upstream.recv(65536)
                    if not data:
                        break
                    ws_send(browser, data)
            except OSError:
                pass
            finally:
                try:
                    browser.sendall(b"\x88\x00")
                except OSError:
                    pass
                browser.close()

        threading.Thread(target=upstream_to_browser, daemon=True).start()
        try:
            while True:
                data = ws_recv(browser)
                if data is None:
                    break
                upstream.sendall(data)
        except (OSError, ConnectionError):
            pass
        finally:
            upstream.close()
        self.close_connection = True


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--port", type=int, default=8090)
    parser.add_argument("--dist", default=os.path.join(here, "..", "dist"))
    args = parser.parse_args()
    os.chdir(args.dist)
    server = http.server.ThreadingHTTPServer(("0.0.0.0", args.port), Handler)
    print(f"phone app on http://localhost:{args.port}/")
    server.serve_forever()


if __name__ == "__main__":
    main()
