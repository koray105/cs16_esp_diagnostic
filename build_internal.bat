@echo off
setlocal enabledelayedexpansion

echo ==================================================
echo [V.I.I.B.E Build Engine] Modular CS 1.6 Suite v3.0
echo ==================================================
echo.

set "COMPILER_DIR=C:\Users\pc\.gemini\antigravity\scratch\w64devkit\bin"
if not exist "!COMPILER_DIR!\g++.exe" (
    echo [ERROR] MinGW GCC not found in !COMPILER_DIR!
    pause
    exit /b 1
)

set "PATH=!COMPILER_DIR!;%PATH%"

set "SOURCES=dllmain.cpp src\core\math.cpp src\core\logger.cpp src\render\renderer.cpp src\engine\engine.cpp src\hooks\hooks.cpp src\features\esp.cpp src\features\radar.cpp src\features\misc.cpp src\features\aimbot.cpp src\features\config.cpp src\features\menu.cpp"

echo [1/2] Compiling cs16_esp_internal.dll...
if exist cs16_esp_internal.dll (
    del /f /q cs16_esp_internal.dll 2>nul
    if exist cs16_esp_internal.dll (
        echo [WARNING] cs16_esp_internal.dll is in use by a process! Close game before compiling.
    )
)
g++.exe -shared -O2 -m32 -static-libgcc -static-libstdc++ !SOURCES! -o cs16_esp_internal.dll -lopengl32 -lgdi32 -luser32 -lpsapi
if %errorlevel% neq 0 (
    echo [ERROR] Modular DLL build failed!
    pause
    exit /b 1
)
echo [OK] cs16_esp_internal.dll
echo.

echo [2/2] Compiling Injector...
g++.exe -O2 -m32 -static-libgcc -static-libstdc++ injector.cpp -o injector.exe
if %errorlevel% neq 0 (
    echo [ERROR] Injector build failed!
    pause
    exit /b 1
)
echo [OK] injector.exe
echo.

echo ==================================================
echo [SUCCESS] Modular Suite v3.0 build complete!
echo ==================================================
echo   Target DLL: cs16_esp_internal.dll
echo   Injector:   injector.exe
echo.
echo Architecture:
echo   [src/sdk]      - GoldSrc structures, entity types ^& MenuState
echo   [src/core]     - Math (W2S, WorldToRadar) ^& Telemetry Logger
echo   [src/render]   - OpenGL 2D batching, primitives ^& font rasterizer
echo   [src/engine]   - Engine resolver, player reader ^& world entity scanner
echo   [src/hooks]    - Safe hw.dll dispatch slot hooks ^& wglSwap
echo   [src/features] - ESP, 2D Radar, Misc (Bhop/C4/FOV), Config ^& Menu
echo ==================================================
