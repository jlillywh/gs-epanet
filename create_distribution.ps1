# EPANET-GoldSim Bridge Distribution Package Creator
# Creates a distribution package with all necessary files

param(
    [Parameter(Mandatory=$false)]
    [string]$Version = "1.0",
    
    [Parameter(Mandatory=$false)]
    [switch]$IncludeSource
)

$ErrorActionPreference = "Stop"

# Distribution directory name
$distName = "EPANET_Bridge_v$Version"
$distPath = $distName

Write-Host "Creating EPANET-GoldSim Bridge Distribution Package v$Version" -ForegroundColor Cyan
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""

# Clean up existing distribution directory
if (Test-Path $distPath) {
    Write-Host "Removing existing distribution directory..." -ForegroundColor Yellow
    Remove-Item -Path $distPath -Recurse -Force
}

# Create distribution directory structure
Write-Host "Creating directory structure..." -ForegroundColor Green
New-Item -ItemType Directory -Path $distPath | Out-Null
New-Item -ItemType Directory -Path "$distPath\bin" | Out-Null
New-Item -ItemType Directory -Path "$distPath\bin\x86" | Out-Null
New-Item -ItemType Directory -Path "$distPath\bin\x64" | Out-Null
New-Item -ItemType Directory -Path "$distPath\examples" | Out-Null
New-Item -ItemType Directory -Path "$distPath\scripts" | Out-Null
New-Item -ItemType Directory -Path "$distPath\docs" | Out-Null

# Copy bin/ directory (DLLs)
Write-Host "Copying binaries..." -ForegroundColor Green
if (Test-Path "bin\x86\gs_epanet.dll") {
    Copy-Item "bin\x86\gs_epanet.dll" -Destination "$distPath\bin\x86\" -Force
    Write-Host "  - Copied x86 DLL" -ForegroundColor Gray
}
if (Test-Path "bin\x86\epanet2.dll") {
    Copy-Item "bin\x86\epanet2.dll" -Destination "$distPath\bin\x86\" -Force
    Write-Host "  - Copied x86 EPANET DLL" -ForegroundColor Gray
}
if (Test-Path "bin\x64\gs_epanet.dll") {
    Copy-Item "bin\x64\gs_epanet.dll" -Destination "$distPath\bin\x64\" -Force
    Write-Host "  - Copied x64 DLL" -ForegroundColor Gray
}
if (Test-Path "bin\x64\epanet2.dll") {
    Copy-Item "bin\x64\epanet2.dll" -Destination "$distPath\bin\x64\" -Force
    Write-Host "  - Copied x64 EPANET DLL" -ForegroundColor Gray
}

# Copy examples/ directory
Write-Host "Copying examples..." -ForegroundColor Green
if (Test-Path "examples") {
    $exampleDirs = Get-ChildItem -Path "examples" -Directory
    foreach ($dir in $exampleDirs) {
        $destDir = "$distPath\examples\$($dir.Name)"
        New-Item -ItemType Directory -Path $destDir -Force | Out-Null
        
        # Copy all files except .gsm files (GoldSim models are large)
        Get-ChildItem -Path $dir.FullName -File | Where-Object { $_.Extension -ne ".gsm" } | ForEach-Object {
            Copy-Item $_.FullName -Destination $destDir -Force
        }
        
        Write-Host "  - Copied $($dir.Name)" -ForegroundColor Gray
    }
}

# Copy scripts/ directory
Write-Host "Copying scripts..." -ForegroundColor Green
if (Test-Path "scripts\generate_mapping.py") {
    Copy-Item "scripts\generate_mapping.py" -Destination "$distPath\scripts\" -Force
    Write-Host "  - Copied generate_mapping.py" -ForegroundColor Gray
}
if (Test-Path "scripts\README.md") {
    Copy-Item "scripts\README.md" -Destination "$distPath\scripts\" -Force
    Write-Host "  - Copied scripts README" -ForegroundColor Gray
}

