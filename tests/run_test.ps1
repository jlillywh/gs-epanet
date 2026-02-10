# Quick test script for GoldSim simulator
$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "GoldSim Simulator Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Setup Visual Studio environment and compile
$vcvarsall = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat"

Write-Host "Compiling goldsim_simulator.cpp..." -ForegroundColor Yellow

# Create temp batch file to compile
$tempBat = "temp_compile_test.bat"
@"
@echo off
call "$vcvarsall" x64 >nul 2>&1
cl /EHsc /MD /W3 /O2 /Fe:goldsim_simulator.exe goldsim_simulator.cpp /link /SUBSYSTEM:CONSOLE
"@ | Out-File -FilePath $tempBat -Encoding ASCII

# Run compilation
$compileOutput = & cmd /c $tempBat 2>&1
Remove-Item $tempBat -ErrorAction SilentlyContinue

if ($LASTEXITCODE -ne 0) {
    Write-Host "Compilation failed!" -ForegroundColor Red
    Write-Host $compileOutput
    exit 1
}

Write-Host "Compilation successful!" -ForegroundColor Green
Write-Host ""

# Clean up
Remove-Item "goldsim_simulator.obj" -ErrorAction SilentlyContinue

# Run the simulator
Write-Host "Running simulator with 10 timesteps..." -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

& ".\goldsim_simulator.exe" "gs_epanet.dll" 10

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan

if ($LASTEXITCODE -eq 0) {
    Write-Host "TEST PASSED!" -ForegroundColor Green
} else {
    Write-Host "TEST FAILED with exit code $LASTEXITCODE" -ForegroundColor Red
}

Write-Host ""
Write-Host "Log file: epanet_bridge_debug.log" -ForegroundColor Cyan
