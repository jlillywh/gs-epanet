@echo off
REM Compile and test pump POWER and EFFICIENCY properties
REM This test validates that the bridge correctly retrieves pump power and efficiency

echo ========================================
echo Compiling test_pump_properties.cpp
echo ========================================

REM Clean up old files
if exist test_pump_properties.exe del test_pump_properties.exe
if exist test_pump_properties.obj del test_pump_properties.obj
if exist EpanetBridge.json del EpanetBridge.json

REM Copy the test configuration
copy test_data\pump_properties_config.json EpanetBridge.json

REM Compile the test
cl /EHsc /std:c++17 /I..\include ^
   test_pump_properties.cpp ^
   EpanetBridge.obj ^
   EpanetWrapper.obj ^
   MappingLoader.obj ^
   Logger.obj ^
   /link /OUT:test_pump_properties.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo COMPILATION FAILED
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo Running test_pump_properties.exe
echo ========================================
echo.

test_pump_properties.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo ALL TESTS PASSED
    echo ========================================
    exit /b 0
) else (
    echo.
    echo ========================================
    echo TESTS FAILED
    echo ========================================
    exit /b 1
)
