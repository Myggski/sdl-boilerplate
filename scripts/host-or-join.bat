@echo off
setlocal

rem Local multiplayer test helper. Run this once to host; run it again (any number of times, even
rem from other terminals/windows) to join as another client. A lock file in %TEMP% tracks whether
rem a host from this script is already running, so the first run becomes the host and every run
rem after that joins it - no need to remember --host/--connect or pick a port yourself.
rem
rem If a previous host crashed or was force-killed instead of closing normally, the lock file is
rem left behind and this script will try to join a host that no longer exists. Delete
rem %TEMP%\sdl-boilerplate-host.lock by hand in that case, then run this again.

set PORT=27015
rem Change x64-debug to x64-release here to test that build instead.
set BUILD_DIR=%~dp0..\build\x64-debug
set EXE=%BUILD_DIR%\sdl-boilerplate.exe
set LOCK=%TEMP%\sdl-boilerplate-host.lock

if not exist "%EXE%" (
    echo Could not find %EXE% - build that preset first.
    exit /b 1
)

rem Assets are loaded via relative paths (assets/images/...), so the exe needs to actually run
rem with the build directory as its working directory - launching it by path alone from here
rem would leave the working directory wherever this script itself was invoked from instead.
pushd "%BUILD_DIR%"

if not exist "%LOCK%" (
    echo Starting as HOST on port %PORT%...
    type nul > "%LOCK%"
    "%EXE%" --host %PORT%
    del "%LOCK%"
) else (
    echo Joining as CLIENT - connecting to 127.0.0.1:%PORT%...
    "%EXE%" --connect 127.0.0.1 %PORT%
)

popd
