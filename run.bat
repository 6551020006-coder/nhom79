@echo off
title Build & Run Game
echo ========================================
echo   Dang build game...
echo ========================================

set RAYLIB=C:\raylib\raylib
set GPP=C:\raylib\w64devkit\bin\g++.exe
set PATH=C:\raylib\w64devkit\bin;%PATH%

"%GPP%" src/main.cpp src/game1.cpp src/spaceship.cpp src/laser.cpp src/boss.cpp src/powerup.cpp ^
    -o game1.exe ^
    -I"%RAYLIB%\src" ^
    -L"%RAYLIB%\src" ^
    -lraylib -lopengl32 -lgdi32 -lwinmm ^
    -std=c++14

if %ERRORLEVEL% == 0 (
    echo [OK] Build thanh cong! Dang khoi dong game...
    echo ========================================
    start "" game1.exe
) else (
    echo.
    echo [LOI] Build that bai! Kiem tra code.
    pause
)
