# Test the updated Pump_to_Tank configuration with POWER and EFFICIENCY

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Pump_to_Tank Configuration Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Copy required files
Write-Host "Copying required files..."
Copy-Item ..\bin\x64\Release\gs_epanet.dll . -Force -ErrorAction SilentlyContinue
Copy-Item ..\bin\x64\Release\gs_epanet.lib . -Force -ErrorAction SilentlyContinue
Copy-Item ..\lib\epanet2.dll . -Force -ErrorAction SilentlyContinue
Copy-Item ..\examples\Pump_to_Tank\Pump_to_Tank.inp . -Force
Copy-Item ..\examples\Pump_to_Tank\EpanetBridge.json . -Force

if (-not (Test-Path "gs_epanet.dll")) {
    Write-Host "ERROR: gs_epanet.dll not found. Build the project first." -ForegroundColor Red
    exit 1
}

# Find VS environment
$vsPath = "C:\Program Files\Microsoft Visual Studio\18\Community"
if (-not (Test-Path $vsPath)) {
    $vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community"
}

$vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"

if (Test-Path $vcvarsPath) {
    Write-Host "Compiling test_pump_to_tank_config.cpp..."
    
    $tempBat = "temp_compile_config_test.bat"
    @"
@echo off
call "$vcvarsPath" >nul 2>&1
cl /EHsc /MD /W3 /O2 /I..\include /Fe:test_pump_to_tank_config.exe test_pump_to_tank_config.cpp gs_epanet.lib /link /SUBSYSTEM:CONSOLE /LIBPATH:.
"@ | Out-File -FilePath $tempBat -Encoding ASCII
    
    & cmd /c $tempBat
    Remove-Item $tempBat -ErrorAction SilentlyContinue
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "ERROR: Compilation failed" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "Compilation successful." -ForegroundColor Green
}

if (-not (Test-Path "test_pump_to_tank_config.exe")) {
    Write-Host "ERROR: test_pump_to_tank_config.exe not found" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Running Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

& .\test_pump_to_tank_config.exe

$testResult = $LASTEXITCODE

Write-Host ""
if ($testResult -eq 0) {
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "TEST PASSED" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
} else {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "TEST FAILED" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
}

exit $testResult
