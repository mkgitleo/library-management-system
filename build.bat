@echo off
REM Build script for Library Management System (Windows + MinGW)

set SRC=src\lib_management_sys_sqlite3.cpp
set OUT=build\library_management_system.exe

if not exist build (
    mkdir build
)

g++ -std=c++17 -O2 -Wall "%SRC%" -o "%OUT%" -lsqlite3

if %errorlevel% neq 0 (
    echo.
    echo Build FAILED.
    pause
    exit /b 1
)

echo.
echo Build SUCCESSFUL. Output: %OUT%
pause
