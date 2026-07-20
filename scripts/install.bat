@echo off
setlocal

rem Builds Release and produces a clean, shareable build zipped up - just the .exe, the DLLs it
rem needs, and assets/, no .pdb/.lib/build artifacts. Handy for handing a build to someone else to
rem playtest (e.g. multiplayer over a real network, not just two processes on this machine).

set ROOT=%~dp0..
set INSTALL_DIR=%ROOT%\install\x64
set ZIP_PATH=%ROOT%\install\sdl-boilerplate-x64.zip

echo Configuring Release...
cmake --preset x64-release
if errorlevel 1 goto :error

echo Building Release...
cmake --build --preset x64-release
if errorlevel 1 goto :error

echo Installing to %INSTALL_DIR%...
cmake --install "%ROOT%\build\x64-release"
if errorlevel 1 goto :error

echo Zipping...
powershell -NoProfile -Command "Compress-Archive -Path '%INSTALL_DIR%\*' -DestinationPath '%ZIP_PATH%' -Force"
if errorlevel 1 goto :error

echo Done: %ZIP_PATH%
exit /b 0

:error
echo Failed - see output above.
exit /b 1
