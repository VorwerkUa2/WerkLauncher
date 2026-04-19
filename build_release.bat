@echo off
setlocal

set "VCVARSALL=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat"

:: Disable Build Insights collection
set "VCPERF_COLLECTOR_ENABLED=0"
set "MSVC_BUILD_INSIGHTS_DISABLED=1"

if not exist "%VCVARSALL%" (
    echo Error: vcvarsall.bat not found at "%VCVARSALL%"
    exit /b 1
)

call "%VCVARSALL%" x64
if %errorlevel% neq 0 exit /b %errorlevel%

echo Configuring Release build...
cmake -S . -B out/build/x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/Users/knyaz/vcpkg/scripts/buildsystems/vcpkg.cmake
if %errorlevel% neq 0 exit /b %errorlevel%

echo Building Release...
cmake --build out/build/x64-Release
if %errorlevel% neq 0 exit /b %errorlevel%

echo.
echo Release build successful!
echo Output: out\build\x64-Release\WerkLauncher.exe
exit /b 0
