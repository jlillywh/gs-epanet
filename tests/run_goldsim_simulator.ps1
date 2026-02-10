# PowerShell script to compile and run the GoldSim simulator
# This simulates a 32-bit GoldSim application calling the EPANET Bridge DLL

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "GoldSim Simulator - Build and Run" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Set error action preference
$ErrorActionPreference = "Stop"

# Function to find Visual Studio installation
function Find-VSInstallation {
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (Test-Path $vsWhere) {
        $vsPath = & $vsWhere -latest -property installationPath
        if ($vsPath) {
            return $vsPath
        }
    }
    
    # Fallback: Check common locations
    $commonPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}

# Function to setup Visual Studio environment
function Setup-VSEnvironment {
    $vsPath = Find-VSInstallation
    
    if (-not $vsPath) {
        Write-Host "ERROR: Visual Studio installation not found!" -ForegroundColor Red
        Write-Host "Please install Visual Studio 2019 or later with C++ tools." -ForegroundColor Yellow
        exit 1
    }
    
    Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Green
    
    # Find vcvarsall.bat
    $vcvarsall = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"
    
    if (-not (Test-Path $vcvarsall)) {
        Write-Host "ERROR: vcvarsall.bat not found!" -ForegroundColor Red
        exit 1
    }
    
    return $vcvarsall
}

# Step 1: Setup environment
Write-Host "Step 1: Setting up build environment..." -ForegroundColor Yellow
Write-Host "----------------------------------------"

$vcvarsall = Setup-VSEnvironment

# Step 2: Compile
Write-Host ""
Write-Host "Step 2: Compiling goldsim_simulator.cpp..." -ForegroundColor Yellow
Write-Host "----------------------------------------"

# Create a temporary batch file to run compilation
$tempBat = "temp_compile.bat"
@"
@echo off
call "$vcvarsall" x64
cl /EHsc /MD /W3 /O2 /Fe:goldsim_simulator.exe goldsim_simulator.cpp /link /SUBSYSTEM:CONSOLE
"@ | Out-File -FilePath $tempBat -Encoding ASCII

# Run compilation
$result = & cmd /c $tempBat 2>&1
Remove-Item $tempBat -ErrorAction SilentlyContinue

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: Compilation failed!" -ForegroundColor Red
    Write-Host $result
    exit 1
}

Write-Host ""
Write-Host "Compilation successful!" -ForegroundColor Green
Write-Host ""

# Clean up intermediate files
Remove-Item "goldsim_simulator.obj" -ErrorAction SilentlyContinue

# Step 3: Check for required files
Write-Host "Step 3: Checking for required files..." -ForegroundColor Yellow
Write-Host "----------------------------------------"

# Check if DLL exists
if (-not (Test-Path "gs_epanet.dll")) {
    Write-Host "WARNING: gs_epanet.dll not found in tests directory." -ForegroundColor Yellow
    Write-Host "Looking in parent bin directory..."
    
    $dllPaths = @(
        "..\bin\x64\Release\gs_epanet.dll",
        "..\bin\x64\Debug\gs_epanet.dll"
    )
    
    $found = $false
    foreach ($path in $dllPaths) {
        if (Test-Path $path) {
            Write-Host "Found DLL at: $path" -ForegroundColor Green
            Copy-Item $path "gs_epanet.dll"
            Write-Host "Copied gs_epanet.dll to tests directory."
            $found = $true
            break
        }
    }
    
    if (-not $found) {
        Write-Host "ERROR: gs_epanet.dll not found!" -ForegroundColor Red
        Write-Host "Please build the project first using build.ps1" -ForegroundColor Yellow
        exit 1
    }
}

# Check if EPANET DLL exists
if (-not (Test-Path "epanet2.dll")) {
    Write-Host "WARNING: epanet2.dll not found in tests directory." -ForegroundColor Yellow
    if (Test-Path "..\lib\epanet2.dll") {
        Write-Host "Found epanet2.dll in ..\lib\" -ForegroundColor Green
        Copy-Item "..\lib\epanet2.dll" "epanet2.dll"
        Write-Host "Copied epanet2.dll to tests directory."
    } else {
        Write-Host "ERROR: epanet2.dll not found!" -ForegroundColor Red
        exit 1
    }
}

# Check if Example2 files exist
if (-not (Test-Path "..\examples\Example2_MultiJunction\EpanetBridge.json")) {
    Write-Host "ERROR: Example2 configuration not found!" -ForegroundColor Red
    Write-Host "Please ensure examples are properly set up." -ForegroundColor Yellow
    exit 1
}

# Copy Example2 files to tests directory
Write-Host "Copying Example2 files to tests directory..."
Copy-Item "..\examples\Example2_MultiJunction\EpanetBridge.json" "." -ErrorAction SilentlyContinue
Copy-Item "..\examples\Example2_MultiJunction\multi_junction.inp" "." -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "All required files present." -ForegroundColor Green
Write-Host ""

# Step 4: Run simulator
Write-Host "Step 4: Running GoldSim Simulator..." -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Run the simulator with 10 timesteps
& ".\goldsim_simulator.exe" "gs_epanet.dll" 10

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($LASTEXITCODE -eq 0) {
    Write-Host "SUCCESS: Simulation completed successfully!" -ForegroundColor Green
} else {
    Write-Host "ERROR: Simulation failed with error code $LASTEXITCODE" -ForegroundColor Red
}

Write-Host ""
Write-Host "Check epanet_bridge_debug.log for detailed execution logs." -ForegroundColor Cyan
Write-Host ""
