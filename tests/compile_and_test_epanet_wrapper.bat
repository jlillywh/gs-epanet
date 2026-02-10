@echo off
REM Compile and test EpanetWrapper hydraulic solver methods (Task 4.2)

echo ========================================
echo Compiling EpanetWrapper Test
echo ========================================

REM Set paths
set INCLUDE_DIR=..\include
set SRC_DIR=..\src
set LIB_DIR=..\lib

REM Compile EpanetWrapper.cpp
cl /c /EHsc /std:c++17 /I"%INCLUDE_DIR%" "%SRC_DIR%\EpanetWrapper.cpp" /Fo:EpanetWrapper.obj
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile EpanetWrapper.cpp
    exit /b 1
)

REM Compile test file
cl /c /EHsc /std:c++17 /I"%INCLUDE_DIR%" test_epanet_wrapper_standalone.cpp /Fo:test_epanet_wrapper_standalone.obj
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile test_epanet_wrapper_standalone.cpp
    exit /b 1
)

REM Link executable
cl /Fe:test_epanet_wrapper_standalone.exe test_epanet_wrapper_standalone.obj EpanetWrapper.obj "%LIB_DIR%\epanet2_x64.lib"
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to link executable
    exit /b 1
)

echo.
echo ========================================
echo Running EpanetWrapper Tests
echo ========================================
echo.

REM Run the test
test_epanet_wrapper_standalone.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
if %TEST_RESULT% EQU 0 (
    echo ========================================
    echo ALL TESTS PASSED
    echo ========================================
) else (
    echo ========================================
    echo SOME TESTS FAILED
    echo ========================================
)

exit /b %TEST_RESULT%
