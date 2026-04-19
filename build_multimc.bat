@echo off
set "VCVARSALL=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat"

:: Disable Build Insights collection if it was somehow enabled
set "VCPERF_COLLECTOR_ENABLED=0"
set "MSVC_BUILD_INSIGHTS_DISABLED=1"

if not exist "%VCVARSALL%" (
    echo Error: vcvarsall.bat not found at "%VCVARSALL%"
    exit /b 1
)

call "%VCVARSALL%" x64
if %errorlevel% neq 0 exit /b %errorlevel%

echo Configuring...
cmake -S . -B out/build/x64-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=C:/Users/knyaz/vcpkg/scripts/buildsystems/vcpkg.cmake
if %errorlevel% neq 0 exit /b %errorlevel%

echo Building...
cmake --build out/build/x64-Debug
if %errorlevel% neq 0 exit /b %errorlevel%

echo Build successful!
exit /b 0
