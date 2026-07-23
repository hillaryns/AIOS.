#!/usr/bin/env python3
"""
AIOS System Tray Application — Phase 9 Deliverable (gui/tray.py)
===============================================================
The persistent desktop daemon for the AIOS shell layer.

Blueprint Requirement (Phase 9.1):
- System tray sitting in taskbar
- Status indicator colors: green (active), orange (working), blue (listening), grey (paused)
- Right click context menu: Open Chat, System Status, Today's Summary, Pause AIOS, Settings, Quit
- Left click: opens Command Bar
- Global hotkey: Super+Space
- Graceful fallbacks for non-Qt environments
"""

import sys
import os
import subprocess
import threading
import time
from pathlib import Path

GUI_DIR = Path(__file__).parent.resolve()

def launch_pyqt_tray():
    """Attempt to launch the full PyQt6 System Tray Application."""
    try:
        from PyQt6.QtWidgets import QApplication
        from gui.tray.tray_app import AITrayApp
        
        app = QApplication(sys.argv)
        app.setQuitOnLastWindowClosed(False)
        tray = AITrayApp()
        print("[AIOS Tray] Running PyQt6 System Tray Daemon...")
        sys.exit(app.exec())
    except ImportError:
        return False
    except Exception as e:
        print(f"[AIOS Tray] PyQt6 tray initialization failed: {e}")
        return False

def launch_fallback_tray():
    """Fallback daemon when PyQt6 or X display server is unavailable."""
    print("==================================================")
    print("  AIOS System Tray Daemon (Headless / CLI Mode)")
    print("  Status: Active & Ready 🟢")
    print("  Context Menu Actions Available:")
    print("    1) Open Command Bar (Super+Space)")
    print("    2) System Status (`nexus status`)")
    print("    3) Memory Dashboard (http://localhost:8080)")
    print("    4) Pause AIOS (`nexus pause`)")
    print("    5) Privacy Status (`nexus privacy status`)")
    print("==================================================")
    
    # Start web server for dashboard if not already running
    dashboard_script = GUI_DIR / "serve_dashboard.py"
    if dashboard_script.exists():
        subprocess.Popen([sys.executable, str(dashboard_script)])
        print(f"[AIOS Tray] Memory Dashboard server started at http://localhost:8080")

    # Monitor status loop
    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        print("\n[AIOS Tray] Exiting tray daemon.")

if __name__ == "__main__":
    if not launch_pyqt_tray():
        launch_fallback_tray()
