@echo off
echo ================================================================
echo Building V.I.I.B.E CS 1.6 Modular Suite (MinGW32)
echo ================================================================

if not exist "build" mkdir "build"

echo [*] Compiling cs16_esp_internal.dll...
g++.exe -shared -O2 -m32 -static-libgcc -static-libstdc++ ^
    dllmain.cpp ^
    src\core\math.cpp ^
    src\core\logger.cpp ^
    src\core\input.cpp ^
    src\core\framework.cpp ^
    src\render\font.cpp ^
    src\render\primitives.cpp ^
    src\render\renderer.cpp ^
    src\engine\memory.cpp ^
    src\engine\resolver.cpp ^
    src\engine\studio.cpp ^
    src\engine\entity.cpp ^
    src\engine\player.cpp ^
    src\engine\engine.cpp ^
    src\hooks\hooks.cpp ^
    src\features\esp.cpp ^
    src\features\radar.cpp ^
    src\features\misc.cpp ^
    src\features\aimbot.cpp ^
    src\features\config.cpp ^
    src\features\menu\widgets.cpp ^
    src\features\menu\hud.cpp ^
    src\features\menu\tabs.cpp ^
    src\features\menu.cpp ^
    -o build\cs16_esp_internal.dll ^
    -lopengl32 -lgdi32 -luser32 -lpsapi

if %ERRORLEVEL% NEQ 0 (
    echo [-] DLL Build Failed!
    exit /b %ERRORLEVEL%
)

echo [+] DLL Build Succeeded: build\cs16_esp_internal.dll

echo [*] Compiling injector.exe...
g++.exe -O2 -m32 -static-libgcc -static-libstdc++ injector.cpp -o build\injector.exe

if %ERRORLEVEL% NEQ 0 (
    echo [-] Injector Build Failed!
    exit /b %ERRORLEVEL%
)

echo [+] Injector Build Succeeded: build\injector.exe
echo ================================================================
echo Build Complete! Outputs placed in build\ directory.
echo ================================================================
