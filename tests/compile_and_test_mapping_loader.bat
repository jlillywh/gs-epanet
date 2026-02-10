@echo off
REM Compile and test MappingLoader standalone test
REM This script compiles the test without requiring the full Visual Studio project

echo ========================================
echo Compiling MappingLoader Test
echo ========================================

REM Set compiler path (adjust if needed)
set CL_PATH=cl.exe

REM Compile the test (now includes EpanetWrapper for ResolveIndices tests)
%CL_PATH% /EHsc /std:c++17 /I..\include /I..\src test_mapping_loader_standalone.cpp ..\src\MappingLoader.cpp ..\src\EpanetWrapper.cpp /link /LIBPATH:..\lib epanet2.lib /Fe:test_mapping_loader_standalone.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo Compilation FAILED!
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo Compilation successful!
echo ========================================
echo.

REM Run the test
echo ========================================
echo Running MappingLoader Test
echo ========================================
echo.

test_mapping_loader_standalone.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo Tests FAILED!
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo All tests PASSED!
echo ========================================

exit /b 0
