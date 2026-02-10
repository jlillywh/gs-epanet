@echo off
REM Compile and test XF_REP_ARGUMENTS handler
REM This script uses the Visual Studio Developer Command Prompt

echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64

echo.
echo Compiling test_xf_rep_arguments.cpp...

cl.exe /EHsc /std:c++17 /I..\include ^
    test_xf_rep_arguments.cpp ^
    /link ..\bin\x64\Release\gs_epanet.lib ^
    /OUT:test_xf_rep_arguments.exe

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    exit /b 1
)

echo.
echo Running tests...
echo.

REM Copy DLL to test directory
copy ..\bin\x64\Release\gs_epanet.dll . >nul 2>&1
copy ..\bin\x64\Release\epanet2.dll . >nul 2>&1

REM Run the test
test_xf_rep_arguments.exe

set TEST_RESULT=%ERRORLEVEL%

echo.
if %TEST_RESULT% EQU 0 (
    echo All tests passed!
) else (
    echo Some tests failed!
)

exit /b %TEST_RESULT%
