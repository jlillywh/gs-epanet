# GoldSim Simulator

## Overview

The GoldSim Simulator (`goldsim_simulator.cpp`) is a standalone Windows application that mimics how GoldSim calls the EPANET Bridge DLL. It demonstrates the complete lifecycle of the GoldSim External Function API through multiple timesteps.

## Purpose

This simulator serves several purposes:

1. **Testing**: Validate the EPANET Bridge DLL without requiring GoldSim
2. **Debugging**: Easier to debug than running inside GoldSim
3. **Documentation**: Shows exactly how GoldSim interacts with the DLL
4. **Development**: Rapid testing during development cycles

## What It Does

The simulator performs the following sequence, exactly as GoldSim would:

### 1. Load DLL
- Loads `gs_epanet.dll` using Windows LoadLibrary
- Gets the `EpanetBridge` function pointer

### 2. Query Version (XF_REP_VERSION)
- Calls the DLL to get version information
- Displays the DLL version number

### 3. Query Arguments (XF_REP_ARGUMENTS)
- Asks the DLL how many inputs and outputs it expects
- Allocates arrays based on these counts

### 4. Initialize (XF_INITIALIZE)
- Initializes the EPANET model
- Sets initial elapsed time to 0
- Sets initial demand values for junctions

### 5. Run Timesteps (XF_CALCULATE)
- Executes multiple timesteps (default: 10)
- For each timestep:
  - Updates elapsed time (increments by 3600 seconds = 1 hour)
  - Simulates time-varying demands using realistic patterns:
    - **Residential** (J1): Morning/evening peaks
    - **Commercial** (J5): Daytime peak
    - **Industrial** (J10): Constant with shift changes
  - Calls XF_CALCULATE to run EPANET simulation
  - Displays results (pressures and flows)

### 6. Cleanup (XF_CLEANUP)
- Closes EPANET model
- Releases all resources
- Unloads the DLL

## Usage

### Quick Start

**Option 1: Using Visual Studio Developer Command Prompt (Recommended)**

1. Open "x64 Native Tools Command Prompt for VS" from Start Menu
2. Navigate to the tests directory:
   ```cmd
   cd path\to\gs_epanet\tests
   ```
3. Run the compilation script:
   ```cmd
   compile_simulator_simple.bat
   ```

This will compile and immediately run the simulator with 10 timesteps.

**Option 2: Manual Compilation**

From a Visual Studio Developer Command Prompt:

```cmd
cd tests
cl /EHsc /MD /W3 /O2 /Fe:goldsim_simulator.exe goldsim_simulator.cpp /link /SUBSYSTEM:CONSOLE
goldsim_simulator.exe gs_epanet.dll 10
```

**Option 3: Using MSBuild**

```cmd
cd tests
msbuild goldsim_simulator.vcxproj /p:Configuration=Release /p:Platform=x64
goldsim_simulator.exe gs_epanet.dll 10
```

### Running the Simulator

```cmd
goldsim_simulator.exe [dll_path] [num_timesteps]
```

**Parameters:**
- `dll_path` (optional): Path to gs_epanet.dll (default: "gs_epanet.dll")
- `num_timesteps` (optional): Number of timesteps to simulate (default: 10)

**Examples:**

```cmd
# Run with defaults (10 timesteps)
goldsim_simulator.exe

# Run 24 timesteps (24 hours)
goldsim_simulator.exe gs_epanet.dll 24

# Use DLL from specific location
goldsim_simulator.exe ..\bin\x64\Release\gs_epanet.dll 10
```

## Required Files

The simulator needs these files in the same directory:

1. **gs_epanet.dll** - The EPANET Bridge DLL
2. **epanet2.dll** - The EPANET engine DLL
3. **EpanetBridge.json** - Configuration file (from Example2)
4. **multi_junction.inp** - EPANET model file (from Example2)

The PowerShell script automatically copies these files from the appropriate locations.

## Example Output

```
========================================
GoldSim EPANET Bridge Simulator
========================================

DLL Path: gs_epanet.dll
Number of Timesteps: 10

Loading DLL...
DLL loaded successfully.

Function 'EpanetBridge' found.

STEP 1: Querying DLL Version (XF_REP_VERSION)
----------------------------------------
DLL Version: 1.00

STEP 2: Querying Input/Output Counts (XF_REP_ARGUMENTS)
----------------------------------------
Number of Inputs: 4
Number of Outputs: 12

STEP 3: Initializing EPANET Model (XF_INITIALIZE)
----------------------------------------
Initialization successful.

STEP 4: Running 10 Timesteps (XF_CALCULATE)
========================================

Timestep 1 / 10
----------------------------------------
Inputs:
  ElapsedTime: 0 sec (0.00 hrs)
  J1 Demand:   50.00 GPM
  J5 Demand:   13.50 GPM
  J10 Demand:  68.00 GPM

Outputs:
  Junction Pressures (psi):
    J1: 65.23 psi
    J2: 62.45 psi
    J3: 59.87 psi
    J4: 58.12 psi
    J5: 56.34 psi
    J6: 54.67 psi
    J7: 52.89 psi
    J8: 51.23 psi
    J9: 49.56 psi
    J10: 47.89 psi
  Pipe Flows (GPM):
    P1: 245.67 GPM
    P6: 89.34 GPM

[... additional timesteps ...]

STEP 5: Cleaning Up (XF_CLEANUP)
----------------------------------------
Cleanup successful.

Unloading DLL...

========================================
Simulation Complete!
========================================
```

