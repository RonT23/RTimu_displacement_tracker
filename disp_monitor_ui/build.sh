#!/bin/bash

set -e

echo "Creating virtual environment..."
python3 -m venv venv
source venv/bin/activate

echo "Installing dependencies..."
pip install --upgrade pip
pip install -r requirements.txt
pip install pyinstaller

echo "Building application with PyInstaller..."
pyinstaller --onefile --windowed --icon=logo.ico --noconsole rtdt.py

echo "Build complete. Executable in ./dist/"