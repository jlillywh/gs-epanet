# EPANET-GoldSim Bridge Build Script
# Builds the project using MSBuild for specified configuration and platform

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    
    [Parameter(Mandatory=$false)]
    [ValidateSet("x86", "x64", "Both")]
    [string]$Platform = "x64",
    
    [Parameter(Mandatory=$false)]
    [switch]$Clean,
    
    [Parameter(Mandatory=$false)]
    [switch]$Rebuild
)

# MSBuild path - try multiple Visual Studio versions
$msbuildPaths = @(
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\18\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
)

$msbuildPath = $null
foreach ($path in $msbuildPaths) {
    if (Test-Path $path) {
        $msbuildPath = $path
        break
    }
}

if (-not $msbuildPath) {
    Write-Error "MSBuild not found. Please install Visual Studio 2022 or later."
    exit 1
}

Write-Host "Using MSBuild: $msbuildPath" -ForegroundColor Green

# Solution file
$solutionFile = "EpanetGoldSimBridge.sln"

if (-not (Test-Path $solutionFile)) {
    Write-Error "Solution file not found: $solutionFile"
    exit 1
}

# Build function
function Build-Project {
    param(
        [string]$Config,
        [string]$Plat
    )
    
    $platformArg = if ($Plat -eq "x86") { "Win32" } else { "x64" }
    
    Write-Host "`nBuilding $Config|$platformArg..." -ForegroundColor Cyan
    
    $target = if ($Rebuild) { "Rebuild" } elseif ($Clean) { "Clean" } else { "Build" }
    
    $arguments = @(
        $solutionFile,
        "/t:$target",
        "/p:Configuration=$Config",
        "/p:Platform=$platformArg",
        "/m",
        "/v:minimal"
    )
    
    & $msbuildPath $arguments
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed for $Config|$platformArg"
        exit $LASTEXITCODE
    }
    
    Write-Host "Build succeeded for $Config|$platformArg" -ForegroundColor Green
}

# Build based on platform selection
if ($Platform -eq "Both") {
    Build-Project -Config $Configuration -Plat "x86"
    Build-Project -Config $Configuration -Plat "x64"
} else {
    Build-Project -Config $Configuration -Plat $Platform
}

# Copy output DLLs to bin/ directory
Write-Host "`nCopying output files to bin/ directory..." -ForegroundColor Cyan

# Create bin directory if it doesn't exist
if (-not (Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" | Out-Null
}

# Function to copy build outputs
function Copy-BuildOutputs {
    param(
        [string]$Config,
        [string]$Plat
    )
    
    $platformArg = if ($Plat -eq "x86") { "Win32" } else { "x64" }
    $sourceDir = "bin\$platformArg\$Config"
    $destDir = "bin\$Plat"
    
    # Create destination directory
    if (-not (Test-Path $destDir)) {
        New-Item -ItemType Directory -Path $destDir | Out-Null
    }
    
    # Copy main DLL
    $dllPath = "$sourceDir\gs_epanet.dll"
    if (Test-Path $dllPath) {
        Copy-Item $dllPath -Destination $destDir -Force
        Write-Host "  Copied gs_epanet.dll to $destDir" -ForegroundColor Gray
    } else {
        Write-Warning "  gs_epanet.dll not found at $dllPath"
    }
    
    # Copy PDB file for debugging
    $pdbPath = "$sourceDir\gs_epanet.pdb"
    if (Test-Path $pdbPath) {
        Copy-Item $pdbPath -Destination $destDir -Force
        Write-Host "  Copied gs_epanet.pdb to $destDir" -ForegroundColor Gray
    }
    
    # Copy EPANET DLL
    $epanetDll = "lib\epanet2.dll"
    if (Test-Path $epanetDll) {
        Copy-Item $epanetDll -Destination $destDir -Force
        Write-Host "  Copied epanet2.dll to $destDir" -ForegroundColor Gray
    } else {
        Write-Warning "  epanet2.dll not found at $epanetDll"
    }
}

# Copy outputs based on platform selection
if ($Platform -eq "Both") {
    Copy-BuildOutputs -Config $Configuration -Plat "x86"
    Copy-BuildOutputs -Config $Configuration -Plat "x64"
} else {
    Copy-BuildOutputs -Config $Configuration -Plat $Platform
}

Write-Host "`nBuild completed successfully!" -ForegroundColor Green
Write-Host "Output directory: bin\$Platform\" -ForegroundColor Yellow
Write-Host "`nNote: Visual C++ runtime (vcruntime140.dll) may be required for distribution." -ForegroundColor Yellow
Write-Host "      It is typically installed with Visual Studio or can be distributed separately." -ForegroundColor Yellow
