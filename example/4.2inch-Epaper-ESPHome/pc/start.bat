@echo off
cd /d "%~dp0"
if not exist ".venv\Scripts\python.exe" (
  echo Virtual environment missing. Run setup.bat first.
  pause
  exit /b 1
)
".venv\Scripts\python.exe" server.py
pause
