@echo off
REM Simple compilation script for GoldSim simulator
REM Run this from a Visual Studio Developer Command Prompt

echo Compiling GoldSim Simulator...
echo.

cl /EHsc /MD /W3 /O2 /Fe:goldsim_simulator.exe goldsim_simulator.cpp /link /SUBSYSTEM:CONSOLE

if %ERRORLEVEL% EQU 0 (
    echo.
    echo SUCCESS: Compilation complete!
    echo.
    del goldsim_simulator.obj 2>nul
    
    echo Running simulator...
    echo.
    goldsim_simulator.exe gs_epanet.dll 10
) else (
    echo.
    echo ERROR: Compilation failed!
    echo Please run this from a Visual Studio Developer Command Prompt.
)

pause
