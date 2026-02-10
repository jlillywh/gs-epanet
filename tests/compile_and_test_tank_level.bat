@echo off
REM Compile and run the tank level validation test

echo ========================================
echo Compiling Tank Level Validation Test
echo ========================================
echo.

REM Setup test environment
echo Copying required files...
copy /Y ..\bin\x64\Release\gs_epanet.dll . >nul 2>&1
copy /Y ..\lib\epanet2.dll . >nul 2>&1
copy /Y ..\examples\Pump_to_Tank\Pump_to_Tank.inp . >nul 2>&1
copy /Y ..\examples\Pump_to_Tank\EpanetBridge.json . >nul 2>&1

if not exist gs_epanet.dll (
    echo ERROR: gs_epanet.dll not found. Build the project first with:
    echo   ..\build.ps1
    exit /b 1
)

echo Compiling test_tank_level_validation.cpp...
cl /EHsc /MD /W3 /O2 /Fe:test_tank_level_validation.exe test_tank_level_validation.cpp /link /SUBSYSTEM:CONSOLE

if errorlevel 1 (
    echo.
    echo ERROR: Compilation failed
    echo Make sure you're running from a Visual Studio Developer Command Prompt
    exit /b 1
)

echo Compilation successful.
echo.
echo ========================================
echo Running Tank Level Validation Test
echo ========================================
echo.

test_tank_level_validation.exe

set TEST_RESULT=%ERRORLEVEL%

echo.
if %TEST_RESULT% EQU 0 (
    echo ========================================
    echo TEST PASSED
    echo ========================================
) else (
    echo ========================================
    echo TEST FAILED
    echo ========================================
)

exit /b %TEST_RESULT%
