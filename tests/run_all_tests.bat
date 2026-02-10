@echo off
REM Run all tests for EPANET-GoldSim Bridge
REM This script runs unit tests, property tests, and memory leak detection

echo ========================================
echo EPANET-GoldSim Bridge Test Suite
echo ========================================
echo.

REM Create test report file
set REPORT_FILE=test_report.txt
echo EPANET-GoldSim Bridge Test Report > %REPORT_FILE%
echo Generated: %DATE% %TIME% >> %REPORT_FILE%
echo ======================================== >> %REPORT_FILE%
echo. >> %REPORT_FILE%

set TOTAL_TESTS=0
set PASSED_TESTS=0
set FAILED_TESTS=0

REM Test 1: Logger Component
echo [1/6] Running Logger Tests...
echo [1/6] Logger Tests >> %REPORT_FILE%
call compile_and_test_logger.bat > logger_test_output.tmp 2>&1
if %ERRORLEVEL% EQU 0 (
    set /a PASSED_TESTS+=1
    echo [PASS] Logger Tests
    echo [PASS] Logger Tests >> %REPORT_FILE%
) else (
    set /a FAILED_TESTS+=1
    echo [FAIL] Logger Tests
    echo [FAIL] Logger Tests >> %REPORT_FILE%
    type logger_test_output.tmp >> %REPORT_FILE%
)
set /a TOTAL_TESTS+=1
echo. >> %REPORT_FILE%
echo.

REM Test 2: MappingLoader Component
echo [2/6] Running MappingLoader Tests...
echo [2/6] MappingLoader Tests >> %REPORT_FILE%
call compile_and_test_mapping_loader.bat > mapping_test_output.tmp 2>&1
if %ERRORLEVEL% EQU 0 (
    set /a PASSED_TESTS+=1
    echo [PASS] MappingLoader Tests
    echo [PASS] MappingLoader Tests >> %REPORT_FILE%
) else (
    set /a FAILED_TESTS+=1
    echo [FAIL] MappingLoader Tests
    echo [FAIL] MappingLoader Tests >> %REPORT_FILE%
    type mapping_test_output.tmp >> %REPORT_FILE%
)
set /a TOTAL_TESTS+=1
echo. >> %REPORT_FILE%
echo.

REM Test 3: EpanetWrapper Component
echo [3/6] Running EpanetWrapper Tests...
echo [3/6] EpanetWrapper Tests >> %REPORT_FILE%
call compile_and_test_epanet_wrapper.bat > wrapper_test_output.tmp 2>&1
if %ERRORLEVEL% EQU 0 (
    set /a PASSED_TESTS+=1
    echo [PASS] EpanetWrapper Tests
    echo [PASS] EpanetWrapper Tests >> %REPORT_FILE%
) else (
    set /a FAILED_TESTS+=1
    echo [FAIL] EpanetWrapper Tests
    echo [FAIL] EpanetWrapper Tests >> %REPORT_FILE%
    type wrapper_test_output.tmp >> %REPORT_FILE%
)
set /a TOTAL_TESTS+=1
echo. >> %REPORT_FILE%
echo.

REM Test 4: XF_REP_ARGUMENTS Handler
echo [4/6] Running XF_REP_ARGUMENTS Tests...
echo [4/6] XF_REP_ARGUMENTS Tests >> %REPORT_FILE%
call compile_and_test_xf_rep_arguments.bat > xf_rep_test_output.tmp 2>&1
if %ERRORLEVEL% EQU 0 (
    set /a PASSED_TESTS+=1
    echo [PASS] XF_REP_ARGUMENTS Tests
    echo [PASS] XF_REP_ARGUMENTS Tests >> %REPORT_FILE%
) else (
    set /a FAILED_TESTS+=1
    echo [FAIL] XF_REP_ARGUMENTS Tests
    echo [FAIL] XF_REP_ARGUMENTS Tests >> %REPORT_FILE%
    type xf_rep_test_output.tmp >> %REPORT_FILE%
)
set /a TOTAL_TESTS+=1
echo. >> %REPORT_FILE%
echo.

REM Test 5: Performance Properties
echo [5/6] Running Performance Tests...
echo [5/6] Performance Tests >> %REPORT_FILE%
call compile_and_test_performance.bat > performance_test_output.tmp 2>&1
if %ERRORLEVEL% EQU 0 (
    set /a PASSED_TESTS+=1
    echo [PASS] Performance Tests
    echo [PASS] Performance Tests >> %REPORT_FILE%
) else (
    set /a FAILED_TESTS+=1
    echo [FAIL] Performance Tests
    echo [FAIL] Performance Tests >> %REPORT_FILE%
    type performance_test_output.tmp >> %REPORT_FILE%
)
set /a TOTAL_TESTS+=1
echo. >> %REPORT_FILE%
echo.

REM Test 6: Memory Leak Detection
echo [6/6] Running Memory Leak Detection...
echo [6/6] Memory Leak Detection >> %REPORT_FILE%
call compile_and_test_memory_leak.bat > memory_test_output.tmp 2>&1
if %ERRORLEVEL% EQU 0 (
    set /a PASSED_TESTS+=1
    echo [PASS] Memory Leak Detection
    echo [PASS] Memory Leak Detection >> %REPORT_FILE%
) else (
    set /a FAILED_TESTS+=1
    echo [FAIL] Memory Leak Detection
    echo [FAIL] Memory Leak Detection >> %REPORT_FILE%
    type memory_test_output.tmp >> %REPORT_FILE%
)
set /a TOTAL_TESTS+=1
echo. >> %REPORT_FILE%
echo.

REM Clean up temporary files
del /q *_test_output.tmp 2>nul

REM Summary
echo ======================================== >> %REPORT_FILE%
echo Test Summary >> %REPORT_FILE%
echo ======================================== >> %REPORT_FILE%
echo Total Tests:  %TOTAL_TESTS% >> %REPORT_FILE%
echo Passed:       %PASSED_TESTS% >> %REPORT_FILE%
echo Failed:       %FAILED_TESTS% >> %REPORT_FILE%
echo ======================================== >> %REPORT_FILE%

echo ========================================
echo Test Summary
echo ========================================
echo Total Tests:  %TOTAL_TESTS%
echo Passed:       %PASSED_TESTS%
echo Failed:       %FAILED_TESTS%
echo ========================================

if %FAILED_TESTS% EQU 0 (
    echo. >> %REPORT_FILE%
    echo ALL TESTS PASSED! >> %REPORT_FILE%
    echo.
    echo ALL TESTS PASSED!
    echo.
    echo Test report saved to: %REPORT_FILE%
    echo.
    exit /b 0
) else (
    echo. >> %REPORT_FILE%
    echo SOME TESTS FAILED! >> %REPORT_FILE%
    echo Please review the output above for details. >> %REPORT_FILE%
    echo.
    echo SOME TESTS FAILED!
    echo Please review the output above for details.
    echo.
    echo Test report saved to: %REPORT_FILE%
    echo.
    exit /b 1
)
