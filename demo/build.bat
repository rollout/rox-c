@echo off
setlocal enableextensions

set ROX_SDK_ROOT=C:\Program Files\rollout-sdk
SET ROOT_DIR=%cd%

if "%ROX_SDK_ROOT%" == "" set ROX_SDK_ROOT="%ROX_SDK_ROOT%"

echo Looking for Rollout SDK in %ROX_SDK_ROOT% (specify ROX_SDK_ROOT variable to override it)
if not exist "%ROX_SDK_ROOT%" (
    echo Directory %ROX_SDK_ROOT% not exists. Existing.
    exit /b -1
)

if not exist .\build\release md .\build\release
cd .\build\release

echo "Building Rollout SDK demo..."
cmake ..\.. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DROX_SDK_ROOT="%ROX_SDK_ROOT%"
if %errorlevel% neq 0 exit /b %errorlevel%

nmake
if %errorlevel% neq 0 exit /b %errorlevel%

cd "%ROOT_DIR%"
echo "Rollout SDK demo build finished."

endlocal