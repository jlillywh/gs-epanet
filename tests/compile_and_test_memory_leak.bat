@echo off
REM Compile and run memory leak verification test
REM This test validates proper memory management and resource cleanup

echo ========================================
echo Compiling Memory Leak Verification Test
echo ========================================

REM Compile the test
cl /EHsc /std:c++17 /I..\include /I..\src ^
   test_memory_leak_verification.cpp ^
   ..\src\EpanetBridge.cpp ^
   ..\src\EpanetWrapper.cpp ^
   ..\src\MappingLoader.cpp ^
   ..\src\Logger.cpp ^
   /link ..\lib\epanet2.lib ^
   psapi.lib ^
   /OUT:test_memory_leak_verification.exe

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    exit /b 1
)

echo.
echo ========================================
echo Running Memory Leak Verification Test
echo ========================================
echo.

REM Copy required files to test directory
copy ..\lib\epanet2.dll . >nul 2>&1
copy test_data\simple_test.inp . >nul 2>&1
copy test_data\valid_config.json EpanetBridge.json >nul 2>&1

REM Run the test
test_memory_leak_verification.exe

set TEST_RESULT=%ERRORLEVEL%

echo.
if %TEST_RESULT% EQU 0 (
    echo ========================================
    echo Memory Leak Verification: PASSED
    echo ========================================
) else (
    echo ========================================
    echo Memory Leak Verification: FAILED
    echo ========================================
)

exit /b %TEST_RESULT%