## Demand Patterns

The simulator uses realistic demand patterns to test the bridge:

### Residential Pattern (J1)
- **Night (0-6 hrs)**: 0.5x base demand
- **Morning peak (6-9 hrs)**: 1.3x base demand
- **Day (9-17 hrs)**: 1.0x base demand
- **Evening peak (17-20 hrs)**: 1.5x base demand
- **Night (20-24 hrs)**: 1.0x base demand

### Commercial Pattern (J5)
- **Night (0-6 hrs)**: 0.3x base demand
- **Morning (6-9 hrs)**: 0.8x base demand
- **Business hours (9-17 hrs)**: 1.4x base demand
- **Evening (17-24 hrs)**: 0.3x base demand

### Industrial Pattern (J10)
- **Night shift (0-7 hrs)**: 0.8x base demand
- **Day shift (7-19 hrs)**: 1.2x base demand
- **Night shift (19-24 hrs)**: 0.8x base demand

## Debugging

The simulator generates detailed logs in `epanet_bridge_debug.log`. To enable maximum logging:

1. Edit `EpanetBridge.json`
2. Set `"logging_level": "DEBUG"`
3. Run the simulator
4. Check the log file for detailed execution traces

## Comparison with GoldSim

This simulator replicates GoldSim's behavior:

| Aspect | GoldSim | Simulator |
|--------|---------|-----------|
| DLL Loading | LoadLibrary | LoadLibrary |
| Function Call | EpanetBridge() | EpanetBridge() |
| Input Array | double[] | std::vector<double> |
| Output Array | double[] | std::vector<double> |
| Timestep Control | GoldSim clock | Manual loop |
| Error Handling | Status codes | Status codes |
| Cleanup | XF_CLEANUP | XF_CLEANUP |

The main difference is that GoldSim controls the timestep advancement based on its simulation clock, while the simulator uses a simple loop.

## Troubleshooting

### "Failed to load DLL"
- Ensure `gs_epanet.dll` is in the same directory as the executable
- Check that the DLL architecture matches (x64 vs x86)
- Verify all dependencies are present (epanet2.dll, Visual C++ runtime)

### "Failed to find EpanetBridge function"
- The DLL must export a function named exactly "EpanetBridge" (case-sensitive)
- Use `dumpbin /exports gs_epanet.dll` to verify exports

### "Initialization failed"
- Check that `EpanetBridge.json` exists and is valid
- Verify `multi_junction.inp` exists and is readable
- Check `epanet_bridge_debug.log` for detailed error messages

### "Calculation failed"
- Review input values (demands must be positive)
- Check EPANET model for errors
- Increase logging level to DEBUG for more details

## Integration Testing

This simulator is ideal for integration testing:

```cmd
# Test 24-hour simulation
goldsim_simulator.exe gs_epanet.dll 24

# Test extended simulation (7 days)
goldsim_simulator.exe gs_epanet.dll 168

# Test with different DLL builds
goldsim_simulator.exe ..\bin\x64\Debug\gs_epanet.dll 10
goldsim_simulator.exe ..\bin\x64\Release\gs_epanet.dll 10
```

## Performance Testing

Use the simulator to benchmark performance:

```cmd
# Measure time for 100 timesteps
powershell -Command "Measure-Command { .\goldsim_simulator.exe gs_epanet.dll 100 }"

# Measure time for 1000 timesteps
powershell -Command "Measure-Command { .\goldsim_simulator.exe gs_epanet.dll 1000 }"
```

## Future Enhancements

Potential improvements to the simulator:

- [ ] Command-line option to specify configuration file
- [ ] Support for different EPANET models
- [ ] CSV output for results analysis
- [ ] Performance profiling mode
- [ ] Stress testing with random inputs
- [ ] Comparison with EPANET standalone results
- [ ] Multi-threaded simulation testing

## See Also

- **GoldSim API Documentation**: `goldsim_api.md`
- **EPANET Bridge Implementation**: `src/EpanetBridge.cpp`
- **Example2 Documentation**: `examples/Example2_MultiJunction/README.md`
- **Integration Tests**: `tests/README.md`
