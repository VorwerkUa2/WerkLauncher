@echo off
setlocal

echo ============================================
echo   WerkLauncher Installer Builder (Inno Setup)
echo ============================================
echo.

:: Try to find Inno Setup compiler (ISCC.exe)
set "ISCC="

if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    set "ISCC=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
) else if exist "C:\Program Files\Inno Setup 6\ISCC.exe" (
    set "ISCC=C:\Program Files\Inno Setup 6\ISCC.exe"
) else (
    for /f "delims=" %%I in ('where iscc 2^>nul') do set "ISCC=%%I"
)

if "%ISCC%"=="" (
    echo ERROR: Inno Setup compiler (ISCC.exe) not found! 
    echo Please install it: winget install JRSoftware.InnoSetup
    pause
    exit /b 1
)

echo Found Inno Setup at: "%ISCC%"
echo.

:: Check if Release build exists
if not exist "out\build\x64-Release\WerkLauncher.exe" (
    echo Release build not found. Building now...
    echo.
    call build_release.bat
    if %errorlevel% neq 0 (
        echo ERROR: Release build failed!
        pause
        exit /b 1
    )
    echo.
)

echo Creating installer...
"%ISCC%" "installer.iss"
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Inno Setup compilation failed!
    pause
    exit /b 1
)

echo.
echo ============================================
echo   SUCCESS! Installer created in:
echo   out\installer\WerkLauncher_Setup_1.0.1.exe
echo ============================================
pause
exit /b 0
