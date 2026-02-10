# GoldSim Simulator - Quick Reference

## What Is It?

A standalone Windows application that simulates how GoldSim calls the EPANET Bridge DLL. It executes 10 timesteps (or more) of a real EPANET model, demonstrating the complete GoldSim External Function API lifecycle.

## Files Created

| File | Purpose |
|------|---------|
| `goldsim_simulator.cpp` | Main simulator source code (C++) |
| `goldsim_simulator.vcxproj` | Visual Studio project file |
| `compile_simulator_simple.bat` | Simple compilation script |
| `compile_and_run_simulator.bat` | Full compilation and execution script |
| `run_goldsim_simulator.ps1` | PowerShell build and run script |
| `GOLDSIM_SIMULATOR.md` | Complete documentation |
| `SIMULATOR_QUICK_REFERENCE.md` | This file |

## How to Use

### Method 1: Quick Compile and Run (Easiest)

1. Open "x64 Native Tools Command Prompt for VS" from Windows Start Menu
2. Navigate to tests directory:
   ```cmd
   cd C:\path\to\gs_epanet\tests
   ```
3. Run:
   ```cmd
   compile_simulator_simple.bat
   ```

This compiles and immediately runs the simulator with 10 timesteps.

### Method 2: Manual Compilation

```cmd
# From Visual Studio Developer Command Prompt
cd tests
cl /EHsc /MD /W3 /O2 /Fe:goldsim_simulator.exe goldsim_simulator.cpp /link /SUBSYSTEM:CONSOLE
goldsim_simulator.exe gs_epanet.dll 10
```

### Method 3: Using MSBuild

```cmd
cd tests
msbuild goldsim_simulator.vcxproj /p:Configuration=Release /p:Platform=x64
goldsim_simulator.exe gs_epanet.dll 10
```

## Command Line Options

```cmd
goldsim_simulator.exe [dll_path] [num_timesteps]
```

**Parameters**:
- `dll_path` (optional): Path to gs_epanet.dll (default: "gs_epanet.dll")
- `num_timesteps` (optional): Number of timesteps to simulate (default: 10)

**Examples**:

```cmd
# Run with defaults (10 timesteps)
goldsim_simulator.exe

# Run 24 timesteps (24 hours)
goldsim_simulator.exe gs_epanet.dll 24

# Run 168 timesteps (7 days)
goldsim_simulator.exe gs_epanet.dll 168

# Use DLL from specific location
goldsim_simulator.exe ..\bin\x64\Release\gs_epanet.dll 10
```

## What It Simulates

The simulator uses **Example2 (Multi-Junction Network)**:
- 10 junctions with varying elevations
- 12 pipes connecting the network
- 1 reservoir supplying water
- Time-varying demands with realistic patterns

### Demand Patterns

**Residential (J1)**: Morning and evening peaks
- Night (0-6 hrs): 0.5x base demand
- Morning (6-9 hrs): 1.3x base demand
- Evening (17-20 hrs): 1.5x base demand

**Commercial (J5)**: Daytime peak
- Night (0-6 hrs): 0.3x base demand
- Business hours (9-17 hrs): 1.4x base demand

**Industrial (J10)**: Constant with shift changes
- Night shift: 0.8x base demand
- Day shift (7-19 hrs): 1.2x base demand

## Required Files

The simulator needs these files in the tests directory:

1. **gs_epanet.dll** - The EPANET Bridge DLL
2. **epanet2.dll** - The EPANET engine DLL
3. **EpanetBridge.json** - Configuration file (from Example2)
4. **multi_junction.inp** - EPANET model file (from Example2)

The compilation scripts automatically copy these files if they're missing.

## Output

The simulator displays:

1. **DLL Version**: Reports the bridge DLL version (e.g., 1.00)
2. **Input/Output Counts**: Shows how many inputs and outputs are configured
3. **Initialization Status**: Confirms EPANET model loaded successfully
4. **Timestep Results**: For each timestep shows:
   - Elapsed time (seconds and hours)
   - Input demands for J1, J5, J10 (GPM)
   - Output pressures for all 10 junctions (psi)
   - Output flows for pipes P1 and P6 (GPM)
