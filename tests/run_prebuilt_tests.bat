@echo off
REM Run all pre-built tests for EPANET-GoldSim Bridge
REM This script runs existing test executables without recompilation

echo ========================================
echo EPANET-GoldSim Bridge Test Suite
echo (Using Pre-Built Executables)
echo ========================================
echo.

set TOTAL_TESTS=0
set PASSED_TESTS=0
set FAILED_TESTS=0

REM Test 1: Logger Component
echo [1/4] Running Logger Tests...
if exist test_logger_standalone.exe (
    test_logger_standalone.exe >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        set /a PASSED_TESTS+=1
        echo [PASS] Logger Tests
    ) else (
        set /a FAILED_TESTS+=1
        echo [FAIL] Logger Tests
    )
    set /a TOTAL_TESTS+=1
) else (
    echo [SKIP] Logger Tests (executable not found)
)
echo.

REM Test 2: MappingLoader Component
echo [2/4] Running MappingLoader Tests...
if exist test_mapping_loader_standalone.exe (
    test_mapping_loader_standalone.exe >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        set /a PASSED_TESTS+=1
        echo [PASS] MappingLoader Tests
    ) else (
        set /a FAILED_TESTS+=1
        echo [FAIL] MappingLoader Tests
    )
    set /a TOTAL_TESTS+=1
) else (
    echo [SKIP] MappingLoader Tests (executable not found)
)
echo.

REM Test 3: XF_REP_ARGUMENTS Handler
echo [3/4] Running XF_REP_ARGUMENTS Tests...
if exist test_xf_rep_arguments.exe (
    call test_xf_rep_arguments.exe >nul 2>&1
    if errorlevel 1 (
        set /a FAILED_TESTS+=1
        echo [FAIL] XF_REP_ARGUMENTS Tests
    ) else (
        set /a PASSED_TESTS+=1
        echo [PASS] XF_REP_ARGUMENTS Tests
    )
    set /a TOTAL_TESTS+=1
) else (
    echo [SKIP] XF_REP_ARGUMENTS Tests (executable not found)
)
echo.

REM Test 4: Memory Leak Detection
echo [4/4] Running Memory Leak Detection...
if exist test_memory_leak_verification.exe (
    REM Copy required files
    copy lib\epanet2.dll . >nul 2>&1
    copy test_data\simple_test.inp . >nul 2>&1
    copy test_data\valid_config.json EpanetBridge.json >nul 2>&1
    
    test_memory_leak_verification.exe >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        set /a PASSED_TESTS+=1
        echo [PASS] Memory Leak Detection
    ) else (
        set /a FAILED_TESTS+=1
        echo [FAIL] Memory Leak Detection
    )
    set /a TOTAL_TESTS+=1
) else (
    echo [SKIP] Memory Leak Detection (executable not found)
)
echo.

REM Summary
echo ========================================
echo Test Summary
echo ========================================
echo Total Tests:  %TOTAL_TESTS%
echo Passed:       %PASSED_TESTS%
echo Failed:       %FAILED_TESTS%
echo ========================================

if %FAILED_TESTS% EQU 0 (
    echo.
    echo ALL TESTS PASSED!
    echo.
    exit /b 0
) else (
    echo.
    echo SOME TESTS FAILED!
    echo.
    exit /b 1
)
