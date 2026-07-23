#!/usr/bin/env python3
"""
AIOS Spotlight Command Bar — Phase 9 Deliverable (gui/command_bar.py)
=====================================================================
Floating Spotlight-style AI command overlay (Super+Space).

Blueprint Requirement (Phase 9.2):
- Centered rounded dark rectangle overlaying current window
- Natural language queries + structured commands (`nexus mode coding`, etc.)
- Real-time suggestions & streaming AI response inline
- Hotkey: Super+Space / Escape to close
"""

import sys
import os
import subprocess
from pathlib import Path

GUI_DIR = Path(__file__).parent.resolve()

def launch_pyqt_bar():
    """Attempt to launch the PyQt6 Spotlight Command Bar."""
    try:
        from PyQt6.QtWidgets import QApplication
        from gui.command_bar.nexus_bar import NexusBar
        
        app = QApplication(sys.argv)
        bar = NexusBar()
        bar.show_bar()
        print("[AIOS Command Bar] Launched PyQt6 Spotlight Bar.")
        sys.exit(app.exec())
    except ImportError:
        return False
    except Exception as e:
        print(f"[AIOS Command Bar] PyQt6 launch error: {e}")
        return False

def launch_cli_bar():
    """Interactive CLI Command Bar overlay for terminal/web execution."""
    print("\n==================================================")
    print("  ⦿  AIOS NEXUS COMMAND BAR (Interactive CLI)")
    print("  Type your question, search memory, or run command.")
    print("  Press Ctrl+C or type 'exit' to close.")
    print("==================================================\n")
    
    import urllib.request
    import json
    
    API_URL = "http://localhost:11435"
    
    while True:
        try:
            query = input("\n⦿ AIOS > ").strip()
            if not query:
                continue
            if query.lower() in ("exit", "quit", "q"):
                break
                
            print("\n[AIOS Thinking...]")
            # Attempt call to Nexus API or local handling
            try:
                req = urllib.request.Request(
                    f"{API_URL}/api/chat",
                    data=json.dumps({"message": query}).encode("utf-8"),
                    headers={"Content-Type": "application/json"}
                )
                with urllib.request.urlopen(req, timeout=10) as resp:
                    data = json.loads(resp.read().decode())
                    response_text = data.get("response", data.get("content", str(data)))
                    print(f"\n🤖 Response:\n{response_text}")
            except Exception:
                print(f"\n⚡ [Nexus API Offline] Query received: '{query}'")
                print(f"   To connect live model, start Nexus daemon: `nexus start`")
                print(f"   Opening Web Command Overlay at http://localhost:8080...")
                subprocess.Popen(["python3", str(GUI_DIR / "serve_dashboard.py"), "--open"])
                break
        except (KeyboardInterrupt, EOFError):
            print("\n[Command Bar Closed]")
            break

if __name__ == "__main__":
    if not launch_pyqt_bar():
        launch_cli_bar()
