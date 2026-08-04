@echo off
setlocal enabledelayedexpansion

echo ================================================
echo    SHAR ModLoader - Build Script
echo ================================================
echo.
echo Requires: Visual Studio 2022 (17.6+) and CMake 3.25+
echo Output:   build\bin\SHARModLoader.asi
echo.

where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found. Install from https://cmake.org/download/
    pause
    exit /b 1
)

echo [1/4] CMake version:
cmake --version | findstr /C:"version"
echo.

echo [2/4] Detecting Visual Studio...

set VS_GENERATOR=

if exist "C:\Program Files\Microsoft Visual Studio\2022" (
    set VS_GENERATOR=Visual Studio 17 2022
    echo Found: Visual Studio 2022
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022" (
    set VS_GENERATOR=Visual Studio 17 2022
    echo Found: Visual Studio 2022
) else if exist "C:\Program Files\Microsoft Visual Studio\2019" (
    set VS_GENERATOR=Visual Studio 16 2019
    echo Found: Visual Studio 2019 (VS 2022 recommended for C++23)
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019" (
    set VS_GENERATOR=Visual Studio 16 2019
    echo Found: Visual Studio 2019 (VS 2022 recommended for C++23)
)

if not defined VS_GENERATOR (
    echo ERROR: Visual Studio 2019 or 2022 not found.
    pause
    exit /b 1
)

echo.
echo [3/4] Configuring (Win32)...
if exist build rmdir /S /Q build
mkdir build && cd build

cmake .. -G "%VS_GENERATOR%" -A Win32
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configure failed.
    cd .. && pause && exit /b 1
)

echo.
echo [4/4] Building...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed.
    cd .. && pause && exit /b 1
)

cd ..

echo.
echo ================================================
echo    Done — build\bin\SHARModLoader.asi
echo ================================================
echo.
echo Copy SHARModLoader.asi into your SHAR game folder.
echo ASI Ultimate Loader will pick it up automatically.
echo.
pause
