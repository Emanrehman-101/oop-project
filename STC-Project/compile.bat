@echo off
echo ===================================================
echo Compiling C++ CGI Backend for STC Ordering System
echo ===================================================

if not exist cgi-bin mkdir cgi-bin

:: 1. Check if g++ is available in path
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [INFO] g++ compiler found in PATH. Compiling...
    g++ -O2 -std=c++11 backend/backend.cpp -o cgi-bin/backend.exe
    goto check_result
)

:: 2. Check if cl.exe is already available in path
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [INFO] cl.exe compiler found in PATH. Compiling...
    cl /EHsc /O2 backend/backend.cpp /Focgi-bin/backend.obj /Fecgi-bin/backend.exe
    goto check_result
)

:: 3. Check for Visual Studio Build Tools
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    echo [INFO] Visual Studio 2022 Build Tools found.
    echo [INFO] Configuring environment for cl.exe...
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
    echo [INFO] Compiling C++ backend using cl.exe...
    cl /EHsc /O2 backend/backend.cpp /Focgi-bin/backend.obj /Fecgi-bin/backend.exe
    goto check_result
)

:: 4. If nothing found, error out
echo [ERROR] No C++ compiler found (g++ or MSVC cl.exe)!
echo Please install MinGW (g++) or Visual Studio C++ Build Tools.
exit /b 1

:check_result
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation FAILED!
    exit /b %ERRORLEVEL%
)
echo ===================================================
echo [SUCCESS] Compiled binary written to cgi-bin/backend.exe.
echo ===================================================
