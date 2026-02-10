# EPANET Library Files

This directory should contain the EPANET 2.2 library files for linking.

## Required Files

### For x86 (32-bit) builds:
- `epanet2_x86.lib` - Import library for linking
- `epanet2_x86.dll` - Runtime DLL (copy to bin/ for distribution)

### For x64 (64-bit) builds:
- `epanet2_x64.lib` - Import library for linking
- `epanet2_x64.dll` - Runtime DLL (copy to bin/ for distribution)

## How to Obtain

### Option 1: Download Pre-built Binaries
1. Visit https://github.com/USEPA/EPANET2.2/releases
2. Download the latest Windows release (EPANET_2.2.0_Setup.exe or similar)
3. Install EPANET 2.2
4. Copy the DLL from the installation directory (typically `C:\Program Files (x86)\EPANET 2.2\`)
5. Use `lib.exe` (from Visual Studio) to create import libraries:
   ```
   lib /def:epanet2.def /out:epanet2_x86.lib /machine:x86
   lib /def:epanet2.def /out:epanet2_x64.lib /machine:x64
   ```

### Option 2: Build from Source
1. Clone the EPANET repository: https://github.com/OpenWaterAnalytics/EPANET
2. Checkout the v2.2 tag
3. Build using CMake for both x86 and x64 configurations
4. Copy the resulting .lib and .dll files to this directory

### Option 3: Use OWA-EPANET Pre-built Releases
1. Visit https://github.com/OpenWaterAnalytics/EPANET/releases/tag/v2.2
2. Download the Windows binaries
3. Extract and rename files as needed

## Notes

- The DLL architecture (x86/x64) must match your build target
- For distribution, include the appropriate DLL in the bin/ directory
- Visual C++ Runtime libraries may also be required
