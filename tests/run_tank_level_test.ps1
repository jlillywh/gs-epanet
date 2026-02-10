# PowerShell script to compile and run tank level validation test

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Tank Level Validation Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Copy required files
Write-Host "Copying required files..."
Copy-Item ..\bin\x64\Release\gs_epanet.dll . -Force -ErrorAction SilentlyContinue
Copy-Item ..\lib\epanet2.dll . -Force -ErrorAction SilentlyContinue
Copy-Item ..\examples\Pump_to_Tank\Pump_to_Tank.inp . -Force -ErrorAction SilentlyContinue
Copy-Item ..\examples\Pump_to_Tank\EpanetBridge.json . -Force -ErrorAction SilentlyContinue

if (-not (Test-Path "gs_epanet.dll")) {
    Write-Host "ERROR: gs_epanet.dll not found. Build the project first." -ForegroundColor Red
    exit 1
}

# Find MSBuild and setup VS environment
$vsPath = "C:\Program Files\Microsoft Visual Studio\18\Community"
if (-not (Test-Path $vsPath)) {
    $vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community"
}

$vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"

if (Test-Path $vcvarsPath) {
    Write-Host "Compiling test_tank_level_validation.cpp..."
    
    # Create a temporary batch file to setup environment and compile
    $tempBat = "temp_compile_tank_test.bat"
    @"
@echo off
call "$vcvarsPath" >nul 2>&1
cl /EHsc /MD /W3 /O2 /Fe:test_tank_level_validation.exe test_tank_level_validation.cpp /link /SUBSYSTEM:CONSOLE
"@ | Out-File -FilePath $tempBat -Encoding ASCII
    
    & cmd /c $tempBat
    Remove-Item $tempBat -ErrorAction SilentlyContinue
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "ERROR: Compilation failed" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "Compilation successful." -ForegroundColor Green
} else {
    Write-Host "WARNING: Visual Studio not found, checking for pre-compiled executable..." -ForegroundColor Yellow
}

if (-not (Test-Path "test_tank_level_validation.exe")) {
    Write-Host "ERROR: test_tank_level_validation.exe not found and could not be compiled" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Running Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

& .\test_tank_level_validation.exe

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
