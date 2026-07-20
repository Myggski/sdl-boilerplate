@echo off
setlocal

rem Full clean rebuild of both Debug and Release. Cheap enough to run casually (~10-15s each
rem thanks to vcpkg-cached dependencies + sccache) - not the multi-minute ordeal it used to be
rem before those were wired in, so this is worth reaching for whenever you want to confirm both
rem configs still build cleanly from scratch, not just incrementally.

set ROOT=%~dp0..

echo Stopping any running instance (would otherwise lock build files)...
taskkill /IM sdl-boilerplate.exe /F >nul 2>nul

echo Cleaning build\x64-debug and build\x64-release...
rmdir /s /q "%ROOT%\build\x64-debug" 2>nul
rmdir /s /q "%ROOT%\build\x64-release" 2>nul

echo Configuring + building Debug...
cmake --preset x64-debug
if errorlevel 1 goto :error
cmake --build --preset x64-debug
if errorlevel 1 goto :error

echo Configuring + building Release...
cmake --preset x64-release
if errorlevel 1 goto :error
cmake --build --preset x64-release
if errorlevel 1 goto :error

echo Done - both configs rebuilt cleanly.
exit /b 0

:error
echo Failed - see output above.
exit /b 1
