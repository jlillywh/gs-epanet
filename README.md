# EPANET-GoldSim Bridge

A Windows DLL that connects GoldSim simulation software with EPA EPANET hydraulic modeling engine for coupled water distribution system analysis.

## What It Does

The EPANET-GoldSim Bridge allows you to:
- Control EPANET models from within GoldSim simulations
- Adjust pump speeds, valve settings, and demands at each timestep
- Monitor pressures, flows, and tank levels at each timestep
- Implement advanced control strategies using GoldSim's capabilities

## System Requirements

- Windows 10/11 (64-bit)
- GoldSim software
- Visual C++ Redistributable 2022 (included with GoldSim)

## Quick Start

### 1. Get the Files

Download or build the bridge DLL. You'll need:
- `gs_epanet.dll` - The bridge DLL
- `epanet2.dll` - EPANET engine (included)
- Your EPANET network file (`.inp` file)
- Configuration file (`EpanetBridge.json`)

### 2. Prepare Your EPANET Network

You'll need an EPANET network file (`.inp` format). You can:
- Create one using the EPANET GUI application
- Use an existing network file from a previous project
- Start with the included example: `examples\Pump_to_Tank\Pump_to_Tank.inp`

**Optional - Test your network file**:
If you have EPANET installed or the command-line tool, verify it runs:
```powershell
# If using the included command-line tool
lib\runepanet.exe your_network.inp

# Or open in EPANET GUI and run the analysis
```

This step is optional but recommended to catch any network errors before using with GoldSim.

### 3. Create Configuration File

The JSON file maps GoldSim inputs/outputs to EPANET elements. See the example below or use the Pump_to_Tank example as a template.

**Basic structure**:
```json
{
  "version": "1.0",
  "logging_level": "ERROR",
  "inp_file": "your_network.inp",
  "hydraulic_timestep": 300,
  "input_count": 2,
  "output_count": 3,
  "inputs": [
    {"index": 0, "name": "ElapsedTime", "object_type": "SYSTEM", "property": "ELAPSEDTIME"},
    {"index": 1, "name": "Pump1", "object_type": "LINK", "property": "SETTING"}
  ],
  "outputs": [
    {"index": 0, "name": "Tank1", "object_type": "NODE", "property": "HEAD"},
    {"index": 1, "name": "Junction1", "object_type": "NODE", "property": "PRESSURE"},
    {"index": 2, "name": "Pump1", "object_type": "LINK", "property": "FLOW"}
  ]
}
```

### 4. Set Up GoldSim

1. Add an **External** element to your GoldSim model
2. Configure the DLL settings:
   - **DLL File**: Browse to `gs_epanet.dll`
   - **Function Name**: `EpanetBridge` (case-sensitive!)
   - **Unload DLL after each use**: ☐ Unchecked
   - **Run Cleanup after each realization**: ☑ Checked
   - **Run in separate process**: ☑ Checked
