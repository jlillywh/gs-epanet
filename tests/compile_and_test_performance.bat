@echo off
REM Compile and run performance property tests
REM This script requires Visual Studio Developer Command Prompt

echo ========================================
echo Compiling Performance Property Tests
echo ========================================

REM Ensure we're in the tests directory
cd /d %~dp0

REM Create output directory if it doesn't exist
if not exist "property_tests" mkdir property_tests

REM Compile the performance test
cl.exe /EHsc /std:c++17 /I..\include /I..\src ^
   property_tests\test_performance_properties.cpp ^
   /link /LIBPATH:..\lib epanet2.lib Psapi.lib ^
   /OUT:property_tests\test_performance_properties.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo Compilation failed!
    echo Make sure you run this from Visual Studio Developer Command Prompt
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo Running Performance Property Tests
echo ========================================
echo.

REM Copy required DLLs to property_tests directory
copy ..\lib\epanet2.dll property_tests\ >nul 2>&1
copy gs_epanet.dll property_tests\ >nul 2>&1
copy ..\bin\x64\Debug\gs_epanet.dll property_tests\ >nul 2>&1

REM Copy test data
if not exist "property_tests\test_data" mkdir property_tests\test_data
copy test_data\*.* property_tests\test_data\ >nul 2>&1

REM Run the tests from property_tests directory
cd property_tests
test_performance_properties.exe
set TEST_RESULT=%ERRORLEVEL%
cd ..

echo.
if %TEST_RESULT% EQU 0 (
    echo ========================================
    echo Performance Tests Completed
    echo ========================================
) else (
    echo ========================================
    echo Performance Tests Failed
    echo ========================================
)

exit /b %TEST_RESULT%