# Copy documentation
Write-Host "Copying documentation..." -ForegroundColor Green
if (Test-Path "README.md") {
    Copy-Item "README.md" -Destination "$distPath\" -Force
    Write-Host "  - Copied README.md" -ForegroundColor Gray
}
if (Test-Path "LICENSE") {
    Copy-Item "LICENSE" -Destination "$distPath\LICENSE.txt" -Force
    Write-Host "  - Copied LICENSE" -ForegroundColor Gray
}

# Copy additional documentation files if they exist
$docFiles = @("goldsim_api.md", "gs_swmm_bridge.md")
foreach ($docFile in $docFiles) {
    if (Test-Path $docFile) {
        Copy-Item $docFile -Destination "$distPath\docs\" -Force
        Write-Host "  - Copied $docFile" -ForegroundColor Gray
    }
}

# Copy images if they exist
if (Test-Path "docs\images") {
    New-Item -ItemType Directory -Path "$distPath\docs\images" -Force | Out-Null
    Copy-Item "docs\images\*" -Destination "$distPath\docs\images\" -Recurse -Force
    Write-Host "  - Copied documentation images" -ForegroundColor Gray
}

# Optionally include source code
if ($IncludeSource) {
    Write-Host "Including source code..." -ForegroundColor Green
    New-Item -ItemType Directory -Path "$distPath\src" | Out-Null
    New-Item -ItemType Directory -Path "$distPath\include" | Out-Null
    New-Item -ItemType Directory -Path "$distPath\lib" | Out-Null
    
    Copy-Item "src\*.cpp" -Destination "$distPath\src\" -Force
    Copy-Item "src\*.h" -Destination "$distPath\src\" -Force
    Copy-Item "src\*.vcxproj" -Destination "$distPath\src\" -Force
    Copy-Item "include\*.h" -Destination "$distPath\include\" -Force
    Copy-Item "lib\*.lib" -Destination "$distPath\lib\" -Force
    Copy-Item "lib\*.h" -Destination "$distPath\lib\" -Force
    Copy-Item "*.sln" -Destination "$distPath\" -Force
    
    Write-Host "  - Copied source files" -ForegroundColor Gray
}

# Create a version info file
$versionInfo = @"
EPANET-GoldSim Bridge
Version: $Version
Build Date: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Platform: Windows x86/x64

Contents:
- bin/x86/       : 32-bit DLL and dependencies
- bin/x64/       : 64-bit DLL and dependencies
- examples/      : Example EPANET models and configurations
- scripts/       : Python mapping generator script
- docs/          : Additional documentation
- README.md      : Main documentation
- LICENSE.txt    : MIT License

For installation and usage instructions, see README.md
"@

$versionInfo | Out-File -FilePath "$distPath\VERSION.txt" -Encoding UTF8
Write-Host "  - Created VERSION.txt" -ForegroundColor Gray

# Create ZIP archive
Write-Host ""
Write-Host "Creating ZIP archive..." -ForegroundColor Green
$zipPath = "$distName.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}

Compress-Archive -Path $distPath -DestinationPath $zipPath -CompressionLevel Optimal
Write-Host "  - Created $zipPath" -ForegroundColor Gray

# Summary
Write-Host ""
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "Distribution package created successfully!" -ForegroundColor Green
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Package location: $zipPath" -ForegroundColor Yellow
Write-Host "Package size: $([math]::Round((Get-Item $zipPath).Length / 1MB, 2)) MB" -ForegroundColor Yellow
Write-Host ""
Write-Host "Distribution directory: $distPath" -ForegroundColor Yellow
Write-Host ""

# List contents
Write-Host "Package contents:" -ForegroundColor Cyan
Get-ChildItem -Path $distPath -Recurse -File | ForEach-Object {
    $relativePath = $_.FullName.Substring($distPath.Length + 1)
    Write-Host "  $relativePath" -ForegroundColor Gray
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
