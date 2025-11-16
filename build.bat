@echo off
REM FileVault Build Script for Windows (MSYS2/UCRT64)

echo.
echo ====================================
echo FileVault Build Script
echo ====================================
echo.

REM Create build directory
if not exist "build" mkdir build
cd build

REM Step 1: Install dependencies with Conan
echo [1/3] Installing dependencies...
conan install .. --output-folder=. --build=missing -s build_type=Release -s compiler.cppstd=20

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Conan install failed!
    pause
    exit /b 1
)

echo.
echo [2/3] Configuring CMake...
cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=build\Release\generators\conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo [3/3] Building...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ====================================
echo Build completed successfully!
echo Executable: build\bin\filevault.exe
echo ====================================
echo.

cd ..
pause
