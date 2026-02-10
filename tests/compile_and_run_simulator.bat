@echo off
REM Compile and run the GoldSim simulator
REM This simulates a 32-bit GoldSim application calling the EPANET Bridge DLL

echo ========================================
echo GoldSim Simulator - Build and Run
echo ========================================
echo.

REM Check if Visual Studio is available
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Visual Studio compiler not found in PATH.
    echo Please run this from a Visual Studio Developer Command Prompt.
    echo.
    echo To open Developer Command Prompt:
    echo   1. Press Windows key
    echo   2. Type "Developer Command Prompt"
    echo   3. Select "x64 Native Tools Command Prompt for VS 2022" or similar
    pause
    exit /b 1
)

echo Step 1: Compiling goldsim_simulator.cpp...
echo ----------------------------------------

REM Compile as 32-bit executable (to match typical GoldSim deployment)
REM Use /MD for dynamic runtime linking (matches DLL)
cl /EHsc /MD /W3 /O2 ^
   /Fe:goldsim_simulator.exe ^
   goldsim_simulator.cpp ^
   /link /SUBSYSTEM:CONSOLE

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Compilation failed!
    pause
    exit /b 1
)

echo.
echo Compilation successful!
echo.

REM Clean up intermediate files
del goldsim_simulator.obj 2>nul

echo Step 2: Checking for required files...
echo ----------------------------------------

REM Check if DLL exists
if not exist "gs_epanet.dll" (
    echo WARNING: gs_epanet.dll not found in tests directory.
    echo Looking in parent bin directory...
    
    if exist "..\bin\x64\Release\gs_epanet.dll" (
        echo Found DLL in ..\bin\x64\Release\
        copy "..\bin\x64\Release\gs_epanet.dll" . >nul
        echo Copied gs_epanet.dll to tests directory.
    ) else if exist "..\bin\x64\Debug\gs_epanet.dll" (
        echo Found DLL in ..\bin\x64\Debug\
        copy "..\bin\x64\Debug\gs_epanet.dll" . >nul
        echo Copied gs_epanet.dll to tests directory.
    ) else (
        echo ERROR: gs_epanet.dll not found!
        echo Please build the project first using build.ps1
        pause
        exit /b 1
    )
)

REM Check if EPANET DLL exists
if not exist "epanet2.dll" (
    echo WARNING: epanet2.dll not found in tests directory.
    if exist "..\lib\epanet2.dll" (
        echo Found epanet2.dll in ..\lib\
        copy "..\lib\epanet2.dll" . >nul
        echo Copied epanet2.dll to tests directory.
    ) else (
        echo ERROR: epanet2.dll not found!
        pause
        exit /b 1
    )
)

REM Check if Example2 files exist
if not exist "..\examples\Example2_MultiJunction\EpanetBridge.json" (
    echo ERROR: Example2 configuration not found!
    echo Please ensure examples are properly set up.
    pause
    exit /b 1
)

REM Copy Example2 files to tests directory for easy access
echo Copying Example2 files to tests directory...
copy "..\examples\Example2_MultiJunction\EpanetBridge.json" . >nul 2>&1
copy "..\examples\Example2_MultiJunction\multi_junction.inp" . >nul 2>&1

echo.
echo All required files present.
echo.

echo Step 3: Running GoldSim Simulator...
echo ========================================
echo.

REM Run the simulator with 10 timesteps
goldsim_simulator.exe gs_epanet.dll 10

echo.
echo ========================================
echo.

if %ERRORLEVEL% EQU 0 (
    echo SUCCESS: Simulation completed successfully!
) else (
    echo ERROR: Simulation failed with error code %ERRORLEVEL%
)

echo.
echo Check epanet_bridge_debug.log for detailed execution logs.
echo.
pause