3. Connect inputs:
   - Input 1: Link to `ETime` (GoldSim's elapsed time - Required)
   - Input 2+: Your control variables (pump speeds, valve settings, etc.)
4. Connect outputs to result elements
5. **Set Basic Time Step** to match EPANET's hydraulic timestep (e.g., 300 seconds)

### 5. Copy Files

Place these files in the same folder as your `.gsm` file:
- `gs_epanet.dll`
- `epanet2.dll`
- `EpanetBridge.json`
- `your_network.inp`

### 6. Run

Press F5 in GoldSim. Check `epanet_bridge_debug.log` if you encounter errors.

**Tip**: For troubleshooting, set `"logging_level": "DEBUG"` in your `EpanetBridge.json` file to see detailed information about each calculation step. Change back to `"INFO"` for normal operation.

## Example: Pump to Tank System

A complete working example is included in `examples/Pump_to_Tank/`:

<img src="examples/Pump_to_Tank/GoldSim%20Model%20with%20EPANET%20Screen%20Capture.png" alt="GoldSim Model with EPANET" width="75%">

**System**:
- Reservoir → Pump → Tank → Outlet
- Pump on/off controlled by GoldSim deadband controller (0 = off, 1 = on)
- EPANET calculates pump flow based on pump curve
- Outlet flow controlled by emitter coefficient
- Tank level monitored at each timestep

<img src="examples/Pump_to_Tank/EPANET%20Pump%20Curve.png" alt="EPANET Pump Curve" width="50%">

**Files**:
- `Pump_to_Tank.inp` - EPANET model
- `EpanetBridge.json` - Configuration
- `GoldSim Model with EPANET Screen Capture.png` - Screenshot

**To run**:
```powershell
# Test the EPANET network file
lib\runepanet.exe examples\Pump_to_Tank\Pump_to_Tank.inp

# Validate tank level updates
cd tests
.\run_tank_level_test.ps1
```

**Expected behavior**:
- Pump controlled by deadband: on (1) or off (0)
- When pump is on: ~100 GPM flow (based on pump curve)
- Tank fills at ~0.8 ft per 5 minutes when pump is on
- Outlet flow controlled by emitter: ~1 GPM
- Deadband controller maintains tank level within setpoint range

## Supported Properties

### Inputs (Control from GoldSim)

| Element Type | Property | Description | Units | Range |
|--------------|----------|-------------|-------|-------|
| SYSTEM | ELAPSEDTIME | Simulation time (always input 0) | seconds | ≥ 0 |
| NODE | DEMAND | Junction demand | GPM/LPS | any |
| NODE | EMITTER | Emitter coefficient | GPM/psi^0.5 | ≥ 0 |
| LINK | STATUS | Pipe/valve open/closed | - | 0 or 1 |
| LINK | SETTING | Pump speed or valve position | - | 0.0-1.0 |

### Outputs (Monitor in GoldSim)

| Element Type | Property | Description | Units |
|--------------|----------|-------------|-------|
| NODE | PRESSURE | Pressure at junction | psi |
| NODE | HEAD | Hydraulic head (elevation + pressure) | ft |
| NODE | DEMAND | Actual demand (computed) | GPM/LPS |
| LINK | FLOW | Flow rate | GPM/LPS |
| LINK | VELOCITY | Flow velocity | ft/s |
| LINK | POWER | Pump power consumption (computed) | HP |
| LINK | EFFICIENCY | Pump efficiency (computed) | % |

## Important Notes

### Tank Levels
- Use **HEAD** property (not TANKLEVEL) to monitor tanks
- HEAD = Tank Elevation + Water Level
- To get water level: HEAD - Tank Elevation
- Example: If HEAD = 215 ft and tank elevation = 200 ft, water level = 15 ft

### Timesteps
- GoldSim's Basic Time Step **must match** EPANET's hydraulic timestep
- Check `hydraulic_timestep` in your JSON file
- Mismatch causes incorrect results

### DLL Settings
- **Never** check "Unload DLL after each use" - this resets tank levels
- **Always** check "Run Cleanup after each realization"

## Troubleshooting

**DLL won't load**:
- Ensure all files are in the same folder as your `.gsm` file
- Check that you're using 64-bit DLLs with 64-bit GoldSim

**Tank level doesn't change**:
- Use HEAD property instead of TANKLEVEL
- Verify "Unload DLL after each use" is unchecked
- Check `epanet_bridge_debug.log` for errors

**Results don't match EPANET**:
- Verify GoldSim timestep matches EPANET hydraulic timestep
- Check that element names in JSON exactly match your `.inp` file
- Ensure pump/valve settings are in valid range (0.0-1.0)

**Error messages**:
- Check `epanet_bridge_debug.log` in your model folder
- Common issues: missing files, name mismatches, timestep problems

## Building from Source

If you need to build the DLL yourself:

```powershell
# Requires Visual Studio 2022 or later
.\build.ps1 -Configuration Release -Platform x64
```

Output: `bin\x64\Release\gs_epanet.dll`

See `docs/DEVELOPER.md` for detailed build instructions and API documentation.

## Support

- Check `epanet_bridge_debug.log` for detailed error messages
- Review the Pump_to_Tank example for a working configuration
- See `docs/` folder for additional documentation

## License

MIT License - see [LICENSE](LICENSE) file

## Version

**Current Version: 1.1.0** (February 2026)

Version 1.1.0 adds support for pump power and efficiency monitoring:
- POWER property: Monitor pump power consumption in horsepower
- EFFICIENCY property: Monitor pump efficiency percentage
- Enables energy cost analysis and pump performance tracking

First stable release (1.0.0) features:
- GoldSim External Function API implementation
- JSON-based configuration system
- Support for pump control, valve control, and emitter-based flow control
- Monitoring of pressures, flows, and tank levels at each timestep
- Validated tank level dynamics
- Pump_to_Tank working example
- Comprehensive error handling and logging

## Acknowledgments

- **EPANET**: U.S. Environmental Protection Agency
- **OWA-EPANET**: Open Water Analytics (v2.3.3)
- **GoldSim**: GoldSim Technology Group
