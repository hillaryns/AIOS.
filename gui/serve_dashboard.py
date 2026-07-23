#!/usr/bin/env python3
"""
AIOS GUI Dashboard Server — Phase 9 Implementation
===================================================
Serves the AIOS Desktop GUI and Memory Dashboard at http://localhost:8080.
Binds strictly to 127.0.0.1 for local privacy guarantee.
"""

import os
import sys
import http.server
import socketserver
import webbrowser
from pathlib import Path

PORT = 8080
HOST = "127.0.0.1"
GUI_DIR = Path(__file__).parent.resolve()

class AIOSHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(GUI_DIR), **kwargs)

    def log_message(self, format, *args):
        # Quiet logger for clean terminal output
        sys.stdout.write(f"[AIOS GUI Server] {self.address_string()} - {format % args}\n")

def run_server(open_browser=False):
    os.chdir(GUI_DIR)
    handler = AIOSHTTPRequestHandler
    socketserver.TCPServer.allow_reuse_address = True
    
    with socketserver.TCPServer((HOST, PORT), handler) as httpd:
        url = f"http://{HOST}:{PORT}"
        print(f"==================================================")
        print(f"  AIOS Desktop GUI & Dashboard Server Running")
        print(f"  URL: {url}")
        print(f"  Dashboard: {url}/dashboard/index.html")
        print(f"  Bound to: {HOST} (Localhost Only)")
        print(f"==================================================")
        
        if open_browser:
            webbrowser.open(url)
            
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[AIOS GUI Server] Shutting down gracefully...")

if __name__ == "__main__":
    open_b = "--open" in sys.argv
    run_server(open_browser=open_b)
