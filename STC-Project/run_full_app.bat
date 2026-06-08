@echo off
REM Run the full STC project locally with the built-in Python server.
REM Save this file in the project root and double-click it.

cd /d "%~dp0"

echo Starting STC Ordering System...

REM Try the Windows Python launcher first, then python.exe if available.
py server.py 2>NUL || python server.py

echo.
echo Server stopped. Press any key to close.
pause >nul
