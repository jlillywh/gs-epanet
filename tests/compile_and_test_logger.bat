@echo off
REM Compile and test Logger standalone

echo Compiling Logger standalone test...

cl.exe /EHsc /std:c++17 /I..\src test_logger_standalone.cpp ..\src\Logger.cpp /Fe:test_logger_standalone.exe

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    exit /b 1
)

echo.
echo Running Logger test...
echo.

test_logger_standalone.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Logger test FAILED!
    exit /b 1
)

echo.
echo Cleaning up...
del test_logger_standalone.exe
del test_logger_standalone.obj
del Logger.obj

echo.
echo Logger test completed successfully!
exit /b 0
