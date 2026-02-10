@echo off
REM Test script to verify tank levels update correctly over multiple timesteps
REM Uses the Pump_to_Tank example with goldsim_simulator

echo ========================================
echo Tank Level Update Test
echo ========================================
echo.
echo This test verifies that tank levels change correctly over time
echo when there is net inflow or outflow.
echo.

REM Setup test environment
echo Setting up test environment...
copy /Y ..\bin\x64\Release\gs_epanet.dll . >nul 2>&1
copy /Y ..\lib\epanet2.dll . >nul 2>&1
copy /Y ..\examples\Pump_to_Tank\Pump_to_Tank.inp . >nul 2>&1
copy /Y ..\examples\Pump_to_Tank\EpanetBridge.json . >nul 2>&1

if not exist gs_epanet.dll (
    echo ERROR: gs_epanet.dll not found. Build the project first.
    exit /b 1
)

if not exist goldsim_simulator.exe (
    echo Compiling goldsim_simulator...
    cl /EHsc /MD /W3 /O2 /Fe:goldsim_simulator.exe goldsim_simulator.cpp /link /SUBSYSTEM:CONSOLE >nul 2>&1
    if errorlevel 1 (
        echo ERROR: Failed to compile goldsim_simulator
        exit /b 1
    )
)

echo.
echo Running simulation with 3 timesteps (15 minutes)...
echo Expected behavior:
echo   - Pump flow: ~100 GPM into tank
echo   - Outlet flow: ~1 GPM out of tank
echo   - Net inflow: ~99 GPM
echo   - Tank should RISE over time
echo.

goldsim_simulator.exe gs_epanet.dll 3

echo.
echo ========================================
echo Test Complete
echo ========================================
echo.
echo Check the output above:
echo   1. Tank level at timestep 1 should be ~10.0 ft (initial)
echo   2. Tank level at timestep 2 should be HIGHER than timestep 1
echo   3. Tank level at timestep 3 should be HIGHER than timestep 2
echo.
echo If tank level stays constant at 10.0 ft, the test FAILED.
echo.

pause
