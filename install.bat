@echo off

set DEFAULT_INSTALL_DIR=C:\Program Files\rollout-sdk
set INSTALL_DIR=%DEFAULT_INSTALL_DIR%
set PROJECT_NAME=ROX C SDK
SET ROOT_DIR=%cd%

if not "%1" == "" set INSTALL_DIR="%1"
REM TODO: check if INSTALL_DIR is writable

echo Building third party libraries.
if not exist .\vendor\build mkdir .\vendor\build
cd .\vendor\build

cmake .. -G "NMake Makefiles"
if %errorlevel% neq 0 exit /b %errorlevel%

nmake
if %errorlevel% neq 0 exit /b %errorlevel%

cd %ROOT_DIR%

echo Building %PROJECT_NAME%.
if not exist .\build\release mkdir .\build\release
cd .\build\release

cmake ..\.. -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 exit /b %errorlevel%

nmake
if %errorlevel% neq 0 exit /b %errorlevel%

echo Running tests.
ctest
if %errorlevel% neq 0 exit /b %errorlevel%

echo Installing %PROJECT_NAME% into %INSTALL_DIR%.
nmake install
if %errorlevel% neq 0 exit /b %errorlevel%

cd "%ROOT_DIR%"

REM TODO: add to PATH like this: (current version cuts PATH to 1024 chars)
REM if "%INSTALL_DIR%" == "%DEFAULT_INSTALL_DIR%" setx PATH "%PATH%;%INSTALL_DIR%\bin" /M

echo %PROJECT_NAME% is successfully installed into %INSTALL_DIR%.
echo %PROJECT_NAME% build finished.
