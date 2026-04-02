@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo Pixeon Image Viewer Build and Run Script
echo ========================================================

:: Standard Qt installation paths
set "QT_DIR=C:\Qt\6.11.0\mingw_64"
set "MINGW_DIR=C:\Qt\Tools\mingw1310_64\bin"
set "CMAKE_DIR=C:\Qt\Tools\CMake_64\bin"
set "NINJA_DIR=C:\Qt\Tools\Ninja"

IF NOT EXIST "%QT_DIR%" (
    echo Error: Could not find Qt at "%QT_DIR%"
    echo Please make sure you installed Qt 6.11.0 with MinGW 13.1.0 64-bit through Qt Maintenance Tool.
    pause
    exit /b 1
)

IF NOT EXIST "%MINGW_DIR%\g++.exe" (
    echo Error: Could not find MinGW compiler at "%MINGW_DIR%"
    pause
    exit /b 1
)

:: Add Qt and Tools to PATH
set "PATH=%MINGW_DIR%;%CMAKE_DIR%;%NINJA_DIR%;%QT_DIR%\bin;%PATH%"

echo.
echo [1/3] Configuring the project with CMake...
if not exist "build" mkdir build
cd build

cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="%QT_DIR%" -DCMAKE_CXX_COMPILER="%MINGW_DIR%\g++.exe" ..
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b %errorlevel%
)

echo.
echo [2/3] Building the project...
cmake --build . -j 4
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo.
echo [3/3] Running the application...
:: The executable name is PixeonImageViewer.exe
start "" PixeonImageViewer.exe

cd ..
echo Launch successful!
exit /b 0