5. **Cleanup Status**: Confirms resources released properly

## Debugging

The simulator generates `epanet_bridge_debug.log` with detailed execution traces.

To enable maximum logging:
1. Edit `EpanetBridge.json`
2. Set `"logging_level": "DEBUG"`
3. Run the simulator
4. Check the log file

## Troubleshooting

### "Failed to load DLL"
- Ensure `gs_epanet.dll` is in the tests directory
- Copy from `bin\x64\Release\gs_epanet.dll` if needed

### "Failed to find EpanetBridge function"
- The DLL must export "EpanetBridge" function
- Verify with: `dumpbin /exports gs_epanet.dll`

### "Initialization failed"
- Check that `EpanetBridge.json` exists
- Verify `multi_junction.inp` exists
- Check `epanet_bridge_debug.log` for details

### "Compilation failed"
- Must run from Visual Studio Developer Command Prompt
- Verify Visual Studio C++ tools are installed

## Performance Testing

Measure execution time:

```powershell
# Time 100 timesteps
Measure-Command { .\goldsim_simulator.exe gs_epanet.dll 100 }

# Time 1000 timesteps
Measure-Command { .\goldsim_simulator.exe gs_epanet.dll 1000 }
```

## Integration with CI/CD

The simulator can be used in automated testing:

```cmd
REM Run simulator and check exit code
goldsim_simulator.exe gs_epanet.dll 10
if %ERRORLEVEL% NEQ 0 (
    echo Test failed!
    exit /b 1
)
echo Test passed!
```

## Comparison with GoldSim

| Aspect | GoldSim | Simulator |
|--------|---------|-----------|
| DLL Loading | LoadLibrary | LoadLibrary ✓ |
| Function Call | EpanetBridge() | EpanetBridge() ✓ |
| XF_REP_VERSION | Yes | Yes ✓ |
| XF_REP_ARGUMENTS | Yes | Yes ✓ |
| XF_INITIALIZE | Yes | Yes ✓ |
| XF_CALCULATE | Yes | Yes ✓ |
| XF_CLEANUP | Yes | Yes ✓ |
| Timestep Control | GoldSim clock | Manual loop |
| Input Array | double[] | std::vector<double> ✓ |
| Output Array | double[] | std::vector<double> ✓ |
| Error Handling | Status codes | Status codes ✓ |

The simulator replicates GoldSim's behavior exactly, except for timestep control which is manual in the simulator.

## Next Steps

After running the simulator successfully:

1. **Verify Results**: Check that pressures and flows are reasonable
2. **Test Different Scenarios**: Run with different timestep counts
3. **Compare with EPANET**: Run the .inp file standalone and compare results
4. **Integrate with GoldSim**: Use the same configuration in your GoldSim model
5. **Customize**: Modify the simulator to test your specific scenarios

## See Also

- **Complete Documentation**: `GOLDSIM_SIMULATOR.md`
- **GoldSim API**: `../goldsim_api.md`
- **Example2 Documentation**: `../examples/Example2_MultiJunction/README.md`
- **Bridge Implementation**: `../src/EpanetBridge.cpp`
- **Main README**: `../README.md`

## Quick Command Reference

```cmd
# Compile
cl /EHsc /MD /W3 /O2 /Fe:goldsim_simulator.exe goldsim_simulator.cpp /link /SUBSYSTEM:CONSOLE

# Run default (10 timesteps)
goldsim_simulator.exe

# Run 24 hours
goldsim_simulator.exe gs_epanet.dll 24

# Run 7 days
goldsim_simulator.exe gs_epanet.dll 168

# Enable debug logging
# Edit EpanetBridge.json: "logging_level": "DEBUG"

# Check log file
type epanet_bridge_debug.log

# Measure performance
powershell -Command "Measure-Command { .\goldsim_simulator.exe gs_epanet.dll 100 }"
```

---

**Created**: February 7, 2026  
**Version**: 1.0  
**Status**: Ready to use
