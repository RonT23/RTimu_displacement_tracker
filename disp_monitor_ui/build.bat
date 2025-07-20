@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

echo Creating virtual environment...
python -m venv venv
call venv\Scripts\activate

echo Installing dependencies...
pip install --upgrade pip
pip install -r requirements.txt
pip install pyinstaller

echo Building application with PyInstaller...
pyinstaller --onefile --windowed --icon=logo.ico --noconsole rtdt.py

echo Done! Executable is in the "dist" folder.
pause
