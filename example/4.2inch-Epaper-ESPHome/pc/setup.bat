@echo off
cd /d "%~dp0"
py -3.14 -m venv .venv
if errorlevel 1 goto failed
".venv\Scripts\python.exe" -m pip install -r requirements.txt
if errorlevel 1 goto failed
echo Setup complete. Run start-device.bat, then open http://127.0.0.1:8099
pause
exit /b 0
:failed
echo Setup failed. Check the error above. Python 3.14 x64 is required by this guide.
pause
exit /b 1
